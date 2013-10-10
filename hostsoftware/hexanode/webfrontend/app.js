var application_root = __dirname
  , path=require('path')
  , express=require('express')
  , connect = require('connect')
  , app = module.exports = express()
  , server=require("http").createServer(app)
  , io = require('socket.io').listen(server)
	, Cache = require("./lib/sensorcache")
	, SensorRegistry = require("./lib/sensorregistry")
	, Hexabus = require("./lib/hexabus")
	, hexabus = new Hexabus()
	, Wizard = require("./lib/wizard")
	, fs = require("fs")
  , nconf=require('nconf');

nconf.env().argv();
// Setting default values
nconf.defaults({
  'port': '3000',
	'config': '.'
});

server.listen(nconf.get('port'));

// drop root if we ever had it, our arguments say so.
// this includes all supplemental groups, of course
if (nconf.get('uid')) {
	var uid = nconf.get('uid');
	uid = parseInt(uid) || uid;
	var gid = nconf.get('gid') || uid;
	gid = parseInt(gid) || gid;

	process.setgid(gid);
	process.setgroups([gid]);
	process.setuid(uid);
}

var configDir = nconf.get('config');
var sensors_file = configDir + '/sensors.json';

var sensor_registry;
var sensor_cache;

function open_config() {
	try {
		sensor_registry = new SensorRegistry(fs.existsSync(sensors_file) ? sensors_file : undefined);
	} catch (e) {
		sensor_registry = new SensorRegistry();
	}
	sensor_cache = new Cache();
}
open_config();

var save_sensor_registry = function() {
	sensor_registry.save(sensors_file);
};

setInterval(save_sensor_registry, 10 * 60 * 1000);
process.on('SIGINT', function() {
	save_sensor_registry();
	process.exit();
});
process.on('SIGTERM', function() {
	save_sensor_registry();
	process.exit();
});

console.log("Using configuration: ");
console.log(" - port: " + nconf.get('port'));
console.log(" - config dir: " + nconf.get('config'));

if (nconf.get('socket-debug') !== undefined) {
	io.set('log level', nconf.get('socket-debug'));
} else {
	io.set('log level', 1);
}


// see http://stackoverflow.com/questions/4600952/node-js-ejs-example
// for EJS
app.configure(function () {
  app.set('views', __dirname + '/views');
  app.set('view engine', 'ejs');
//  app.use(connect.logger('dev'));
  app.use(express.bodyParser());
  app.use(express.methodOverride());
  app.use(app.router);
  app.use(express.static(path.join(application_root, "public")));
  app.use(express.errorHandler({ dumpExceptions: true, showStack: true }));
});


app.get('/api', function(req, res) {
  res.send("API is running.");
});

app.get('/api/sensor', function(req, res) {
	res.send(JSON.stringify(sensor_registry.get_sensors()));
});

app.get('/api/sensor/:ip/:eid/latest', function(req, res) {
	var sensor = sensor_registry.get_sensor(req.params.ip, req.params.eid);
	if (!sensor) {
		res.send("Sensor not found", 404);
	} else if (sensor_cache.get_current_value(sensor) == undefined) {
		res.send("No value yet", 404);
	} else {
		res.send(JSON.stringify(sensor_cache.get_current_value(sensor.id)));
	}
});

app.get('/api/sensor/:ip/:eid/info', function(req, res) {
	var sensor = sensor_registry.get_sensor(req.params.ip, req.params.eid);
	if (sensor) {
		res.send(JSON.stringify(sensor));
	} else {
		res.send("Sensor not found", 404);
	}
});

app.put('/api/sensor/:ip/:eid', function(req, res) {
	if (sensor_registry.get_sensor(req.params.ip, req.params.eid)) {
		res.send("Sensor exists", 200);
	} else {
		try {
			var sensor = sensor_registry.add_sensor(req.params.ip, req.params.eid, req.body);
			sensor_cache.add_value(sensor, parseFloat(req.body.value));
			save_sensor_registry();
			res.send("Sensor added", 200);
		} catch(err) {
			res.send(JSON.stringify(err), 400);
		}
	}
});

app.post('/api/sensor/:ip/:eid', function(req, res) {
	var sensor = sensor_registry.get_sensor(req.params.ip, req.params.eid);
	var value = parseFloat(req.body.value);
	if (!sensor) {
		res.send("Not found", 404);
	} else if (isNaN(value)) {
		res.send("Bad value", 400);
	} else {
		sensor_cache.add_value(sensor, value);
		sensor_registry.value_received(req.params.ip, req.params.eid);
		res.send("Value added", 200);
	}
});

app.post('/api/device/rename/:ip', function(req, res) {
	try {
		if (!req.body.name) {
			res.send("No name given", 400);
		}
		hexabus.rename_device(req.params.ip, req.body.name, function(err) {
			if (err) {
				res.send(JSON.stringify(err), 500);
			} else {
				res.send("Success");
			}
		});
	} catch (err) {
		console.log(err);
		res.send(JSON.stringify(err), 500);
	}
});

app.get('/', function(req, res) {
	fs.exists('/etc/radvd.conf', function(exists) {
		if (exists) {
			res.render('index.ejs', { active_tabpage: 'dashboard' });
		} else {
			res.redirect('/wizard/new');
		}
	});
});
app.get('/wizard', function(req, res) {
	fs.exists('/etc/radvd.conf', function(exists) {
		if (exists) {
			res.redirect('/wizard/current');
		} else {
			res.redirect('/wizard/new');
		}
	});
});
app.get('/wizard/current', function(req, res) {
	var config = hexabus.read_current_config();
	hexabus.get_heartbeat_state(function(err, state) {
		config.active_tabpage = 'configuration';

		if (err) {
			config.heartbeat_ok = false;
			config.heartbeat_messages = [err];
		} else {
			config.heartbeat_ok = state.code == 0;
			config.heartbeat_code = state.code;
			config.heartbeat_messages = state.messages;
			config.heartbeat_state = state;
		}

		res.render('wizard/current.ejs', config);
	});
});
app.post('/wizard/reset', function(req, res) {
	try {
		fs.unlinkSync(sensors_file);
	} catch (e) {
	}
	open_config();

	var wizard = new Wizard();
	wizard.deconfigure_network(function(err) {
		if (err) {
			res.send(JSON.stringify({ step: "deconfigure_network", error: err }), 500);
			return;
		}
		wizard.unregisterMSG(function(err) {
			if (err) {
				res.send(JSON.stringify({ step: "unregisterMSG", error: err }), 500);
				return;
			}
			res.redirect('/wizard/resetdone');
		});
	});
});
app.get('/wizard/resetdone', function(req, res) {
	res.render('wizard/resetdone.ejs', { active_tabpage: 'configuration' });
	var wizard = new Wizard();
	wizard.complete_reset();
});
app.get('/wizard/devices', function(req, res) {
	var devices = {};

	sensor_registry.get_sensors(function(sensor) {
		var device = (devices[sensor.ip] = devices[sensor.ip] || { name: sensor.name, ip: sensor.ip, eids: [] });

		device.eids.push({
			eid: sensor.eid,
			description: sensor.description,
			unit: sensor.unit
		});

		var later_update = Math.max(sensor.last_update || 0, sensor.last_value_received || 0);
		if (device.last_update === undefined || device.last_update > later_update) {
			device.last_update = later_update;
		}
	});

	for (var key in devices) {
		devices[key].eids.sort(function(a, b) {
			return b.eid - a.eid;
		});
	}

	res.render('wizard/devices.ejs', { active_tabpage: 'configuration', devices: devices });
});
app.get('/wizard/:step', function(req, res) {
	res.render('wizard/' + req.params.step  + '.ejs', { active_tabpage: 'configuration' });
});

io.sockets.on('connection', function (socket) {
  console.log("Registering new client.");
	sensor_cache.on('sensor_update', function(update) {
		if (update.sensor.function != "infrastructure") {
			socket.volatile.emit('sensor_update', { sensor: update.sensor.id, device: update.sensor.ip, value: update.value });
		}
	});
	sensor_registry.on('sensor_new', function(sensor) {
		socket.volatile.emit('sensor_new', sensor);
	});
	socket.on('sensor_request_metadata', function(id) {
		var sensor = sensor_registry.get_sensor_by_id(id);
		if (sensor) {
			socket.volatile.emit('sensor_metadata', sensor);
			var value = { sensor: id, device: sensor.ip, value: sensor_cache.get_current_value(sensor) };
			if (value.value != undefined) {
				socket.volatile.emit('sensor_update', value);
			}
		}
	});
  // a client can also initiate a data update.
  socket.on('sensor_refresh_data', function() {
  //  console.log("Refresh data event.");
		sensor_cache.enumerate_current_values(function(update) {
			socket.volatile.emit('sensor_update', udpate);
		});
  });
	socket.on('device_rename', function(msg) {
		var sensors = sensor_registry.get_sensors(function(sensor) {
			return sensor.ip == msg.device;
		});
		if (sensors.length > 0) {
			try {
				hexabus.rename_device(msg.device, msg.name, function(err) {
					if (err) {
						socket.emit('device_rename_error', { device: msg.device, error: err });
						return;
					}
					for (var key in sensors) {
						sensors[key].name = msg.name;
						socket.broadcast.emit('sensor_metadata', sensors[key]);
						socket.emit('sensor_metadata', sensors[key]);
						var value = { sensor: sensors[key].id, value: sensor_cache.get_current_value(sensors[key]) };
						if (value.value) {
							socket.broadcast.emit('sensor_update', value);
							socket.emit('sensor_update', value);
						}
					}
				});
			} catch (err) {
				console.log(err);
				socket.emit('device_rename_error', { device: msg.device, error: err });
			}
		} else {
			socket.emit('device_rename_error', { device: msg.device, error: "No such device" });
		}
	});
	socket.on('sensor_change_metadata', function(msg) {
		var sensor = sensor_registry.get_sensor_by_id(msg.id);
		if (sensor) {
			var keys = ["minvalue", "maxvalue"];
			for (var i in keys) {
				var key = keys[i];
				sensor[key] = (msg.data[key] != undefined) ? msg.data[key] : sensor[key];
			}
			save_sensor_registry();
			socket.broadcast.emit('sensor_metadata', sensor);
			socket.emit('sensor_metadata', sensor);
			var value = { sensor: sensor.id, devic: sensor.ip, value: sensor_cache.get_current_value(sensor) };
			if (value.value) {
				socket.broadcast.emit('sensor_update', value);
				socket.emit('sensor_update', value);
			}
		}
	});

	socket.on('wizard_configure', function() {
		var wizard = new Wizard();

		wizard.configure_network(function(progress) {
			socket.emit('wizard_configure_step', progress);
		});
	});

	socket.on('wizard_register', function() {
		var wizard = new Wizard();

		wizard.registerMSG(function(progress) {
			socket.emit('wizard_register_step', progress);
		});
	});

	socket.emit('clear_state');

	var list = sensor_registry.get_sensors(function(sensor) {
		if (sensor.function != "infrastructure") {
			socket.emit('sensor_metadata', sensor);
		}
	});
	sensor_cache.enumerate_current_values(function(value) {
		if (sensor_registry.get_sensor_by_id(value.sensor).function != "infrastructure") {
			socket.emit('sensor_update', value);
		}
	});
});

