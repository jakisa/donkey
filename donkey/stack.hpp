#ifndef __stack_hpp__
#define __stack_hpp__

#include <variables.hpp>

namespace donkey{

template<int i, int mod2 = i%2>
struct stack_block_size_traits;

template<>
struct stack_block_size_traits<2, 0>{
	enum{
		mask = 1,
		shift = 1
	};
};

template<int i>
struct stack_block_size_traits<i, 0>{
	enum{
		mask = i - 1,
		shift = stack_block_size_traits<i/2>::shift + 1
	};
};

enum{
	stack_block_size = 256,
	stack_block_idx_mask = stack_block_size_traits<stack_block_size>::mask,
	stack_block_idx_shift = stack_block_size_traits<stack_block_size>::shift,
};

class stack_block{
	stack_block(const stack_block&) = delete;
	void operator=(const stack_block&) = delete;
private:
	variable* _data;
	size_t _size;
public:
	stack_block():
		_data(nullptr),
		_size(0){
	}
	
	void init(){
		_data = new variable[stack_block_size];
	}
	
	size_t size() const{
		return _size;
	}
	
	variable& operator[](size_t idx){
		return _data[idx];
	}
	
	void push_back(variable&& v){
		_data[_size++] = v;
	}
	
	void push_default(size_t sz){
		_size += sz;
	}
	
	void pop_back(size_t sz){
		for(size_t i = _size; i != _size - sz; --i){
			_data[i-1].reset();
		}
		
		_size -= sz;
	}
	
	~stack_block(){
		delete[] _data;
	}	
};

class stack_shelf{
	stack_shelf(const stack_shelf&) = delete;
	void operator=(const stack_shelf&) = delete;
private:
	stack_block* _data;
	size_t _blocks_count;
	size_t _size;
	
	void _grow_capacity(size_t new_blocks_count);
	
	void _check_capacity(size_t block_idx){
		if(block_idx >= _blocks_count){
			_grow_capacity(block_idx + 1);
		}
	}
	
	void _push_to_blocks(size_t first_block_idx, size_t last_block_idx, size_t first_idx, size_t cnt);
	
	void _pop_from_blocks(size_t first_block_idx, size_t last_block_idx, size_t last_idx, size_t cnt);
	
	
	void _push_default(size_t cnt){
		size_t first_idx = _size;
	
		size_t last_idx = _size + cnt - 1;
		
		size_t first_block_idx = (first_idx >> stack_block_idx_shift);
		
		size_t last_block_idx = (last_idx >> stack_block_idx_shift);
	
		_check_capacity(last_block_idx);
		
		if(first_block_idx == last_block_idx){
			_data[first_block_idx].push_default(cnt);
		}else{
			_push_to_blocks(first_block_idx, last_block_idx, first_idx, cnt);
		}
	
		_size += cnt;
	}
	
	void _pop_back(size_t cnt){
		size_t first_idx = _size - cnt;
		size_t last_idx = _size - 1;
		
		size_t first_block_idx = (first_idx >> stack_block_idx_shift);
		
		size_t last_block_idx = (last_idx >> stack_block_idx_shift);
		
		if(first_block_idx == last_block_idx){
			_data[first_block_idx].pop_back(cnt);
		}else{
			_pop_from_blocks(first_block_idx, last_block_idx, last_idx, cnt);
		}
		
		_size -= cnt;
	}
public:
	stack_shelf():
		_data(new stack_block[1]),
		_blocks_count(1),
		_size(0){
		_data[0].init();
	}
	
	size_t size() const{
		return _size;
	}
	
	variable& operator[](size_t idx){
		return _data[idx >> stack_block_idx_shift][idx & stack_block_idx_mask];
	}
	
	void push_back(variable&& v){
		size_t idx = _size;
		size_t block_idx = (idx >> stack_block_idx_shift);
	
		_check_capacity(block_idx);
		
		_data[block_idx].push_back(std::move(v));
		
		++_size;
	}
	
	void push_default(size_t sz){
		if(sz != 0){
			_push_default(sz);
		}
	}
	
	void pop_back(size_t sz){
		if(sz != 0){
			_pop_back(sz);
		}
	}
	
	~stack_shelf(){
		delete[] _data;
	}
};

class stack{
	stack(const stack&) = delete;
	void operator=(const stack&) = delete;
private:
	enum{
		block_size = 256
	};
	stack_shelf _shelf;
public:
	stack(){
	}
	
	void add_size(size_t sz){
		_shelf.push_default(sz);
	}
	
	void push(variable&& v){
		_shelf.push_back(std::move(v));
	}
	
	void pop(size_t cnt){
		_shelf.pop_back(cnt);
	}
	
	variable& top(size_t idx){
		return _shelf[_shelf.size() - idx - 1];
	}
	
	variable& operator[](size_t idx){
		return _shelf[idx];
	}
	
	size_t size() const{
		return _shelf.size();
	}
};


}//donkey

#endif /* __stack_hpp__*/

