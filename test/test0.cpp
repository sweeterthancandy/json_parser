#include <gtest/gtest.h>
#include <unordered_map>

#include "json_parser.h"

#include <boost/type_index.hpp>

using namespace json_parser;

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

struct JsonObject;

namespace std{
        template<>
        struct hash<JsonObject>{
                template<class TypeErased>
                std::size_t operator()(TypeErased const& obj)const{
                        return obj.to_string();
                }
        };
} // std

template<unsigned Order>
struct precedence_device : precedence_device<Order-1>{};
template<>
struct precedence_device<0>{};

struct JsonObject{
        using array_type = std::vector<JsonObject>;
        using map_type = std::map<JsonObject, JsonObject>;
        enum Type{
                Begin_Primitive,
                        Type_Nil,
                        Type_Bool,
                        Type_Integer,
                        Type_Float,
                        Type_String,
                End_Primitive,
                Type_Array,
                Type_Map,
        };
        #ifdef NOT_DEFINED
                switch(type_){
                case Type_Nil:
                        break;
                case Type_Bool:
                        break;
                case Type_Integer:
                        break;
                case Type_Float:
                        break;
                case Type_String:
                        break;
                case Type_Array:
                        break;
                case Type_Map:
                        break;
                }
        #endif
        /*
                The point of these are to allow construction of the
                form
                        auto arr = Tag_Array(1,2,3)
         */
        using Tag_Nil    = std::integral_constant<Type, Type_Bool>;
        using Tag_Bool    = std::integral_constant<Type, Type_Bool>;
        using Tag_Integer = std::integral_constant<Type, Type_Integer>;
        using Tag_Float   = std::integral_constant<Type, Type_Float>;
        using Tag_String  = std::integral_constant<Type, Type_String>;
        using Tag_Array   = std::integral_constant<Type, Type_Array>;
        using Tag_Map     = std::integral_constant<Type, Type_Map>;

        JsonObject():
                JsonObject{Type_Nil}
        {}
        explicit JsonObject(Type type):type_{type}{
                switch(type_){
                case Type_Nil:
                        break;
                case Type_Bool:
                        new (&as_bool_) bool(false);
                        break;
                case Type_Integer:
                        new (&as_int_) std::int64_t(static_cast<std::int64_t>(0));
                        break;
                case Type_Float:
                        new (&as_float_) double(static_cast<double>(.0));
                        break;
                case Type_String:
                        new (&as_string_) std::string{};
                        break;
                case Type_Array:
                        new (&as_array_) array_type();
                        break;
                case Type_Map:
                        new (&as_map_) map_type();
                        break;
                }
        }
        JsonObject(Tag_Bool, bool val):type_{Type_Bool}{
                new (&as_bool_) bool(val);
        }
        JsonObject(Tag_Integer, std::int64_t val):type_{Type_Integer}{
                new (&as_int_) std::int64_t{val};
        }
        JsonObject(Tag_Float, double val):type_{Type_Float}{
                new (&as_float_) double{val};
        }
        template<class Arg>
        JsonObject(Tag_String, Arg&& arg):type_{Type_String}{
                new (&as_string_) std::string{arg};
        }
        explicit JsonObject(Tag_Array):type_{Type_Array}{
                new (&as_array_) array_type{};
        }
        explicit JsonObject(Tag_Map):type_{Type_Map}{
                new (&as_map_) map_type{};
        }
        template<class Value>
        explicit JsonObject( precedence_device<0>, Value&& val)
        {
                std::cout << "val = " << boost::typeindex::type_id_with_cvr<Value>() << "\n";
        }
        template<class Value, class = std::enable_if_t< ! std::is_same<decltype( std::declval<Value>().ToJsonObject() ), void >::value > >
        explicit JsonObject( precedence_device<5>, Value&& val)
                : JsonObject{ val.ToJsonObject() }
        {}
        template<class Value, class = std::enable_if_t< std::is_constructible<std::string,Value>::value > >
        explicit JsonObject( precedence_device<6>, Value&& val)
                : JsonObject{ Tag_String{}, std::forward<Value>(val) }
        {}
        template<class Value, class = std::enable_if_t< std::is_same<std::decay_t<Value>, std::string >::value > >
        explicit JsonObject( precedence_device<7>, Value&& val)
                : JsonObject{ Tag_String{}, std::forward<Value>(val) }
        {}
        template<class Value, class = std::enable_if_t< std::is_floating_point<std::decay_t<Value> >::value > >
        explicit JsonObject( precedence_device<8>, Value&& val)
                : JsonObject{ Tag_Float{}, static_cast<double>(val) }
        {}
        template<class Value, class = std::enable_if_t< std::is_integral<std::decay_t<Value> >::value > >
        explicit JsonObject( precedence_device<9>, Value&& val)
                : JsonObject{ Tag_Integer{}, std::forward<Value>(val) }
        {}
        template<class Value, class = std::enable_if_t< std::is_same<std::decay_t<Value>, bool>::value > >
        explicit JsonObject( precedence_device<10>, Value&& val)
                : JsonObject{ Tag_Bool{}, std::forward<Value>(val) }
        {}
        template<class Value, class = std::enable_if_t< ! std::is_same<JsonObject, std::decay_t<Value> >::value > >
        explicit JsonObject(Value&& val)
                : JsonObject{ precedence_device<10>{}, std::forward<Value>(val) }
        {}
        JsonObject(JsonObject const& that)
                :type_{that.type_}
        {
                switch(type_){
                case Type_Nil:
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
                        new (&as_array_) array_type(that.as_array_);
                        break;
                case Type_Map:
                        new (&as_map_) map_type(that.as_map_);
                        break;
                }
        }
        JsonObject(JsonObject&& that)
                :type_{that.type_}
        {
                switch(type_){
                case Type_Nil:
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
                        new (&as_array_) array_type(std::move(that.as_array_));
                        break;
                case Type_Map:
                        new (&as_map_) map_type(std::move(that.as_map_));
                        break;
                }
        }
        ~JsonObject(){
        }

        auto AsInteger()const{
                if( type_ == Type_Integer )
                        return as_int_;
                throw std::domain_error("not an integer");
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

        static JsonObject MakeBool(bool val){
                return JsonObject( std::integral_constant<Type, Type_Bool>{}, val);
        }


        template<class Value>
        JsonObject& operator=(Value&& value){
                return *this = JsonObject{std::forward<Value>(value)};
        }
        template<class Key, class = std::enable_if_t< std::is_integral<std::decay_t<Key> >::value > >
        JsonObject& operator[](Key key){
                if( type_ ==  Type_Array ){
                        return as_array_.at(static_cast<typename array_type::size_type>(key));
                } else if( type_ == Type_Map ){
                        std::int64_t casted = static_cast<std::int64_t>(key);
                        JsonObject key(Tag_Integer{}, casted);
                        return as_map_[key];
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
        void Display(std::ostream& ostr, unsigned indent = 0)const{
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
        }
        friend std::ostream& operator<<(std::ostream& ostr, JsonObject const& self){
                self.Display(ostr);
                return ostr;
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
                                JsonObject obj(JsonObject::Type_Map);
                                for(size_t idx =0;idx < vec_.size();idx += 2 ){
                                        obj.emplace_unchecked( std::move( vec_[0] ),
                                                               std::move( vec_[1] ) );
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
                        JsonObject obj(JsonObject::Type_Map);
                        return std::move(obj);
                }
        };

        ArrayType Array = {};
        MapType Map = {};

} // Frontend

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
TEST(JsonObject, Frontend_Map){
        using namespace Frontend;
        JsonObject obj = Map;
        std::cout << "obj = " << obj << "\n";
        obj = Map("one", 1).ToJsonObject();
        std::cout << "obj = " << obj << "\n";
}

TEST(other, kjk){
        debug_maker m;
        auto iter = json_sample_text.begin(), end = json_sample_text.end();
        detail::basic_parser<debug_maker,decltype(iter)> p(m,iter, end);
        p.parse();
        auto ret = m.make();

        JsonObject obj(JsonObject::Type_Integer);
}
