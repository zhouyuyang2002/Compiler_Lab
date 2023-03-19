#include <vector>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include "arr.h"

using namespace std;


/*size of each dimension*/
vector<int> arr_size;
/*size of sub array with the higher * dims*/
vector<int> sub_arr_size;
/*initial value in sub array*/
vector<string> arr_val;
/*the pointer of each InitValArr*/
vector<int> initval_ptr;
/*the size of each InitValArr*/
vector<int> initval_size;
/*the number of dimensions of the array*/
int dim_num;
/*the pointer of current number*/
int arr_ptr;
/*the upper_bound of current pointer*/
int arr_ptr_ub;
/*the type of InitValArr, const or variable*/
arr_type_t arr_type;

void clear_arr_info(){
    arr_size.resize(0);
    arr_size.push_back(1);//initial {
    sub_arr_size.resize(0);
    arr_val.resize(0);
    initval_ptr.resize(0);
    initval_size.resize(0);
    arr_ptr = 0;
    dim_num = 0;
    arr_ptr_ub = 0;
}

void alloc_dim(int dim_size){
    assert(dim_size >= 1);
    arr_size.push_back(dim_size);
    ++dim_num;
}

void pre_arr_initialize(arr_type_t type){
    arr_type = type;
    assert(arr_size.size() >= 1);
    //for (auto i:arr_size)
    //   cout << i << ' '; cout << endl;
    sub_arr_size = arr_size;
    dim_num = sub_arr_size.size() - 1;
    for (int i = dim_num; i ; i--){
        int val = (1 << 25) / sub_arr_size[i] + 1;
        assert(val >= sub_arr_size[i - 1]);
        sub_arr_size[i - 1] *= sub_arr_size[i];
    }
    sub_arr_size.push_back(1);
    initval_ptr.push_back(0);
    initval_size.push_back(0);
    arr_ptr = 0;
    arr_ptr_ub = sub_arr_size[0];
    arr_val.resize(arr_ptr_ub);
}

void left_bracket(){
    int dim = initval_size.back();
    assert(arr_ptr == 0 || initval_ptr.size() > 1);
    if (dim <= dim_num) ++dim;
    while (dim <= dim_num && arr_ptr % sub_arr_size[dim]) ++dim;
    if (arr_ptr == arr_ptr_ub)
        initval_ptr.push_back(arr_ptr - sub_arr_size[dim]);
    else
        initval_ptr.push_back(arr_ptr);
    initval_size.push_back(dim);
    arr_ptr_ub = initval_ptr.back() + sub_arr_size[dim];
}

void insert_arr_val(const string &val){
    if (arr_ptr != arr_ptr_ub)
        arr_val[arr_ptr++] = val;
}

void right_bracket(){
    assert(initval_size.size());
    initval_ptr.pop_back();
    initval_size.pop_back();
    arr_ptr = arr_ptr_ub;
    arr_ptr_ub = initval_ptr.back() + sub_arr_size[initval_size.back()];
}


string arr_val_merge(const vector<string>&res, int l, int r){
    if (l == r)
        return res[l];
    int mid = (l + r) / 2;
    return arr_val_merge(res, l, mid) + ", " + arr_val_merge(res, mid + 1, r);
}

string get_arr_val_range(int l, int r, int depth){
    if (depth > dim_num){
        assert(l == r);
        if (arr_val[l] == "")
            return "0";
        return arr_val[l];
    }
    vector<string> res;
    bool all_zero = true;
    for (int i = 0; i < arr_size[depth]; i++){
        int mid = l + sub_arr_size[depth + 1];
        string temp = get_arr_val_range(l, mid - 1, depth + 1);
        if (temp == "zeroinit" || temp == "0");
        else all_zero = false;
        res.push_back(temp);
        l = mid;
    }
    assert(l == r + 1);
    if (all_zero) return "zeroinit";
    return "{" + arr_val_merge(res, 0, res.size() - 1) + "}";
}

string get_arr_val(){
    return get_arr_val_range(0, sub_arr_size[0] - 1, 1);
}

string get_arr_exp_id(){
    string res = "i32";
    for (int i = (int)arr_size.size() - 1; i >= 1; i--)
        res = "[" + res + ", " + to_string(arr_size[i]) + "]";
    return res;
}

vector<int> dim_param;
void fit_local_arr(const string &name, int l, int r, int depth){
    if (depth == dim_num + 1){
        static int local_arr_idx = 0;
        if (arr_val[l] != "" && arr_val[l] != "0"){
            string temp = "@" + name;
            local_arr_idx++;
            for (int d = 1; d <= dim_num; d++){
                string cur = "@fit_" + to_string(local_arr_idx) + "_" + to_string(d) + "_local_arr";
                cout << "  " << cur << " = getelemptr " << temp << ", " << dim_param[d] << endl;
                temp = cur;
            }
            cout << "  store " << arr_val[l] << ", " << temp << endl;
        }
        return;
    }
    for (int i = 0; i < arr_size[depth]; i++){
        int mid = l + sub_arr_size[depth + 1];
        dim_param[depth] = i;
        fit_local_arr(name, l, mid - 1, depth + 1);
        l = mid;
    }
}
void fit_local_arr(const string &name){
    dim_param.resize(dim_num + 2);
    fit_local_arr(name, 0, sub_arr_size[0] - 1, 1);
}