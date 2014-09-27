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
	none                 =   0,
	operand              =   1,
	bracket_open         =   2,
	bracket_close        =   3,

	comma                =  20,
	or_assignment        =  40,
	xor_assignment       =  41,
	and_assignment       =  42,
	shiftr_assignment    =  43,
	shiftl_assignment    =  44,
	concat_assignment    =  45,
	minus_assignment     =  46,
	plus_assignment      =  47,
	mod_assignment       =  48,
	idiv_assignment      =  49,
	div_assignment       =  50,
	mul_assignment       =  51,
	assignment           =  52,
	conditional_question =  60,
	conditional_colon    =  61,
	logical_or           =  80,
	logical_and          = 100,
	unequal              = 120,
	equal                = 121,
	greater_equal        = 140,
	less_equal           = 141,
	greater              = 142,
	less                 = 143,
	shiftr               = 160,
	shiftl               = 161,
	bitwise_or           = 180,
	bitwise_xor          = 181,
	concat               = 182,
	minus                = 183,
	plus                 = 184,
	bitwise_and          = 185,
	mod                  = 200,
	idiv                 = 201,
	div                  = 202,
	mul                  = 203,
	logical_not          = 220,
	bitwise_not          = 221,
	unary_minus          = 222,
	unary_plus           = 223,
	pre_dec              = 224,
	pre_inc              = 225,
	post_dec             = 240,
	post_inc             = 241,
};

enum class precedence{
	none                   =  0,
	
	sequential_evaluation  =  1,
	assignment             =  2,
	conditional_expression =  3,
	logical_or             =  4,
	logical_and            =  5,
	equality               =  6,
	relational             =  7,
	bitwise_shift          =  8,
	additive               =  9,
	multiplicative         = 10,
	unary_prefix           = 11,
	unary_postfix          = 12,
};

enum class operator_type{
	none          = 0,
	unary_prefix  = 1,
	unary_postfix = 2,
	binary        = 3,
	ternary       = 4,
	opening       = 5,
	closing       = 6,
};

inline precedence get_precedence(oper op){
	return precedence(int(op)/20);
}

inline bool is_right_associative(precedence p){
	return p == precedence::unary_prefix || p == precedence::assignment || p == precedence::conditional_expression;
}

inline operator_type get_operator_type(oper op){
	switch(get_precedence(op)){
		case precedence::none:
			switch(op){
				case oper::bracket_open:
					return operator_type::opening;
				case oper::bracket_close:
					return operator_type::closing;
				default:
					return operator_type::none;
			}
		case precedence::conditional_expression:
			return operator_type::ternary;
		case precedence::unary_postfix:
			return operator_type::unary_postfix;
		case precedence::unary_prefix:
			return operator_type::unary_prefix;
		default:
			return operator_type::binary;
	}
}

inline oper bin_post_variant(oper op){
	switch(op){
		case oper::pre_inc:
			return oper::post_inc;
		case oper::pre_dec:
			return oper::post_dec;
		case oper::unary_plus:
			return oper::plus;
		case oper::unary_minus:
			return oper::minus;
		default:
			return op;
	}
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
	nul,
	num,
	str,
	var,
	fun,
	opr,
};

struct part{
	part_type type;
	std::string str;
	double d;
	oper op;
};

inline std::pair<tokenizer::token_type, std::string> get_next_token(tokenizer parser){
	++parser;
	return std::make_pair(parser.get_token_type(), *parser);
}

inline bool is_right_operand(tokenizer parser, bool should_be){
	auto next = get_next_token(parser);
	switch(next.first){
		case tokenizer::tt_number:
		case tokenizer::tt_string:
		case tokenizer::tt_word:
			return true;
		case tokenizer::tt_operator:
			{
				oper op = string_to_oper(next.second);
				if(!should_be && (op == oper::pre_inc || op == oper::pre_dec || op == oper::unary_plus || op == oper::unary_minus)){
					return false;
				}
				operator_type ot = get_operator_type(op);
				return ot == operator_type::unary_prefix || ot == operator_type::opening;
			}
		default:
			return false;
	}
}

inline bool bigger_precedence(oper next, oper on_stack){
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

inline bool is_opening(std::string str){
	return str == "(" || str == "?";
}

inline bool is_closing(std::string str){
	return str == "" || str == ";" || str == ")" || str == ":";
}

inline bool matching_brackets(oper opening, oper closing){
	switch(opening){
		case oper::bracket_open:
			return closing == oper::bracket_close;
		case oper::conditional_question:
			return closing == oper::conditional_colon;
		default:
			return false;
	}
}

inline double parse_number(tokenizer& parser){
	double ret = parse_double(*parser);
	if(isnan(ret)){
		syntax_error(parser.get_line_number(), "invalid number constant");
	}
	return ret;
}

inline int consume_stack(oper op, std::stack<oper>& stack, std::vector<part>& rpn, bool function_call){
	int ret = 0;
	while(!stack.empty() && !bigger_precedence(op, stack.top())){
		if(function_call && stack.top() == oper::comma){
			++ret;
		}else{
			rpn.push_back(part{part_type::opr, "", 0, stack.top()});
		}
		stack.pop();
	}
	return ret;
}

inline void check_left_operand(bool is_left_operand, tokenizer& parser, bool should_be){
	if(is_left_operand != should_be){
		unexpected_error(parser.get_line_number(), *parser);
	}
}

inline void check_right_operand(tokenizer& parser, bool should_be){
	if(is_right_operand(parser, should_be) != should_be){
		tokenizer tmp = parser;
		++tmp;
		unexpected_error(tmp.get_line_number(), *tmp);
	}
}

inline int parse_to_rpn(std::vector<part>& rpn, tokenizer& parser, bool function_call){
	std::stack<oper> stack;
	
	int number_of_params = (function_call && !is_closing(*parser)) ? 1 : 0;
	
	bool is_left_operand = false;
	
	std::string f;
	
	for(; !is_closing(*parser); ++parser){
		if(is_opening(*parser)){
			oper opening = string_to_oper(*parser);
			
			if(opening == oper::conditional_question){
				check_left_operand(is_left_operand, parser, true);
				number_of_params += consume_stack(oper::conditional_question, stack, rpn, function_call);
			}else{
				check_left_operand(is_left_operand, parser, false);
			}
			
			++parser;
			size_t sz = rpn.size();
			int prms = parse_to_rpn(rpn, parser, !f.empty());
			
			oper closing = string_to_oper(*parser);
			
			if(!matching_brackets(opening, closing)){
				unexpected_error(parser.get_line_number(), *parser);
			}
			if(sz == rpn.size() && f.empty()){
				unexpected_error(parser.get_line_number(), *parser);
			}
			
			if(!f.empty()){
				rpn.push_back(part{part_type::fun, f, (double)prms, oper::none});
			}
			
			if(closing == oper::conditional_colon){
				check_right_operand(parser, true);
				stack.push(oper::conditional_question);
				is_left_operand = false;
			}else{
				check_right_operand(parser, false);
				is_left_operand = true;
			}
		}else{
			switch(parser.get_token_type()){
				case tokenizer::tt_number:
					check_left_operand(is_left_operand, parser, false);
					rpn.push_back(part{part_type::num, "", parse_number(parser), oper::none});
					is_left_operand = true;
					break;
				case tokenizer::tt_string:
					check_left_operand(is_left_operand, parser, false);
					rpn.push_back(part{part_type::str, *parser, 0, oper::none});
					is_left_operand = true;
					break;
				case tokenizer::tt_word:
					check_left_operand(is_left_operand, parser, false);
					if(get_next_token(parser).second == "("){
						f = *parser;
					}else if(*parser == "null"){
						rpn.push_back(part{part_type::nul, "", 0, oper::none});
						is_left_operand = true;
					}else{
						rpn.push_back(part{part_type::var, *parser, 0, oper::none});
						is_left_operand = true;
					}
					break;
				case tokenizer::tt_operator:
					{
						oper op = string_to_oper(*parser);
						
						if(is_left_operand){
							op = bin_post_variant(op);
						}
						
						operator_type ot = get_operator_type(op);
						
						switch(ot){
							case operator_type::unary_prefix:
								check_left_operand(is_left_operand, parser, false);
								check_right_operand(parser, true);
								break;
							case operator_type::unary_postfix:
								check_left_operand(is_left_operand, parser, true);
								check_right_operand(parser, false);
								break;
							case operator_type::binary:
								check_left_operand(is_left_operand, parser, true);
								check_right_operand(parser, true);
								break;
							default:
								unexpected_error(parser.get_line_number(), *parser);
						}
						
						number_of_params += consume_stack(op, stack, rpn, function_call);
						stack.push(op);
						
						is_left_operand = (ot == operator_type::unary_postfix);
					}
					break;
				default:
					break;
			}
		}
	}
	
	number_of_params += consume_stack(oper::none, stack, rpn, function_call);
	
	return number_of_params;
}

inline expression_ptr identifier_to_expression(std::string str, const identifier_lookup& lookup){
	identifier_ptr id = lookup.get_identifier(str);
	switch(id->get_type()){
		case identifier_type::function:
			return expression_ptr(new const_function_expression(std::static_pointer_cast<function_identifier>(id)->get_function()));
		case identifier_type::global_variable:
			return expression_ptr(new global_variable_expression(std::static_pointer_cast<global_variable_identifier>(id)->get_index()));
		case identifier_type::local_variable:
			return expression_ptr(new local_variable_expression(std::static_pointer_cast<local_variable_identifier>(id)->get_index()));
		default:
			return expression_ptr();
	}
}

inline bool build_next(part p, std::stack<expression_ptr> stack, const identifier_lookup& lookup){
	switch(p.type){
		case part_type::nul:
			stack.push(expression_ptr(new null_expression()));
			return true;
		case part_type::num:
			stack.push(expression_ptr(new const_number_expression(p.d)));
			return true;
		case part_type::str:
			stack.push(expression_ptr(new const_string_expression(p.str)));
			return true;
		case part_type::var:
			{
				expression_ptr e = identifier_to_expression(p.str, lookup);
				if(e){
					stack.push(e);
					return true;
				}
				return false;
			}
		case part_type::fun:
			{
				expression_ptr e = identifier_to_expression(p.str, lookup);
				if(e){
					std::vector<expression_ptr> params((int)p.d);
					for(int i = (int)p.d - 1; i >= 0; --i){
						if(stack.empty()){
							return false;
						}
						params[i] = stack.top();
						stack.pop();
					}
					stack.push(expression_ptr(new function_call_expression(e, params)));
					return true;
				}
				return false;
			}
		default:
			return false;
			
	}
}

}//namespace expression_builder

inline const char* oper_to_str(expression_builder::oper op){
	switch(op){
		case expression_builder::oper::none:
			return "ERR N";
		case expression_builder::oper::operand:
			return "ERR O";
		case expression_builder::oper::bracket_open:
			return "ERR (";
		case expression_builder::oper::bracket_close:
			return "ERR )";
		case expression_builder::oper::comma:
			return ",";
		case expression_builder::oper::or_assignment:
			return "|=";
		case expression_builder::oper::xor_assignment:
			return "^=";
		case expression_builder::oper::and_assignment:
			return "&=";
		case expression_builder::oper::shiftr_assignment:
			return ">>=";
		case expression_builder::oper::shiftl_assignment:
			return "<<=";
		case expression_builder::oper::concat_assignment:
			return "..=";
		case expression_builder::oper::minus_assignment:
			return "-=";
		case expression_builder::oper::plus_assignment:
			return "+=";
		case expression_builder::oper::mod_assignment:
			return "%=";
		case expression_builder::oper::idiv_assignment:
			return "\\=";
		case expression_builder::oper::div_assignment:
			return "/=";
		case expression_builder::oper::mul_assignment:
			return "*=";
		case expression_builder::oper::assignment:
			return "=";
		case expression_builder::oper::conditional_question:
			return "?";
		case expression_builder::oper::conditional_colon:
			return ":";
		case expression_builder::oper::logical_or:
			return "||";
		case expression_builder::oper::logical_and:
			return "&&";
		case expression_builder::oper::unequal:
			return "!=";
		case expression_builder::oper::equal:
			return "==";
		case expression_builder::oper::greater_equal:
			return ">=";
		case expression_builder::oper::less_equal:
			return "<=";
		case expression_builder::oper::greater:
			return ">";
		case expression_builder::oper::less:
			return "<";
		case expression_builder::oper::shiftr:
			return ">>";
		case expression_builder::oper::shiftl:
			return "<<";
		case expression_builder::oper::bitwise_or:
			return "|";
		case expression_builder::oper::bitwise_xor:
			return "^";
		case expression_builder::oper::concat:
			return "..";
		case expression_builder::oper::minus:
			return "-";
		case expression_builder::oper::plus:
			return "+";
		case expression_builder::oper::bitwise_and:
			return "&";
		case expression_builder::oper::mod:
			return "%";
		case expression_builder::oper::idiv:
			return "\\";
		case expression_builder::oper::div:
			return "/";
		case expression_builder::oper::mul:
			return "*";
		case expression_builder::oper::logical_not:
			return "!";
		case expression_builder::oper::bitwise_not:
			return "~";
		case expression_builder::oper::unary_minus:
			return "-0";
		case expression_builder::oper::unary_plus:
			return "+0";
		case expression_builder::oper::pre_dec:
			return "--0";
		case expression_builder::oper::pre_inc:
			return "++0";
		case expression_builder::oper::post_dec:
			return "0--";
		case expression_builder::oper::post_inc:
			return "0++";
		default:
			return "ERR";
	}
}

inline void testRPN(const char* expression){
	tokenizer parser(expression, expression + strlen(expression));
	std::vector<expression_builder::part> rpn;
	expression_builder::parse_to_rpn(rpn, parser, false);
	
	for(const expression_builder::part& part: rpn){
		switch(part.type){
			case expression_builder::part_type::fun:
				printf("%s(%d) ", part.str.c_str(), int(part.d));
				break;
			case expression_builder::part_type::num:
				printf("%f ", part.d);
				break;
			case expression_builder::part_type::opr:
				printf("%s ", oper_to_str(part.op));
				break;
			case expression_builder::part_type::var:
				printf("%s ", part.str.c_str());
				break;
			case expression_builder::part_type::str:
				printf("\"%s\" ", part.str.c_str());
				break;
			case expression_builder::part_type::nul:
				printf("(null) ");
				break;
		}
	}
	printf("\n");
}

inline expression_ptr build_expression(const identifier_lookup& lookup, tokenizer& parser, bool can_be_empty){
	std::vector<expression_builder::part> rpn;
	expression_builder::parse_to_rpn(rpn, parser, false);
	if(rpn.empty()){
		if(can_be_empty){
			return expression_ptr(new null_expression());
		}
		unexpected_error(parser.get_line_number(), *parser);
	}
	std::stack<expression_ptr> expressions;
	for(const auto& part: rpn){
		if(!expression_builder::build_next(part, expressions, lookup)){
			syntax_error(parser.get_line_number(), "invalid expression");
		}
	}
	if(expressions.size() == 1){
		return expressions.top();
	}
	syntax_error(parser.get_line_number(), "invalid expression");
}

}//namespace donkey

#endif /* __expression_builder_hpp__ */
