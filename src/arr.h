#ifndef __MY_ARR_H
#define __MY_ARR_H

#include <vector>
#include <string>

using namespace std;

enum arr_type_t{
    CONST_ARR,
    VAR_ARR
};

void clear_arr_info();
void alloc_dim(int dim_size);
void pre_arr_initialize(arr_type_t type);
void left_bracket();
void insert_arr_val(const std::string &val);
void right_bracket();
std::string get_arr_val();
std::string get_arr_exp_id();
void fit_local_arr(const std::string &str);

#endif
