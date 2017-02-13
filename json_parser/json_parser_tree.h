#pragma once

#include <vector>
#include <utility>
#include <cstdint>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>


namespace json_parser{


        struct array_;
        struct map_;

        struct null_{};

        using node = boost::variant<
                std::string,
                std::int64_t,
                long double,
                bool,
                boost::recursive_wrapper<array_>,
                boost::recursive_wrapper<map_>,
                null_
        >;
        struct array_{
                template<class T>
                void push_back(T&& value){
                        data_.push_back( std::forward<T>(value) );
                }
                template<class T>
                void emplace_back(T&& value){
                        data_.emplace_back( std::forward<T>(value) );
                }
                template<class F>
                void for_each(F&& f){
                        boost::for_each( data_, f );
                }
                template<class F>
                void for_each(F&& f)const{
                        boost::for_each( data_, f );
                }
                auto size()const{return data_.size();}
        private:
                std::vector<node> data_;
        };
        struct map_{
                template<class K, class V>
                void push_back(K&& key, V&& value){
                        data_.emplace_back(
                                std::forward<K>(key),
                                std::forward<V>(value)
                        );
                }
                template<class F>
                void for_each(F&& f){
                        boost::for_each( data_, f );
                }
                template<class F>
                void for_each(F&& f)const{
                        boost::for_each( data_, f );
                }
                auto size()const{return data_.size();}
        private:
                std::vector<std::pair<node,node> > data_;
        };

        namespace detail{
                namespace mpl = boost::mpl;

                using node_vec = mpl::vector<
                        std::string,
                        std::int64_t,
                        long double,
                        bool,
                        array_,
                        map_,
                        null_
                >;

                template<class T>
                struct try_cast_visitor : boost::static_visitor<T&>{
                        static_assert( 
                                mpl::contains<detail::node_vec,T>::value,
                                "type not in node!"
                        );

                        template<class U>
                        std::enable_if_t<!std::is_same<std::decay_t<U>,T>::value,T&>
                        operator()(U&&)const{
                                BOOST_THROW_EXCEPTION(std::domain_error("not target type"));
                                __builtin_unreachable();
                        } 
                        T& operator()(T& that)const{
                                return that;
                        }
                };
                
                template<class T>
                struct numeric_cast_visitor : boost::static_visitor<T>{

                        using valid_nodes = mpl::vector<std::string,std::int64_t,long double,bool>;

                        template<class U>
                        std::enable_if_t<!mpl::contains<valid_nodes,std::decay_t<U>>::value,T>
                        operator()(U&&)const{
                                std::stringstream sstr;
                                sstr << "try is a container or null_ (";
                                sstr << boost::typeindex::type_id<U>().pretty_name();
                                sstr << ")";
                                BOOST_THROW_EXCEPTION(std::domain_error(sstr.str()));
                                __builtin_unreachable();
                        } 
                        template<class U>
                        std::enable_if_t<
                                mpl::contains<
                                        mpl::vector<std::int64_t,long double,bool>
                                        ,std::decay_t<U>
                                >::value,T
                        >
                        operator()(U&& val)const{
                                return static_cast<T>(val);
                        }
                        T operator()(std::string const& s)const{
                                return boost::lexical_cast<T>(s);
                        }
                };
        }

        template<class T>
        T const& try_cast(node const& root){
                return boost::apply_visitor( detail::try_cast_visitor<T const>(), root);
        }
        template<class T>
        T& try_cast(node& root){
                return boost::apply_visitor( detail::try_cast_visitor<T>(), root);
        }
        template<class T>
        T numeric_cast(node const& root){
                return boost::apply_visitor( detail::numeric_cast_visitor<T>(), root);
        }
}

