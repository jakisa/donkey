#ifndef __helpers_hpp__
#define __helpers_hpp__

#include <string>
#include <cmath>

namespace donkey{

inline bool parse_hex_number(std::string str, size_t from, number& ret){
	int iret = 0;
	for(size_t i = from; i < str.size(); ++i){
		iret <<= 4;
		wchar_t ch = str[i];
		if('0' <= ch && ch <= '9'){
			iret += (ch - '0');
			continue;
		}
		if('A' <= ch && ch <= 'F'){
			iret += (ch - 'A');
			iret += 10;
			continue;
		}
		if('a' <= ch && ch <= 'f'){
			iret += (ch - 'a');
			iret += 10;
			continue;
		}
		return false;
	}
	ret = iret;
	return true;
}

inline bool parse_dec_number(std::string str, size_t from, number& ret){
	ret = 0;
	number div_by = 1;
	bool after_dot = false;
	for(size_t i = from; i < str.size(); ++i){
		wchar_t ch = str[i];
		if(ch == '.'){
			after_dot = true;
			continue;
		}
		ret *= 10;
		if('0' <= ch && ch <= '9'){
			ret += (ch - '0');
			if(after_dot){
				div_by *= 10;
			}
			continue;
		}
		return false;
	}
	ret /= div_by;
	return true;
}

inline number parse_number(std::string str){
	if(str.empty()){
		return nan("");
	}
	bool minus = false;
	std::size_t begin = 0;
	if(str[0] == '-'){
		minus = true;
		++begin;
	}else if(str[0] == '+'){
		++begin;
	}
	if(str.substr(begin, 2) == "0x"){
		begin += 2;
		if(begin == str.length()){
			return nan("");
		}
		number h;
		return parse_hex_number(str, begin, h) ? (minus ? -h : h) : nan("");
	}else{
		number d;
		return parse_dec_number(str, begin, d) ? (minus ? -d : d) : nan("");
	}
}

inline std::string to_string(number d){
	bool minus = (d < 0);
	if(minus){
		d = -d;
	}
	
	d += 0.000005;
	
	number before_dot = floor(d);
	number after_dot = d - before_dot;

	std::string before_dot_str;
	std::string after_dot_str;

	if(before_dot == 0){
		before_dot_str = "0";
	}else{
		while(before_dot){
			int digit = (int)fmod(before_dot, 10);
			before_dot_str = char('0' + digit) + before_dot_str;

			before_dot = floor(before_dot/10);
		}
	}
	
	for(int i = 0; i < 5 && after_dot; ++i){
		after_dot *= 10;
		int digit = floor(after_dot);
		after_dot_str += char('0' + digit);
		after_dot -= digit;
	}
	
	while(!after_dot_str.empty() && after_dot_str.back() == '0'){
		after_dot_str.pop_back();
	}

	return (minus ? "-" : "") + before_dot_str + (after_dot_str.empty() ? std::string("") : ".") + after_dot_str;
}

}

#endif /*__helpers_hpp__*/
