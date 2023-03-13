#include "koopa.h"
#include <set>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cassert>
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

static int depth = -1;
static int index_num = 0;
vector<unordered_map<string, symbol_value_t> > symbol_tables;
unordered_map<string, set<int> > symbol_depth;
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