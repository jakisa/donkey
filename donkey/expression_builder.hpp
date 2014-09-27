#ifndef __expression_builder_hpp__
#define __expression_builder_hpp__

#include "expressions.hpp"
#include "identifiers.hpp"
#include "tokenizer.hpp"

#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <stack>
#include "helpers.hpp"

#include <cstdio>

namespace donkey{

namespace expression_builder{

enum class oper{
	comma                =   0,
	param_comma          =   1,
	or_assignment        =  20,
	xor_assignment       =  21,
	and_assignment       =  22,
	shiftr_assignment    =  23,
	shiftl_assignment    =  24,
	concat_assignment    =  25,
	minus_assignment     =  26,
	plus_assignment      =  27,
	mod_assignment       =  28,
	idiv_assignment      =  29,
	div_assignment       =  30,
	mul_assignment       =  31,
	assignment           =  32,
	conditional_question =  40,
	conditional_colon    =  41,
	logical_or           =  60,
	logical_and          =  80,
	unequal              = 100,
	equal                = 101,
	greater_equal        = 120,
	less_equal           = 121,
	greater              = 122,
	less                 = 123,
	shiftr               = 140,
	shiftl               = 141,
	bitwise_or           = 160,
	bitwise_xor          = 161,
	concat               = 162,
	minus                = 163,
	plus                 = 164,
	bitwise_and          = 165,
	mod                  = 180,
	idiv                 = 181,
	div                  = 182,
	mul                  = 183,
	logical_not          = 200,
	bitwise_not          = 201,
	unary_minus          = 202,
	unary_plus           = 203,
	pre_dec              = 204,
	pre_inc              = 205,
	post_dec             = 220,
	post_inc             = 221,
	
	bracket_open         = 1000,
	bracket_close        = 1001,
	bracket_f_open       = 1002,
	bracket_f_close      = 1003,
	
	none                 = 2000
};

enum class precedence{
	sequential_evaluation  =  0,
	assignment             =  1,
	conditional_expression =  2,
	logical_or             =  3,
	logical_and            =  4,
	equality               =  5,
	relational             =  6,
	bitwise_shift          =  7,
	additive               =  8,
	multiplicative         =  9,
	unary_prefix           = 10,
	unary_postfix          = 11,
	
	bracket                = 50
};

inline precedence get_precedence(oper op){
	return precedence(int(op)/20);
}

inline bool is_right_associative(precedence p){
	return p == precedence::unary_prefix || p == precedence::assignment || p == precedence::conditional_expression;
}

template<int i>
struct tlookup{
	static const std::pair<const char*, oper> string_to_oper[];
	static const size_t string_to_oper_size = sizeof(string_to_oper)/sizeof(string_to_oper[0]);
};

template<int i>
const std::pair<const char*, oper> tlookup<i>::string_to_oper[] = {
	{"!", oper::logical_not},
	{"!=", oper::unequal},
	{"%", oper::mod},
	{"%=", oper::mod_assignment},
	{"&", oper::bitwise_and},
	{"&&", oper::logical_and},
	{"&=", oper::and_assignment},
	{"(", oper::bracket_open},
	{")", oper::bracket_close},
	{"*", oper::mul},
	{"*=", oper::mul_assignment},
	{"+", oper::unary_plus},
	{"++", oper::pre_inc},
	{"+=", oper::plus_assignment},
	{",", oper::comma},
	{"-", oper::unary_minus},
	{"--", oper::pre_dec},
	{"-=", oper::minus_assignment},
	{"..", oper::concat},
	{"..=", oper::concat_assignment},
	{"/", oper::div},
	{"/=", oper::div_assignment},
	{":", oper::conditional_colon},
	{"<", oper::less},
	{"<<", oper::shiftl},
	{"<<=", oper::shiftl_assignment},
	{"<=", oper::less_equal},
	{"=", oper::assignment},
	{"==", oper::equal},
	{">", oper::greater},
	{">=", oper::greater_equal},
	{">>", oper::shiftr},
	{">>=", oper::shiftr_assignment},
	{"?", oper::conditional_question},
	{"\\", oper::idiv},
	{"\\=", oper::idiv_assignment},
	{"^", oper::bitwise_xor},
	{"^", oper::xor_assignment},
	{"|", oper::bitwise_or},
	{"|=", oper::or_assignment},
	{"||", oper::logical_or},
	{"~", oper::bitwise_not},
};

typedef tlookup<0> lookup;

inline oper string_to_oper(std::string s){
	auto it = std::lower_bound(
		lookup::string_to_oper,
		lookup::string_to_oper + lookup::string_to_oper_size,
		s.c_str(),
		[](const std::pair<const char*, oper>& l, const char* r){
			return strcmp(l.first, r) < 0;
	});
	if(it == lookup::string_to_oper + lookup::string_to_oper_size || strcmp(it->first, s.c_str())){
		return oper::none;
	}
	return it->second;
}

enum class part_type{
	opr,
	var,
	str,
	num,
	fun,
};

struct part{
	part_type type;
	std::string str;
	double d;
	oper o;
};

inline std::pair<tokenizer::token_type, std::string> get_next_token(tokenizer parser){
	++parser;
	return std::make_pair(parser.get_token_type(), *parser);
}

inline bool bigger_precedence(oper next, oper on_stack){
	if(on_stack == oper::conditional_question){
		return true;
	}
	precedence next_p = get_precedence(next);
	precedence on_stack_p = get_precedence(on_stack);
	
	if(next_p < on_stack_p){
		return false;
	}
	if(next_p > on_stack_p){
		return true;
	}
	
	return is_right_associative(next_p);
}

inline std::vector<part> parse_to_rpn(tokenizer& parser){
	struct stack_item{
		oper o;
		std::string f;
	};

	struct prev_token_type{
		oper o;
		tokenizer::token_type t;
	};

	std::vector<part> rpn;
	std::stack<stack_item> stack;
	std::stack<bool> function_call;
	function_call.push(false);
	
	prev_token_type prev_token{oper::none, tokenizer::tt_eof};
	
	int nest_level = 0;
	
	for(; parser && *parser != ";" && (nest_level > 0 || *parser != ")"); ++parser){
		switch(parser.get_token_type()){
			case tokenizer::tt_string:
				rpn.push_back(part{part_type::str, *parser, 0, oper::none});
				prev_token.o = oper::none;
				prev_token.t = tokenizer::tt_string;
				break;
			case tokenizer::tt_number:
				{
					double d = parse_double(*parser);
					if(isnan(d)){
						syntax_error(parser.get_line_number(), "invalid number constant");
					}
					rpn.push_back(part{part_type::num, "", d, oper::none});
				}
				prev_token.o = oper::none;
				prev_token.t = tokenizer::tt_number;
				break;
			case tokenizer::tt_operator:
				{
					oper op = string_to_oper(*parser);
					if(op == oper::none){
						syntax_error(parser.get_line_number(), "unexpected " + *parser);
					}else if(op == oper::pre_inc || op == oper::pre_dec){
						if(prev_token.t == tokenizer::tt_word ||
						   prev_token.o == oper::bracket_close || prev_token.o == oper::bracket_f_close ||
						   prev_token.o == oper::post_inc || prev_token.o == oper::post_dec){
							op = (op == oper::pre_inc ? oper::post_inc : oper::post_dec);
						}
					}else if(op == oper::unary_plus || op == oper::unary_minus){
						if(prev_token.t == tokenizer::tt_word || prev_token.t == tokenizer::tt_number ||
						   prev_token.o == oper::bracket_close || prev_token.o == oper::bracket_f_close ||
						   prev_token.o == oper::post_inc || prev_token.o == oper::post_dec){
							op = (op == oper::unary_plus ? oper::plus : oper::minus);
						}
					}else if(op == oper::comma){
						if(function_call.top()){
							op = oper::param_comma;
						}
					}else if(op == oper::bracket_close){
						if(function_call.top()){
							op = oper::bracket_f_close;
						}
					}
					
					if(op == oper::bracket_open){
						++nest_level;
						function_call.push(false);
						stack.push(stack_item{op, ""});
					}else if(op == oper::bracket_close){
						--nest_level;
						while(!stack.empty() && stack.top().o != oper::bracket_open){
							if(stack.top().o != oper::conditional_colon && stack.top().o != oper::param_comma){
								rpn.push_back(part{part_type::opr, "", 0, stack.top().o});
							}
							stack.pop();
						}
						if(stack.empty()){
							syntax_error(parser.get_line_number(), "unexpected )");
						}
						stack.pop();
						function_call.pop();
					}else if(op == oper::bracket_f_close){
						--nest_level;
						while(stack.top().f.empty()){
							if(stack.top().o != oper::conditional_colon && stack.top().o != oper::param_comma){
								rpn.push_back(part{part_type::opr, "", 0, stack.top().o});
							}
							stack.pop();
						}
						rpn.push_back(part{part_type::fun, stack.top().f, 0, oper::none});
						
						stack.pop();
						
						function_call.pop();
					}else{
						while(!stack.empty() && stack.top().o != oper::none && stack.top().o != oper::bracket_open && !bigger_precedence(op, stack.top().o)){
							if(stack.top().o != oper::conditional_colon && stack.top().o != oper::param_comma){
								rpn.push_back(part{part_type::opr, "", 0, stack.top().o});
							}
							stack.pop();
						}
						stack.push(stack_item{op, ""});
					}
					
					
					
					prev_token.o = op;
					prev_token.t = tokenizer::tt_operator;
				}
				
				break;
			default: //identifier
				{
					bool is_function = false;
					auto next_token = get_next_token(parser);
					if(next_token.first == tokenizer::tt_operator){
						oper op = string_to_oper(next_token.second);
						if(op == oper::bracket_open){
							is_function = true;
						}
					}
					if(is_function){
						stack.push(stack_item{oper::none, *parser});
						function_call.push(true);
						++parser;
						prev_token.o = oper::bracket_f_open;
						prev_token.t = tokenizer::tt_operator;
						++nest_level;
					}else{
						rpn.push_back(part{part_type::var, *parser, 0, oper::none});
						prev_token.o = oper::none;
						prev_token.t = tokenizer::tt_word;
					}
				}
				break;
		}
	}
	
	if(nest_level){
		syntax_error(parser.get_line_number(), "missing )");
	}
	
	while(!stack.empty()){
		if(stack.top().o != oper::conditional_colon && stack.top().o != oper::param_comma){
			rpn.push_back(part{part_type::opr, "", 0, stack.top().o});
		}
		stack.pop();
	}
	
	return rpn;
}

}//namespace expression_builder

inline void testRPN(const char* expression){
	tokenizer parser(expression, expression + strlen(expression));
	std::vector<expression_builder::part> rpn = expression_builder::parse_to_rpn(parser);
	
	for(const expression_builder::part& part: rpn){
		switch(part.type){
			case expression_builder::part_type::fun:
				printf("%s() ", part.str.c_str());
				break;
			case expression_builder::part_type::num:
				printf("%f ", part.d);
				break;
			case expression_builder::part_type::opr:
				printf("(%d) ", part.o);
				break;
			case expression_builder::part_type::var:
				printf("%s ", part.str.c_str());
				break;
			case expression_builder::part_type::str:
				printf("\"%s\" ", part.str.c_str());
				break;
		}
	}
	printf("\n");
}

inline expression_ptr build_expression(const identifier_lookup&, tokenizer& parser){
	std::vector<expression_builder::part> rpn = expression_builder::parse_to_rpn(parser);
	return expression_ptr();
}

}//namespace donkey

#endif /* __expression_builder_hpp__ */
