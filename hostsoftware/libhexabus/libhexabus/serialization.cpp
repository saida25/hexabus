#include <libhexabus/private/serialization.hpp>

#include <stdexcept>
#include <algorithm>
#include <arpa/inet.h>

#include "crc.hpp"
#include "error.hpp"

using namespace hexabus;

// {{{ Binary serialization visitor

class BinarySerializer : public PacketVisitor {
	private:
		std::vector<char>& _target;
		size_t _headerStart;

		void append_u8(uint8_t value);
		void append_u16(uint16_t value);
		void append_u32(uint32_t value);
		void append_float(float value);

		void appendHeader(const Packet& packet);
		void appendHeaderWithEID(const EIDPacket& packet);

		void appendValue(const ValuePacket<bool>& value);
		void appendValue(const ValuePacket<uint8_t>& value);
		void appendValue(const ValuePacket<uint32_t>& value);
		void appendValue(const ValuePacket<float>& value);
		void appendValue(const ValuePacket<boost::posix_time::ptime>& value);
		void appendValue(const ValuePacket<boost::posix_time::time_duration>& value);
		void appendValue(const ValuePacket<std::string>& value);
		void appendValue(const ValuePacket<std::vector<char> >& value);

		void appendCRC();

	public:
		BinarySerializer(std::vector<char>& target);

		virtual void visit(const ErrorPacket& error);
		virtual void visit(const QueryPacket& query);
		virtual void visit(const EndpointQueryPacket& endpointQuery);
		virtual void visit(const EndpointInfoPacket& endpointInfo);

		virtual void visit(const InfoPacket<bool>& info);
		virtual void visit(const InfoPacket<uint8_t>& info);
		virtual void visit(const InfoPacket<uint32_t>& info);
		virtual void visit(const InfoPacket<float>& info);
		virtual void visit(const InfoPacket<boost::posix_time::ptime>& info);
		virtual void visit(const InfoPacket<boost::posix_time::time_duration>& info);
		virtual void visit(const InfoPacket<std::string>& info);
		virtual void visit(const InfoPacket<std::vector<char> >& info);

		virtual void visit(const WritePacket<bool>& write);
		virtual void visit(const WritePacket<uint8_t>& write);
		virtual void visit(const WritePacket<uint32_t>& write);
		virtual void visit(const WritePacket<float>& write);
		virtual void visit(const WritePacket<boost::posix_time::ptime>& write);
		virtual void visit(const WritePacket<boost::posix_time::time_duration>& write);
		virtual void visit(const WritePacket<std::string>& write);
		virtual void visit(const WritePacket<std::vector<char> >& write);
};

BinarySerializer::BinarySerializer(std::vector<char>& target)
	: _target(target)
{
	_target.reserve(HXB_MAX_PACKET_SIZE);
}

void BinarySerializer::append_u8(uint8_t value)
{
	_target.push_back(value);
}

void BinarySerializer::append_u16(uint16_t value)
{
	union {
		uint16_t u16;
		char raw[sizeof(value)];
	} c = { htons(value) };

	_target.insert(_target.end(), c.raw, c.raw + sizeof(c.raw));
}

void BinarySerializer::append_u32(uint32_t value)
{
	union {
		uint32_t u32;
		char raw[sizeof(value)];
	} c = { htonl(value) };

	_target.insert(_target.end(), c.raw, c.raw + sizeof(c.raw));
}

void BinarySerializer::append_float(float value)
{
	union {
		float f;
		uint32_t u32;
		char raw[sizeof(value)];
	} c = { value };
	c.u32 = htonl(c.u32);

	_target.insert(_target.end(), c.raw, c.raw + sizeof(c.raw));
}

void BinarySerializer::appendHeader(const Packet& packet)
{
	_headerStart = _target.size();

	_target.insert(_target.end(), HXB_HEADER, HXB_HEADER + strlen(HXB_HEADER));
	_target.push_back(packet.type());
	_target.push_back(packet.flags());
}

void BinarySerializer::appendHeaderWithEID(const EIDPacket& packet)
{
	appendHeader(packet);
	append_u32(packet.eid());
}

void BinarySerializer::appendCRC()
{
	uint16_t crc = hexabus::crc(&_target[_headerStart], _target.size() - _headerStart);

	append_u16(crc);
}

void BinarySerializer::visit(const ErrorPacket& error)
{
	appendHeader(error);
	append_u8(error.code());
	appendCRC();
}

void BinarySerializer::visit(const QueryPacket& query)
{
	appendHeaderWithEID(query);
	appendCRC();
}

void BinarySerializer::visit(const EndpointQueryPacket& endpointQuery)
{
	appendHeaderWithEID(endpointQuery);
	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<bool>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_BOOL);
	append_u8(value.value());

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<uint8_t>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_UINT8);
	append_u8(value.value());

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<uint32_t>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_UINT32);
	append_u32(value.value());

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<float>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_FLOAT);
	append_float(value.value());

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<boost::posix_time::ptime>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_DATETIME);
	append_u8(value.value().time_of_day().hours());
	append_u8(value.value().time_of_day().minutes());
	append_u8(value.value().time_of_day().seconds());
	append_u8(value.value().date().day());
	append_u8(value.value().date().month());
	append_u16(value.value().date().year());
	append_u8(value.value().date().day_of_week());

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<boost::posix_time::time_duration>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_TIMESTAMP);
	append_u32(value.value().total_seconds());

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<std::string>& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_128STRING);
	_target.insert(_target.end(), value.value().begin(), value.value().end());
	_target.insert(_target.end(), HXB_STRING_PACKET_MAX_BUFFER_LENGTH + 1 - value.value().size(), '\0');

	appendCRC();
}

void BinarySerializer::appendValue(const ValuePacket<std::vector<char> >& value)
{
	appendHeaderWithEID(value);

	append_u8(HXB_DTYPE_66BYTES);
	_target.insert(_target.end(), value.value().begin(), value.value().end());
	_target.insert(_target.end(), HXB_BYTES_PACKET_MAX_BUFFER_LENGTH - value.value().size(), '\0');

	appendCRC();
}


void BinarySerializer::visit(const EndpointInfoPacket& endpointInfo) { appendValue(endpointInfo); }

void BinarySerializer::visit(const InfoPacket<bool>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<uint8_t>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<uint32_t>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<float>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<boost::posix_time::ptime>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<boost::posix_time::time_duration>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<std::string>& info) { appendValue(info); }
void BinarySerializer::visit(const InfoPacket<std::vector<char> >& info) { appendValue(info); }

void BinarySerializer::visit(const WritePacket<bool>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<uint8_t>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<uint32_t>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<float>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<boost::posix_time::ptime>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<boost::posix_time::time_duration>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<std::string>& write) { appendValue(write); }
void BinarySerializer::visit(const WritePacket<std::vector<char> >& write) { appendValue(write); }

// }}}

std::vector<char> hexabus::serialize(const Packet& packet)
{
	std::vector<char> result;
	BinarySerializer serializer(result);

	packet.accept(serializer);

	return result;
}

// {{{ Binary deserialization

class BinaryDeserializer {
	private:
		const char* _packet;
		size_t _offset;
		size_t _size;

		void checkLength(size_t min);

		void readHeader();

		uint8_t read_u8();
		uint16_t read_u16();
		uint32_t read_u32();
		float read_float();
		std::vector<char> read_bytes();
		std::string read_string();

		template<typename T>
		Packet::Ptr checkInfo(bool info, uint8_t eid, const T& value, uint8_t flags);

		template<typename T>
		Packet::Ptr check(const T& packet);

	public:
		BinaryDeserializer(const char* packet, size_t size)
			: _packet(packet), _offset(0), _size(size)
		{
			if (!packet)
				throw std::invalid_argument("packet");
		}

		Packet::Ptr deserialize();
};

void BinaryDeserializer::checkLength(size_t min)
{
	if (_size - _offset < min)
		throw BadPacketException("Packet too short");
}

void BinaryDeserializer::readHeader()
{
	checkLength(sizeof(HXB_HEADER));

	if (!memcmp(HXB_HEADER, _packet + _offset, sizeof(HXB_HEADER)))
		throw BadPacketException("Invalid header");

	_offset += sizeof(HXB_HEADER);
}

uint8_t BinaryDeserializer::read_u8()
{
	checkLength(sizeof(uint8_t));

	return *(_packet + _offset++);
}

uint16_t BinaryDeserializer::read_u16()
{
	checkLength(sizeof(uint16_t));

	union {
		char raw[sizeof(uint16_t)];
		uint16_t u16;
	} c;
	memcpy(c.raw, _packet + _offset, sizeof(c.raw));

	_offset += sizeof(uint16_t);
	return ntohs(c.u16);
}

uint32_t BinaryDeserializer::read_u32()
{
	checkLength(sizeof(uint32_t));

	union C {
		char raw[sizeof(uint32_t)];
		uint32_t u32;
	} c;
	memcpy(c.raw, _packet + _offset, sizeof(c.raw));

	_offset += sizeof(uint32_t);
	return ntohl(c.u32);
}

float BinaryDeserializer::read_float()
{
	union {
		uint32_t u32;
		float f;
	} c = { read_u32() };

	return c.f;
}

std::vector<char> BinaryDeserializer::read_bytes()
{
	checkLength(HXB_BYTES_PACKET_MAX_BUFFER_LENGTH);

	std::vector<char> result(_packet + _offset, _packet + _offset + HXB_BYTES_PACKET_MAX_BUFFER_LENGTH);
	_offset += HXB_BYTES_PACKET_MAX_BUFFER_LENGTH;

	return result;
}

std::string BinaryDeserializer::read_string()
{
	checkLength(HXB_STRING_PACKET_MAX_BUFFER_LENGTH);

	if (!std::find(_packet + _offset, _packet + _offset + HXB_STRING_PACKET_MAX_BUFFER_LENGTH, '\0'))
		throw BadPacketException("Unterminated string");

	std::string result(_packet + _offset);
	_offset += HXB_STRING_PACKET_MAX_BUFFER_LENGTH + 1;

	return result;
}

template<typename T>
Packet::Ptr BinaryDeserializer::checkInfo(bool info, uint8_t eid, const T& value, uint8_t flags)
{
	if (info) {
		return check(InfoPacket<T>(eid, value, flags));
	} else {
		return check(WritePacket<T>(eid, value, flags));
	}
}

template<typename T>
Packet::Ptr BinaryDeserializer::check(const T& packet)
{
	uint8_t crc = hexabus::crc(_packet, _offset);
	if (crc != read_u16()) {
		throw BadPacketException("Bad checksum");
	}

	return Packet::Ptr(new T(packet));
}

Packet::Ptr BinaryDeserializer::deserialize()
{
	readHeader();

	uint8_t type = read_u8();
	uint8_t flags = read_u8();

	switch (type) {
		case HXB_PTYPE_ERROR:
			{
				uint8_t code = read_u8();
				return check(ErrorPacket(code, flags));
			}

		case HXB_PTYPE_INFO:
		case HXB_PTYPE_WRITE:
			{
				bool info = type == HXB_PTYPE_INFO;
				uint8_t eid = read_u8();
				uint8_t datatype = read_u8();

				switch (datatype) {
					case HXB_DTYPE_BOOL:
						return checkInfo<bool>(info, eid, read_u8(), flags);

					case HXB_DTYPE_UINT8:
						return checkInfo<uint8_t>(info, eid, read_u8(), flags);

					case HXB_DTYPE_UINT32:
						return checkInfo<uint32_t>(info, eid, read_u32(), flags);

					case HXB_DTYPE_FLOAT:
						return checkInfo<float>(info, eid, read_float(), flags);

					case HXB_DTYPE_DATETIME:
						{
							uint8_t hour = read_u8();
							uint8_t minute = read_u8();
							uint8_t second = read_u8();
							uint8_t day = read_u8();
							uint8_t month = read_u8();
							uint16_t year = read_u8();
							uint8_t weekday = read_u8();

							boost::posix_time::ptime dt(
								boost::gregorian::date(year, month, day),
								boost::posix_time::hours(hour)
									+ boost::posix_time::minutes(minute)
									+ boost::posix_time::seconds(second));

							if (dt.date().day_of_week() != weekday)
								throw BadPacketException("Invalid datetime format");

							return checkInfo<boost::posix_time::ptime>(info, eid, dt, flags);
						}

					case HXB_DTYPE_TIMESTAMP:
						return checkInfo<boost::posix_time::time_duration>(info, eid, boost::posix_time::seconds(read_u32()), flags);

					case HXB_DTYPE_128STRING:
						return checkInfo<std::string>(info, eid, read_string(), flags);

					case HXB_DTYPE_66BYTES:
						return checkInfo<std::vector<char> >(info, eid, read_bytes(), flags);

					default:
						throw BadPacketException("Invalid datatype");
				}
			}

		case HXB_PTYPE_QUERY:
			{
				uint8_t eid = read_u8();
				return check(QueryPacket(eid, flags));
			}

		case HXB_PTYPE_EPQUERY:
			{
				uint8_t eid = read_u8();
				return check(EndpointQueryPacket(eid, flags));
			}

		case HXB_PTYPE_EPINFO:
			{
				uint8_t eid = read_u8();
				return check(EndpointInfoPacket(eid, read_string(), flags));
			}

		default:
			throw BadPacketException("Unknown packet type");
	}
}

// }}}

void hexabus::deserialize(const void* packet, size_t size, PacketVisitor& handler)
{
	deserialize(packet, size)->accept(handler);
}

Packet::Ptr hexabus::deserialize(const void* packet, size_t size)
{
	return BinaryDeserializer(reinterpret_cast<const char*>(packet), size).deserialize();
}
