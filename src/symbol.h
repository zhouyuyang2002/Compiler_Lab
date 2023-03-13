#ifndef __MY_SYMBOL_H
#define __MY_SYMBOL_H

#include <cstring>

void set_symbol_value(std::string name, int value);
void set_symbol_var_id(std::string name, int var_id);
bool is_symbol_value_set(std::string name);
bool is_symbol_var_id_set(std::string name);
int get_symbol_value(std::string name);
int get_symbol_var_id(std::string name);

#endif