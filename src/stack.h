#ifndef __MY_STACK_H
#define __MY_STACK_H

#include "koopa.h"
#include <cassert>
#include "reg.h"

using namespace std;

typedef int32_t reg_id_t;

int parse_stack(const koopa_raw_slice_t &slice);
int parse_stack(const koopa_raw_basic_block_t &bb);
int parse_stack(const koopa_raw_value_t &value);

int parse_stack(const koopa_raw_slice_t &slice) {
    int sum = 0;
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        switch (slice.kind){
            case KOOPA_RSIK_BASIC_BLOCK:
                sum += parse_stack(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
                break;
            case KOOPA_RSIK_VALUE:
                sum += parse_stack(reinterpret_cast<koopa_raw_value_t>(ptr));
                break;
            default:
                assert(false);
        }
    }
    return sum;
}

int parse_stack(const koopa_raw_basic_block_t &bb) {
    return parse_stack(bb -> insts);
}

int parse_stack(const koopa_raw_value_t &value) {
    /*printf("parse value name: %s\n", value -> name);*/
    const auto &kind = value->kind;
    string cur;
    switch (kind.tag) {
        case KOOPA_RVT_INTEGER:
            return 1;
            // load integer
        case KOOPA_RVT_ALLOC:
            return 1;
            // alloc a integer
        case KOOPA_RVT_LOAD:
            return 0;
            // load requires only the stack pointer
            break;
        case KOOPA_RVT_BINARY:
            return 1;
            // operation;
        default:
            return 0;
    }
}

#endif