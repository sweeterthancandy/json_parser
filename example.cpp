#include "JsonObject.h"


int main(){
        using namespace json_parser;
        JsonObject obj;

        // parse
        obj.Parse(R"( { "primes":[2,5,7,11,13] } )");
        
        // empty aggregates
        obj["empty_array"] = Array;
        obj["empty_map"] = Map;

        // assign an array
        obj["nice_numbers"] = Array("two", 3, 4.4);

        // assign a map
        obj["next"] = Map(1,"one")
                         ("two",2);


        

        // we can use an integer as a key as well
        obj[23]  = "twentry three";

        // iterator over the intergers
        for( auto p : obj["primes"] ){
                std::cout << p.AsInteger() << "\n";
        }
        // or use as an zero index array
        auto const& primes = obj["primes"];
        for(size_t i = 0; i!= primes.size(); ++i){
                std::cout << primes[0] << "\n";
        }

        for( auto p : obj["nice_numbers"] ){
                switch(p.GetType()){
                case Type_Integer:
                        std::cout << p.AsInteger() << "\n";
                        break;
                case Type_String:
                        std::cout << p.AsString() << "\n";
                        break;
                case Type_Float:
                        std::cout << p.AsFloat() << "\n";
                        break;
                default:
                        break;
                }
        }

        // iteratate over a map
        using MI = JsonObject::const_iterator;
        for(MI iter( obj["next"].begin() ), end( obj["next"].end());iter!=end;++iter){
                std::cout << iter.key() << " => " << iter.value() << "\n";
        }


        // display pretty
        obj.Display();

        // print to a single line
        std::string s = obj.ToString();

        JsonObject other;
        // reparse
        other.Parse(s);

}

