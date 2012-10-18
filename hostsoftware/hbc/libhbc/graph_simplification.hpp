#ifndef LIBHBC_GRAPH_SIMPLIFICATION
#define LIBHBC_GRAPH_SIMPLIFICATION

#include <libhbc/graph.hpp>
#include <libhbc/ast_datatypes.hpp>

namespace hexabus {
  class GraphSimplification {
    public:
      GraphSimplification(std::map<std::string, graph_t_ptr> state_machines) : _in_state_machines(state_machines) { }

      void operator()();

    private:
      void addTransition(vertex_id_t from, vertex_id_t to, command_block_doc& commands);
      void deleteOthersWrites();
      command_block_doc commandBlockTail(command_block_doc& commands);

      std::map<std::string, graph_t_ptr> _in_state_machines;
  };
};

#endif // LIBHBC_GRAPH_SIMPLIFICATION