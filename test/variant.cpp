#include <gtest/gtest.h>
#include <unordered_map>
#include <list>

#include "gjson/basic_parser.h"
#include "gjson/variant.h"

#include <boost/type_index.hpp>
#include <boost/optional.hpp>

using namespace gjson;
using namespace gjson::variant;

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
                "{1:{}}",
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

TEST_F( Parser, bad_valid_strings){
}

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
TEST_F( Parser, to_string ){
        for( auto const& str : valid_strings ){
                auto s = to_string( parse(str) );
                //std::cout << s << "\n";
        }
}
#if 0
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


TEST_F( Parser, from_wiki ){
        auto root = parse(json_sample_text);
        display(root);
}

