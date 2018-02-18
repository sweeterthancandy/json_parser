#include "gjson/tokenizer.h"

#include <gtest/gtest.h>
#include <unordered_map>
#include <list>

#include <boost/type_index.hpp>
#include <boost/optional.hpp>


using namespace gjson;

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

TEST(tokenizer, _){
        tokenizer tok(json_sample_text);
        for(auto t : tok){}

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
TEST(tokenizer, bad){
}

TEST(tokenizer, badtoken){
        EXPECT_ANY_THROW( [](){
        tokenizer tok(R"( 34a )");
        for(auto t : tok){} }());
}

TEST(tokenizer, integers){
        tokenizer tok(R"( [ 0, 23, -4545, 8393939393]  )");
        for(auto t : tok){}
}
TEST(tokenizer, leading_dot){
        tokenizer tok(R"( .0)");
        auto iter=tok.token_begin(), end=tok.token_end();
        EXPECT_NE( iter, end);
        EXPECT_EQ(token_type::float_,  iter->type());
        EXPECT_EQ(".0",  iter->value());
        ++iter;
        EXPECT_EQ( iter, end);
}
TEST(tokenizer, sci_float){
        tokenizer tok(R"( -1e-45)");
        auto iter=tok.token_begin(), end=tok.token_end();
        EXPECT_NE( iter, end);
        EXPECT_EQ(token_type::float_,  iter->type());
        EXPECT_EQ("-1e-45",  iter->value());
        ++iter;
        EXPECT_EQ( iter, end);
}
TEST(tokenizer, token){
        EXPECT_NO_THROW( [](){
        tokenizer tok(R"(hello)");
        for(auto t : tok){} }());
}

TEST(tokenizer, floats){
        std::vector<std::string> literals = {
                "0.123",
                "+0.123",
                "-0.123",
                ".123",
                "0.",
                "+0.123",
                "1e+1",
                "1e-1",
                "+1e+1",
                "-1e-1",
                "+121e-121",
                "1e-7",

                "12e34",
                "+12e34",
                "-12e34",
                
                "1.2e34",
                "+1.2e34",
                "-1.2e34",
                
                "12e3.4",
                "+12e3.4",
                "-12e3.4",
                
                "1.2e3.4",
                "+1.2e3.4",
                "-1.2e3.4",
                
                ".12e3.4",
                "+.12e3.4",
                "-.12e3.4",
                
                ".12e.34",
                "+.12e.34",
                "-.12e.34"
        };
        for( auto lit : literals ){
                tokenizer tok(lit);
                auto iter=tok.token_begin(), end=tok.token_end();
                EXPECT_NE( iter, end) << lit;
                EXPECT_EQ(token_type::float_,  iter->type()) << lit;
                EXPECT_EQ(lit,  iter->value()) << lit;
                ++iter;
                EXPECT_EQ( iter, end) << lit;
        }
}




