#pragma once

#include "json_parser_tree.h"
#include "json_parser_io.h"

#include <iostream>
#include <boost/optional.hpp>
#include <boost/format.hpp>

namespace json_parser{

        struct maker{
        private:
                struct detail{
                        struct start_{
                                template<class T>
                                void append(T&& value){
                                        if( opt_ )
                                                BOOST_THROW_EXCEPTION(std::domain_error("can only have one root"));
                                        opt_ = value;
                                }
                                node to_node()const{
                                        if( opt_ )
                                                return opt_.get();
                                        BOOST_THROW_EXCEPTION(std::domain_error("start_ has no node"));
                                        __builtin_unreachable();
                                }
                        private:
                                boost::optional<node> opt_;
                        };
                        struct making_array{
                                template<class T>
                                void append(T&& value){
                                        data_.emplace_back(std::forward<T>(value));
                                }
                                array_ const& to_node()const{return data_;}
                        private:
                                array_ data_;
                        };
                        struct making_map{
                                // to add {"one":1}, it goes
                                //      begin_map();
                                //              make_string("one");
                                //              make_int(1);
                                //      end_map();
                                //
                                // thus, want to buffer the first one
                                template<class T>
                                void append(T&& value){
                                        if( key_ ){
                                                data_.push_back(key_.get(), value );
                                                key_ = boost::none;
                                        } else{
                                                key_ = std::forward<T>(value);
                                        }
                                }
                                map_ const& to_node()const{
                                        if( key_ )
                                                BOOST_THROW_EXCEPTION(std::domain_error("having matched keys with values"));
                                        return data_;
                                }
                        private:
                                boost::optional<node> key_;
                                map_ data_;
                        };
                };

                using state_t = boost::variant<
                        detail::start_,
                        detail::making_array,
                        detail::making_map
                >;

                // for primitives
                template<class T>
                struct append : boost::static_visitor<>
                {
                        explicit append(T const& value):value_(value){}

                        template<class State>
                        void operator()(State&& state)const{
                                state.append(std::move(value_));
                        }
                private:
                        T value_;
                };

                struct node_cast : boost::static_visitor<node>{
                        template<class State>
                        node operator()(State&& state)const{
                                return state.to_node();
                        }
                };
        public:

                maker(){
                        reset();
                }

                void begin_map(){
                        state_.emplace_back( detail::making_map() );
                } 
                void end_map(){
                        detail::making_map aux = std::move(boost::get<detail::making_map>(state_.back()));
                        state_.pop_back();
                        boost::apply_visitor( append<map_>(aux.to_node()), state_.back() );
                } 
                void begin_array(){
                        state_.emplace_back( detail::making_array() );
                } 
                void end_array(){
                        detail::making_array aux = std::move(boost::get<detail::making_array>(state_.back()));
                        state_.pop_back();
                        boost::apply_visitor( append<array_>(aux.to_node()), state_.back() );
                } 
                void make_string(std::string const& value){
                        boost::apply_visitor( append<std::string>(value), state_.back() );
                }
                void make_int(std::int64_t value){
                        boost::apply_visitor( append<std::int64_t>(value), state_.back() );
                }
                void make_float(long double value){
                        boost::apply_visitor( append<long double>(value), state_.back() );
                }
                void make_null(){
                        boost::apply_visitor( append<null_>(null_()), state_.back() );
                }
                void make_bool(bool value){
                        boost::apply_visitor( append<bool>(value), state_.back() );
                }
                void make_true(){
                        boost::apply_visitor( append<bool>(true), state_.back() );
                }
                void make_false(){
                        boost::apply_visitor( append<bool>(false), state_.back() );
                }

                node make(){ 
                        switch(state_.size()){
                                case 1:
                                        return boost::apply_visitor( node_cast(), state_.back());
                                default:
                                        BOOST_THROW_EXCEPTION(std::domain_error("too many beings, not ehough endz"));
                        }

                } 
                void reset(){
                        state_.clear();
                        state_.emplace_back( detail::start_() );
                }
        private:
                std::vector<state_t> state_;
        };
}
