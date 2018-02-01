#include <gtest/gtest.h>
#include <unordered_map>
#include <list>

#include "json_parser.h"
#include "JsonObject.h"

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
  },
  "dummy":{},
  "one_to_ten":[1,2,3,4,5,6,7,8,9,10]
}
)";
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
        EXPECT_EQ( Type_Integer, obj.GetType() );
        EXPECT_EQ( "12", obj.to_string() );
        EXPECT_EQ( 12, obj.AsInteger() );
        obj = 34;
        EXPECT_EQ( Type_Integer, obj.GetType() );
        EXPECT_EQ( "34", obj.to_string() );
        EXPECT_EQ( 34, obj.AsInteger() );
}
TEST(JsonObject, String){
        JsonObject obj("hello");
        EXPECT_EQ( Type_String, obj.GetType() );
        EXPECT_EQ( "hello", obj.AsString() );
        obj = std::string("world");
        EXPECT_EQ( "world", obj.AsString() );
}

TEST(JsonObject, assignment){
        JsonObject obj("hello");
        obj = 23;
        EXPECT_EQ( Type_Integer, obj.GetType() );
        EXPECT_EQ( 23, obj.AsInteger() );
        obj = 12.23;
        EXPECT_EQ( Type_Float, obj.GetType() );
        obj = "hello";
        EXPECT_EQ( Type_String, obj.GetType() );
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
        EXPECT_EQ( Type_Array, arr.GetType());
        EXPECT_EQ( Type_Integer, arr[0].GetType());
        EXPECT_EQ( Type_String,  arr[1].GetType());
        EXPECT_EQ( Type_Bool,    arr[2].GetType());
        EXPECT_EQ( Type_Array,   arr[3].GetType());
        EXPECT_EQ( Type_Bool,    arr[4].GetType());
        EXPECT_EQ( Type_Array,   arr[5].GetType());
        EXPECT_EQ( Type_Array,   arr[5][0].GetType());

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
                case Type_Array:
                        for( JsonObject::const_iterator iter(_.begin()),end(_.end());iter!=end;++iter){
                                std::cout << "-" << *iter << "\n";
                        }
                        break;
                case Type_Map:
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
        JsonObject::debug_visitor dbg;
        ret.Accept(dbg);
        std::cout << ret << "\n";
}

TEST(JsonObject, access){
        JsonObject obj;
        obj.Parse(json_sample_text);
        std::cout << obj << "\n";

        EXPECT_EQ( "John", obj["firstName"].AsString() );
        EXPECT_EQ( 25, obj["age"].AsInteger() );
        EXPECT_EQ( "New York", obj["address"]["city"].AsString() );
        EXPECT_ANY_THROW(  obj["address"]["city"].AsInteger());
        EXPECT_EQ( 10021 , obj["address"]["postalCode"].AsInteger());
}
TEST(JsonObject, IntArray){

        JsonObject obj; 
        obj.Parse("[1,2,3]");
        EXPECT_EQ( 3, obj.size());
        int sum = 0;
        for(auto _ : obj ){
                sum += _.AsInteger();
        }
        EXPECT_EQ(6, sum);
        sum = obj[0].AsInteger() + obj[1].AsInteger() + obj[2].AsInteger();
        EXPECT_EQ(6, sum);


}

TEST(JsonObject, ToString){
        JsonObject proto, obj;
        proto.Parse("[1,2,3]");
        obj.Parse( proto.ToString());
        obj.Display();
}

TEST(JsonObject, ToStringWiki){
        std::stringstream to, from;
        JsonObject proto, obj;
        proto.Parse(json_sample_text);
        obj.Parse( proto.ToString());

        EXPECT_EQ( proto.ToString(), obj.ToString());
}

