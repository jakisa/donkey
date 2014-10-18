#include "vtable.hpp"
#include "cpp/native_function.hpp"
#include <unordered_set>
#include <algorithm>

namespace donkey{

static integer string_length(std::string that){
	return that.size();
}

static std::string string_substr(const std::string& that, integer pos, integer len){
	if((size_t)pos >= that.size()){
		return "";
	}
	return that.substr(pos, len);
}

static variable string_to_string(const variable& that, runtime_context&, size_t){
	return that;
}


static variable string_split(const std::string& that, std::string separator){
	std::vector<std::string> parts;
	
	size_t begin = 0;
	
	for(size_t i = 0; i != that.size();){
		if(!that.compare(i, separator.size(), separator)){
			parts.push_back(that.substr(begin, i - begin));
			i += separator.size();
			begin = i;
		}else{
			++i;
		}
	}
	
	parts.push_back(that.substr(begin));
	
	std::unique_ptr<variable[]> arr(new variable[parts.size()]);
	
	for(size_t i = 0; i < parts.size(); ++i){
		arr[i] = variable(parts[i]);
	}
	
	variable ret =  create_initialized_array(arr.get(), parts.size());
	arr.release();
	
	return ret;
}

static std::string string_trim(const std::string& that){
	auto begin = std::find_if(that.begin(), that.end(), [](char c){return !isspace(c);});
	if(begin == that.end()){
		return "";
	}
	
	auto end = std::find_if(that.rbegin(), that.rend(), [](char c){return !isspace(c);}).base();
	
	return std::string(begin, end);
}

vtable_ptr string_vtable(){
	static vtable_ptr ret;
	if(!ret){
		std::unordered_map<std::string, method_ptr> methods;
		std::unordered_map<std::string, size_t> fields;
		
		methods.emplace("toString", method_ptr(new method(&string_to_string)));
		
		methods.emplace("length", create_native_method("string::length", &string_length));
		
		methods.emplace("substr", create_native_method("string::substr", &string_substr, std::make_tuple(0, integer(std::string::npos))));
		
		methods.emplace("split", create_native_method("string::split", &string_split));
		methods.emplace("trim", create_native_method("string::trim", &string_trim));
		
		ret.reset(new vtable("", "string", method_ptr(), method_ptr(), std::move(methods), std::move(fields), 0, true, true));
		ret->derive_from(*object_vtable());
	}
	return ret;
}


}//donkey