#ifndef JSON_PARSER_JSONOBJECT_H
#define JSON_PARSER_JSONOBJECT_H

#include <cassert>
#include <string>
#include <list>
#include <sstream>
#include <map>
#include <iostream>
#include <vector>
#include <iterator>

#include <boost/lexical_cast.hpp>

namespace gjson{

namespace tt{
        template< bool B, class T, class F >
        using conditional_t = typename std::conditional<B,T,F>::type;
        template< bool B, class T = void >
        using enable_if_t = typename std::enable_if<B,T>::type;
        template< class T >
        using decay_t = typename std::decay<T>::type;
        template< class T >
        using remove_cv_t = typename std::remove_cv<T>::type;
} // std

namespace Detail{
        template<unsigned Order>
        struct precedence_device : precedence_device<Order-1>{};
        template<>
        struct precedence_device<0>{};
} // Detail

enum Type{
        Begin_Primitive,
                Type_Nil = Begin_Primitive,
                Type_Bool,
                Type_Integer,
                Type_Float,
                Type_String,
        End_Primitive,
        Begin_Aggregate,
                Type_Array = Begin_Aggregate,
                Type_Map,
        End_Aggregate,
        Type_NotAType = 100,
};
static std::string Type_to_string(Type e) {
        switch (e) {
        case Type_Nil:
                return "Type_Nil";
        case Type_Bool:
                return "Type_Bool";
        case Type_Integer:
                return "Type_Integer";
        case Type_Float:
                return "Type_Float";
        case Type_String:
                return "Type_String";
        case Type_Array:
                return "Type_Array";
        case Type_Map:
                return "Type_Map";
        default:{
                std::stringstream sstr;
                sstr << "unknown(" << (int)e << ")";
                return sstr.str();
        }
        }
}
inline std::ostream& operator<<(std::ostream& ostr, Type e){
        return ostr << Type_to_string(e);
}

enum VisitorCtrl{
        VisitorCtrl_Skip,
        VisitorCtrl_Decend,
        VisitorCtrl_Nop,
};


struct JsonObject{
        using array_type = std::vector<JsonObject>;
        using map_type = std::map<JsonObject, JsonObject>;

        /*
                The point of these are to allow construction of the
                form
                        auto arr = Tag_Array(1,2,3)
         */
        using Tag_Nil     = std::integral_constant<Type, Type_Nil>;
        using Tag_Bool    = std::integral_constant<Type, Type_Bool>;
        using Tag_Integer = std::integral_constant<Type, Type_Integer>;
        using Tag_Float   = std::integral_constant<Type, Type_Float>;
        using Tag_String  = std::integral_constant<Type, Type_String>;
        using Tag_Array   = std::integral_constant<Type, Type_Array>;
        using Tag_Map     = std::integral_constant<Type, Type_Map>;

        enum IteratorType{
                IteratorType_Array,
                IteratorType_Map,
        };
        template<bool is_constant>
        struct basic_iterator : std::iterator<std::forward_iterator_tag, JsonObject>{
                using IterTag_Array = std::integral_constant<IteratorType, IteratorType_Array>;
                using IterTag_Map   = std::integral_constant<IteratorType, IteratorType_Map>;
                using array_iter_type = tt::conditional_t<is_constant,
                      typename array_type::const_iterator,
                      typename array_type::iterator
                >;
                using map_iter_type = tt::conditional_t<is_constant,
                      typename map_type::const_iterator,
                      typename map_type::iterator
                >;
                using ptr_type = tt::conditional_t<
                        is_constant,
                        JsonObject const*,
                        JsonObject *>;
                using ref_type = tt::conditional_t<
                        is_constant,
                        JsonObject const&,
                        JsonObject&>;

                friend struct basic_iterator<true>;

                template<class Iter>
                basic_iterator(Iter const& that)
                        :iter_type_(that.iter_type_)
                {
                        if( iter_type_ == IteratorType_Map ){
                                map_iter_ = that.map_iter_;
                        } else {
                                array_iter_ = that.array_iter_;
                        }
                }

                basic_iterator(IterTag_Array, array_iter_type iter){
                        iter_type_ = IteratorType_Array;
                        array_iter_ = iter;
                }
                basic_iterator(IterTag_Map, map_iter_type iter){
                        iter_type_ = IteratorType_Map;
                        map_iter_ = iter;
                }
                ~basic_iterator(){
                        if( iter_type_ == IteratorType_Map )
                                map_iter_.~map_iter_type();
                        else 
                                array_iter_.~array_iter_type();
                }
                basic_iterator& operator++(){
                        if( iter_type_ == IteratorType_Map )
                                ++map_iter_;
                        else 
                                ++array_iter_;
                        return *this;
                }
                bool operator==(basic_iterator const& that)const{
                        if( this->iter_type_ != that.iter_type_ )
                                return false;
                        if( iter_type_ == IteratorType_Map )
                                return this->map_iter_ == that.map_iter_;
                        else 
                                return this->array_iter_ == that.array_iter_;
                }
                bool operator!=(basic_iterator const& that)const{
                        return ! ( *this == that);
                }
                ptr_type operator->(){
                        if( iter_type_ == IteratorType_Map ){
                                // we have to cast because in std::map you
                                // can't modify a pointer to a map value,
                                // but we want to treat maps and array the same,
                                // so we just cast away and kindly ask people
                                // to not modify the map values
                                return const_cast<ptr_type>(&map_iter_->first); // return key
                        } else  {
                                return &*array_iter_;
                        }
                }
                ref_type operator*(){
                        return key();
                }
                ptr_type map_value__(){
                        if( iter_type_ == IteratorType_Map ){
                                return &map_iter_->second;
                        } else {
                                return &*array_iter_;
                        }
                }
                ref_type value(){
                        return *map_value__();
                }
                ref_type key(){
                        if( iter_type_ == IteratorType_Map ){
                                return *const_cast<ptr_type>(&map_iter_->first); // return key
                        } else {
                                return *array_iter_;
                        }
                }
        private:
                IteratorType iter_type_;
                union{
                        array_iter_type array_iter_;
                        map_iter_type map_iter_;
                };
        };

        using iterator       = basic_iterator<false>;
        using const_iterator = basic_iterator<true>;

        void DoAssign(Tag_Nil){
                type_ = Type_Nil;
        }
        void DoAssign(Tag_Bool, bool val = false){
                type_ = Type_Bool;
                new (&as_bool_) bool(val);
        }
        void DoAssign(Tag_Integer, std::int64_t val = 0){
                type_ = Type_Integer;
                new (&as_int_) std::int64_t{val};
        }
        void DoAssign(Tag_Float, double val = .0){
                type_ = Type_Float;
                new (&as_float_) double{val};
        }
        void DoAssign(Tag_String){
                DoAssign(Tag_String{}, "");
        }
        template<class Arg>
        void DoAssign(Tag_String, Arg&& arg){
                type_ = Type_String;
                new (&as_string_) std::string{arg};
        }
        void DoAssign(Tag_Array){
                type_ = Type_Array;
                new (&as_array_) array_type{};
        }
        template<class ArrayTypeParam>
        void DoAssign(Tag_Array, ArrayTypeParam&& val){
                type_ = Type_Array;
                new (&as_array_) array_type{std::forward<ArrayTypeParam>(val)};
        }
        void DoAssign(Tag_Map){
                type_ = Type_Map;
                new (&as_map_) map_type{};
        }
        template<class MapTypeParam>
        void DoAssign(Tag_Map, MapTypeParam&& val){
                type_ = Type_Map;
                new (&as_map_) map_type{std::forward<MapTypeParam>(val)};
        }

        #if 0
        template<class Value>
        void AssignImpl( Detail::precedence_device<0>, Value&& val)
        {
                std::cout << "val = " << boost::typeindex::type_id_with_cvr<Value>() << "\n";
                std::cout << "val = " << boost::typeindex::type_id_with_cvr< tt::remove_cv_t<tt::decay_t<Value> > >() << "\n";
                throw std::domain_error("not a known type");
        }
        #endif
        template<class Value>
        tt::enable_if_t< std::is_same<tt::decay_t<Value>, Tag_Array >::value > 
        AssignImpl( Detail::precedence_device<3>, Value&& val){
                DoAssign(Tag_Array{});
        }
        template<class Value>
        tt::enable_if_t< std::is_same<tt::remove_cv_t<tt::decay_t<Value> >, Tag_Map >::value > 
        AssignImpl( Detail::precedence_device<4>, Value&& val){
                DoAssign(Tag_Map{});
        }
        template<class Value>
        tt::enable_if_t< ! std::is_same<decltype( std::declval<Value>().ToJsonObject() ), void >::value >
        AssignImpl( Detail::precedence_device<5>, Value&& val){
                // XXX is this right?
                Assign(val.ToJsonObject());
        }
        template<class Value>
        tt::enable_if_t< std::is_constructible<std::string,Value>::value >
        AssignImpl( Detail::precedence_device<6>, Value&& val){
                DoAssign( Tag_String{}, std::forward<Value>(val));
        }
        template<class Value>
        tt::enable_if_t< std::is_same<tt::decay_t<Value>, std::string >::value >
        AssignImpl( Detail::precedence_device<7>, Value&& val){
                DoAssign( Tag_String{}, std::forward<Value>(val) );
        }
        template<class Value>
        tt::enable_if_t< std::is_floating_point<tt::decay_t<Value> >::value >
        AssignImpl( Detail::precedence_device<8>, Value&& val){
                DoAssign( Tag_Float{}, static_cast<double>(val) );
        }
        template<class Value>
        tt::enable_if_t< std::is_integral<tt::decay_t<Value> >::value >
        AssignImpl( Detail::precedence_device<9>, Value&& val){
                DoAssign( Tag_Integer{}, std::forward<Value>(val) );
        }
        template<class Value>
        tt::enable_if_t< std::is_same<typename tt::decay_t<Value>::IAmAnEmptyArray, int>::value >
        AssignImpl( Detail::precedence_device<13>, Value&& val){
                DoAssign( Tag_Array{} );
        }
        template<class Value>
        tt::enable_if_t< std::is_same<typename tt::decay_t<Value>::IAmAnEmptyMap, int>::value >
        AssignImpl( Detail::precedence_device<14>, Value&& val){
                DoAssign( Tag_Map{} );
        }
        template<class Value>
        tt::enable_if_t< std::is_same<tt::decay_t<Value>, bool>::value >
        AssignImpl( Detail::precedence_device<15>, Value&& val){
                DoAssign( Tag_Bool{}, std::forward<Value>(val));
        }
        template<class Value>
        tt::enable_if_t< ! std::is_same<JsonObject, tt::remove_cv_t<tt::decay_t<Value> > >::value >
        Assign(Value&& val){
                AssignImpl( Detail::precedence_device<15>{}, std::forward<Value>(val) );
        }
        template<class Value>
        tt::enable_if_t< std::is_same<JsonObject, tt::remove_cv_t<tt::decay_t<Value> > >::value >
        Assign(Value&& that)
        {
                type_ = that.type_;
                switch(type_){
                case Type_Nil:
                        DoAssign(Tag_Nil{});
                        break;
                case Type_Bool:
                        new (&as_bool_) bool(that.as_bool_);
                        break;
                case Type_Integer:
                        new (&as_int_) std::int64_t(that.as_int_);
                        break;
                case Type_Float:
                        new (&as_float_) double(that.as_float_);
                        break;
                case Type_String:
                        new (&as_string_) std::string{that.as_string_};
                        break;
                case Type_Array:
                        if( std::is_rvalue_reference<Value>::value ){
                                new (&as_array_) array_type(std::move(that.as_array_));
                        } else{
                                new (&as_array_) array_type(that.as_array_);
                        }
                        break;
                case Type_Map:
                        if( std::is_rvalue_reference<Value>::value ){
                                new (&as_map_) map_type(std::move(that.as_map_));
                        } else{
                                new (&as_map_) map_type(that.as_map_);
                        }
                        break;
                }
        }

        JsonObject(){
                /*
                        By default we are a map
                 */
                DoAssign(Tag_Map{});
        }
        JsonObject(JsonObject const& that){
                Assign(that);
        }
        JsonObject(JsonObject&& that){
                Assign(that);
        }
        template<class Arg>
        JsonObject(Arg&& arg)
        {
                Assign(std::forward<Arg>(arg));
        }
        ~JsonObject(){
                using std::string;
                switch(type_){
                case Type_String:
                        as_string_.~string();
                        break;
                case Type_Array:
                        as_array_.~array_type();
                        break;
                case Type_Map:
                        as_map_.~map_type();
                        break;
                }
        }
        
        template<class Value>
        JsonObject& operator=(Value&& value){
                Assign(std::forward<Value>(value));
                return *this;
        }

        [[noreturn]]
        void ThrowCastError_(std::string const& msg)const{
                std::stringstream sstr;
                sstr << msg << "(" << Type() << ", " << ToString();
                throw std::domain_error(sstr.str());
        }
        std::int64_t AsInteger()const{
                switch(type_){
                case Type_Integer:
                        return as_int_;
                case Type_String:
                        {
                                std::stringstream sstr;
                                sstr << as_string_;
                                std::int64_t result;
                                sstr >> result;
                                if( sstr.eof() && sstr ){
                                        return result;
                                }
                                ThrowCastError_("bad cast");
                        }
                default:
                        ThrowCastError_("not an integer");
                }
        }
        double AsFloat()const{
                switch(type_){
                case Type_Float:
                        return as_float_;
                case Type_Integer:
                        return static_cast<double>(as_int_);
                case Type_String:
                        {
                                std::stringstream sstr;
                                sstr << as_string_;
                                double result;
                                sstr >> result;
                                if( sstr.eof() && sstr ){
                                        return result;
                                }
                                throw std::domain_error("bad cast");
                        }
                default:
                ThrowCastError_("not an float");
                }
        }
        bool AsBool()const{
                switch(type_){
                case Type_Bool:
                        return as_bool_;
                case Type_Integer:
                        return as_int_ != static_cast<decltype(as_int_)>(0);
                default:
                        ThrowCastError_("not an bool");
                }
        }
        std::string AsString()const{
                #if 0
                auto caster = [](auto prim){
                        std::string str;
                        std::stringstream sstr;
                        sstr << std::fixed;
                        sstr << prim;
                        sstr >> str;
                        return str;
                };
                #endif
                switch(type_){
                case Type_String:
                        return as_string_;
                case Type_Float:
                        return boost::lexical_cast<std::string>(as_float_);
                case Type_Integer:
                        return boost::lexical_cast<std::string>(as_int_);
                case Type_Bool:
                        return ( as_bool_ ? "True" : "False" );
                // these are errors, idea is that these could be slow options,
                // and these should be done with ToString()
                case Type_Map:
                        ThrowCastError_("Use ToString() to convert Map to string");
                case Type_Array:
                        ThrowCastError_("Use ToString() to convert List to string");
                default:
                        ThrowCastError_("unhandles");
                }
        }
        template<class Value>
        void push_back(Value&& val){
                if( type_ != Type_Array )
                        ThrowCastError_("not a array");
                this->push_back_unchecked(std::forward<Value>(val));
        }
        template<class Value>
        void push_back_unchecked(Value&& val){
                as_array_.emplace_back( val );
        }
        template<class Key, class Value>
        void emplace(Key&& key, Value&& val){
                if( type_ != Type_Map )
                        ThrowCastError_("not a map");
                this->emplace_unchecked(std::forward<Key>(key), std::forward<Value>(val));
        }
        template<class Key, class Value>
        void emplace_unchecked(Key&& key, Value&& val){
                as_map_.emplace(std::forward<Key>(key), std::forward<Value>(val));
        }
        size_t size()const{
                switch(type_){
                case Type_Array:
                        return as_array_.size();
                case Type_Map:
                        return as_map_.size();
                default:
                        ThrowCastError_("not sizeable");
                }
        }

private:
        struct HasKeyPolicy{
                using return_type = bool;
                // this is called then the key isn't found
                void MaybeThrow(std::string const& msg){
                }
                // then the key is found
                bool Return( JsonObject const* ptr){
                        return true;
                }
                bool ReturnDefaultKey( JsonObject const* ptr, JsonObject const& key){
                        return false;
                }
        };
        struct ConstLookupPolicy{
                using return_type = JsonObject const*;
                void MaybeThrow(std::string const& msg){
                        throw std::domain_error(msg);
                }
                JsonObject const* Return( JsonObject const* ptr){
                        return ptr;
                }
                JsonObject const* ReturnDefaultKey( JsonObject const* ptr, JsonObject const& key){
                        __builtin_unreachable();
                }
        };
        struct MutableLookupPolicy{
                using return_type = JsonObject const*;
                void MaybeThrow(std::string const& msg){
                }
                JsonObject const* Return( JsonObject const* ptr){
                        return ptr;
                }
                JsonObject const* ReturnDefaultKey( JsonObject const* ptr, JsonObject const& key){
                        JsonObject* self = const_cast<JsonObject*>(ptr);
                        switch(ptr->GetType()){
                        case Type_Map:
                                return &self->as_map_[key];
                        default:
                                throw std::domain_error("not sure what to do");
                        } 
                }
        };
        friend struct LookupPolicy;
public:

        template<class Policy, class Key>
        typename tt::decay_t<Policy>::return_type
        ExecuteLookup_(Detail::precedence_device<0>&&, Policy&& p, Key key)const{
                if( type_ == Type_Map ){
                        JsonObject casted{key};
                        auto iter = as_map_.find(casted);
                        if( iter != as_map_.end() ){
                                return p.Return(&iter->second);
                        }
                        p.MaybeThrow( "don't have key");
                        return p.ReturnDefaultKey( this, casted);
                } else {
                        ThrowCastError_("not a map");
                        __builtin_unreachable();
                }
        }
        template<class Policy, class Key>
        tt::enable_if_t< std::is_integral<tt::decay_t<Key> >::value, typename tt::decay_t<Policy>::return_type >
        ExecuteLookup_(Detail::precedence_device<1>&&, Policy&& p, Key key)const{
                if( type_ ==  Type_Array ){
                        auto idx = static_cast<typename array_type::size_type>(key);
                        if(  0 <= key && idx < as_array_.size() ){
                                return p.Return( &as_array_.at(idx) );
                        }
                        p.MaybeThrow( "out of range");
                        return p.ReturnDefaultKey( this, idx);
                } else if( type_ == Type_Map ){
                        std::int64_t casted = static_cast<std::int64_t>(key);
                        JsonObject tmp(casted);
                        // unlike the non-const version, we don't create keys
                        // on demand
                        auto iter = as_map_.find(tmp);
                        if( iter != as_map_.end() ){
                                return p.Return(&iter->second);
                        }
                        p.MaybeThrow( "don't have key");
                        return p.ReturnDefaultKey( this, casted);
                } else {
                        ThrowCastError_("not a map or array");
                        __builtin_unreachable();
                }
        }
        template<class Policy, class Key>
        tt::enable_if_t< std::is_same<tt::decay_t<Key>, bool >::value, typename tt::decay_t<Policy>::return_type >
        ExecuteLookup_(Detail::precedence_device<2>&&, Policy&& p, Key key)const{
                return ExecuteLookup_( Detail::precedence_device<0>{}, p, std::forward<Key>(key));
        }

        template<class Key>
        JsonObject const& operator[](Key&& key)const{
                auto ret =  ExecuteLookup_( Detail::precedence_device<2>{}, ConstLookupPolicy{}, key);
                return *ret;
        }
        template<class Key>
        JsonObject& operator[](Key&& key){
                auto ret = ExecuteLookup_( Detail::precedence_device<2>{},  MutableLookupPolicy{}, key);
                return *const_cast<JsonObject*>(ret);
        }
        template<class Key>
        bool HasKey(Key&& key)const{
                auto ret = ExecuteLookup_( Detail::precedence_device<2>{},  HasKeyPolicy{}, key);
                return ret;
        }




        Type GetType()const{ return type_; }
        std::string to_string()const{
                std::stringstream sstr;
                switch(type_){
                case Type_Nil:
                        sstr << "{}";
                        break;
                case Type_Bool:
                        sstr << as_bool_;
                        break;
                case Type_Integer:
                        sstr << as_int_;
                        break;
                case Type_Float:
                        sstr << as_float_;
                        break;
                case Type_String:
                        sstr << as_string_;
                        break;
                case Type_Array:
                        break;
                case Type_Map:
                        break;
                }
                return sstr.str();
        }
        bool operator<(JsonObject const& that)const{
                if( this->GetType() != that.GetType() )
                        return  this->GetType() < that.GetType() ;
                switch(type_){
                case Type_Nil:
                        return false;
                case Type_Bool:
                        return this->as_bool_ < that.as_bool_;
                case Type_Integer:
                        return this->as_int_ < that.as_int_;
                case Type_Float:
                        return this->as_float_ < that.as_float_;
                case Type_String:
                        return this->as_string_ < that.as_string_;
                case Type_Array:
                case Type_Map:
                        // we don't compare aggregates
                        break;
                }
                return false;

        }
        bool operator==(JsonObject const& that)const{
                return ! ( ( *this < that ) || ( that < *this ) );
        }
        #if 0
        template<class LeftParam>
        friend
        bool operator==(LeftParam&& lp, JsonObject const& rp){
                return rp == lp;
        }
        template<class LeftParam>
        friend
        bool operator!=(LeftParam&& lp, JsonObject const& rp){
                return rp != lp;
        }
        #endif
        bool operator!=(JsonObject const& that)const{
                return ! ( *this == that);
        }
        // way to communitate weather to
        // go into every sub or not
        struct visitor{
                virtual void on_nil(){}
                virtual void on_bool(bool value){}
                virtual void on_integer(std::int64_t value){}
                virtual void on_float(double value){}
                virtual void on_string(std::string const& value){}
                virtual VisitorCtrl begin_array(size_t n){ return VisitorCtrl_Decend; }
                virtual void end_array(){ }
                virtual VisitorCtrl begin_map(size_t n){ return VisitorCtrl_Decend; }
                virtual void end_map(){ }
        };


        bool IsPrimitive()const{ 
                return Begin_Primitive <= GetType() && 
                        GetType() < End_Primitive;
        }
        bool IsAggregate()const{ 
                return Begin_Aggregate <= GetType() && 
                        GetType() < End_Aggregate;
        }
        VisitorCtrl AcceptNonRecursive(visitor& v)const{
                switch(this->GetType()){
                case Type_Nil:
                        v.on_nil();
                        return VisitorCtrl_Nop;
                case Type_Bool:
                        v.on_bool(as_bool_);
                        return VisitorCtrl_Nop;
                case Type_Integer:
                        v.on_integer(as_int_);
                        return VisitorCtrl_Nop;
                case Type_Float:
                        v.on_float(as_float_);
                        return VisitorCtrl_Nop;
                case Type_String:
                        v.on_string(as_string_);
                        return VisitorCtrl_Nop;
                case Type_Array:
                        return v.begin_array( this->size() );
                case Type_Map:
                        return v.begin_map( this->size() );
                default:
                        __builtin_unreachable();
                }
        }

        void Accept(visitor& v)const{

                auto ctrl = AcceptNonRecursive(v);

                switch(ctrl){
                case VisitorCtrl_Nop:
                case VisitorCtrl_Skip:
                        return;
                case VisitorCtrl_Decend:
                        break;
                }


                struct StackFrame{
                        Type type;
                        const_iterator iter, end;
                        JsonObject const* map_obj_;
                };

                StackFrame start = { this->GetType(), this->begin(), this->end(), this};
                std::vector< StackFrame> stack{std::move(start)};
                for(; stack.size(); ){
                        for(; stack.back().iter != stack.back().end;){

                                auto& iter = stack.back().iter;

                                auto next_ctrl = iter->AcceptNonRecursive(v);

                                // in the case of a map, we need to first
                                // process the key, now we need to do
                                // the value
                                JsonObject const* mapValue = 0;
                                if( stack.back().type == Type_Map ){
                                        mapValue = iter.map_value__();
                                        next_ctrl = mapValue->AcceptNonRecursive(v);
                                }

                                switch(next_ctrl){
                                case VisitorCtrl_Nop:
                                case VisitorCtrl_Skip:
                                        ++iter;
                                        break;
                                // note that we we decend, we don't increment stack.back().iter,
                                // this is why we recursivly pop at the end
                                case VisitorCtrl_Decend:
                                        if( mapValue ){
                                                StackFrame start = { mapValue->GetType(), mapValue->begin(), mapValue->end(), mapValue };
                                                stack.push_back( std::move(start) );
                                        } else{
                                                StackFrame start = { iter->GetType(), iter->begin(), iter->end(), &*iter };
                                                stack.push_back( std::move(start) );
                                        }
                                }
                        }

                        // now we need to pop the start, and increment
                        // the iterator for every stack, and then apply
                        // recursivly
                        for(;stack.size() && stack.back().iter == stack.back().end;){
                                if( stack.back().type == Type_Array )
                                        v.end_array();
                                else
                                        v.end_map();
                                stack.pop_back();

                                // if the stack isn't empty, we need to increment the next one
                                if( stack.empty() )
                                        break;
                                assert( stack.size() && "expected");
                                assert( stack.back().iter != stack.back().end && "unexpcted");
                                ++stack.back().iter;
                        }
                }
        }
        // multiline pretty
        void Display(std::ostream& ostr = std::cout, unsigned indent = 0)const;
        // single line
        std::string ToString()const;
        std::string ToDebugString()const;

        friend std::ostream& operator<<(std::ostream& ostr, JsonObject const& self){
                return ostr << self.ToString();
        }
        void Debug()const;
        #if 0
        void Debug()const{
                std::cout << "{type=" << Type_to_string(type_) 
                        << ", <data>=" << to_string() << "}\n";
        }
        #endif


        const_iterator begin()const{
                switch(type_){
                case Type_Array:
                        return const_iterator(const_iterator::IterTag_Array{}, as_array_.begin() );
                case Type_Map:
                        return const_iterator(const_iterator::IterTag_Map{}, as_map_.begin() );
                default:
                        throw std::domain_error("not a map or array");
                }
        }
        const_iterator end()const{
                switch(type_){
                case Type_Array:
                        return const_iterator(const_iterator::IterTag_Array{}, as_array_.end() );
                case Type_Map:
                        return const_iterator(const_iterator::IterTag_Map{}, as_map_.end() );
                default:
                        throw std::domain_error("not a map or array");
                }
        }
        iterator begin(){
                switch(type_){
                case Type_Array:
                        return iterator(iterator::IterTag_Array{}, as_array_.begin() );
                case Type_Map:
                        return iterator(iterator::IterTag_Map{}, as_map_.begin() );
                default:
                        throw std::domain_error("not a map or array");
                }
        }
        iterator end(){
                switch(type_){
                case Type_Array:
                        return iterator(iterator::IterTag_Array{}, as_array_.end() );
                case Type_Map:
                        return iterator(iterator::IterTag_Map{}, as_map_.end() );
                default:
                        throw std::domain_error("not a map or array");
                }
        }

        void Parse(std::string const& s);
private:
        Type type_;
        union {
                bool as_bool_;
                std::int64_t as_int_;
                double as_float_;
                std::string as_string_;
                array_type as_array_;
                map_type as_map_;
        };
};

namespace Detail{
        struct ArrayType{
                template<class... Args>
                        JsonObject operator()(Args&&... args)const{
                                JsonObject obj(JsonObject::Tag_Array{});
                                int aux[] = {0, ( obj.push_back_unchecked( args), 0 )... };
                                return obj;
                        }
                using IAmAnEmptyArray = int;
        };
        struct MapType{
                struct Impl{
                        Impl(){
                                vec_.reserve(128);
                        }
                        template<class Key, class Value>
                                Impl operator()(Key&& key, Value&& value){
                                        vec_.emplace_back(std::forward<Key>(key));
                                        vec_.emplace_back(std::forward<Value>(value));
                                        return *this;
                                }
                        operator JsonObject const()const{
                                return ToJsonObject();
                        }
                        JsonObject ToJsonObject()const{
                                JsonObject obj(JsonObject::Tag_Map{});
                                for(size_t idx =0;idx < vec_.size();idx += 2 ){
                                        obj.emplace_unchecked( std::move( vec_[idx+0] ),
                                                               std::move( vec_[idx+1] ) );
                                }
                                return obj;
                        }
                        Impl& Debug(){
                                std::cerr << "{";
                                const char* comma = "";
                                for(auto const& _ : vec_){
                                        std::cerr << comma << _.ToString();
                                        comma = ", ";
                                }
                                std::cerr << "}\n";
                                return *this;
                        }
                private:
                        std::vector< JsonObject> vec_;
                };
                template<class Key, class Value>
                        Impl operator()(Key&& key, Value&& value)const{
                                Impl impl;
                                return impl(std::forward<Key>(key), std::forward<Value>(value));
                        }
                operator JsonObject const()const{
                        JsonObject obj(JsonObject::Tag_Map{});
                        return obj;
                }
                using IAmAnEmptyMap = int;
        };
} // Defailt
// theese are per translation unit
namespace{
        Detail::ArrayType Array = {};
        Detail::MapType Map = {};
}



} // gjson

#endif // JSON_PARSER_JSONOBJECT_H
