This is my implemention of a json parser

        int main(){        
                auto str = R"(
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
                auto root = parse(str);
                display(root);
        }
displays

        {
          firstName:John
          ,lastName:Smith
          ,age:25
          ,address:{
              streetAddress:21 2nd Street
              ,city:New York
              ,state:NY
              ,postalCode:10021
            }
          ,phoneNumber:[
              {
                type:home
                ,number:212 555-1234
              }
              ,{
                type:fax
                ,number:646 555-4567
              }
            ]
          ,gender:{
              type:male
            }
        }