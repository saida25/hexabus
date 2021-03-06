#ifndef LIBHBA_GENERATOR_FLASH_HPP
#define LIBHBA_GENERATOR_FLASH_HPP 1

#include <libhba/common.hpp>
#include <libhba/ast_datatypes.hpp>
#include <libhba/graph.hpp>
#include <iostream>

namespace hexabus {

  class generator_flash {
    public:
      typedef std::tr1::shared_ptr<generator_flash> Ptr;
      generator_flash (graph_t_ptr g, hba_doc ast, std::string dtypes_filename, bool verbose)
        : _g(g),
         _ast(ast),
         _dtypes(dtypes_filename),
         _verbose(verbose)
      {};
      virtual ~generator_flash() {};

      void operator()(std::vector<uint8_t>& v) const;
    private:
      generator_flash (const generator_flash& original);
      generator_flash& operator= (const generator_flash& rhs);
      graph_t_ptr _g;
      hba_doc _ast;
      std::string _dtypes;
      bool _verbose;
  };
};

#endif /* LIBHBA_GENERATOR_FLASH_HPP */
