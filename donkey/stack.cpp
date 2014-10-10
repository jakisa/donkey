#include "stack.hpp"

namespace donkey{

void stack_shelf::_grow_capacity(size_t new_blocks_count){
	stack_block* new_data = new stack_block[new_blocks_count];
	
	if(!new_data){
		runtime_error("out of memory");
	}
	
	memcpy(new_data, _data, _blocks_count*sizeof(stack_block));
	for(size_t i = _blocks_count; i != new_blocks_count; ++i){
		new_data[i].init();
	}
	memset(_data, 0, _blocks_count*sizeof(stack_block));
	delete[] _data;
	_data = new_data;
	_blocks_count = new_blocks_count;
}

void stack_shelf::_push_to_blocks(size_t first_block_idx, size_t last_block_idx, size_t first_idx, size_t cnt){
	size_t first_block_cnt = stack_block_size - (first_idx & stack_block_idx_mask);
	
	_data[first_block_idx].push_default(first_block_cnt);
	
	cnt -= first_block_cnt;
	
	for(size_t block_idx = first_block_idx + 1; block_idx < last_block_idx; ++block_idx){
		_data[block_idx].push_default(stack_block_size);
	}
	
	cnt -= (last_block_idx - first_block_idx - 1)*stack_block_size;
	
	_data[last_block_idx].push_default(cnt);
}

void stack_shelf::_pop_from_blocks(size_t first_block_idx, size_t last_block_idx, size_t last_idx, size_t cnt){
	size_t last_block_cnt = (last_idx & stack_block_idx_mask) + 1;
	
	_data[last_block_idx].pop_back(last_block_cnt);
	
	cnt -= last_block_cnt;
	
	for(size_t block_idx = last_block_idx - 1; block_idx > first_block_idx; --block_idx){
		_data[block_idx].pop_back(stack_block_size);
	}
	
	cnt -= (last_block_idx - first_block_idx - 1)*stack_block_size;
	
	_data[first_block_idx].pop_back(cnt);
}


}//donkey
