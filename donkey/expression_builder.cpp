#include "expression_builder.hpp"

#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <stack>
#include "helpers.hpp"

namespace donkey{


enum class precedence{
	none                   =  0,
	
	sequential_evaluation  =  1,
	assignment             =  2,
	conditional_expression =  3,
	fallback               =  4,
	logical_or             =  5,
	logical_and            =  6,
	equality               =  7,
	relational             =  8,
	bitwise_shift          =  9,
	additive               = 10,
	multiplicative         = 11,
	unary_prefix           = 12,
	post_dot_call          = 13,
	scope                  = 14,
};

enum class operator_type{
	none          = 0,
	unary_prefix  = 1,
	unary_postfix = 2,
	binary        = 3,
	ternary       = 4,
	opening       = 5,
	closing       = 6,
	call          = 7,
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
		case precedence::post_dot_call:
			switch(op){
				case oper::post_dec:
				case oper::post_inc:
					return operator_type::unary_postfix;
				case oper::call:
					return operator_type::call;
				default:
					return operator_type::binary;
			}
		case precedence::unary_prefix:
			return operator_type::unary_prefix;
		default:
			return operator_type::binary;
	}
}

inline int get_number_of_operands(oper op){
	switch(get_operator_type(op)){
		case operator_type::unary_prefix:
		case operator_type::unary_postfix:
			return 1;
		case operator_type::binary:
			return 2;
		case operator_type::ternary:
			return 3;
		default:
			return 0;
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
			return oper::bitwise_and;
		default:
			return get_precedence(op) == precedence::unary_prefix ? oper::none : op;
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
	{".", oper::dot},
	{"..", oper::concat},
	{"..=", oper::concat_assignment},
	{"/", oper::div},
	{"/=", oper::div_assignment},
	{":", oper::conditional_colon},
	{"::", oper::scope},
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
	{"?\?", oper::fallback},
	{"?\?=", oper::fallback_assignment},
	{"\\", oper::idiv},
	{"\\=", oper::idiv_assignment},
	{"^", oper::bitwise_xor},
	{"^=", oper::xor_assignment},
	{"new", oper::construct},
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

inline bool is_opening(const tokenizer& parser){
	return *parser == "(" || *parser == "?";
}

inline bool is_closing(const tokenizer& parser, bool declaration){
	if(!parser){
		return true;
	}
	return *parser == "" || *parser == ";" || *parser == ")" || *parser == ":" || (declaration && *parser == ",");
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

inline void check_left_operand(bool is_left_operand, tokenizer& parser, bool should_be){
	if(is_left_operand != should_be){
		unexpected_error(parser.get_line_number(), *parser);
	}
}

struct expression_part;

typedef std::shared_ptr<expression_part> part_ptr;

struct expression_part{
	oper op;
	std::string str;
	part_ptr first_child;
	part_ptr next_sibling;
};

static void consume_stack(oper op, int line_number, std::stack<oper>& stack, std::vector<part_ptr>& parts){
	while(!stack.empty() && !bigger_precedence(op, stack.top())){
		part_ptr first_child;
		int number_of_operands = get_number_of_operands(stack.top());
		for(int i = 0; i < number_of_operands; ++i){
			if(parts.empty()){
				semantic_error(line_number, "invalid expression");
			}
			parts.back()->next_sibling = first_child;
			first_child = parts.back();
			parts.pop_back();
		}
		if(first_child){
			parts.push_back(part_ptr(new expression_part{stack.top(), "", first_child, part_ptr()}));
		}else{
			syntax_error(line_number, "invalid expression");
		}
		stack.pop();
	}
}

static part_ptr create_expression_tree(tokenizer& parser, bool declaration, bool can_be_empty){
	if(is_closing(parser, declaration)){
		if(can_be_empty){
			return part_ptr();
		}
		unexpected_error(parser.get_line_number(), *parser);
		return part_ptr();
	}
	
	std::vector<part_ptr> parts;
	
	std::stack<oper> stack;
	
	bool is_left_operand = false;
	
	for(; !is_closing(parser, declaration); ++parser){
		if(is_opening(parser)){
			oper opening = string_to_oper(*parser);
			bool function_call = false;
			
			if(opening == oper::conditional_question){
				check_left_operand(is_left_operand, parser, true);
				consume_stack(oper::conditional_question, parser.get_line_number(), stack, parts);
			}else if(opening == oper::bracket_open){
				function_call = is_left_operand;
			}
			
			++parser;
			
			part_ptr inner;
			
			if(function_call){
				consume_stack(oper::call, parser.get_line_number(), stack, parts);
				
				inner = part_ptr(new expression_part{oper::call, "", parts.back(), part_ptr()});
				
				parts.pop_back();
				
				part_ptr last_param = inner->first_child;
				
				while(!is_closing(parser, false)){
					bool byref = false;
					if(*parser == "ref"){
						byref = true;
						++parser;
					}
					part_ptr param =  create_expression_tree(parser, true, false);
					
					if(*parser == ","){
						++parser;
					}
					
					if(byref){
						param = part_ptr(new expression_part{oper::ref, "", param, part_ptr()});
					}
					
					last_param->next_sibling = param;
					
					last_param = param;
				}
			}else{
				inner = create_expression_tree(parser, false, false);
			}
			
			oper closing = string_to_oper(*parser);
			
			if(!matching_brackets(opening, closing)){
				unexpected_error(parser.get_line_number(), *parser);
			}
			
			parts.push_back(inner);
			
			if(opening == oper::conditional_question){
				stack.push(oper::conditional_question);
				is_left_operand = false;
			}else{
				is_left_operand = true;
			}
		}else{
			switch(parser.get_token_type()){
				case tokenizer::tt_number:
				case tokenizer::tt_string:
				case tokenizer::tt_word:
					check_left_operand(is_left_operand, parser, false);
					parts.push_back(part_ptr(new expression_part{oper::none, *parser, part_ptr(), part_ptr()}));
					is_left_operand = true;
					break;
				case tokenizer::tt_operator:
					{
						oper op = string_to_oper(*parser);
						
						if(is_left_operand){
							op = bin_post_variant(op);
						}else if(get_operator_type(op) != operator_type::unary_prefix){
							unexpected_error(parser.get_line_number(), *parser);
						}
						
						if(op == oper::none){
							unexpected_error(parser.get_line_number(), *parser);
						}
						
						consume_stack(op, parser.get_line_number(), stack, parts);
						stack.push(op);
						
						is_left_operand = (get_operator_type(op) == operator_type::unary_postfix);
					}
					break;
				default:
					break;
			}
		}
	}
	
	if(!is_left_operand){
		unexpected_error(parser.get_line_number(), *parser);
	}
	
	consume_stack(oper::none, parser.get_line_number(), stack, parts);
	
	if(parts.size() != 1){
		syntax_error(parser.get_line_number(), "invalid expression");
	}
	
	return parts.front();
}

static expression_ptr str_to_expression(const std::string& str, const identifier_lookup& lookup, int line_number){
	if(isdigit(str.front())){
		double n = parse_number(str);
		if(isnan(n)){
			syntax_error(line_number, "invalid number constant");
		}
		return build_const_number_expression(n);
	}
	if(str.front() == '"'){
		return build_const_string_expression(tokenizer::unquoted_string(str, line_number));
	}
	
	if(str == "null"){
		return build_null_expression();
	}
	
	if(str == "this"){
		if(!lookup.in_class()){
			semantic_error(line_number, "this is only allowed in class methods");
		}
		return build_this_expression();
	}
	
	identifier_ptr id = lookup.get_identifier(str);
	
	if(!id){
		semantic_error(line_number, "unknown identifier " + str);
	}
	
	switch(id->get_type()){
		case identifier_type::global_variable:
			return build_global_variable_expression(static_cast<global_variable_identifier&>(*id).get_index());
		case identifier_type::local_variable:
			return build_local_variable_expression(static_cast<local_variable_identifier&>(*id).get_index());
		case identifier_type::parameter:
			return build_parameter_expression(static_cast<parameter_identifier&>(*id).get_index());
		case identifier_type::function:
			return build_const_function_expression(static_cast<function_identifier&>(*id).get_function());
		case identifier_type::classname:
			semantic_error(line_number, "unexpected class " + str);
			break;
		case identifier_type::module:
			semantic_error(line_number, "unexpected module " + str);
	}
	
	return expression_ptr();
}

static expression_ptr tree_to_expression(part_ptr tree, const identifier_lookup& lookup, int line_number);

static void fetch_params(part_ptr head, const identifier_lookup& lookup, std::vector<expression_ptr>& params, std::vector<size_t>& byref, int line_number){
	for(part_ptr p = head->next_sibling; p; p = p->next_sibling){
		if(p->op == oper::ref){
			byref.push_back(params.size());
			if(p->first_child->op != oper::none || p->first_child->first_child || p->first_child->next_sibling){
				syntax_error(line_number, "only variables can be passed by reference");
			}
			identifier_ptr id = lookup.get_identifier(p->first_child->str);
			
			expression_ptr e;
			switch(id->get_type()){
				case identifier_type::global_variable:
					e =  build_global_variable_expression(static_cast<global_variable_identifier&>(*id).get_index());
					break;
				case identifier_type::local_variable:
					e =  build_local_variable_expression(static_cast<global_variable_identifier&>(*id).get_index());
					break;
				case identifier_type::parameter:
					e =  build_parameter_expression(static_cast<global_variable_identifier&>(*id).get_index());
					break;
				default:
					semantic_error(line_number, "only variables can be passed by reference");
					break;
			}
			
			params.push_back(e);
		}else{
			params.push_back(tree_to_expression(p, lookup, line_number));
		}
	}
}

bool is_keyword(const std::string& name); //donkey.cpp

static std::pair<std::string, std::string> get_member_name(part_ptr tree, const identifier_lookup& lookup, bool self, int line_number){
	std::string classname;
	if(self){
		classname = lookup.get_current_class();
	}else if(tree->op == oper::scope){
		if(!lookup.has_class(tree->first_child->str)){
			semantic_error(line_number, "unknown class name " + tree->first_child->str);
		}
		classname = tree->first_child->str;
		tree = tree->first_child->next_sibling;
	}
	
	std::string membername = tree->str;
	if(membername.empty() || membername[0] == '"' || isdigit(membername[0]) || is_keyword(membername)){
		semantic_error(line_number, "invalid member name");
	}
	return std::pair<std::string, std::string>(classname, membername);
}

static expression_ptr tree_to_expression(part_ptr tree, const identifier_lookup& lookup, int line_number){
	if(!tree){
		return expression_ptr();
	}
	switch(tree->op){
		case oper::none:
			return str_to_expression(tree->str, lookup, line_number);
		case oper::construct:
			{
				if(tree->first_child->op != oper::call){
					syntax_error(line_number, "invalid constructor call");
				}
				
				part_ptr f = tree->first_child->first_child;
				
				identifier_ptr classname = lookup.get_identifier(f->str);
				
				if(!classname){
					semantic_error(line_number, "unknown identifier " + f->str);
				}
				if(classname->get_type() != identifier_type::classname){
					semantic_error(line_number, "cannot construct " + f->str);
				}
				
				std::string name = static_cast<class_identifier&>(*classname).get_name();
				
				if(name == "object" || name == "number" || name == "string" || name == "function" || name == "null"){
					semantic_error(line_number, "cannot construct " + name);
				}
				
				
				std::vector<expression_ptr> params;
				std::vector<size_t> byref;
				
				fetch_params(f, lookup, params, byref, line_number);
				
				if(!byref.empty()){
					syntax_error(line_number, "cannot pass constructor parameters as reference");
				}
				return build_constructor_call_expression(name, params);
			}
			break;
		case oper::dot:
			{
				expression_ptr that;
				bool self = false;
				if(tree->first_child->str == "self"){
					if(!lookup.in_class()){
						semantic_error(line_number, "self is only allowed inside class");
					}
					self = true;
					that = build_this_expression();
				}else{
					that = tree_to_expression(tree->first_child, lookup, line_number);
				}
				
				std::pair<std::string, std::string> member = get_member_name(tree->first_child->next_sibling, lookup, self, line_number);
				
				if(member.first.empty()){
					return build_member_expression(that, member.second);
				}else{
					vtable* vt = lookup.get_vtable(member.first);
					
					if(vt->has_method(member.second)){
						return build_method_expression(that, member.first, *(vt->get_method(member.second)));
					}
					if(vt->has_field(member.second)){
						return build_field_expression(that, member.first, vt->get_field_index(member.second));
					}
					
					semantic_error(line_number, "class " + member.first + " doesn't have field " + member.second);
					
					return expression_ptr();
				}
				
			}
		case oper::call:
			{
				expression_ptr f = tree_to_expression(tree->first_child, lookup, line_number);
				std::vector<expression_ptr> params;
				std::vector<size_t> byref;
				
				fetch_params(tree->first_child, lookup, params, byref, line_number);
				
				return build_function_call_expression(f, params, byref);
			}
			break;
		default:
			switch(get_number_of_operands(tree->op)){
				case 1:
					return build_unary_expression(
						tree->op,
						tree_to_expression(tree->first_child, lookup, line_number),
						line_number
					);
				case 2:
					return build_binary_expression(
						tree->op,
						tree_to_expression(tree->first_child, lookup, line_number),
						tree_to_expression(tree->first_child->next_sibling, lookup, line_number),
						line_number
					);
				case 3:
					return build_ternary_expression(
						tree->op,
						tree_to_expression(tree->first_child, lookup, line_number),
						tree_to_expression(tree->first_child->next_sibling, lookup, line_number),
						tree_to_expression(tree->first_child->next_sibling->next_sibling, lookup, line_number),
						line_number
					);
				default:
					semantic_error(line_number, "invalid expression");
			}
	}
	return expression_ptr();
}

expression_ptr build_expression(const identifier_lookup& lookup, tokenizer& parser, bool can_be_empty, bool declaration){
	int line_number = parser.get_line_number();
	
	
	part_ptr tree = create_expression_tree(parser, declaration, can_be_empty);

	expression_ptr ret = tree_to_expression(tree, lookup, line_number);

	if(!ret){
		if(can_be_empty){
			return build_null_expression();
		}
		unexpected_error(parser.get_line_number(), *parser);
	}
	
	return ret;
}

}//donkey


