#include <gtest/gtest.h>
#include <unordered_map>

#include "json_parser.h"

#include <boost/type_index.hpp>
#include <boost/optional.hpp>

using namespace json_parser;

static std::string json_sample_text = R"(
{
  "firstName": "John",
  "lastName": "Smith",
  "age": 25,
  "address": {
    "streetAddress": "21 2nd Street",
    "city": "New York",
    "state": "NY",
    "postalCode": "10021"
  },
  "phoneNumber": [
    {
      "type": "home",
      "number": "212 555-1234"
    },
    {
      "type": "fax",
      "number": "646 555-4567"
    }
  ],
  "gender": {
    "type": "male"
  }
}
)";

struct Parser : testing::Test{
protected:
        void SetUp()final{
        }
        void TearDown()final{
        }
        std::vector<std::string> valid_strings = {
                "{}",
                "[]",
                "   {  }    ",
                "  [ ]   ",

                "{1:{}}",
                "[[]]",
                "[{}]",
                "{1:[]}",
                " { 1:{ } } ",
                " [ [ ] ] ",
                " [ { } ] ",
                " { 1:[ ] } ",

                "[1]",
                "[1,2]",
                "[{},1]",
                "[1,{}]",
                "{1:1,1:1}",
                "{1:1,1:{}}",
                "{1:{},1:{}}",
                "{1:[],1:[]}",
                "{1:[],1:[1,2]}",
                
                R"([23])",
                R"([+23])",
                R"([-3455])",
                R"([23.433])",
                R"(["hello"])",
                R"([null])",
                R"([true])",
                R"([false])",
                R"([23,23])",
                R"([-3455,-3455])",
                R"([23.433,23.433])",
                R"(["hello","hello"])",
                R"([null,null])",
                R"([true,true])",
                R"([false,false])",
                R"([23])",
                R"([23,-3455])",
                R"([23,-3455,23.433])",
                R"([23,-3455,23.433,"hello"])",
                R"([23,-3455,23.433,"hello",null])",
                R"([23,-3455,23.433,"hello",null,true])",
                R"([23,-3455,23.433,"hello",null,true,false])",
                
                R"( [ 23 ] )" , 
                R"( [ +23 ] )" , 
                R"( [ -3455 ] )" , 
                R"( [ 23.433 ] )" , 
                R"( [ "hello" ] )" , 
                R"( [ null ] )" , 
                R"( [ true ] )" , 
                R"( [ false ] )" , 
                R"( [ 23 , 23 ] )" , 
                R"( [ -3455 , -3455 ] )" , 
                R"( [ 23.433 , 23.433 ] )" , 
                R"( [ "hello" , "hello" ] )" , 
                R"( [ null , null ] )" , 
                R"( [ true , true ] )" , 
                R"( [ false , false ] )" , 
                R"( [ 23 ] )" , 
                R"( [ 23 , -3455 ] )" , 
                R"( [ 23 , -3455 , 23.433 ] )" , 
                R"( [ 23 , -3455 , 23.433 , "hello" ] )" , 
                R"( [ 23 , -3455 , 23.433 , "hello" , null ] )" , 
                R"( [ 23 , -3455 , 23.433 , "hello" , null , true ] )" , 
                R"( [ 23 , -3455 , 23.433 , "hello" , null , true , false ] )" , 
                
                R"( ["|","|","|","|","|","|","|","|"] )" , 
                R"( ["|","|","_","_","_","_","|","|"] )" , 
                R"( ["|","|","|","|","_","_","_","_"] )" , 
                R"( ["_","|","|","|","_","_","_","_"] )" , 
                R"( ["_","|","a","|","_","_","_","a"] )" , 

                "{}"
        };
        std::vector<std::string> invalid_strings = {
                "{",
                "[",
                "[[]",
                "[][",
                R"( [ 23 -3455 ] )" , 
                R"( [ 23a ] )" , 
        };
};

#if 0
TEST_F( Parser, valid_strings){
        for( auto const& str : valid_strings ){
                EXPECT_TRUE( !!try_parse(str) );
                EXPECT_NO_THROW( parse(str) );
        }
}

TEST_F( Parser, invalid_strings){
        EXPECT_FALSE( try_parse("{") );
        EXPECT_FALSE( try_parse("{}{{") );
        EXPECT_FALSE( try_parse("{} {{") );
        EXPECT_FALSE( try_parse("{}{") );
        EXPECT_FALSE( try_parse("{} {") );
        EXPECT_FALSE( try_parse("{} { ") );
        EXPECT_FALSE( try_parse("{}}") );
        EXPECT_FALSE( try_parse("{} 4") );
        EXPECT_FALSE( try_parse("{} a ") );
        for( auto const& str : invalid_strings ){
                EXPECT_FALSE( try_parse(str) );
                EXPECT_ANY_THROW( parse(str) );
        }
}
#if 0
TEST_F( Parser, to_string ){
        for( auto const& str : valid_strings ){
                auto s = to_string( parse(str) );
                std::cout << s << "\n";
        }
}
TEST_F( Parser, display ){
        for( auto const& str : valid_strings ){
                display( parse(str) );
        }
}
#endif

TEST_F( Parser, try_cast ){
        auto root = parse("[1,2,3]");
        auto arr = try_cast<array_>(root);
        EXPECT_EQ( 3, arr.size());
        EXPECT_EQ( 1, try_cast<std::int64_t>( arr[0] ));
        EXPECT_EQ( 2, try_cast<std::int64_t>( arr[1] ));
        EXPECT_EQ( 3, try_cast<std::int64_t>( arr[2] ));
}


#if 0
TEST_F( Parser, from_wiki ){
        auto root = parse(json_sample_text);
        display(root);
}
#endif

TEST(tokenizer, _){
        tokenizer tok(json_sample_text);
        for(auto iter=tok.token_begin(), end=tok.token_end();iter!=end;++iter){
        }

}
TEST(tokenizer, hello_world){
        tokenizer tok("hello world");
        unsigned n = 0;
        auto iter=tok.token_begin(), end=tok.token_end();
        EXPECT_NE( iter, end);
        EXPECT_EQ(token_type::string_,  iter->type());
        ++iter;
        EXPECT_EQ(token_type::string_,  iter->type());
        ++iter;
        EXPECT_EQ( iter, end);
}

TEST(tokenizer, eos){
        EXPECT_TRUE(tokenizer("").eos());
        EXPECT_TRUE(tokenizer("   ").eos());
        EXPECT_FALSE(tokenizer("and").eos());
}
TEST(tokenizer, one){
        tokenizer tok("  1");
        EXPECT_FALSE(tok.eos());
        tok.next();
        EXPECT_TRUE(tok.eos());
}

#endif


template<unsigned Order>
struct precedence_device : precedence_device<Order-1>{};
template<>
struct precedence_device<0>{};

struct JsonObject{
        using array_type = std::vector<JsonObject>;
        using map_type = std::map<JsonObject, JsonObject>;

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

        template<bool is_constant>
        struct basic_iterator{
                enum IteratorType{
                        IteratorType_Array,
                        IteratorType_Map,
                };
                using IterTag_Array = std::integral_constant<IteratorType, IteratorType_Array>;
                using IterTag_Map   = std::integral_constant<IteratorType, IteratorType_Map>;
                using array_iter_type = std::conditional_t<is_constant,
                      typename array_type::const_iterator,
                      typename array_type::iterator
                >;
                using map_iter_type = std::conditional_t<is_constant,
                      typename map_type::const_iterator,
                      typename map_type::iterator
                >;
                using ptr_type = std::conditional_t<
                        is_constant,
                        JsonObject const*,
                        JsonObject *>;
                using ref_type = std::conditional_t<
                        is_constant,
                        JsonObject const&,
                        JsonObject&>;

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

        template<class Value>
        void AssignImpl( precedence_device<0>, Value&& val)
        {
                std::cout << "val = " << boost::typeindex::type_id_with_cvr<Value>() << "\n";
                std::cout << "val = " << boost::typeindex::type_id_with_cvr< std::remove_cv_t<std::decay_t<Value> > >() << "\n";
                throw std::domain_error("not a known type");
        }
        template<class Value>
        std::enable_if_t< std::is_same<std::decay_t<Value>, Tag_Array >::value > 
        AssignImpl( precedence_device<3>, Value&& val){
                DoAssign(Tag_Array{});
        }
        template<class Value>
        std::enable_if_t< std::is_same<std::remove_cv_t<std::decay_t<Value> >, Tag_Map >::value > 
        AssignImpl( precedence_device<4>, Value&& val){
                DoAssign(Tag_Map{});
        }
        template<class Value>
        std::enable_if_t< ! std::is_same<decltype( std::declval<Value>().ToJsonObject() ), void >::value >
        AssignImpl( precedence_device<5>, Value&& val){
                // XXX is this right?
                Assign(val.ToJsonObject());
        }
        template<class Value>
        std::enable_if_t< std::is_constructible<std::string,Value>::value >
        AssignImpl( precedence_device<6>, Value&& val){
                DoAssign( Tag_String{}, std::forward<Value>(val));
        }
        template<class Value>
        std::enable_if_t< std::is_same<std::decay_t<Value>, std::string >::value >
        AssignImpl( precedence_device<7>, Value&& val){
                DoAssign( Tag_String{}, std::forward<Value>(val) );
        }
        template<class Value>
        std::enable_if_t< std::is_floating_point<std::decay_t<Value> >::value >
        AssignImpl( precedence_device<8>, Value&& val){
                DoAssign( Tag_Float{}, static_cast<double>(val) );
        }
        template<class Value>
        std::enable_if_t< std::is_integral<std::decay_t<Value> >::value >
        AssignImpl( precedence_device<9>, Value&& val){
                DoAssign( Tag_Integer{}, std::forward<Value>(val) );
        }
        template<class Value>
        std::enable_if_t< std::is_same<std::decay_t<Value>, bool>::value >
        AssignImpl( precedence_device<10>, Value&& val){
                DoAssign( Tag_Bool{}, std::forward<Value>(val));
        }
        template<class Value>
        std::enable_if_t< ! std::is_same<JsonObject, std::remove_cv_t<std::decay_t<Value> > >::value >
        Assign(Value&& val){
                AssignImpl( precedence_device<10>{}, std::forward<Value>(val) );
        }
        template<class Value>
        std::enable_if_t< std::is_same<JsonObject, std::remove_cv_t<std::decay_t<Value> > >::value >
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
                DoAssign(Tag_Nil{});
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

        auto AsInteger()const{
                if( type_ == Type_Integer )
                        return as_int_;
                throw std::domain_error("not an integer");
        }
        auto AsFloat()const{
                if( type_ == Type_Float )
                        return as_float_;
                throw std::domain_error("not an float");
        }
        auto AsBool()const{
                if( type_ == Type_Bool )
                        return as_bool_;
                throw std::domain_error("not an bool");
        }
        auto const& AsString()const{
                if( type_ == Type_String )
                        return as_string_;
                throw std::domain_error("not an string");
        }
        template<class Value>
        void push_back(Value&& val){
                if( type_ != Type_Array )
                        throw std::domain_error("not a array");
                this->push_back_unchecked(std::forward<Value>(val));
        }
        template<class Value>
        void push_back_unchecked(Value&& val){
                as_array_.emplace_back( val );
        }
        template<class Key, class Value>
        void emplace(Key&& key, Value&& val){
                if( type_ != Type_Map )
                        throw std::domain_error("not a map");
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
                        throw std::domain_error("not sizeable");
                }
        }


        template<class Key>
        std::enable_if_t< std::is_integral<std::decay_t<Key> >::value, JsonObject& >
        operator[](Key key){
                if( type_ ==  Type_Array ){
                        return as_array_.at(static_cast<typename array_type::size_type>(key));
                } else if( type_ == Type_Map ){
                        std::int64_t casted = static_cast<std::int64_t>(key);
                        JsonObject mapped(casted);
                        return as_map_[mapped];
                } else {
                        throw std::domain_error("not a map or array");
                }
        }
        template<class Key>
        std::enable_if_t< std::is_integral<std::decay_t<Key> >::value, JsonObject const& >
        operator[](Key key)const{
                if( type_ ==  Type_Array ){
                        return as_array_.at(static_cast<typename array_type::size_type>(key));
                } else if( type_ == Type_Map ){
                        std::int64_t casted = static_cast<std::int64_t>(key);
                        JsonObject mapped(casted);
                        // unlike the non-const version, we don't create keys
                        // on demand
                        auto iter = as_map_.find(mapped);
                        if( iter != as_map_.end() )
                                return iter->second;
                        throw std::domain_error("can't find key");
                } else {
                        throw std::domain_error("not a map or array");
                }
        }
        template<class Key>
        std::enable_if_t< ! std::is_integral<std::decay_t<Key> >::value, JsonObject& >
        operator[](Key key){
                if( type_ == Type_Map ){
                        JsonObject tmp{key};
                        return as_map_[tmp];
                } else {
                        throw std::domain_error("not a map or array");
                }
        }
        template<class Key>
        std::enable_if_t< ! std::is_integral<std::decay_t<Key> >::value, JsonObject const& >
        operator[](Key key)const{
                if( type_ == Type_Map ){
                        JsonObject tmp{key};
                        auto iter = as_map_.find(tmp);
                        if( iter != as_map_.end() )
                                return iter->second;
                        throw std::domain_error("can't find key");
                } else {
                        throw std::domain_error("not a map or array");
                }
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
        enum VisitorCtrl{
                VisitorCtrl_Skip,
                VisitorCtrl_Decend,
                VisitorCtrl_Nop,
        };
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
        struct pretty_visitor{
                enum Options{
                        Option_QuoteString = 1,
                        Option_Newlines = 2,
                };
                explicit to_string_visitor(std::ostream* ostr):ostr_{ostr}{}
        private:
                std::ostream* ostr_;
        };
        struct debug_visitor : visitor{
                void on_nil()override{
                        std::cout << make_indent_() << "on_nil()\n";
                }
                void on_bool(bool value)override{
                        std::cout << make_indent_() << "on_bool(" << value << ")\n";
                }
                void on_integer(std::int64_t value)override{
                        std::cout << make_indent_() << "on_integer(" << value << ")\n";
                }
                void on_float(double value)override{
                        std::cout << make_indent_() << "on_float(" << value << ")\n";
                }
                void on_string(std::string const& value)override{
                        std::cout << make_indent_() << "on_string(" << value << ")\n";
                }
                VisitorCtrl begin_array(size_t n)override{
                        std::cout << make_indent_() << "begin_array(" << n << ")\n";
                        ++indent_;
                        return VisitorCtrl_Decend;
                }
                void end_array()override{
                        --indent_;
                        std::cout << make_indent_() << "end_array()\n";
                }
                VisitorCtrl begin_map(size_t n)override{
                        std::cout << make_indent_() << "begin_map(" << n << ")\n";
                        ++indent_;
                        return VisitorCtrl_Decend;
                }
                void end_map()override{
                        --indent_;
                        std::cout << make_indent_() << "end_map()\n";
                }
        private:
                std::string make_indent_()const{
                        return std::string(indent_*4,' ');
                }
                unsigned indent_{0};
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
                };
                StackFrame start = { this->GetType(), this->begin(), this->end()  };
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
                                        if( stack.size() == 1 ){
                                                // now this is a special case, because when treating
                                                // recursive objects we need to reference an iterator
                                                // for the map, except on the first iteration, as we
                                                // don't have an iterator to this, so just put in 
                                                // an 'if' statement here
                                                mapValue = &this->operator[](*iter);
                                        } else{
                                                mapValue = &stack[stack.size()-2].iter->operator[](*iter);
                                        }
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
                                                StackFrame start = { Type_Array, mapValue->begin(), mapValue->end()  };
                                                stack.push_back( std::move(start) );
                                        } else{
                                                StackFrame start = { Type_Array, iter->begin(), iter->end()  };
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
                                assert( stack.back().iter != stack.back().end && "unexpcted");
                                ++stack.back().iter;
                        }
                }
        }
        void Display(std::ostream& ostr, unsigned indent = 0)const{
                #if 0
                auto print_indent = [&](){
                        ostr << std::string(indent*4,' ');
                };
                const char* comma = "";
                switch(type_){
                case Type_Nil:
                        print_indent();
                        ostr << "{}";
                        break;
                case Type_Bool:
                        print_indent();
                        ostr << as_bool_;
                        break;
                case Type_Integer:
                        print_indent();
                        ostr << as_int_;
                        break;
                case Type_Float:
                        print_indent();
                        ostr << as_float_;
                        break;
                case Type_String:
                        print_indent();
                        ostr << as_string_;
                        break;
                case Type_Array:
                        print_indent();
                        ostr << "[\n";
                        comma = ", ";
                        for(auto const& _ : as_array_ ){
                                _.Display(ostr, indent+1);
                                ostr << comma << "\n";
                                comma = ", ";
                        }
                        print_indent();
                        ostr << "]";
                        break;
                case Type_Map:
                        print_indent();
                        ostr << "{\n";
                        comma = ", ";
                        for(auto const& _ : as_map_ ){
                                _.first.Display(ostr, indent+1);
                                _.second.Display(ostr, indent+1);
                                ostr << comma << "\n";
                                comma = ", ";
                        }
                        print_indent();
                        ostr << "}";
                        break;
                }
                #endif
                debug_visitor v;
                this->Accept(v);
        }
        friend std::ostream& operator<<(std::ostream& ostr, JsonObject const& self){
                self.Display(ostr);
                return ostr;
        }
        void Debug()const{
                std::cout << "{type=" << Type_to_string(type_) 
                        << ", <data>=" << to_string() << "}\n";
        }


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

namespace Frontend{
        struct ArrayType{
                template<class... Args>
                JsonObject operator()(Args&&... args)const{
                        JsonObject obj(JsonObject::Tag_Array{});
                        int aux[] = {0, ( obj.push_back_unchecked( args), 0 )... };
                        return std::move(obj);
                }
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
                                return std::move(obj);
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
                        return std::move(obj);
                }
        };

        ArrayType Array = {};
        MapType Map = {};

} // Frontend

namespace Detail{
        struct JsonObjectMaker{

                struct StackFrame{
                        JsonObject object;
                        // only for when we have a map, get need to save the key first
                        std::vector<JsonObject> param_stack_;
                };


                void begin_map(){
                        StackFrame frame;
                        frame.object = JsonObject{JsonObject::Tag_Map{}};
                        stack_.emplace_back(std::move(frame));
                } 
                void end_map(){
                        end_any_();
                }
                void begin_array(){
                        StackFrame frame;
                        frame.object = JsonObject{JsonObject::Tag_Array{}};
                        stack_.emplace_back(std::move(frame));
                } 
                void end_array(){
                        end_any_();
                } 
                void make_string(std::string const& value){
                        add_any_( JsonObject{value});
                }
                void make_int(std::int64_t value){
                        add_any_( JsonObject{value});
                }
                void make_float(long double value){
                        add_any_( JsonObject{value});
                }
                void make_null(){
                        add_any_( JsonObject{JsonObject::Tag_Nil{}});
                }
                void make_true(){
                        add_any_( JsonObject{true} );
                }
                void make_false(){
                        add_any_( JsonObject{false} );
                }
                JsonObject make(){ 
                        JsonObject tmp = std::move(out_.back());
                        out_.pop_back();
                        return std::move(tmp);
                } 
        private:
                void add_any_(JsonObject&& obj){
                        if( stack_.back().object.GetType() == JsonObject::Type_Array ){
                                stack_.back().object.push_back_unchecked( obj );
                        } else if( stack_.back().object.GetType() == JsonObject::Type_Map ){
                                if( stack_.back().param_stack_.empty() ){
                                        // this must be the key, save it because we 
                                        // need to add key/value pair atomically
                                        stack_.back().param_stack_.push_back(obj);
                                } else{
                                        stack_.back().object.emplace_unchecked( 
                                                std::move( stack_.back().param_stack_.back()),
                                                std::move( obj ) );
                                        stack_.back().param_stack_.pop_back();
                                }
                        } else{
                                throw std::domain_error("unexpcted");
                        }
                }
                void end_any_(){
                        if( stack_.back().param_stack_.size() != 0 )
                                throw std::domain_error("not an even number of args");
                        auto last = std::move(stack_.back());
                        stack_.pop_back();
                        if( stack_.empty() ){
                                out_.push_back(std::move(last.object));
                                return;
                        }

                        add_any_( std::move( last.object) );
                } 
                std::vector<StackFrame> stack_;
                std::vector<JsonObject> out_;
        };
}

struct debug_maker{
        debug_maker(){
        }
        void begin_map(){
                std::cout << "begin_map()\n";
        } 
        void end_map(){
                std::cout << "end_map()\n";
        } 
        void begin_array(){
                std::cout << "begin_array()\n";
        } 
        void end_array(){
                std::cout << "end_array()\n";
        } 
        void make_string(std::string const& value){
                std::cout << "make_string()\n";
        }
        void make_int(std::int64_t value){
                std::cout << "make_int()\n";
        }
        void make_float(long double value){
                std::cout << "make_float()\n";
        }
        void make_null(){
                std::cout << "make_null()\n";
        }
        void make_bool(bool value){
                std::cout << "make_bool()\n";
        }
        void make_true(){
                std::cout << "make_true()\n";
        }
        void make_false(){
                std::cout << "make_false()\n";
        }
        int make(){ 
                return 1;
        } 
        void reset(){
        }
};

TEST(JsonObject, Integer){
        JsonObject obj(12);
        EXPECT_EQ( JsonObject::Type_Integer, obj.GetType() );
        EXPECT_EQ( "12", obj.to_string() );
        EXPECT_EQ( 12, obj.AsInteger() );
        obj = 34;
        EXPECT_EQ( JsonObject::Type_Integer, obj.GetType() );
        EXPECT_EQ( "34", obj.to_string() );
        EXPECT_EQ( 34, obj.AsInteger() );
}
TEST(JsonObject, String){
        JsonObject obj("hello");
        EXPECT_EQ( JsonObject::Type_String, obj.GetType() );
        EXPECT_EQ( "hello", obj.AsString() );
        obj = std::string("world");
        EXPECT_EQ( "world", obj.AsString() );
}

TEST(JsonObject, assignment){
        JsonObject obj("hello");
        obj = 23;
        EXPECT_EQ( JsonObject::Type_Integer, obj.GetType() );
        EXPECT_EQ( 23, obj.AsInteger() );
        obj = 12.23;
        EXPECT_EQ( JsonObject::Type_Float, obj.GetType() );
        obj = "hello";
        EXPECT_EQ( JsonObject::Type_String, obj.GetType() );
        EXPECT_EQ( "hello", obj.AsString() );
}
TEST(JsonObject, Array){
        JsonObject arr{JsonObject::Tag_Array{}};
        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);
        std::cout << "arr = " << arr << "\n";
        EXPECT_EQ( 3, arr.size());
        EXPECT_TRUE( arr[0] == 1 );
        EXPECT_TRUE( arr[0] != "Hello");
        //EXPECT_ANY_THROW( arr[4] );
        //EXPECT_ANY_THROW( arr["hello"]);
        arr = JsonObject{JsonObject::Tag_Array{}};
        EXPECT_EQ( 0, arr.size());
        arr.push_back(23.34);
        EXPECT_NEAR(23.34, arr[0].AsFloat(), 0.001 );
}
TEST(JsonObject, Map){
        JsonObject m(JsonObject::Tag_Map{});
        EXPECT_EQ( 0, m.size());
        m.emplace( "one", 1);
        m.emplace( "two", 2);
        EXPECT_EQ( 2, m.size());
        EXPECT_TRUE( m["one"] == 1 );
        EXPECT_TRUE( m["one"] != 2 );
        EXPECT_TRUE( m["two"] == 2 );
        m["two"] = 3;
        EXPECT_TRUE( m["two"] != 2 );
        EXPECT_TRUE( m["two"] == 3 );
        m["two"] = m["one"];
        m = 1;
        //EXPECT_ANY_THROW( m.size() );
        m = JsonObject{JsonObject::Tag_Map{}};
        EXPECT_EQ( 0, m.size());
}

TEST(JsonObject, FrontendArray){
        using namespace Frontend;
        auto arr = Array(2,"hello",false);
        EXPECT_EQ( 3, arr.size());
        EXPECT_TRUE( arr[0] == 2 );
        EXPECT_TRUE( arr[1] == "hello" );
        EXPECT_TRUE( arr[2] == false );

        arr.push_back( Array());
        arr.push_back( true);
        arr.push_back( Array(Array()));

        EXPECT_EQ( 6, arr.size());
        EXPECT_EQ( JsonObject::Type_Array, arr.GetType());
        EXPECT_EQ( JsonObject::Type_Integer, arr[0].GetType());
        EXPECT_EQ( JsonObject::Type_String,  arr[1].GetType());
        EXPECT_EQ( JsonObject::Type_Bool,    arr[2].GetType());
        EXPECT_EQ( JsonObject::Type_Array,   arr[3].GetType());
        EXPECT_EQ( JsonObject::Type_Bool,    arr[4].GetType());
        EXPECT_EQ( JsonObject::Type_Array,   arr[5].GetType());
        EXPECT_EQ( JsonObject::Type_Array,   arr[5][0].GetType());

        arr = Array(45);
        EXPECT_TRUE(  arr[0] == 45 );

        arr = Array(Array("hello"));
        EXPECT_TRUE(  arr[0][0] == "hello" );
        
        arr = Array(Array(Array(12.34)));
        EXPECT_TRUE(  arr[0][0][0] == 12.34 );

}

TEST(JsonObject, FrontendMap){
        using namespace Frontend;

        JsonObject m = Map("one",1)(2, "two");
        EXPECT_EQ( 2, m.size() );
        EXPECT_TRUE( m["one"] == 1 );
        EXPECT_TRUE( m[2] == "two" );
}

TEST(JsonObject, simple){
        using namespace Frontend;
        //JsonObject obj{JsonObject::Tag_Array{}};
        auto obj = Array(1,"hello", 23.25, Array(1,2,3), false, Map(1,"one")(2,"two"));
        obj.push_back(true);
        obj.push_back("hello");
        std::cout << "obj = " << obj << "\n";

        std::cout << "obj[0] = " << obj[0] << "\n";
        std::cout << "obj[1] = " << obj[1] << "\n";
        std::cout << "obj[2] = " << obj[2] << "\n";
        std::cout << "obj[3] = " << obj[3] << "\n";
        std::cout << "obj[4] = " << obj[4] << "\n";
}

TEST(JsonObject, foreach){
        using namespace Frontend;
        auto obj = Array(1, "two", 3.333, Array(1,2,3), Map("a", "A")("b", 35.555));
        for( auto const& _ : obj){
                switch(_.GetType()){
                case JsonObject::Type_Array:
                        for( JsonObject::const_iterator iter(_.begin()),end(_.end());iter!=end;++iter){
                                std::cout << "-" << *iter << "\n";
                        }
                        break;
                case JsonObject::Type_Map:
                        for( JsonObject::const_iterator iter(_.begin()),end(_.end());iter!=end;++iter){
                                std::cout << "-" << *iter << "=>" << _[*iter] << "\n";
                        }
                        break;
                default:
                        break;
                }
                std::cout << _ << "\n";
        }
}
TEST(JsonObject, maker){
        Detail::JsonObjectMaker m;
        auto iter = json_sample_text.begin(), end = json_sample_text.end();
        detail::basic_parser<Detail::JsonObjectMaker,decltype(iter)> p(m,iter, end);
        p.parse();
        auto ret = m.make();
        std::cout << ret << "\n";
}

#if 0
TEST(other, kjk){
        debug_maker m;
        auto iter = json_sample_text.begin(), end = json_sample_text.end();
        detail::basic_parser<debug_maker,decltype(iter)> p(m,iter, end);
        p.parse();
        auto ret = m.make();
}
#endif
