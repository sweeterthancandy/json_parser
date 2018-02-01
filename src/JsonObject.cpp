#include "JsonObject.h"
#include "JsonObjectMaker.h"

namespace json_parser{

void JsonObject::Parse(std::string const& s){
        JsonObjectMaker m;
        auto iter = s.begin(), end = s.end();
        basic_parser<JsonObjectMaker,decltype(iter)> p(m,iter, end);
        p.parse();
        auto ret = m.make();
        *this = ret;
}

} // json_parser
