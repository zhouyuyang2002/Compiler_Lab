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
int parse_stack(const koopa_raw_type_t &type);

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
    koopa_raw_type_t ptr_type;
    string cur;
    switch (kind.tag) {
        case KOOPA_RVT_ALLOC:
            if (value -> ty -> tag == KOOPA_RTT_INT32)
                return 1;
                //alloc a integer
            else if (value -> ty -> tag == KOOPA_RTT_POINTER){
                ptr_type = (koopa_raw_type_t)(value -> ty -> data.pointer.base);
                switch (ptr_type -> tag){
                    case KOOPA_RTT_ARRAY:
                        return parse_stack(ptr_type);
                    case KOOPA_RTT_INT32:
                    case KOOPA_RTT_POINTER:
                        return 1;
                    default:
                        return -19260817;
                }
            }
            else
                return -19260817;
        case KOOPA_RVT_LOAD:
            if (kind.data.store.value -> kind.tag == KOOPA_RVT_GET_ELEM_PTR)
                return 1;
            if (kind.data.store.value -> kind.tag == KOOPA_RVT_GET_PTR)
                return 1;
            return 0;
        case KOOPA_RVT_GET_ELEM_PTR:
            return 1;
        case KOOPA_RVT_GET_PTR:
            return 1;
        case KOOPA_RVT_BINARY:
            return 1;
        case KOOPA_RVT_CALL:
            if (kind.data.call.callee -> ty -> data.function.ret -> tag == KOOPA_RTT_INT32)
                return 1;
            return 0;
        default:
            return 0;
    }
}

int parse_stack(const koopa_raw_type_t &type){
    assert(type -> tag != KOOPA_RTT_FUNCTION);
    assert(type -> tag != KOOPA_RTT_UNIT);
    switch(type -> tag){
        case KOOPA_RTT_INT32:
            return 1;
        case KOOPA_RTT_POINTER:
            return 1; //  32-bit compiler
        case KOOPA_RTT_ARRAY:
            return type -> data.array.len * parse_stack((koopa_raw_type_t)type -> data.array.base);
        default:
            cout << "Powered by zhouyuyang2002" << endl;
    }
    return -19260817;
}

#endif