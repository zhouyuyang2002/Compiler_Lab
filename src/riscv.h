#pragma once

#include "koopa.h"
#include "reg.h"

using namespace std;

typedef int32_t reg_id_t;

void parse(const koopa_raw_program_t program);
void parse(const koopa_raw_slice_t &slice);
void parse(const koopa_raw_function_t &func);
void parse(const koopa_raw_basic_block_t &bb);
string parse(const koopa_raw_value_t &value);
void parse(const koopa_raw_return_t &ret);
string parse(const koopa_raw_binary_t &binary);
void parse(const koopa_raw_integer_t &integer);

void parse(const koopa_raw_program_t program){
    cout << "  .text" << endl;
    cout << "  .globl main" << endl;
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
    cout << string((const char*)(&func -> name[1])) << ":" << endl;
    parse(func -> bbs);
}

void parse(const koopa_raw_basic_block_t &bb) {
    parse(bb -> insts);
}

string parse(const koopa_raw_value_t &value) {
    /*printf("parse value name: %s\n", value -> name);*/
    const auto &kind = value->kind;
    string cur;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            parse(kind.data.ret);
            return "??? Return";
            // the register name of this one should not be used
        case KOOPA_RVT_INTEGER:
            if (kind.data.integer.value == 0)
                set_value_reg(&(kind.data.integer), string("x0"));
            else if (is_value_reg_set(&(kind.data.integer)));
            else{
                string cur = alloc_reg();
                cout << "  li    " << cur << ", " << kind.data.integer.value << endl;
                set_value_reg(&(kind.data.integer), cur);
            }
            return get_value_reg(&(kind.data.integer));
        case KOOPA_RVT_ZERO_INIT:
            cout << "??? unknown type: RVT_ZERO_INIT" << endl;
            break;  
        case KOOPA_RVT_UNDEF:
            cout << "??? unknown type: RVT_UNDEF" << endl;
            break;
        case KOOPA_RVT_AGGREGATE:
            cout << "??? unknown type: RVT_AGGREGATE" << endl;
            break;
        case KOOPA_RVT_FUNC_ARG_REF:
            cout << "??? unknown type: RVT_FUNC_ARG_REF" << endl;
            break;
        case KOOPA_RVT_BLOCK_ARG_REF:
            cout << "??? unknown type: RVT_BLOCK_ARG_REF" << endl;
            break;
        case KOOPA_RVT_ALLOC:
            cout << "??? unknown type: RVT_ALLOC" << endl;
            break;
        case KOOPA_RVT_GLOBAL_ALLOC:
            cout << "??? unknown type: RVT_RVT_ALLOC" << endl;
            break;
        case KOOPA_RVT_LOAD:
            cout << "??? unknown type: RVT_LOAD" << endl;
            break;
        case KOOPA_RVT_STORE:
            cout << "??? unknown type: RVT_STORE" << endl;
            break;
        case KOOPA_RVT_GET_PTR:
            cout << "??? unknown type: RVT_GET_PTR" << endl;
            break;
        case KOOPA_RVT_GET_ELEM_PTR:
            cout << "??? unknown type: RVT_GET_ELEM_PTR" << endl;
            break;
        case KOOPA_RVT_BINARY:
            if (is_value_reg_set(&(kind.data.binary)));
            else{
                cur = parse(kind.data.binary);
                set_value_reg(&(kind.data.binary), cur);
            }
            return get_value_reg(&(kind.data.binary));
        case KOOPA_RVT_BRANCH:
            cout << "??? unknown type: RVT_BRANCH" << endl;
            break;
        case KOOPA_RVT_JUMP:
            cout << "??? unknown type: RVT_JUMP" << endl;
            break;
        case KOOPA_RVT_CALL:
            cout << "??? unknown type: RVT_CALL" << endl;
            break;
        default:
            assert(false);
    }
    return "??? Not Implemented koopa_raw_value_t type";
}

void parse(const koopa_raw_return_t &ret){
    string loc;
    loc = parse(ret.value);
    loc = realloc_reg(loc, "a0");
    cout << "  ret" << endl;
}

string parse(const koopa_raw_binary_t &ret){

    string cur, lhs_reg, rhs_reg;
    switch (ret.op){
        case KOOPA_RBO_NOT_EQ:
        case KOOPA_RBO_EQ:
            lhs_reg = parse(ret.lhs);
            rhs_reg = parse(ret.rhs);
            if (ret.lhs->kind.tag == KOOPA_RVT_INTEGER && lhs_reg != string("x0"))
                cur = lhs_reg;
            else if (ret.rhs->kind.tag == KOOPA_RVT_INTEGER && rhs_reg != string("x0"))
                cur = rhs_reg;
            else cur = alloc_reg();
            cout << "  xor   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_NOT_EQ)
                cout << "  snez  " << cur << ", " << cur << endl;
            if (ret.op == KOOPA_RBO_EQ)
                cout << "  seqz  " << cur << ", " << cur << endl;
            return cur;
        case KOOPA_RBO_GT:
        case KOOPA_RBO_LE:
            lhs_reg = parse(ret.lhs);
            rhs_reg = parse(ret.rhs);
            if (ret.lhs->kind.tag == KOOPA_RVT_INTEGER && lhs_reg != string("x0"))
                cur = lhs_reg;
            else if (ret.rhs->kind.tag == KOOPA_RVT_INTEGER && rhs_reg != string("x0"))
                cur = rhs_reg;
            else cur = alloc_reg();
            cout << "  sgt   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_LE)
                cout << "  seqz  " << cur << ", " << cur << endl;
            return cur;
        case KOOPA_RBO_LT:
        case KOOPA_RBO_GE:
            lhs_reg = parse(ret.lhs);
            rhs_reg = parse(ret.rhs);
            if (ret.lhs->kind.tag == KOOPA_RVT_INTEGER && lhs_reg != string("x0"))
                cur = lhs_reg;
            else if (ret.rhs->kind.tag == KOOPA_RVT_INTEGER && rhs_reg != string("x0"))
                cur = rhs_reg;
            else cur = alloc_reg();
            cout << "  slt   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_GE)
                cout << "  seqz  " << cur << ", " << cur << endl;
            return cur;
        case KOOPA_RBO_ADD:
        case KOOPA_RBO_SUB:
        case KOOPA_RBO_MUL:
        case KOOPA_RBO_DIV:
        case KOOPA_RBO_MOD:
        case KOOPA_RBO_AND:
        case KOOPA_RBO_OR:
            lhs_reg = parse(ret.lhs);
            rhs_reg = parse(ret.rhs);
            if (ret.lhs->kind.tag == KOOPA_RVT_INTEGER && lhs_reg != string("x0"))
                cur = lhs_reg;
            else if (ret.rhs->kind.tag == KOOPA_RVT_INTEGER && rhs_reg != string("x0"))
                cur = rhs_reg;
            else cur = alloc_reg();
            if (ret.op == KOOPA_RBO_ADD)
                cout << "  add   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_SUB)
                cout << "  sub   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_MUL)
                cout << "  mul   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_DIV)
                cout << "  div   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_MOD)
                cout << "  rem   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_AND)
                cout << "  and   " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            if (ret.op == KOOPA_RBO_OR)
                cout << "  or    " << cur << ", " << lhs_reg << ", " << rhs_reg << endl;
            return cur;
        case KOOPA_RBO_XOR:
            cout << "??? undealed operator: XOR" << endl;
            break;
        case KOOPA_RBO_SHL:
            cout << "??? undealed operator: SHL" << endl;
            break;
        case KOOPA_RBO_SHR:
            cout << "??? undealed operator: SHR" << endl;
            break;
        case KOOPA_RBO_SAR:
            cout << "??? undealed operator: SAR" << endl;
            break;
        default:
            assert(false);
    }
    return "???";
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

    parse(raw);

    koopa_delete_raw_program_builder(builder);
}