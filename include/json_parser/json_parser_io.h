#pragma once

#include <iostream>
#include <sstream>
#include <tuple>
#include <string>

#include <boost/lexical_cast.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/char.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/string.hpp>

#include "json_parser_tree.h"

namespace json_parser{
        namespace detail{
                
                struct map_multi_line_policy
                        : mpl::vector<
                                mpl::string<'{'>,
                                mpl::string<'}'>,
                                mpl::string<','>,
                                mpl::true_,
                                mpl::true_
                        >
                {};
                struct array_multi_line_policy
                        : mpl::vector<
                                mpl::string<'['>,
                                mpl::string<']'>,
                                mpl::string<','>,
                                mpl::true_,
                                mpl::true_
                        >
                {};
                struct pair_multi_line_policy
                        : mpl::vector<
                                mpl::string<>,
                                mpl::string<>,
                                mpl::string<':'>,
                                mpl::false_,
                                mpl::false_
                        >
                {};

                struct multi_line_policy
                        : mpl::vector<
                                map_multi_line_policy
                                , array_multi_line_policy
                                , pair_multi_line_policy
                        >
                {};
                
                struct map_single_line_policy
                        : mpl::vector<
                                mpl::string<'{'>,
                                mpl::string<'}'>,
                                mpl::string<','>,
                                mpl::false_,
                                mpl::false_
                        >
                {};
                struct array_single_line_policy
                        : mpl::vector<
                                mpl::string<'['>,
                                mpl::string<']'>,
                                mpl::string<','>,
                                mpl::false_,
                                mpl::false_
                        >
                {};
                struct pair_single_line_policy
                        : mpl::vector<
                                mpl::string<>,
                                mpl::string<>,
                                mpl::string<':'>,
                                mpl::false_,
                                mpl::false_
                        >
                {};

                struct single_line_policy
                        : mpl::vector<
                                map_single_line_policy
                                , array_single_line_policy
                                , pair_single_line_policy
                        >
                {};


                template<class Policy>
                struct to_string_context{

                        enum class state_e{
                                doing_map=0,
                                doing_pair,
                                doing_array,
                                pseudo
                        };
                        
                        enum tuple_helperz{
                                _state = 0,
                                count_,
                                begin_,
                                end_,
                                sep_,
                                indent_,
                                newline_
                        };

                        explicit to_string_context(std::ostream& ostr)
                                :ostr_(&ostr)
                        {
                                state_.emplace_back(state_e::pseudo,0,"","","",false,false);
                        }
                        void begin_map(){   do_begin_( state_e::doing_map ); }
                        void end_map(){     do_end_  ( state_e::doing_map ); }
                        void begin_array(){ do_begin_( state_e::doing_array ); }
                        void end_array(){   do_end_  ( state_e::doing_array ); }
                        void begin_pair(){  do_begin_( state_e::doing_pair ); }
                        void end_pair(){    do_end_  ( state_e::doing_pair ); }
                        // indent opt_sep token newline
                        void put(std::string const& token){ 
                                if( std::get<indent_>(state_.back()) )
                                        do_indent_(state_.size()-1);
                                try_sep_();
                                ++std::get<count_>(state_.back());
                                *ostr_ << token;
                                if( std::get<newline_>(state_.back()) )
                                        do_newline_();
                        }
                private:
                        // indent opt_sep token newline
                        void do_begin_( state_e s){
                                if( std::get<indent_>(state_.back()) )
                                        do_indent_( state_.size()-1 );
                                try_sep_();
                                switch(s){
                                        //case state_e::doing_map:   state_.emplace_back(s,0,"{","}",",",true ,true); break;
                                        case state_e::doing_map:
                                                state_.emplace_back(s,0,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,0>::type,0>::type>::value,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,0>::type,1>::type>::value,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,0>::type,2>::type>::value,
                                                        typename mpl::at_c<typename mpl::at_c<Policy,0>::type,3>::type(),
                                                        typename mpl::at_c<typename mpl::at_c<Policy,0>::type,4>::type()
                                                );
                                                break;
                                        case state_e::doing_array:
                                                state_.emplace_back(s,0,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,1>::type,0>::type>::value,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,1>::type,1>::type>::value,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,1>::type,2>::type>::value,
                                                        typename mpl::at_c<typename mpl::at_c<Policy,1>::type,3>::type(),
                                                        typename mpl::at_c<typename mpl::at_c<Policy,1>::type,4>::type()
                                                );
                                                break;
                                        case state_e::doing_pair:
                                                state_.emplace_back(s,0,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,2>::type,0>::type>::value,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,2>::type,1>::type>::value,
                                                        mpl::c_str<typename mpl::at_c<typename mpl::at_c<Policy,2>::type,2>::type>::value,
                                                        typename mpl::at_c<typename mpl::at_c<Policy,2>::type,3>::type(),
                                                        typename mpl::at_c<typename mpl::at_c<Policy,2>::type,4>::type()
                                                );
                                                break;
                                        case state_e::pseudo:      __builtin_unreachable();
                                }
                                *ostr_ << std::get<begin_>(state_.back());
                                if( std::get<newline_>(state_.back()) )
                                        do_newline_();
                        }
                        // indent token newline
                        void do_end_( state_e s){
                                assert( s == std::get<0>(state_.back()) && "inconsistent");
                                if( std::get<indent_>(state_.back()) )
                                        do_indent_( state_.size()-2);
                                *ostr_ << std::get<end_>(state_.back());
                                state_.pop_back();
                                if( std::get<newline_>(state_.back()) )
                                        do_newline_();
                                ++std::get<count_>(state_.back());
                        }
                        void try_sep_(){
                                if( std::get<count_>(state_.back() ) != 0 ){
                                        *ostr_ << std::get<sep_>(state_.back());
                                }
                        }
                        void do_newline_(){
                                *ostr_ << "\n";
                        }
                        void do_indent_(size_t n){
                                *ostr_ << std::string(n*2,' ');
                        }

                        std::ostream* ostr_;
                        std::vector<std::tuple<state_e,size_t,std::string,std::string,std::string,bool,bool> > state_;
                };

                // V(a) -> [a,a] 
                // V(V(a)) = V([a,a]) -> [[a,a],[a,a]]
                // V(V(V(a))) -> [[[a,a],[a,a]],[[a,a],[a,a]]]

                template<class Context>
                struct to_string_visitor : boost::static_visitor<void>
                {
                        explicit to_string_visitor(Context& ctx):ctx_(&ctx){}
                        void operator()(std::string const& str)const{
                                ctx_->put( str );
                        }
                        void operator()(std::int64_t value)const{
                                ctx_->put( boost::lexical_cast<std::string>(value) );
                        }
                        void operator()(long double value)const{
                                ctx_->put( boost::lexical_cast<std::string>(value) );
                        }
                        void operator()(bool value)const{
                                ctx_->put( value ? "true" : "false" );
                        }
                        void operator()(array_ const& con)const{
                                ctx_->begin_array();
                                con.for_each( [&](auto&& ele){
                                        boost::apply_visitor(*this,ele); 
                                });
                                ctx_->end_array();
                        }
                        void operator()(map_ const& con)const{
                                ctx_->begin_map();
                                con.for_each( [&](auto&& p){
                                        ctx_->begin_pair();
                                        boost::apply_visitor(*this,p.first); 
                                        boost::apply_visitor(*this,p.second); 
                                        ctx_->end_pair();
                                });
                                ctx_->end_map();
                        }
                        void operator()(null_)const{
                                ctx_->put( "(null)" );
                        }
                private:
                        Context* ctx_;
                };
        }

        inline
        std::string to_string(node const& root){
                using ctx_t = detail::to_string_context<detail::single_line_policy>;
                std::stringstream sstr;
                ctx_t ctx(sstr);
                detail::to_string_visitor<ctx_t> aux(ctx);
                boost::apply_visitor( aux, root);
                return sstr.str();
        }
        inline
        void display(node const& root, std::ostream& ostr = std::cout){
                using ctx_t = detail::to_string_context<detail::multi_line_policy>;
                ctx_t ctx(ostr);
                detail::to_string_visitor<ctx_t> aux(ctx);
                boost::apply_visitor( aux, root);
                std::cout << std::endl;
        }
}
