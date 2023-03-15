#ifndef __MY_RISCV_H
#define __MY_RISCV_H

#include "koopa.h"
#include "stack.h"
#include "reg.h"

using namespace std;

typedef int32_t reg_id_t;

static int stack_top = 0;
static int stack_size = 0;
static int param_size = 0;

enum params_t{
    NO_PARAMS,
    // not parsing parameters
    DECL_PARAMS,
    // parsing parameters for function decl
    CALL_PARAMS
    // parsing parameters for function call
};
static params_t parse_param_mode = NO_PARAMS;
static int param_idx;

void parse(const koopa_raw_program_t program);
void parse(const koopa_raw_slice_t &slice);
void parse(const koopa_raw_function_t &func);
void parse(const koopa_raw_basic_block_t &bb);
string parse(const koopa_raw_value_t &value);
string parse(const koopa_raw_load_t &load);
void parse(const koopa_raw_return_t &ret);
void parse(const koopa_raw_store_t &store);
void parse(const koopa_raw_branch_t &branch);
void parse(const koopa_raw_call_t &branch);
void parse(const koopa_raw_global_alloc_t &g_alloc);
void parse(const koopa_raw_jump_t &jump);
string parse(const koopa_raw_binary_t &binary);
void parse(const koopa_raw_integer_t &integer);

void parse(const koopa_raw_program_t program){
    parse(program.values);
    parse(program.funcs);
}

void parse(const koopa_raw_slice_t &slice) {
    //if (parse_param_mode == CALL_PARAMS)
    //  cout << "params_num : " << slice.len << endl;
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
                /*called iff an instruction / function param is started */
                temp = parse(reinterpret_cast<koopa_raw_value_t>(ptr));
                //if (parse_param_mode == NO_PARAMS)
                //    cout << "instruction" << endl;
                break;
            default:
                assert(false);
        }
    }
}

void parse(const koopa_raw_function_t &func){
    if (func -> bbs.len != 0){
        /* function */
        cout << "  .text" << endl;
        cout << "  .globl " << string((const char*)(&func -> name[1])) << endl;
        cout << string((const char*)(&func -> name[1])) << ":" << endl;
        stack_size = parse_stack(func -> bbs);
        param_size = parse_param(func -> bbs);
        stack_size += param_size + 1;
        while (stack_size & 3) ++stack_size;
        stack_top = param_size;
        if (stack_size != 0){
            cout << "  addi  sp, sp, " << -stack_size * 4 << endl;
        }
        parse_param_mode = DECL_PARAMS;
        parse(func -> params);
        parse_param_mode = NO_PARAMS;
        if (param_size != 0){
            //with possible functin call
            cout << "  sw    ra, " << (stack_size - 1) * 4 << "(sp)" << endl;
        } 
        parse(func -> bbs);
        assert(stack_top <= stack_size);
        cout << endl;
    }
}

void parse(const koopa_raw_basic_block_t &bb) {
    string name = string((const char*)(&bb -> name[1]));
    cout << name << ":" << endl;
    parse(bb -> insts);
    //cout << "basic block with name (" << name << ") finished" << endl;
}

string parse(const koopa_raw_value_t &value) {
    /**/
    const auto &kind = value->kind;
    string cur;
    string addr, name;
    // cout << (&kind) << endl;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // cout << "ret " << &(kind) << endl;
            parse(kind.data.ret);
            return "??? Return";
            // the register name of this one should not be used
        case KOOPA_RVT_INTEGER:
            if (parse_param_mode == CALL_PARAMS){
                if (param_idx < 8){
                    cur = "a" + to_string(param_idx);
                    cout << "  li    " << cur << ", " << kind.data.integer.value  << endl;
                    param_idx++;
                    return "??? bad func params usage: rvt_integer";
                }
                else{
                    cur = alloc_reg();
                    cout << "  li    " << cur << ", " << kind.data.integer.value << endl;
                    cout << "  sw    " << cur << ", " << to_string(4 * (param_idx - 8)) << "(sp)" << endl;
                    param_idx++;
                    return "??? bad func params usage: rvt_integer";
                }
            }
            // cout << "integer " << &(kind) << endl;
            else if (kind.data.integer.value == 0)
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
            if (is_value_stack_set((&kind))){
                // pre func: return the location of the args
                addr = get_value_stack((&kind));
                if (is_reg(addr))
                    return addr;
                    /* params save in register */
                else{
                    cur = alloc_reg();
                    cout << "  lw    " << cur << ", " << addr << endl;
                    return cur;
                    /* params save in stack */
                }
            }
            else{
                int index = kind.data.func_arg_ref.index;
                // func decl: map the location with the args
                if (index <= 7){
                    cur = "a" + to_string(index);
                    set_value_stack(&(kind), cur);
                    /* params save in register */
                }
                else{
                    cur = to_string(4 * (stack_size + index - 8)) + "(sp)";
                    /* shift = stack_size + one return addr + number of params*/
                    set_value_stack(&(kind), cur);
                    /* params save in stack */
                }
                return "??? Initial RVT_FUNC_ARG_REF";
            }
            break;
        case KOOPA_RVT_BLOCK_ARG_REF:
            cout << "??? unknown type: RVT_BLOCK_ARG_REF" << endl;
            break;
        case KOOPA_RVT_ALLOC:
        
            if (parse_param_mode == CALL_PARAMS)
                cout << "param_alloc" << endl;
            // cout << "alloc " << &(kind) << endl;
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
            //cout << "global_alloc " << &(kind) << " " << value -> name << endl;
            if (is_value_data_set(&(kind))){
                addr = get_value_data(&(kind));
                return addr;
            }
            else{
                cout << "  .data" << endl;
                cout << "  .globl " << string((const char*) &(value -> name[1])) << endl;
                cout << string((const char*) &(value -> name[1])) << ":" << endl;
                parse(kind.data.global_alloc);
                set_value_data(&(kind), string(value -> name));
            }
            break;
        case KOOPA_RVT_LOAD:
            if (parse_param_mode == CALL_PARAMS){
                if (param_idx < 8)
                    cur = "a" + to_string(param_idx);
                else
                    cur = alloc_reg();
                if (is_value_stack_set(&(kind))){
                    addr = get_value_stack(&(kind));
                    cout << "  lw    " << cur << ", " << addr << endl;
                }
                else if (is_value_data_set(&(kind))){
                    addr = get_value_data(&(kind));
                    addr = string((const char*)&addr[1]);
                    cout << "  la    " << cur << ", " << addr << endl;
                    cout << "  lw    " << cur << ", 0(" << cur << ")" << endl;
                }
                if (param_idx >= 8)
                    cout << "  sw    " << cur << ", " << to_string(4 * (param_idx - 8)) << "(sp)" << endl;
                param_idx++;
                return "??? bad func params usage: rvt_load";
            }
            else if (is_value_stack_set(&(kind))){
                addr = get_value_stack(&(kind));
                cur = alloc_reg();
                cout << "  lw    " << cur << ", " << addr << endl;
                return cur;
            }
            else if (is_value_data_set(&(kind))){
                addr = get_value_data(&(kind));
                addr = string((const char*)&addr[1]);
                cur = alloc_reg();
                cout << "  la    " << cur << ", " << addr << endl;
                cout << "  lw    " << cur << ", 0(" << cur << ")" << endl;
                return cur;
            }
            else{
                assert(parse_param_mode != CALL_PARAMS);
                addr = parse(kind.data.load);
                if (addr[0] == '@' || addr[0] == '%')
                    // global variable
                    set_value_data(&(kind), addr);
                else
                    set_value_stack(&(kind), addr);
                return "??? Initial Load";
            }
            break;
        case KOOPA_RVT_STORE:
            // cout << "store " << &(kind) << endl;
            parse(kind.data.store);
            return "??? RVT STORE";
        case KOOPA_RVT_GET_PTR:
            cout << "??? unknown type: RVT_GET_PTR" << endl;
            break;
        case KOOPA_RVT_GET_ELEM_PTR:
            cout << "??? unknown type: RVT_GET_ELEM_PTR" << endl;
            break;
        case KOOPA_RVT_BINARY:
            if (parse_param_mode == CALL_PARAMS){
                //cout << "param_binary" << endl;
                assert(is_value_stack_set(&(kind)));
                if (param_idx < 8){
                    cur = "a" + to_string(param_idx);
                    addr = get_value_stack(&(kind));
                    cout << "  lw    " << cur << ", " << addr << endl;
                    param_idx++;
                    return "??? bad func params usage: rvt_binary";
                }
                else{
                    cur = alloc_reg();
                    addr = get_value_stack(&(kind));
                    cout << "  lw    " << cur << ", " << addr << endl;
                    cout << "  sw    " << cur << ", " << to_string(4 * (param_idx - 8)) << "(sp)" << endl;
                    param_idx++;
                    return "??? bad func params usage: rvt_binary";
                }
            }
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
            // cout << "branch " << &(kind) << endl;
            parse(kind.data.branch);
            return "??? Initial Branch";
        case KOOPA_RVT_JUMP:
            // cout << "jump " << &(kind) << endl;
            parse(kind.data.jump);
            return "??? Initial Jump";
        case KOOPA_RVT_CALL:
            if (parse_param_mode == CALL_PARAMS){
                assert(is_value_stack_set(&(kind)));
                if (param_idx < 8){
                    cur = "a" + to_string(param_idx);
                    addr = get_value_stack(&(kind));
                    cout << "  lw    " << cur << ", " << addr << endl;
                    param_idx++;
                    return "??? bad func params usage: rvt_call";
                }
                else{
                    cur = alloc_reg();
                    addr = get_value_stack(&(kind));
                    cout << "  lw    " << cur << ", " << addr << endl;
                    cout << "  sw    " << cur << ", " << to_string(4 * (param_idx - 8)) << "(sp)" << endl;
                    param_idx++;
                    return "??? bad func params usage: rvt_call";
                }
            }
            if (is_value_stack_set((&kind))){
                if (kind.data.call.callee -> ty -> tag == KOOPA_RTT_UNIT){
                    cout << "??? Use the return value of a void function call" << endl;
                    assert(false);
                }
                cur = alloc_reg();
                addr = get_value_stack((&kind));
                cout << "  lw    " << cur << ", " << addr << endl;
                return cur;
            }
            parse(kind.data.call);
            if (kind.data.call.callee -> ty -> data.function.ret -> tag == KOOPA_RTT_INT32){
                cur = "a0";
                addr = to_string(4 * (stack_top++)) + "(sp)";
                cout << "  sw    " << cur << ", " << addr << endl;
                set_value_stack((&kind), addr);
            }
            else
                set_value_stack((&kind), "???");
            return "Initial Call";
        default:
            assert(false);
    }
    return "??? Not Implemented koopa_raw_value_t type";
}


void parse(const koopa_raw_return_t &ret){
    string loc;
    if (ret.value != NULL){
        loc = parse(ret.value);
        loc = realloc_reg(loc, "a0");
    }
    //cout << param_size << endl;
    if (param_size != 0)
        // load return address in function calls
        cout << "  lw    ra, " << (stack_size - 1) * 4 << "(sp)" << endl;
    if (stack_size != 0)
        cout << "  addi  sp, sp, " << stack_size * 4 << endl;
    cout << "  ret" << endl;
}

string parse(const koopa_raw_load_t &load){
    return parse(load.src);
}

void parse(const koopa_raw_store_t &store){
    string cur = parse(store.value);
    if (is_value_stack_set(&(store.dest->kind))){
        string dest = get_value_stack(&(store.dest->kind));
        cout << "  sw    " << cur << ", " << dest << endl;
    }
    else if (is_value_data_set(&(store.dest->kind))){
        string addr = get_value_data(&(store.dest->kind));
        addr = string((const char*)&addr[1]);
        string temp = alloc_reg();
        cout << "  la    " << temp << ", " << addr << endl;
        cout << "  sw    " << cur << ", 0(" << temp << ")" << endl; 
    }
    else
        assert(false);
}

void parse(const koopa_raw_branch_t &branch){
    string cond = parse(branch.cond);
    cout << "  bnez  " << cond << ", " << (string)((const char*)&(branch.true_bb->name[1])) << endl;
    cout << "  j     " << (string)((const char*)&(branch.false_bb->name[1])) << endl;
}

void parse(const koopa_raw_jump_t &jump){
    cout << "  j     " << (string)((const char*)&(jump.target->name[1])) << endl;
}

void parse(const koopa_raw_call_t &call){
    const auto &func = call.callee;
    const auto &args = call.args;
    parse_param_mode = CALL_PARAMS;
    int cur_stack_top = 0;
    param_idx = 0;
    swap(cur_stack_top, stack_top);
    parse(args);
    assert(stack_top <= param_size);
    swap(cur_stack_top, stack_top);
    parse_param_mode = NO_PARAMS;
    cout << "  call  " << string((const char*)&(func -> name[1])) << endl;
}

void parse(const koopa_raw_global_alloc_t &g_alloc){
    const auto& kind = g_alloc.init -> kind;
    switch (kind.tag){
        case KOOPA_RVT_INTEGER:
            cout << "  .word " << kind.data.integer.value << endl;
            break;
        case KOOPA_RVT_ZERO_INIT:
            cout << "undealed var type: RVT_KOOPA_ZERO_INIT" << endl;
            break;
        default:
            cout << "bad var initial value" << endl;
    }
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