#ifndef __MY_SYMBOL_H
#define __MY_SYMBOL_H

#include <cstring>

void alloc_symbol_space();
void remove_symbol_space();
int get_symbol_depth(const std::string &name);
void set_symbol_value(const std::string &name, int value);
void set_symbol_var_id(const std::string &name, int var_id);
bool is_symbol_value_set(const std::string &name);
bool is_symbol_var_id_set(const std::string &name);
std::string get_symbol_name(const std::string &name);
int get_symbol_value(const std::string &name);
int get_symbol_var_id(const std::string &name);

#endif