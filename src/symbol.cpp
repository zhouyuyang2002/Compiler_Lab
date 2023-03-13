#include "koopa.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

/* 
  map the result of value(the address of the value in raw program)
   , to the name of register who save it.
*/

enum symbol_type_t{
    SYMBOL_CONST,
    SYMBOL_VAR
};

typedef struct symbol_value{
    symbol_type_t type;
    union {
        int value;
        int var_id;
    };
} symbol_value_t;

unordered_map<string, symbol_value_t> symbol_table;

void set_symbol_value(string name, int value){
    symbol_table[name] = (symbol_value_t){SYMBOL_CONST, {value}};
}

void set_symbol_var_id(string name, int var_id){
    symbol_table[name] = (symbol_value_t){SYMBOL_VAR, {var_id}};
}

bool is_symbol_value_set(string name){
    auto itr = symbol_table.find(name);
    if (itr == symbol_table.end())
        return false;
    return itr->second.type == SYMBOL_CONST;
}

bool is_symbol_var_id_set(string name){
    auto itr = symbol_table.find(name);
    if (itr == symbol_table.end())
        return false;
    return itr->second.type == SYMBOL_VAR;
}

int get_symbol_value(string name){
    return symbol_table[name].value;
}

int get_symbol_var_id(string name){
    return symbol_table[name].var_id;
}