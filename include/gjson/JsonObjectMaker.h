#ifndef JSON_PARSER_JSONOBJECTMAKER_H
#define JSON_PARSER_JSONOBJECTMAKER_H

namespace gjson{
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
                        //add_any_( JsonObject{JsonObject::Tag_Nil{}});
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
                        return tmp;
                } 
        private:
                void add_any_(JsonObject&& obj){
                        if( stack_.back().object.GetType() == Type_Array ){
                                stack_.back().object.push_back_unchecked( obj );
                        } else if( stack_.back().object.GetType() == Type_Map ){
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
        
} // gjson


#endif // JSON_PARSER_JSONOBJECTMAKER_H
