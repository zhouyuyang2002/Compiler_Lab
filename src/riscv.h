#pragma once

#include "koopa.h"

using namespace std;
void parse(const koopa_raw_program_t program);
void parse(const koopa_raw_slice_t &slice);
void parse(const koopa_raw_function_t &func);
void parse(const koopa_raw_basic_block_t &bb);
void parse(const koopa_raw_value_t &value);
void parse(const koopa_raw_return_t &ret);
void parse(const koopa_raw_integer_t &integer);

void parse(const koopa_raw_program_t program){
    cout << "  .text" << endl;
    cout << "  .globl" << endl;
    parse(program.values);
    parse(program.funcs);
}

void parse(const koopa_raw_slice_t &slice) {
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        switch (slice.kind) {
            case KOOPA_RSIK_FUNCTION:
                parse(reinterpret_cast<koopa_raw_function_t>(ptr));
                break;
            case KOOPA_RSIK_BASIC_BLOCK:
                parse(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
                break;
            case KOOPA_RSIK_VALUE:
                parse(reinterpret_cast<koopa_raw_value_t>(ptr));
                break;
            default:
                assert(false);
        }
    }
}

void parse(const koopa_raw_function_t &func) {
    cout << string(func -> name) << ":" << endl;
    parse(func->bbs);
}

void parse(const koopa_raw_basic_block_t &bb) {
    parse(bb->insts);
}

void parse(const koopa_raw_value_t &value) {
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            parse(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            parse(kind.data.integer);
            break;
        default:
            assert(false);
    }
}

void parse(const koopa_raw_return_t &ret){
    cout << "  li a0 , ";
    parse(ret.value);
    cout << endl << "  ret" << endl;
}

void parse(const koopa_raw_integer_t &integer){
    cout << integer.value;
}



void parse_koopa(const char* koopa){
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(koopa, &program);
    assert(ret == KOOPA_EC_SUCCESS); 
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    // Todo
    parse(raw);

    koopa_delete_raw_program_builder(builder);
}