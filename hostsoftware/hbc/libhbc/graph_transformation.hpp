#ifndef LIBHBC_GRAPH_TRANSFORMATION
#define LIBHBC_GRAPH_TRANSFORMATION

#include <libhbc/common.hpp>
#include <libhbc/graph.hpp>
#include <libhbc/tables.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>

namespace hexabus {

  typedef std::map<unsigned int, std::vector<vertex_id_t> > machine_map_t;
  typedef std::pair<unsigned int, std::vector<vertex_id_t> > machine_map_elem;
  typedef std::multimap<std::string, std::vector<vertex_id_t> > device_map_t;

  class GraphTransformation {
    public:
      typedef std::tr1::shared_ptr<GraphTransformation> Ptr;
      GraphTransformation(device_table_ptr d, endpoint_table_ptr e) : _d(d), _e(e) {};
      virtual ~GraphTransformation() {};
      void writeGraphviz(std::string filename_prefix);
      std::map<std::string, graph_t_ptr> getDeviceGraphs();

      void operator()(graph_t_ptr in_g);

    private:
      device_table_ptr _d;
      endpoint_table_ptr _e;
      std::map<std::string, graph_t_ptr> device_graphs;
      machine_map_t generateMachineMap(graph_t_ptr g);
      std::vector<std::string> findDevices(std::vector<vertex_id_t> stm_vertices, graph_t_ptr g);
      graph_t_ptr reconstructGraph(std::vector<vertex_id_t> vertices, graph_t_ptr in_g);
      template <typename T> bool exists(T elem, std::vector<T> vect);
  };

  // Visitor class for reachability analysis: Takes all nodes which it finds when handed to boost::breadth_first_search and puts them into a vector
  // TODO remove when not needed!
  // Example: boost::breadth_first_search(boost::make_reverse_graph(*g), start_vertex, visitor(bfs_nodelist_maker(&reachable_nodes)));
  class bfs_nodelist_maker : public boost::default_bfs_visitor {
    public:
      bfs_nodelist_maker(std::vector<vertex_id_t>* node_ids) : node_id_list(node_ids) { }
      template <typename Vertex, typename Graph> void discover_vertex(Vertex u, Graph g) {
        if(!contains(*node_id_list, u)) {
          node_id_list->push_back(u);
        }
      }

    private:
      std::vector<vertex_id_t>* node_id_list;
  };
};

#endif // LIBHBC_GRAPH_TRANSFORMATION