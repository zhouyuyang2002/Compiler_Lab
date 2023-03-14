#include <set>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>
#include <unordered_map>

#include "koopa.h"
#include "symbol.h"

using namespace std;

typedef struct symbol_value{
    symbol_type_t type;
    union {
        int value;
        int var_id;
    };
} symbol_value_t;

static int depth = -1;
static int index_num = 0;
vector<unordered_map<string, symbol_value_t> > symbol_tables;
unordered_map<string, set<int> > symbol_depth;
unordered_map<string, func_type_t> symbol_func;
vector<int> table_index;

void alloc_symbol_space(){
    unordered_map<string, symbol_value_t> symbol_table;
    symbol_tables.push_back(symbol_table);
    table_index.push_back(++index_num);
    ++depth;
}

void remove_symbol_space(){
    unordered_map<string, symbol_value_t> symbol_table = symbol_tables.back();
    for (auto symbol: symbol_table)
        symbol_depth[symbol.first].erase(depth);
    symbol_tables.pop_back();
    table_index.pop_back();
    --depth;
}

int get_symbol_depth(const string &name){
    auto itr = symbol_depth.find(name);
    if (itr == symbol_depth.end())
        return -1;
    if (itr->second.begin() == itr->second.end())
        return -1;
    return *(itr->second.rbegin());
}

void set_symbol_value(const string &name, int value){
    symbol_tables[depth][name] = (symbol_value_t){SYMBOL_CONST, {value}};
    symbol_depth[name].insert(depth);
}

void set_symbol_var_id(const string &name, int var_id){
    symbol_tables[depth][name] = (symbol_value_t){SYMBOL_VAR, {var_id}};
    symbol_depth[name].insert(depth);
}

bool is_symbol_value_set(const string &name){
    auto var_depth = get_symbol_depth(name);
    if (var_depth == -1)
        return false;
    auto itr = symbol_tables[var_depth].find(name);
    return itr->second.type == SYMBOL_CONST;
}

bool is_symbol_var_id_set(const string &name){
    auto var_depth = get_symbol_depth(name);
    assert(var_depth <= depth);
    if (var_depth == -1)
        return false;
    auto itr = symbol_tables[var_depth].find(name);
    return itr->second.type == SYMBOL_VAR;
}

string get_symbol_name(const string &name){
    auto var_depth = get_symbol_depth(name);
    assert(var_depth >= 0 && var_depth <= depth);
    return name + "_" + to_string(table_index[var_depth]);
}

string get_symbol_param_name(const string &name){
    return name + "_" + to_string(table_index.back()) + "_param"; 
}

int get_symbol_value(const string &name){
    auto var_depth = get_symbol_depth(name);
    assert(var_depth >= 0 && var_depth <= depth);
    return symbol_tables[var_depth][name].value;
}

int get_symbol_var_id(const string &name){
    auto var_depth = get_symbol_depth(name);
    assert(var_depth >= 0 && var_depth <= depth);
    return symbol_tables[var_depth][name].var_id;
}

void set_symbol_func(const string& name, func_type_t f_type){
    assert(symbol_func.find(name) == symbol_func.end());
    symbol_func[name] = f_type;
}

bool is_symbol_func_set(const string& name){
    return symbol_func.find(name) != symbol_func.end();
}

func_type_t get_symbol_func(const string& name){
    assert(is_symbol_func_set(name));
    return symbol_func[name];
}