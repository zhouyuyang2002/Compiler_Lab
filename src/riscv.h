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
string parse(const koopa_raw_get_ptr_t &get_ptr);
string parse(const koopa_raw_get_elem_ptr_t &get_elem_ptr);
void parse(const koopa_raw_integer_t &integer);


void parse_zeroinit(const string& base, int shift, int len);
void parse_local_aggr(const koopa_raw_aggregate_t &aggr, 
                     const koopa_raw_type_t &type,
                     const string& base,
                     int shift);
void parse_local_arr(const koopa_raw_value_t &value, 
                     const koopa_raw_type_t &type,
                     const string& base,
                     int shift);
void parse_global_aggr(const koopa_raw_aggregate_t &aggr, 
                      const koopa_raw_type_t &type);
void parse_global_arr(const koopa_raw_value_t &value, 
                     const koopa_raw_type_t &type);
void parse_local_arr(const koopa_raw_store_t &store);
void parse_global_arr(const koopa_raw_value_t &value);

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
                parse(reinterpret_cast<koopa_raw_value_t>(ptr));
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
    koopa_raw_type_t ptr_type = NULL;
    int shift;
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
            if (value -> ty -> tag == KOOPA_RTT_INT32){
                /*alloc a integer*/
                if (is_value_stack_set(&(kind))){
                    addr = get_value_stack(&(kind));
                    return addr;
                }
                else{
                    addr = to_string(4 * (stack_top++)) + "(sp)";
                    set_value_stack(&(kind), addr);
                    return "??? Initial Alloc";
                }
            }
            else if (value -> ty -> tag == KOOPA_RTT_POINTER){
                ptr_type = (koopa_raw_type_t)(value -> ty -> data.pointer.base);
                if (is_value_stack_set(&(kind))){
                    addr = get_value_stack(&(kind));
                    switch (ptr_type -> tag){
                        case KOOPA_RTT_ARRAY:
                            cur = alloc_reg();
                            shift = atoi(addr.substr(0, addr.length() - 4).c_str());
                            cout << "  addi  " << cur << ", sp, " << shift << endl;
                            return cur;
                        case KOOPA_RTT_INT32:
                        case KOOPA_RTT_POINTER:
                            return addr;
                        default:
                            cout << "bad pointer type" << endl;
                    }
                    return addr;
                }
                else{
                    //cout << "Alloc an pointer" << endl;
                    switch (ptr_type -> tag){
                        case KOOPA_RTT_ARRAY:
                            //cout << "Alloc an pointer of array" << endl;
                            addr = to_string(4 * stack_top) + "(sp)";
                            set_value_stack(&(kind), addr);
                            stack_top += parse_stack(ptr_type);
                            return "??? Initial Alloc Pointer of Array";
                        case KOOPA_RTT_INT32:
                        case KOOPA_RTT_POINTER:
                            //cout << "Alloc an pointer of int/pointer" << endl;
                            addr = to_string(4 * (stack_top++)) + "(sp)";
                            set_value_stack(&(kind), addr);
                            return "??? Initial Alloc Pointer of Int/Pointer";
                        default:
                            cout << "bad pointer type" << endl;
                    }
                    return "??? undealed alloc pointer type";
                }
            }
            else
                cout << "??? undealed alloc type" << endl;
            return "??? unknown alloc type";
        case KOOPA_RVT_GLOBAL_ALLOC:
            if (value -> ty -> tag == KOOPA_RTT_INT32){
                // alloc an integer
                //cout << "global_alloc " << &(kind) << " " << value -> name << endl;
                if (is_value_data_set(&(kind))){
                    addr = get_value_data(&(kind));
                    return addr;
                }
                else{
                    cout << "  .data" << endl;
                    cout << "  .globl " << string((const char*) &(value -> name[1])) << endl;
                    cout << string((const char*) &(value -> name[1])) << ":" << endl;
                    set_value_data(&(kind), string(value -> name));
                    parse(kind.data.global_alloc);
                    //set_value_type(&(kind), VAR_TYPE, value -> ty);
                    return "??? Initial Global Alloc";
                }
            }
            else if (value -> ty -> tag == KOOPA_RTT_POINTER){
                ptr_type = (koopa_raw_type_t)(value -> ty -> data.pointer.base);
                if (is_value_data_set(&(kind))){
                    addr = get_value_data(&(kind));
                    switch (ptr_type -> tag){
                        case KOOPA_RTT_ARRAY:
                            cur = alloc_reg();
                            addr = string((const char*)&addr[1]);
                            cout << "  la    " << cur << ", " << addr << endl;
                            return cur;
                        case KOOPA_RTT_INT32:
                        case KOOPA_RTT_POINTER:
                            return addr;
                        default:
                            cout << "bad pointer type(global)" << endl;
                    }
                    return addr;
                }
                else{
                    //cout << "Alloc an pointer(global)" << endl;
                    cout << "  .data" << endl;
                    cout << "  .globl " << string((const char*) &(value -> name[1])) << endl;
                    cout << string((const char*) &(value -> name[1])) << ":" << endl;
                    set_value_data(&(kind), string(value -> name));
                    switch (ptr_type -> tag){
                        case KOOPA_RTT_ARRAY:
                            //cout << "Alloc an pointer of array(global)" << endl;
                            parse_global_arr(value);
                            return "??? Initial Alloc Pointer of Array(global)";
                        case KOOPA_RTT_INT32:
                        case KOOPA_RTT_POINTER:
                            //cout << "Alloc an pointer of int/pointer(global)" << endl;
                            parse(kind.data.global_alloc);
                            return "??? Initial Alloc Pointer of Int/Pointer(global)";
                        default:
                            cout << "bad pointer type(global)" << endl;
                    }
                    return "??? undealed alloc pointer type(global)";
                }
            }
            else
                cout << "??? WTF" << endl;
            return "??? unknown global alloc type";
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
                //cout << "LOAD FROM STACK" << endl;
                return cur;
            }
            else if (is_value_data_set(&(kind))){
                addr = get_value_data(&(kind));
                addr = string((const char*)&addr[1]);
                cur = alloc_reg();
                cout << "  la    " << cur << ", " << addr << endl;
                return cur;
            }
            else{
                assert(parse_param_mode != CALL_PARAMS);
                addr = parse(kind.data.load);
                if (addr[0] == '@' || addr[0] == '%')
                    // global variable
                    set_value_data(&(kind), addr);
                else{
                    if (on_stack(addr))
                        set_value_stack(&(kind), addr);
                        // value saved on stack
                    else{
                        cur = alloc_reg();
                        cout << "  lw    " << cur << ", 0(" << addr << ")" << endl;
                        addr = to_string(4 * (stack_top ++)) + "(sp)";
                        cout << "  sw    " << cur << ", " << addr << endl;
                        set_value_stack(&(kind), addr);
                        //value saved not on stack
                    }
                }
                return "??? Initial Load";
            }
            break;
        case KOOPA_RVT_STORE:
            // cout << "store " << &(kind) << endl;
            parse(kind.data.store);
            return "??? Initial Store";
        case KOOPA_RVT_GET_PTR:
            if (is_value_stack_set(&(kind))){
                cur = alloc_reg();
                addr = get_value_stack(&(kind));
                cout << "  lw    " << cur << ", " << addr << endl;
                return cur;
            }
            else{
                cur = parse(kind.data.get_elem_ptr);
                addr = to_string(4 * (stack_top++)) + "(sp)";
                cout << "  sw    " << cur << ", " << addr << endl;
                set_value_stack(&(kind), addr);
                return "??? Initial Get Ptr";
            }
            break;
        case KOOPA_RVT_GET_ELEM_PTR:
            if (is_value_stack_set(&(kind))){
                cur = alloc_reg();
                addr = get_value_stack(&(kind));
                cout << "  lw    " << cur << ", " << addr << endl;
                return cur;
            }
            else{
                cur = parse(kind.data.get_elem_ptr);
                addr = to_string(4 * (stack_top++)) + "(sp)";
                cout << "  sw    " << cur << ", " << addr << endl;
                set_value_stack(&(kind), addr);
                return "??? Initial Get Elem Ptr";
            }
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
    if (store.dest -> ty -> tag == KOOPA_RTT_POINTER)
        if (store.dest -> ty -> data.pointer.base -> tag == KOOPA_RTT_ARRAY){
            parse_local_arr(store);
            return;
            // local array assignment
        }
    // pointer assignment
    // ignore value type check because thay all have 32-bit size
    string cur = parse(store.value);
    if (is_value_stack_set(&(store.dest->kind))){
        string dest = get_value_stack(&(store.dest->kind));
        if (store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
            store.dest->kind.tag == KOOPA_RVT_GET_PTR){
                string temp = alloc_reg();
                cout << "  lw    " << temp << ", " << dest << endl;
                dest = "0(" + temp + ")";
            }
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

void parse_zeroinit(const string& base, int shift, int len){
    if (len <= 5){
        for (int i = 0; i < len; i++){
            string cur = alloc_reg();
            cout << "  sw    x0, " << 4 * shift << "(" << base << ")" << endl;
            remove_reg();
            shift++;
        }
    }
    else{
        string fin = alloc_reg();
        cout << "  addi  " << fin << ", " << base << ", " << 4 * (shift + len) << endl;
        string itr = alloc_reg();
        cout << "  addi  " << itr << ", " << base << ", " << 4 * shift << endl;
        string br = alloc_reg();
        static int zero_init_num = 0;
        string block_name = "br_" + to_string(zero_init_num++) + "_zeroinit";
        cout << block_name << ":" << endl;
        cout << "  sw    x0, " << "0(" << itr << ")" << endl;
        cout << "  addi  " << itr << ", " << itr << ", " << 4 << endl;
        cout << "  xor   " << br << ", " << itr << ", " << fin << endl;
        cout << "  bnez  " << br << ", " << block_name << endl;
    }
}

void parse_local_aggr(const koopa_raw_aggregate_t &aggr, 
                     const koopa_raw_type_t &type,
                     const string& base,
                     int shift){
    int arr_size = parse_stack(type);
    int sub_arr_size = arr_size / type -> data.array.len;
    const koopa_raw_type_t &sub_type = type -> data.array.base;
    int zero_num = 0;
    const koopa_raw_slice_t &slice = aggr.elems;
    assert(slice.kind == KOOPA_RSIK_VALUE);
    for (int i = 0; i < slice.len; i++){
        auto ptr = slice.buffer[i];
        const koopa_raw_value_t& value = reinterpret_cast<koopa_raw_value_t>(ptr);
        const auto &kind = value -> kind;
        if (kind.tag == KOOPA_RVT_ZERO_INIT ||
            (kind.tag == KOOPA_RVT_INTEGER && kind.data.integer.value == 0))
                zero_num += sub_arr_size;
        else{
            if (zero_num){
                parse_zeroinit(base, shift - zero_num, zero_num);
                zero_num = 0;
            }
            parse_local_arr(value, sub_type, base, shift);
        }
        shift += sub_arr_size;
    }
    if (zero_num)
        parse_zeroinit(base, shift - zero_num, zero_num);
}
    
void parse_local_arr(const koopa_raw_value_t &value, 
                     const koopa_raw_type_t &type,
                     const string& base,
                     int shift){
    string cur;
    int arr_size;
    const auto &kind = value -> kind;
    switch (kind.tag){
        case KOOPA_RVT_ZERO_INIT:
            arr_size = parse_stack(type);
            parse_zeroinit(base, shift, arr_size);
            break;
        case KOOPA_RVT_INTEGER:
            assert(type -> tag == KOOPA_RTT_INT32);
            cur = alloc_reg();
            cout << "  li    " << cur << ", " << kind.data.integer.value << endl;
            cout << "  sw    " << cur << ", " << 4 * shift << "(" << base << ")" << endl;
            break;
        case KOOPA_RVT_AGGREGATE:
            parse_local_aggr(kind.data.aggregate, type, base, shift);
            break;
        default:
            cout << "Bad local array initialization" << endl;
    }
}


void parse_global_aggr(const koopa_raw_aggregate_t &aggr, 
                      const koopa_raw_type_t &type){
    int arr_size = parse_stack(type);
    int sub_arr_size = arr_size / type -> data.array.len;
    const koopa_raw_type_t &sub_type = type -> data.array.base;
    int zero_num = 0;
    const koopa_raw_slice_t &slice = aggr.elems;
    assert(slice.kind == KOOPA_RSIK_VALUE);
    for (int i = 0; i < slice.len; i++){
        auto ptr = slice.buffer[i];
        const koopa_raw_value_t& value = reinterpret_cast<koopa_raw_value_t>(ptr);
        const auto &kind = value -> kind;
        if (kind.tag == KOOPA_RVT_ZERO_INIT ||
            (kind.tag == KOOPA_RVT_INTEGER && kind.data.integer.value == 0))
                zero_num += sub_arr_size;
        else{
            if (zero_num){
                cout << "  .zero " << zero_num * 4 << endl;
                zero_num = 0;
            }
            parse_global_arr(value, sub_type);
        }
    }
    if (zero_num)
        cout << "  .zero " << zero_num * 4 << endl;
}
void parse_global_arr(const koopa_raw_value_t &value, 
                     const koopa_raw_type_t &type){
    string cur;
    int arr_size;
    const auto &kind = value -> kind;
    switch (kind.tag){
        case KOOPA_RVT_ZERO_INIT:
            arr_size = parse_stack(type);
            cout << "  .zero " << arr_size * 4 << endl; 
            break;
        case KOOPA_RVT_INTEGER:
            assert(type -> tag == KOOPA_RTT_INT32);
            cout << "  .word " << kind.data.integer.value << endl;
            break;
        case KOOPA_RVT_AGGREGATE:
            parse_global_aggr(kind.data.aggregate, type);
            break;
        default:
            cout << "Bad local array initialization" << endl;
    }
}
void parse_local_arr(const koopa_raw_store_t &store){
    const koopa_raw_type_t &type = store.dest -> ty -> data.pointer.base;
    assert(type -> tag == KOOPA_RTT_ARRAY);
    const koopa_raw_value_t &value = store.value;
    assert(is_value_stack_set(&(store.dest -> kind)));
    string addr = get_value_stack(&(store.dest -> kind));
    int shift = atoi(addr.substr(0, addr.length() - 4).c_str()) / 4;
    parse_local_arr(value, type, "sp", shift);
}
void parse_global_arr(const koopa_raw_value_t &value){
    const koopa_raw_type_t &type = value -> ty -> data.pointer.base;
    assert(type -> tag == KOOPA_RTT_ARRAY);
    const koopa_raw_value_t &arr_value = value -> kind.data.global_alloc.init;
    parse_global_arr(arr_value, type);
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


string parse(const koopa_raw_get_ptr_t &get_ptr){
    const koopa_raw_type_t type = get_ptr.src -> ty;
    assert(type -> tag == KOOPA_RTT_POINTER);
    const koopa_raw_type_t base_type = type -> data.pointer.base;
    int sub_size = parse_stack(base_type) * 4;
    string cur = alloc_reg();
    string base = parse(get_ptr.src);
    string index = parse(get_ptr.index);
    cout << "  li    " << cur << ", " << sub_size << endl;
    cout << "  mul   " << cur << ", " << cur << ", " << index << endl;
    cout << "  add   " << cur << ", " << cur << ", " << base << endl;
    return cur;
}

string parse(const koopa_raw_get_elem_ptr_t &get_elem_ptr){
    const koopa_raw_type_t type = get_elem_ptr.src -> ty;
    assert(type -> tag == KOOPA_RTT_POINTER);
    const koopa_raw_type_t base_type = type -> data.pointer.base;
    assert(base_type -> tag == KOOPA_RTT_ARRAY);
    const koopa_raw_type_t sub_type = base_type -> data.array.base;
    int sub_size = parse_stack(sub_type) * 4;
    string cur = alloc_reg();
    string base = parse(get_elem_ptr.src);
    string index = parse(get_elem_ptr.index);
    cout << "  li    " << cur << ", " << sub_size << endl;
    cout << "  mul   " << cur << ", " << cur << ", " << index << endl;
    cout << "  add   " << cur << ", " << cur << ", " << base << endl;
    return cur;
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