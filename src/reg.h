#ifndef __MY_REG_H
#define __MY_REG_H

#include "koopa.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

bool is_reg(const std::string &loc){
    if (loc.length() != 2)
        return false;
    if (loc == "x0")
        return true;
    if (loc[0] == 't' && loc[1] >= '0' && loc[1] <= '6')
        return true;
    if (loc[0] == 'a' && loc[1] >= '0' && loc[1] <= '7')
        return true;
    return false;
}

/*
register management
*/
static int reg_num = 0;

void reset_reg(){
    reg_num = 0;
}
string alloc_reg(){
    int val = reg_num ++;
    if (val <= 6)
        return "t" + to_string(val);
    return "a" + to_string(val - 7);
}
string realloc_reg(const std::string &src, const std::string &dst){
    if (dst != src)
        cout << "  xor   " << dst << ", " << src << ", x0" << endl;
    return dst;
}

/* 
  map the result of value(the address of the value in raw program)
   , to the name of register who save it.
*/
unordered_map<uint64_t, string> map_value_reg;

void set_value_reg(const void* addr, const string reg){
    map_value_reg[(uint64_t)addr] = reg;
}

bool is_value_reg_set(const void* addr){
    return map_value_reg.find((uint64_t)addr) != map_value_reg.end();
}

string get_value_reg(const void* addr){
    return map_value_reg[(uint64_t)addr];
}

unordered_map<uint64_t, string> map_value_stack;
void set_value_stack(const void* addr, const string stack){
    map_value_stack[(uint64_t)addr] = stack;
}

bool is_value_stack_set(const void* addr){
    return map_value_stack.find((uint64_t)addr) != map_value_stack.end();
}

string get_value_stack(const void* addr){
    return map_value_stack[(uint64_t)addr];
}
#endif