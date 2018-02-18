#ifndef JSON_PARSER_TOKENIZER_H
#define JSON_PARSER_TOKENIZER_H

#include <sstream>
#include <iostream>

#include <boost/preprocessor.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

namespace gjson{

        #define TOKEN_TYPES/**/\
                (left_curl)\
                (right_curl)\
                (left_br)\
                (right_br)\
                (comma)\
                (colon)\
                (string_)\
                (int_)\
                (float_)\
                (true_)\
                (false_)\
                (null_)\
                (id_)\
                (dummy)\

        #define TOKEN_ENUM_AUX(r,data,i,elem) BOOST_PP_COMMA_IF(i) elem
        #define TOKEN_OSTREAM_AUX(r,data,elem)\
                case token_type::elem:\
                        return ostr << BOOST_PP_STRINGIZE(elem);
        enum class token_type{
                BOOST_PP_SEQ_FOR_EACH_I(TOKEN_ENUM_AUX,~,TOKEN_TYPES)
        };
        inline
        std::ostream& operator<<(std::ostream& ostr, token_type type){
                switch(type){
                        BOOST_PP_SEQ_FOR_EACH(TOKEN_OSTREAM_AUX,~,TOKEN_TYPES)
                        default:
                                __builtin_unreachable();
                }
        }
        #undef TOKEN_OSTREAM_AUX
        #undef TOKEN_ENUM_AUX
        #undef TOKEN_TYPES

        #define TOKENIZER_ERROR( MSG )                                                 \
                do {                                                                   \
                        BOOST_THROW_EXCEPTION(                                         \
                            std::domain_error( str( boost::format( "%s (left=%s)" ) %  \
                                                    ( MSG ) % whats_left() ) ) );      \
                } while ( 0 )


        struct token{
                token(token_type _type, std::string const& _value)
                        :type_(_type),value_(_value)
                {}
                token_type type()const{return type_;}
                std::string const& value()const{return value_;}

                friend std::ostream& operator<<(std::ostream& ostr, token const& tok){
                        return ostr << "(" << tok.type() << ",\"" << tok.value() << "\")";
                }
        private:
                token_type type_;
                std::string value_;
        };

        template<class Iter>
        struct basic_tokenizer{

                struct state_t{
                        Iter first_, last_;
                        boost::optional<token> peak_;
                };


                struct iterator{
                        explicit iterator(basic_tokenizer* self = 0)
                                :self_{self}{
                                if( !! self ){
                                        if( ! self_->eos()){
                                                tok_ = self_->peak();
                                        }
                                }
                        }


                        friend bool operator==(iterator const& left,
                                               iterator const& right){
                                /*
                                             |Fake | Real  |
                                        -----+-----+-------+
                                        Fake |  Y  |   N   |
                                        Real |  N  |ByValue|
                                */
                                if( left.self_ == 0 && right.self_ == 0 )
                                        return true;
                                if( left.self_ == 0 ){
                                        return right.self_->eos();
                                 } else {
                                        return left.self_->eos();
                                 }
                                return false;
                        }
                        friend bool operator!=(iterator const& left,
                                               iterator const& right){
                                return ! ( left == right);
                        }
                        iterator operator++(){
                                tok_ = self_->next();
                                return *this;
                        }
                        token const& operator*()const{
                                return *tok_;
                        }
                        token const* operator->()const{
                                return &*tok_;
                        }
                private:
                        basic_tokenizer* self_{0};
                        boost::optional<token> tok_;
                };



                explicit basic_tokenizer(std::string const& str)
                        : mem_{str}, start_{mem_.begin()}, end_{mem_.end()}
                {
                        state_.first_ = start_;
                        state_.last_ = end_;

                        next();
                }
                basic_tokenizer(Iter first, Iter last)
                        : start_{first}, end_{last}
                {
                        state_.first_ = start_;
                        state_.last_ = end_;

                        next();
                }
                bool eos()const{return state_.first_ == state_.last_ && !state_.peak_;}
                boost::optional<token> peak(){return state_.peak_;}
                boost::optional<token> next(){
                        state_.peak_ = next_();
                        //PRINT(std::distance(state_.first_,state_.last_));
                        return state_.peak_;
                }

                state_t save_state_please()const{
                        return state_;
                }
                void restore_this_state_if_you_would_please(state_t proto){
                        state_ = proto;
                }
                std::string whats_left()const{
                        return std::string(
                                state_.first_,
                                std::next(
                                        state_.first_,
                                        std::min<size_t>(
                                                30,
                                                std::distance(state_.first_,state_.last_)
                                        )
                                )
                        );
                }

                iterator token_begin(){
                        return iterator{this};
                }
                iterator token_end(){
                        return iterator{0};
                }
                
                auto begin(){ return token_begin(); }
                auto end(){ return token_end(); }

                std::string get_error()const{
                        return error_;
                }

                boost::optional<token> return_errror_(std::string const& msg){


                        enum{ before = 40, after = 40 };

                        Iter ctx_start{start_};
                        if( before < std::distance(ctx_start, state_.first_) ){
                                ctx_start = state_.first_ - before;
                        }

                        Iter ctx_end = state_.first_ + after;
                        if( end_ < ctx_end )
                                ctx_end = end_;


                        // now make a cursor
                        //
                        //  [a,2, @)
                        //        ^
                        
                        auto d = std::distance(ctx_start, state_.first_);


                        std::stringstream sstr;
                        sstr << "error: " << msg << "\n";
                        sstr << std::string(ctx_start, ctx_end) << "\n";
                        if( d != 0){
                                sstr << std::string(d, ' ');
                        }
                        sstr << "^"; // no newline

                        error_ = sstr.str();

                        std::cerr << get_error() << "\n";

                        throw std::domain_error(get_error());
                        // XXX want to use this
                }
        private:

                /*
                        I'm not using regular expressions on purpose
                 */
                boost::optional<token> next_(){

                        // eat whitespace
                        for(;state_.first_!=state_.last_;++state_.first_)
                                if( ! std::isspace(*state_.first_))
                                        break;
                        if( state_.first_ == state_.last_)
                                return boost::none;

                        switch(*state_.first_){
                                case '{': ++state_.first_; return token(token_type::left_curl ,"{");
                                case '}': ++state_.first_; return token(token_type::right_curl,"}");
                                case '[': ++state_.first_; return token(token_type::left_br   ,"[");
                                case ']': ++state_.first_; return token(token_type::right_br  ,"]");
                                case ',': ++state_.first_; return token(token_type::comma     ,",");
                                case ':': ++state_.first_; return token(token_type::colon     ,":");

                                case '"':{
                                        auto iter = state_.first_;
                                        ++iter;
                                        for(;*iter != '"';++iter){
                                                if( iter == state_.last_)
                                                        return return_errror_("unterminated string");
                                        }
                                        assert( *iter == '"');
                                        token tmp(token_type::string_,
                                                std::string(
                                                        std::next(state_.first_),
                                                        iter));
                                        state_.first_ = std::next(iter);
                                        return tmp;
                                }
                                case '\'':{
                                        auto iter = state_.first_;
                                        ++iter;
                                        for(;*iter != '\'';++iter){
                                                if( iter == state_.last_)
                                                        return return_errror_("unterminated string");
                                        }
                                        assert( *iter == '\'');
                                        token tmp(token_type::string_,
                                                std::string(
                                                        std::next(state_.first_),
                                                        iter));
                                        state_.first_ = std::next(iter);
                                        return tmp;
                                }
                                default:
                                        break;
                        }

                        // is it an int or float literals?
                        if( std::isdigit(*state_.first_) || *state_.first_ == '+' || *state_.first_ == '-' || *state_.first_ == '.'){

                                
                                bool real = false;
                                bool leading_dot = false;

                                auto iter = state_.first_;
                                ++iter;

                                switch(*state_.first_ ){
                                        case '+':
                                        case '-':
                                                // must be followed by digit
                                                if( iter == state_.last_  )
                                                        return return_errror_("+/- not followed by digit");
                                                if( *iter == '.' ){
                                                        real = true;
                                                        leading_dot = true;
                                                        ++iter;
                                                }
                                                if( iter == state_.last_  || ! std::isdigit( *iter ) )
                                                        return return_errror_("+/- not followed by digit");
                                                break;
                                        case '.':
                                                real = true;
                                                leading_dot = true;
                                                // must be followed by digit
                                                if( iter == state_.last_  || ! std::isdigit( *iter ) )
                                                        return return_errror_(". not followed by digit");
                                                break;
                                        default:
                                                break;
                                }

                                for(; iter != state_.last_ && std::isdigit(*iter);++iter);

                                if( ! leading_dot ){
                                        // at the 12.34 etc
                                        //          ^
                                        if( iter != state_.last_ && *iter == '.'){
                                                ++iter;
                                                // we can have 0. etc
                                                for(; iter != state_.last_ && std::isdigit(*iter);++iter);
                                                real = true;
                                        }
                                }
                                /*
                                        [+-]?\d+\(.\d\*\)?[eE][+-]\d+\(.\d|+\)
                                 */
                                bool sci = false;
                                if( iter != state_.last_ && 
                                    ( *iter == 'e' || *iter == 'E') )
                                {
                                        sci = true;
                                        ++iter;
                                        if( iter == state_.last_ )
                                                return return_errror_("expected +/-");

                                        switch(*iter){
                                        case '+': case '-':
                                                ++iter;
                                                  break;
                                        default:
                                        // implicit '+'
                                                  break;
                                        }

                                        if( iter == state_.last_ )
                                                return return_errror_("expected expoonent");
                                        
                                        for(; iter != state_.last_ && std::isdigit(*iter);++iter);
                                        if( iter != state_.last_ && *iter == '.'){
                                                ++iter;
                                                // we can have 0. etc
                                                for(; iter != state_.last_ && std::isdigit(*iter);++iter);
                                                real = true;
                                        }
                                
                                        // most not precede a [a-z]
                                        if( iter != state_.last_ ){
                                                if( std::isalpha( *iter ) ){
                                                        return return_errror_("invalid token");
                                                }
                                        }
                                        auto tmp = token(
                                                token_type::float_,
                                                std::string(
                                                        state_.first_,
                                                        iter));
                                        state_.first_ = iter;
                                        return std::move(tmp);


                                }


                               
                                auto tmp = token(
                                        ( real ? token_type::float_ : token_type::int_ ),
                                        std::string(
                                                state_.first_,
                                                iter));
                                state_.first_ = iter;

                                // most not precede a [a-z]
                                if( iter != state_.last_ ){
                                        if( std::isalpha( *iter ) ){
                                                return return_errror_("invalid token");
                                        }
                                }
                                return std::move(tmp);
                        }  else if( std::isalpha(*state_.first_) || *state_.first_ == '_' ){
                                auto iter = state_.first_;
                                for(; (iter != state_.last_) && ( std::isalnum(*iter) || *iter == '_' );++iter){
                                        if( iter == state_.last_){
                                                break;
                                        }
                                }
                                std::string s( state_.first_, iter);
                                state_.first_ = iter;
                                if( s == "true" ){
                                        return token(token_type::true_, std::move(s));
                                }
                                if( s == "false" ){
                                        return token(token_type::false_, std::move(s));
                                }
                                return token(token_type::string_, std::move(s));
                        } else{
                                return return_errror_("unregognized sequence of chars");
                        }
                }
        private:
                // TODO: reject construction from a string regerence
                std::string mem_;
                Iter start_, end_;
                state_t state_;
                std::string error_;
        };

        using tokenizer = basic_tokenizer<std::string::const_iterator>;

        #undef TOKENIZER_ERROR
} // gjson
#endif // JSON_PARSER_TOKENIZER_H
