#include "block.h"
#include <vector>
#include <cassert>
using namespace std;

vector<block_t> blocks;

block_t& block_back(){
    assert(blocks.size() != 0);
    return blocks[blocks.size() - 1];
}

void alloc_block(){
    block_t block = {false};
    blocks.push_back(block);
}

void remove_block(){
    assert(block_back().fin);
    blocks.pop_back();
}