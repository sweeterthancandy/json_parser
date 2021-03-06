#ifndef JSON_PARSER_BASIC_PARSER_H
#define JSON_PARSER_BASIC_PARSER_H

#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include "tokenizer.h"
#include <iostream>

namespace gjson {

        template <class Maker, class Iter>
        struct basic_parser {



                basic_parser( Maker& maker, Iter first, Iter last  )
                      : tok_( first, last )
                      , not_a_token_( token_type::dummy, "(dummy)" )
                      , maker_(maker)
                {}

                void debug_(){
                        for(;;){
                                auto opt = tok_.peak();
                                if( ! opt )
                                        break;
                                std::cout << opt.get() << "\n";
                                tok_.next();
                        }
                }
                void parse(){
                        bool ret = obj_();
                        if( ! ret )
                               tok_.return_errror_("expected a map");
                        if( ! eos())
                               tok_.return_errror_("unable to parse all the input");
                }
                auto eos()const{return tok_.eos();}
        private:
                bool map_(){
                        if( eat_( token_type::left_curl ) ){
                                maker_.begin_map();

                                comma_seperated_( [&](){ return pair_(); } );

                                if( eat_( token_type::right_curl)){
                                        maker_.end_map();
                                        return true;
                                }
                                tok_.return_errror_("expected a '}'");
                        }
                        return false;
                }
                bool array_(){
                        if( eat_( token_type::left_br ) ){
                                maker_.begin_array();

                                comma_seperated_( [&](){ return prim_or_obj_(); } );

                                if( eat_( token_type::right_br)){
                                        maker_.end_array();
                                        return true;
                                }
                                tok_.return_errror_("expected a ']'");
                        }
                        return false;
                }
                bool prim_or_obj_(){
                        if( prim_() ){
                                return true;
                        } else if( map_() ){
                                return true;
                        } else if( array_() ){
                                return true;
                        } else{
                                return false;
                        }
                }
                bool obj_(){
                        if( map_() ){
                                return true;
                        } else if( array_() ){
                                return true;
                        } else{
                                return false;
                        }
                }
                template<class F>
                bool comma_seperated_(F f){
                        if( f() ){
                                for(;;){
                                        if( eat_( token_type::comma) ){
                                               if( ! f() ){
                                                        tok_.return_errror_("expected a pair");
                                               }
                                        } else {
                                                break;
                                        }
                                }
                                return true;
                        }
                        return false;
                }
                bool pair_(){
                        if( prim_() && eat_( token_type::colon) && prim_or_obj_() ){
                                return true;
                        }
                        return false;
                }
                bool prim_(){
                        switch( boost::get_optional_value_or(tok_.peak(), not_a_token_ ).type()){
                                case token_type::int_:
                                        maker_.make_int( boost::lexical_cast<std::int64_t>(tok_.peak()->value()));
                                        tok_.next();
                                        return true;
                                case token_type::float_:
                                        maker_.make_float( boost::lexical_cast<long double>(tok_.peak()->value()));
                                        tok_.next();
                                        return true;
                                case token_type::string_:
                                        maker_.make_string( tok_.peak()->value() );
                                        tok_.next();
                                        return true;
                                case token_type::true_:
                                        maker_.make_true();
                                        tok_.next();
                                        return true;
                                case token_type::false_:
                                        maker_.make_false();
                                        tok_.next();
                                        return true;
                                case token_type::null_:
                                        maker_.make_null();
                                        tok_.next();
                                        return true;
                                default:
                                        return false;
                        }
                        __builtin_unreachable();
                }
                bool eat_(token_type type){
                        if( boost::get_optional_value_or(tok_.peak(), not_a_token_ ).type() == type ){
                                tok_.next();
                                return true;
                        }
                        return false;
                }

                basic_tokenizer<Iter> tok_;
                token not_a_token_;
                Maker& maker_;
        };


} // json_parser_tokenizer
#endif // JSON_PARSER_BASIC_PARSER_H
