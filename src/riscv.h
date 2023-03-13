#ifndef __MY_RISCV_H
#define __MY_RISCV_H

#include "koopa.h"
#include "stack.h"
#include "reg.h"

using namespace std;

typedef int32_t reg_id_t;

static int stack_top = 0;
static int stack_size = 0;

void parse(const koopa_raw_program_t program);
void parse(const koopa_raw_slice_t &slice);
void parse(const koopa_raw_function_t &func);
void parse(const koopa_raw_basic_block_t &bb);
string parse(const koopa_raw_value_t &value);
string parse(const koopa_raw_load_t &load);
void parse(const koopa_raw_return_t &ret);
void parse(const koopa_raw_store_t &store);
string parse(const koopa_raw_binary_t &binary);
void parse(const koopa_raw_integer_t &integer);

void parse(const koopa_raw_program_t program){
    cout << "  .text" << endl;
    cout << "  .globl main" << endl;
    parse(program.values);
    parse(program.funcs);
}

void parse(const koopa_raw_slice_t &slice) {
    string temp;
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
                reset_reg();
                /*called iff an instruction is started */
                temp = parse(reinterpret_cast<koopa_raw_value_t>(ptr));
                //cout << "instruction" << endl;
                break;
            default:
                assert(false);
        }
    }
}

void parse(const koopa_raw_function_t &func) {
    cout << string((const char*)(&func -> name[1])) << ":" << endl;
    stack_size = parse_stack(func -> bbs);
    while (stack_size & 3) ++stack_size;
    stack_top = 0;
    if (stack_size != 0){
        cout << "  addi  sp, sp, " << -stack_size * 4 << endl;
    }
    parse(func -> bbs);
    assert(stack_top <= stack_size);
}

void parse(const koopa_raw_basic_block_t &bb) {
    parse(bb -> insts);
}

string parse(const koopa_raw_value_t &value) {
    /**/
    const auto &kind = value->kind;
    string cur;
    string addr, name;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            parse(kind.data.ret);
            return "??? Return";
            // the register name of this one should not be used
        case KOOPA_RVT_INTEGER:
            if (kind.data.integer.value == 0)
                set_value_reg(&(kind), string("x0"));
            else if (is_value_reg_set(&(kind)));
            else{
                string cur = alloc_reg();
                cout << "  li    " << cur << ", " << kind.data.integer.value << endl;
                set_value_reg(&(kind), cur);
            }
            return get_value_reg(&(kind));
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
            if (is_value_stack_set(&(kind))){
                addr = get_value_stack(&(kind));
                return addr;
            }
            else{
                addr = to_string(4 * (stack_top++)) + "(sp)";
                set_value_stack(&(kind), addr);
                return "??? Initial Alloc";
            }
        case KOOPA_RVT_GLOBAL_ALLOC:
            cout << "??? unknown type: RVT_GLOBAL_ALLOC" << endl;
            break;
        case KOOPA_RVT_LOAD:
            if (is_value_stack_set(&(kind))){
                addr = get_value_stack(&(kind));
                cur = alloc_reg();
                cout << "  lw    " << cur << ", " << addr << endl;
                return cur;
            }
            else{
                addr = parse(kind.data.load);
                set_value_stack(&(kind), addr);
                return "??? Initial Load";
            }
            break;
        case KOOPA_RVT_STORE:
            parse(kind.data.store);
            return "??? RVT STORE";
        case KOOPA_RVT_GET_PTR:
            cout << "??? unknown type: RVT_GET_PTR" << endl;
            break;
        case KOOPA_RVT_GET_ELEM_PTR:
            cout << "??? unknown type: RVT_GET_ELEM_PTR" << endl;
            break;
        case KOOPA_RVT_BINARY:
            //cout << "bin_addr " << &(kind) << endl;
            if (is_value_stack_set(&(kind))){
                // value usage
                cur = alloc_reg();
                addr = get_value_stack(&(kind));
                cout << "  lw    " << cur << ", " << addr << endl;
                return cur;
            }
            else{
                // value_calculation
                cur = parse(kind.data.binary);
                addr = to_string(4 * (stack_top++)) + "(sp)";
                //cout << addr << ' ' << cur << ' ' << "fuck" << endl;
                cout << "  sw    " << cur << ", " << addr << endl;
                set_value_stack(&(kind), addr);
                return "??? Initial Binary";
            }
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
    if (stack_size != 0){
        cout << "  addi  sp, sp, " << stack_size * 4 << endl;
    }
    cout << "  ret" << endl;
}

string parse(const koopa_raw_load_t &load){
    return parse(load.src);
}

void parse(const koopa_raw_store_t &store){
    string cur = parse(store.value);
    assert(is_value_stack_set(&(store.dest->kind)));
    string dest = get_value_stack(&(store.dest->kind));
    cout << "  sw    " << cur << ", " << dest << endl;
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

#endif