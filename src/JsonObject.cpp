#include "gjson/JsonObject.h"
#include "gjson/JsonObjectMaker.h"
#include "gjson/basic_parser.h"

namespace gjson{

void JsonObject::Parse(std::string const& s){
        JsonObjectMaker m;
        auto iter = s.begin(), end = s.end();
        basic_parser<JsonObjectMaker,decltype(iter)> p(m,iter, end);
        p.parse();
        auto ret = m.make();
        *this = ret;
}


/*
        To actually print json reasonable, I think I need to create a 
        meta object, with logs of new lines, so that we print the following.
                {
                        "name":"bob",
                        "primes":[
                                2,
                                3,
                                5
                        },
                }
 */
namespace Detail{
        struct RenderContext{
                enum Config{
                        Config_Pretty = 1,
                        Config_SingleLine,
                        Config_Default = Config_Pretty,
                };
                explicit RenderContext(Config config, std::ostream& ostr)
                        :config_{config}, ostr_{&ostr}
                {}
                std::ostream& Put(){ return *ostr_; }
                bool ConsumeOptional()const{
                        return config_ == Config_Pretty;
                }
                void NewLine(){
                        if( config_ == Config_SingleLine )
                                return;
                        Put() << "\n";
                }
        private:
                Config config_;
                std::ostream* ostr_;
        };
        /*
                Yes we're creating a direct graph
                to pritn a json object. The idea is 
                that we create a graph with logs of
                newlines, then collage the newlines 
                to something sensible,
         */
        enum NodeType{
                // atomic text
                NodeType_Text,
                /* optional space, for example consimer 
                      {'hello':[1,{'a':[2,3]}]},
                   this may get rendered as
                      {
                        'hello':[
                          1,
                          {
                            'a':[
                              2,
                              3
                            ]
                          }
                        ]
                      },
                   because consider the case where 'a' is replaced by a 
                   100 charcter sentance, or each number repalced by a 
                   50 digit number, the above algrebaric formatting 
                   would be correct. However in the above case, we could
                   'collapase' parts of the formatting using simple rules.
                        We use options to have an optional Text, which 
                   will almost certainly be whitespace. We can then
                   run an optimizer to remove the Optional Text where
                   unneccasry
                */
                NodeType_Optional,
                /*
                  A Vector is going to be the aggragate type we use.
                  for all elements whithin a map or array, they are 
                  part of the same vector etc, meaning the indent 
                  should be consistent. In the above example we should
                  get 
                        Vector("hello:", Vector(1, Vector("a:", Vector(2,3))))
                        Vector("hello:", Optional("\n    "), Vector(1, Vector("a:", Vector(2,3))))

                 */
                NodeType_Vector,

                NodeType_Indent,
                NodeType_NewLine,
                NodeType_MapBegin,
                NodeType_MapEnd,
                NodeType_ArrayBegin,
                NodeType_ArrayEnd,
        };
        struct Node{
                explicit Node(NodeType type):type_{type}{}
                virtual ~Node()=default;

                
                Node(Node const&)=delete;
                Node(Node&&)=delete;
                Node& operator=(Node const&)=delete;
                Node& operator=(Node&&)=delete;

                // maybe make this multiline
                virtual size_t Width()const=0;
                virtual void Render(RenderContext& ctx)const=0;
                virtual std::string DebugString()const=0;
                NodeType GetType()const{ return type_; }
        private:
                NodeType type_;
        };
        struct TextBase : Node{
                explicit TextBase(NodeType type, std::string str):
                        Node{type},
                        str_{std::move(str)}
                {}
                size_t Width()const override{
                        return str_.size();
                }
                void Render(RenderContext& ctx)const override{
                        ctx.Put() << str_;
                }
                std::string DebugString()const override{
                        std::stringstream sstr;
                        sstr << "Text<\"" << str_ << "\">";
                        return sstr.str();
                }
        private:
                std::string str_;
        };
        struct Text : TextBase{
                explicit Text(std::string str):
                        TextBase{NodeType_Text, std::move(str)}
                {}
        };
        struct MapBegin : TextBase{
                explicit MapBegin():
                        TextBase{NodeType_MapBegin, "{"}
                {}
        };
        struct MapEnd : TextBase{
                explicit MapEnd():
                        TextBase{NodeType_MapEnd, "}"}
                {}
        };
        struct ArrayBegin : TextBase{
                explicit ArrayBegin():
                        TextBase{NodeType_ArrayBegin, "["}
                {}
        };
        struct ArrayEnd : TextBase{
                explicit ArrayEnd():
                        TextBase{NodeType_ArrayEnd, "]"}
                {}
        };
        
        struct Indent : Node{
                explicit Indent(unsigned n):
                        Node{NodeType_Indent},
                        n_{n}
                {}
                size_t Width()const override{
                        return n_ * 4;
                }
                void Render(RenderContext& ctx)const override{
                        if( n_ == 0 ) return;
                        ctx.Put() << std::string(n_ * 4, ' ');
                }
                std::string DebugString()const override{
                        std::stringstream sstr;
                        sstr << "Indent<" << n_ << ">";
                        return sstr.str();
                }
        private:
                unsigned n_;
        };

        
        struct NewLine : Node{
                NewLine():Node{NodeType_NewLine}{}
                size_t Width()const override{
                        return 0;
                }
                void Render(RenderContext& ctx)const override{
                        ctx.NewLine();
                }
                std::string DebugString()const override{
                        return "NewLine";
                }
        };

        struct Vector : Node{
                using container_type = std::vector<Node*>;
                using const_iterator = container_type::const_iterator;

                explicit Vector():
                        Node{NodeType_Vector}
                {}
                ~Vector(){
                        std::cerr << "Dying<" << this << ">\n";
                }

                Vector(Vector const&)=delete;
                Vector(Vector&&)=delete;
                Vector& operator=(Vector const&)=delete;
                Vector& operator=(Vector&&)=delete;

                void push(Node* ptr){ 
                        vec_.push_back(ptr);
                }
                auto begin(){ return vec_.begin(); }
                auto end(){ return vec_.end(); }

                auto const& operator[](size_t idx)const{
                        return vec_.at(idx);
                }
                auto size()const{ return vec_.size(); }
                void erase(size_t idx){
                        decltype(vec_) next;
                        for(size_t i=0;i!=vec_.size();++i){
                                if( i == idx )
                                        continue;
                                next.push_back(vec_[i]);
                        }
                        vec_ = std::move(next);
                }
                void clear(){ vec_.clear(); }
                

                // this doesn't make sense unless we ignore newlines etc
                size_t Width()const override{
                        size_t sigma = 0;
                        for(  auto const& ptr : vec_ ){
                                sigma += ptr->Width();
                        }
                        return sigma;
                }
                std::string DebugString()const override{
                        std::stringstream sstr;
                        sstr << "[";
                        const char* comma = "";
                        for( auto const& ptr : vec_ ){
                                sstr << comma << ptr->DebugString();
                                comma = ", ";
                        }
                        sstr << "]";
                        return sstr.str();
                }
                void Render(RenderContext& ctx)const override{
                        for( auto const& ptr : vec_ ){
                                ptr->Render(ctx);
                        }
                }
        private:
                container_type vec_;
        };
        // this class is basucally meta-information, it
        // isn't optional as in 'boost::optional' optional,
        // but optional as in call Optional::Render() or
        // not without effecting correcness, only prettyness
        struct Optional : Node{
                explicit Optional(Node* ptr):
                        Node{NodeType_Optional},
                        ptr_{ptr}
                {}
                size_t Width()const override{
                        return ptr_->Width();
                }
                void Render(RenderContext& ctx)const override{
                        if( ctx.ConsumeOptional() ){
                                ptr_->Render(ctx);
                        }
                }
                std::string DebugString()const override{
                        std::stringstream sstr;
                        sstr << "Optional<" << ptr_->DebugString() << ">";
                        return sstr.str();
                }
        private:
                Node* ptr_;
        };

        struct GVStackFrame{
                explicit GVStackFrame(Type t = Type_NotAType)
                        :type{t}
                        ,vector{ new Vector }
                {}
                GVStackFrame(GVStackFrame const&)=delete;
                GVStackFrame(GVStackFrame&&)=delete;
                GVStackFrame& operator=(GVStackFrame const&)=delete;
                GVStackFrame& operator=(GVStackFrame&&)=delete;
                Type type;
                Vector* vector;
                size_t index{0};
        };
}
namespace Detail{

        
        struct GraphVisitor : JsonObject::visitor{

                GraphVisitor(){
                        stack_.push_back(new Detail::GVStackFrame);
                }
                ~GraphVisitor(){
                        for( auto& ptr : stack_ ){
                                delete ptr;
                        }
                }

                GraphVisitor(GraphVisitor const&)=delete;
                GraphVisitor(GraphVisitor&&)=delete;
                GraphVisitor& operator=(GraphVisitor const&)=delete;
                GraphVisitor& operator=(GraphVisitor&&)=delete;

                
                void on_nil()override{
                        do_primitive_("<nil>");
                }
                void on_bool(bool value)override{
                        do_primitive_( value ? "true" : "false" );
                }
                void on_integer(std::int64_t value)override{
                        do_primitive_( boost::lexical_cast<std::string>(value));
                }
                void on_float(double value)override{
                        do_primitive_( boost::lexical_cast<std::string>(value));
                }
                void on_string(std::string const& value)override{
                        do_primitive_( "\"" + value + "\"");
                }
                VisitorCtrl begin_array(size_t n)override{
                        do_begin_(Type_Array, n);
                        return VisitorCtrl_Decend;
                }
                void end_array()override{
                        do_end_( Type_Array);
                }
                VisitorCtrl begin_map(size_t n)override{
                        do_begin_(Type_Map, n);
                        return VisitorCtrl_Decend;
                }
                void end_map()override{
                        do_end_(Type_Map);
                }
                void Render(RenderContext& ctx)const{
                        stack_.back()->vector->Render(ctx);
                }
                void Debug()const{
                        for(auto& s : stack_ ){
                                std::cout << s->vector->DebugString() << "\n";
                        }
                }
                void Optmize(){
                        using namespace Detail;

                        std::vector<Vector*> stack{stack_.back()->vector};
                        std::vector<Vector*> todo_last;


                        for(;stack.size();){
                                auto head = stack.back();
                                stack.pop_back();

                                todo_last.push_back(head);

                                for(auto item : *head){
                                        if( item->GetType() == NodeType_Vector ){
                                                stack.push_back(reinterpret_cast<Vector*>(item));
                                        }
                                }
                        }

                        /*
                                1) make 
                         */
                        for(size_t idx=todo_last.size();idx!=0;){
                                --idx;
                                auto& head = *todo_last[idx];

                                /* see if we want collapse
                                   [
                                   1
                                   ,2
                                   ,3
                                   ,4
                                   ],
                                   etc
                                   */
                                if( head.size() >= 2 ){
                                        if( head[0]->GetType()             == NodeType_MapBegin &&
                                            head[head.size()-1]->GetType() == NodeType_MapEnd ){
                                                //std::cout << "we git a map\n";
                                        }
                                        if( head[0]->GetType()             == NodeType_ArrayBegin &&
                                            head[head.size()-1]->GetType() == NodeType_ArrayEnd ){
                                                //std::cout << "we git a array\n";
                                        }
                                }
                                auto try_collage_aggregate = [&](){
                                        size_t aggregate_width = 0;
                                        std::vector<Node*> nodes;
                                        for(auto ptr : head ){
                                                switch(ptr->GetType()){
                                                case NodeType_Text:
                                                case NodeType_MapBegin:
                                                case NodeType_MapEnd:
                                                case NodeType_ArrayBegin:
                                                case NodeType_ArrayEnd:
                                                        aggregate_width += ptr->Width();
                                                        nodes.push_back(ptr);
                                                        break;
                                                case NodeType_Optional:
                                                case NodeType_NewLine:
                                                case NodeType_Indent:
                                                        // do nothing, this is what we're skipping
                                                        break;
                                                default:
                                                        // ok we can't collage nested maps etc (or can we?)
                                                        return;
                                                }
                                        }
                                        if( aggregate_width < 80 ){
                                                head.clear();
                                                for(auto _ : nodes)
                                                        head.push(_);
                                        }
                                };
                                try_collage_aggregate();

                        }
                }
        private:
                void do_primitive_(std::string str){
                        enum Transition{
                                Transition_Other,
                                Transition_MapKey,
                                Transition_MapValue,
                        };
                        Transition tran = Transition_Other;

                        if( stack_.back()->type == Type_Map ){
                                if( stack_.back()->index % 2 == 0 ){
                                        tran = Transition_MapKey;
                                } else{
                                        tran = Transition_MapValue;
                                }
                        } 

                        maybe_comma_();

                        if( tran != Transition_MapValue ){
                                auto opt = new Detail::Vector{};
                                opt->push( new Detail::NewLine{} );
                                opt->push( new Detail::Indent(stack_.size()) );
                                stack_.back()->vector->push( new Detail::Optional{opt} );
                        }
                        stack_.back()->vector->push(new Detail::Text{std::move(str)});
                        if( tran == Transition_MapKey ){
                                stack_.back()->vector->push(new Detail::Text{":"});
                        }

                        ++stack_.back()->index;
                }
                void do_begin_(Type type, size_t n){
                        maybe_comma_();

                        stack_.emplace_back(new Detail::GVStackFrame{type});
                        if( type == Type_Map ){
                                stack_.back()->vector->push(new Detail::MapBegin{});
                        } else {
                                stack_.back()->vector->push(new Detail::ArrayBegin{});
                        }

                }
                void do_end_(Type type){
                        auto opt = new Detail::Vector{};
                        opt->push( new Detail::NewLine{} );
                        opt->push( new Detail::Indent(stack_.size() - 1) );
                        stack_.back()->vector->push(new Detail::Optional{opt});
                        if( type == Type_Map ){
                                stack_.back()->vector->push(new Detail::MapEnd{});
                        } else {
                                stack_.back()->vector->push(new Detail::ArrayEnd{});
                        }
                        auto vec = stack_.back()->vector;

                        stack_.pop_back();
                        stack_.back()->vector->push(vec);
                        ++stack_.back()->index;
                }

        private:
                void maybe_comma_(){
                        if( stack_.back()->index == 0 )
                                return;
                        if( stack_.back()->type == Type_Map ){
                                if( stack_.back()->index % 2 == 1 )
                                        return;
                        } 
                        stack_.back()->vector->push( new Detail::Text(", ") );

                }
                std::list<Detail::GVStackFrame*> stack_;
        };

        struct debug_visitor : JsonObject::visitor{
                explicit debug_visitor(std::ostream& ostr):ostr_{&ostr}{}
                void on_nil()override{
                        *ostr_ << make_indent_() << "on_nil()\n";
                }
                void on_bool(bool value)override{
                        *ostr_ << make_indent_() << "on_bool(" << value << ")\n";
                }
                void on_integer(std::int64_t value)override{
                        *ostr_ << make_indent_() << "on_integer(" << value << ")\n";
                }
                void on_float(double value)override{
                        *ostr_ << make_indent_() << "on_float(" << value << ")\n";
                }
                void on_string(std::string const& value)override{
                        *ostr_ << make_indent_() << "on_string(" << value << ")\n";
                }
                VisitorCtrl begin_array(size_t n)override{
                        *ostr_ << make_indent_() << "begin_array(" << n << ")\n";
                        ++indent_;
                        return VisitorCtrl_Decend;
                }
                void end_array()override{
                        --indent_;
                        *ostr_ << make_indent_() << "end_array()\n";
                }
                VisitorCtrl begin_map(size_t n)override{
                        *ostr_ << make_indent_() << "begin_map(" << n << ")\n";
                        ++indent_;
                        return VisitorCtrl_Decend;
                }
                void end_map()override{
                        --indent_;
                        *ostr_ << make_indent_() << "end_map()\n";
                }
        private:
                std::string make_indent_()const{
                        return std::string(indent_*4,' ');
                }
                std::ostream* ostr_;
                unsigned indent_{0};
        };
} // Detail
void JsonObject::Display(std::ostream& ostr, unsigned indent)const{
        Detail::RenderContext ctx(Detail::RenderContext::Config_Pretty, ostr);
        Detail::GraphVisitor v;
        this->Accept(v);
        v.Optmize();
        v.Render(ctx);
        ostr << "\n";
        ostr.flush();
}
std::string JsonObject::ToString()const{
        std::stringstream sstr;
        Detail::RenderContext ctx(Detail::RenderContext::Config_SingleLine, sstr);
        Detail::GraphVisitor v;
        this->Accept(v);
        v.Optmize();
        v.Render(ctx);
        return sstr.str();
}
void JsonObject::Debug()const{
        Detail::debug_visitor v(std::cout);
        this->Accept(v);
}

} // gjson
