#include "expression_builder.hpp"

#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <stack>
#include "helpers.hpp"
#include "compiler.hpp"

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
				case oper::subscript_open:
					return operator_type::opening;
				case oper::bracket_close:
				case oper::subscript_close:
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
		case oper::deref:
			return oper::mul;
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
	{"*", oper::deref},
	{"*=", oper::mul_assignment},
	{"+", oper::unary_plus},
	{"++", oper::pre_inc},
	{"+=", oper::plus_assignment},
	{",", oper::comma},
	{"-", oper::unary_minus},
	{"--", oper::pre_dec},
	{"-=", oper::minus_assignment},
	{"->", oper::arrow},
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
	{"[", oper::subscript_open},
	{"\\", oper::idiv},
	{"\\=", oper::idiv_assignment},
	{"]", oper::subscript_close},
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
	return *parser == "(" || *parser == "?" || *parser == "[";
}

inline bool is_closing(const tokenizer& parser, bool declaration){
	if(!parser){
		return true;
	}
	return *parser == "" || *parser == ";" || *parser == ")" || *parser == "]" || *parser == ":" || (declaration && *parser == ",");
}

inline bool matching_brackets(oper opening, oper closing){
	switch(opening){
		case oper::bracket_open:
			return closing == oper::bracket_close;
		case oper::conditional_question:
			return closing == oper::conditional_colon;
		case oper::subscript_open:
			return closing == oper::subscript_close;
		default:
			return false;
	}
}

inline void check_left_operand(bool is_left_operand, tokenizer& parser, bool should_be){
	if(is_left_operand != should_be){
		unexpected_error(*parser);
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

static void consume_stack(oper op, std::stack<oper>& stack, std::vector<part_ptr>& parts){
	while(!stack.empty() && !bigger_precedence(op, stack.top())){
		part_ptr first_child;
		int number_of_operands = get_number_of_operands(stack.top());
		for(int i = 0; i < number_of_operands; ++i){
			if(parts.empty()){
				semantic_error("invalid expression");
			}
			parts.back()->next_sibling = first_child;
			first_child = parts.back();
			parts.pop_back();
		}
		if(first_child){
			parts.push_back(part_ptr(new expression_part{stack.top(), "", first_child, part_ptr()}));
		}else{
			syntax_error("invalid expression");
		}
		stack.pop();
	}
}

static part_ptr create_expression_tree(tokenizer& parser, bool declaration, bool can_be_empty){
	if(is_closing(parser, declaration)){
		if(can_be_empty){
			return part_ptr();
		}
		unexpected_error(*parser);
		return part_ptr();
	}
	
	std::vector<part_ptr> parts;
	
	std::stack<oper> stack;
	
	bool is_left_operand = false;
	
	for(; !is_closing(parser, declaration); ++parser){
		if(is_opening(parser)){
			oper opening = string_to_oper(*parser);
			bool function_call = false;
			bool subscript = false;
			bool array = false;
			
			if(opening == oper::conditional_question){
				check_left_operand(is_left_operand, parser, true);
				consume_stack(oper::conditional_question, stack, parts);
			}else if(opening == oper::bracket_open){
				consume_stack(oper::call, stack, parts);
				function_call = is_left_operand;
			}else if(opening == oper::subscript_open){
				consume_stack(oper::subscript, stack, parts);
				subscript = is_left_operand;
				if(!subscript){
					array = true;	
					parts.push_back(part_ptr(new expression_part{oper::none, "array", part_ptr(), part_ptr()}));
				}
			}
			
			++parser;
			
			part_ptr inner;
			
			if(function_call || array){
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
				
				if(array){
					inner = part_ptr(new expression_part{oper::array_init, "", inner, part_ptr()});
				}
			}else if(subscript){
				inner = part_ptr(new expression_part{oper::subscript, "", parts.back(), part_ptr()});
				parts.pop_back();
				inner->first_child->next_sibling = create_expression_tree(parser, false, false);
			}else{
				inner = create_expression_tree(parser, false, false);
			}
			
			oper closing = string_to_oper(*parser);
			
			if(!matching_brackets(opening, closing)){
				unexpected_error(*parser);
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
							unexpected_error(*parser);
						}
						
						if(op == oper::none){
							unexpected_error(*parser);
						}
						
						consume_stack(op, stack, parts);
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
		unexpected_error(*parser);
	}
	
	consume_stack(oper::none, stack, parts);
	
	if(parts.size() != 1){
		syntax_error("invalid expression");
	}
	
	return parts.front();
}

static expression_ptr identifier_to_expression(identifier_ptr id){
	switch(id->get_type()){
		case identifier_type::global_variable:
			{
				return build_global_variable_expression(
					static_cast<global_variable_identifier&>(*id).get_module_index(),
					static_cast<global_variable_identifier&>(*id).get_var_index()
				);
			}
		case identifier_type::local_variable:
			return build_local_variable_expression(static_cast<local_variable_identifier&>(*id).get_index());
		case identifier_type::function:
			return build_const_function_expression(static_cast<function_identifier&>(*id).get_function());
		case identifier_type::classname:
			semantic_error("unexpected class " + id->get_name());
			break;
		case identifier_type::module:
			semantic_error("unexpected module " + id->get_name());
		case identifier_type::method:
			semantic_error("unexpected method" + id->get_name());
		case identifier_type::field:
			semantic_error("unexpected field" + id->get_name());
	}
	
	return expression_ptr();
}

static expression_ptr str_to_expression(const std::string& str, const identifier_lookup& lookup){
	if(isdigit(str.front())){
		double n = parse_number(str);
		if(isnan(n)){
			syntax_error("invalid number constant");
		}
		return build_const_number_expression(n);
	}
	if(str.front() == '"'){
		return build_const_string_expression(tokenizer::unquoted_string(str));
	}
	
	if(str == "null"){
		return build_null_expression();
	}
	
	if(str == "this"){
		if(!lookup.in_class()){
			semantic_error("this is only allowed in class methods");
		}
		return build_this_expression();
	}
	
	identifier_ptr id = lookup.get_identifier(str);
	
	if(!id){
		semantic_error("unknown identifier " + str);
	}
	
	return identifier_to_expression(id);
}

static expression_ptr tree_to_expression(part_ptr tree, const identifier_lookup& lookup);

static void fetch_params(part_ptr head, const identifier_lookup& lookup, std::vector<expression_ptr>& params, std::vector<size_t>& byref){
	for(part_ptr p = head->next_sibling; p; p = p->next_sibling){
		if(p->op == oper::ref){
			byref.push_back(params.size());
			if(p->first_child->op != oper::none || p->first_child->first_child || p->first_child->next_sibling){
				syntax_error("only variables can be passed by reference");
			}
			identifier_ptr id = lookup.get_identifier(p->first_child->str);
			
			expression_ptr e;
			switch(id->get_type()){
				case identifier_type::global_variable:
					{
						e = build_global_variable_expression(
							static_cast<global_variable_identifier&>(*id).get_module_index(),
							static_cast<global_variable_identifier&>(*id).get_var_index()
						);
					}
					break;
				case identifier_type::local_variable:
					e =  build_local_variable_expression(static_cast<local_variable_identifier&>(*id).get_index());
					break;
				default:
					semantic_error("only variables can be passed by reference");
					break;
			}
			
			params.push_back(e);
		}else{
			params.push_back(tree_to_expression(p, lookup));
		}
	}
}

static identifier_ptr tree_to_identifier(part_ptr tree, const identifier_lookup& lookup){
	switch(tree->op){
		case oper::none:
			{
				identifier_ptr ret = lookup.get_identifier(tree->str);
				if(!ret){
					semantic_error("unknown identifier " + tree->str);
				}
				return ret;
			}
		case oper::scope:
			{
				identifier_ptr l = tree_to_identifier(tree->first_child, lookup);
				switch(l->get_type()){
					case identifier_type::global_variable:
					case identifier_type::local_variable:
					case identifier_type::method:
					case identifier_type::field:
					case identifier_type::function:
						semantic_error(l->get_name() + " is not module nor class");
						return identifier_ptr();
					case identifier_type::module:
						return tree_to_identifier(tree->first_child->next_sibling, static_cast<module_identifier&>(*l).get_lookup());
					case identifier_type::classname:
					{
						class_identifier& c = static_cast<class_identifier&>(*l);
						std::string r = tree->first_child->next_sibling->str;
						if(c.get_vtable()->has_method(r)){
							return identifier_ptr(new method_identifier(r, *(c.get_vtable()->get_method(r)), c.get_vtable()->get_full_name()));
						}else if(c.get_vtable()->has_field(r)){
							return identifier_ptr(new field_identifier(r, c.get_vtable()->get_field_index(r), c.get_vtable()->get_full_name()));
						}else{
							semantic_error("class " + c.get_vtable()->get_full_name() + " doesn't have member " + r);
							return identifier_ptr();
						}
					}
				}		
			}
			break;
		default:
			syntax_error("invalid expression");
	}
	return identifier_ptr();
}

static expression_ptr tree_to_expression(part_ptr tree, const identifier_lookup& lookup){
	if(!tree){
		return expression_ptr();
	}
	switch(tree->op){
		case oper::none:
			return str_to_expression(tree->str, lookup);
		case oper::construct:
			{
				
				if(tree->first_child->op != oper::call){
					syntax_error("invalid constructor call");
				}
				
				identifier_ptr id = tree_to_identifier(tree->first_child->first_child, lookup);
				
				std::string name = id->get_name();
				
				if(id->get_type() != identifier_type::classname ||
				   name == "object" || name == "number" || name == "string" || name == "function" || name == "null"){
					semantic_error("cannot construct " + name);
				}
				
				std::vector<expression_ptr> params;
				std::vector<size_t> byref;
				
				fetch_params(tree->first_child->first_child, lookup, params, byref);
				
				if(!byref.empty()){
					syntax_error("cannot pass constructor parameters as reference");
				}
				return build_constructor_call_expression(static_cast<class_identifier&>(*id).get_vtable(), params);
				
			}
			break;
		case oper::array_init:
			{
				std::vector<expression_ptr> items;
				std::vector<size_t> byref;
				
				fetch_params(tree->first_child->first_child, lookup, items, byref);
				
				if(!byref.empty()){
					syntax_error("cannot pass array item as reference");
				}
				
				return build_array_initializer(items);
			}
		case oper::dot:
		case oper::arrow:
			{
				expression_ptr that;
				bool self = false;
				if(tree->first_child->str == "self"){
					if(tree->op == oper::arrow){
						unexpected_error("->");
					}
					if(!lookup.in_class()){
						semantic_error("self is only allowed inside class");
					}
					self = true;
					that = build_this_expression();
				}else{
					that = tree_to_expression(tree->first_child, lookup);
					if(tree->op == oper::arrow){
						that = build_unary_expression(oper::deref, that);
					}
				}
				
				if(self){
					if(!lookup.in_class()){
						semantic_error("self is only allowed in class method");
					}
					vtable* vt = lookup.get_current_vtable();
					std::string member = tree->first_child->next_sibling->str;
					if(vt->has_method(member)){
						return build_method_expression(that, lookup.get_current_class(), *vt->get_method(member));
					}else if(vt->has_field(member)){
						return build_field_expression(that, lookup.get_current_class(), vt->get_field_index(member));
					}else{
						semantic_error(lookup.get_current_class() + " doesn't have member " + member);
					}
				}else if(tree->first_child->next_sibling->op == oper::scope){
					identifier_ptr id = tree_to_identifier(tree->first_child->next_sibling, lookup);
					if(id->get_type() == identifier_type::method){
						return build_method_expression(that, static_cast<method_identifier&>(*id).get_classname(), static_cast<method_identifier&>(*id).get_method());
					}else if(id->get_type() == identifier_type::field){
						return build_field_expression(that, static_cast<field_identifier&>(*id).get_classname(), static_cast<field_identifier&>(*id).get_field());
					}else{
						semantic_error("invalid member");
						return expression_ptr();
					}
				}else{
					std::string member_name = tree->first_child->next_sibling->str;
					if(member_name.empty() || is_keyword(member_name) || isdigit(member_name[0]) || member_name[0] == '"'){
						syntax_error("invalid member name " + member_name);
					}
					return build_member_expression(that, member_name);
				}
				
			}
		case oper::call:
			{
				expression_ptr f = tree_to_expression(tree->first_child, lookup);
				std::vector<expression_ptr> params;
				std::vector<size_t> byref;
				
				fetch_params(tree->first_child, lookup, params, byref);
				
				return build_function_call_expression(f, params, byref);
			}
			break;
		case oper::scope:
			{
				return identifier_to_expression(tree_to_identifier(tree, lookup));
			}
			break;
		default:
			switch(get_number_of_operands(tree->op)){
				case 1:
					return build_unary_expression(
						tree->op,
						tree_to_expression(tree->first_child, lookup)
					);
				case 2:
					return build_binary_expression(
						tree->op,
						tree_to_expression(tree->first_child, lookup),
						tree_to_expression(tree->first_child->next_sibling, lookup)
					);
				case 3:
					return build_ternary_expression(
						tree->op,
						tree_to_expression(tree->first_child, lookup),
						tree_to_expression(tree->first_child->next_sibling, lookup),
						tree_to_expression(tree->first_child->next_sibling->next_sibling, lookup)
					);
				default:
					semantic_error("invalid expression");
			}
	}
	return expression_ptr();
}

expression_ptr build_expression(const identifier_lookup& lookup, tokenizer& parser, bool can_be_empty, bool declaration){
	part_ptr tree = create_expression_tree(parser, declaration, can_be_empty);

	expression_ptr ret = tree_to_expression(tree, lookup);

	if(!ret){
		if(can_be_empty){
			return build_null_expression();
		}
		unexpected_error(*parser);
	}
	
	return ret;
}

}//donkey


