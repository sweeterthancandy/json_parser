#pragma once

#define PRINT(X) do{ std::cout << #X " => " << (X) << "\n";}while(0)

#include "json_parser_tree.h"
#include "json_parser_maker.h"
#include "json_parser_io.h"
#include "json_parser_tokenizer.h"
#include "json_parser_fast_parser.h"

namespace json_parser{
        template<class Iter>
        auto parse(Iter first, Iter last)->boost::optional<node>{
                maker m;
                detail::basic_parser<maker,Iter> p(m,first,last);
                try{ 
                        p.parse();
                } catch(std::exception const& e){
                        return boost::none;
                }
                return m.make();
        }
        template<class Maker, class Iter>
        auto parse(Maker& m, Iter first, Iter last)->bool{
                //detail::xpr_parser p;
                detail::basic_parser<Maker,Iter> p(m,first,last);
                try{ 
                        // this throws if ! eos
                        p.parse();
                        assert( p.eos() && "post condition violated");
                        return true;
                } catch(std::exception const& e){
                        return false;
                }
                __builtin_unreachable();
        }
        decltype(auto) parse(std::string const& s){
                return parse(s.begin(), s.end());
        }


}

