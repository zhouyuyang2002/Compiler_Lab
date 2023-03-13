#ifndef __MY_BLOCK_H
#define __MY_BLOCK_H

struct block_t{
    bool fin;
};

block_t& block_back();
void remove_block();
void alloc_block();

#endif