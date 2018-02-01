#pragma once

#define PRINT(X) do{ std::cout << #X " => " << (X) << "\n";}while(0)

#include "json_parser_tree.h"
#include "json_parser_maker.h"
#include "json_parser_io.h"
#include "json_parser_tokenizer.h"
#include "json_parser_fast_parser.h"

namespace json_parser{
        template<class Iter>
        auto try_parse(Iter first, Iter last)->boost::optional<node>{
                maker m;
                detail::basic_parser<maker,Iter> p(m,first,last);
                try{ 
                        p.parse();
                } catch(std::exception const& e){
                        return boost::none;
                }
                return m.make();
        }
        inline
        decltype(auto) try_parse(std::string const& s){
                return try_parse(s.begin(), s.end());
        }
        


        template<class Iter>
        auto parse(Iter first, Iter last)->node{
                maker m;
                detail::basic_parser<maker,Iter> p(m,first,last);
                p.parse();
                return m.make();
        }
        inline
        decltype(auto) parse(std::string const& s){
                return parse(s.begin(), s.end());
        }


}

