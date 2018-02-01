#ifndef JSON_PARSER_VARIANT_H
#define JSON_PARSER_VARIANT_H

#include <vector>
#include <utility>
#include <cstdint>
#include <iostream>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/string.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/variant.hpp>


namespace json_parser{
namespace variant{


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
                decltype(auto) operator[](size_t idx){
                        return data_[idx];
                }
                decltype(auto) operator[](size_t idx)const{
                        return data_[idx];
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

        namespace detail{
                namespace mpl = boost::mpl;
                
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

        template<class Iter>
        auto try_parse(Iter first, Iter last)->boost::optional<variant::node>{
                maker m;
                basic_parser<maker,Iter> p(m,first,last);
                try{ 
                        p.parse();
                } catch(std::exception const& e){
                        return boost::none;
                }
                return m.make();
        }
        template<class Iter>
        auto parse(Iter first, Iter last)->variant::node{
                variant::maker m;
                basic_parser<maker,Iter> p(m,first,last);
                p.parse();
                return m.make();
        }
        inline
        decltype(auto) parse(std::string const& s){
                return parse(s.begin(), s.end());
        }
        inline
        decltype(auto) try_parse(std::string const& s){
                return try_parse(s.begin(), s.end());
        }
} // variant
} // json_parser


#endif // JSON_PARSER_VARIANT_H
