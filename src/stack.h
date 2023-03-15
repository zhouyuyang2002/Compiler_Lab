#ifndef __MY_STACK_H
#define __MY_STACK_H

#include "koopa.h"
#include <cmath>
#include <cassert>

using namespace std;


int parse_param(const koopa_raw_slice_t &slice);
int parse_param(const koopa_raw_basic_block_t &bb);
int parse_param(const koopa_raw_value_t &value);
int parse_stack(const koopa_raw_slice_t &slice);
int parse_stack(const koopa_raw_basic_block_t &bb);
int parse_stack(const koopa_raw_value_t &value);

int parse_param(const koopa_raw_slice_t &slice) {
    int max_args = 0;
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        switch (slice.kind){
            case KOOPA_RSIK_BASIC_BLOCK:
                max_args = max(max_args, parse_param(reinterpret_cast<koopa_raw_basic_block_t>(ptr)));
                break;
            case KOOPA_RSIK_VALUE:
                max_args = max(max_args, parse_param(reinterpret_cast<koopa_raw_value_t>(ptr)));
                break;
            default:
                assert(false);
        }
    }
    return max_args;
}


int parse_param(const koopa_raw_basic_block_t &bb) {
    return parse_param(bb -> insts);
}


int parse_param(const koopa_raw_value_t &value) {
    /*printf("parse value name: %s\n", value -> name);*/
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_CALL:
            if (kind.data.call.args.len <= 8)
                return 1;
            return kind.data.call.args.len - 8 + 1;
            // function call
        default:
            return 0;
    }
}

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
        case KOOPA_RVT_ALLOC:
            return 1;
            // alloc a integer
        case KOOPA_RVT_BINARY:
            return 1;
            // operation;
        case KOOPA_RVT_CALL:
            if (kind.data.call.callee -> ty -> data.function.ret -> tag == KOOPA_RTT_INT32)
                return 1;
            return 0;
        default:
            return 0;
    }
}

#endif