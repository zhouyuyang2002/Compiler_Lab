#include <vector>
#include <string>

using namespace std;

vector<int> loop_idx;
vector<int> loop_body_idx;

void alloc_loop(int br_id){
    loop_idx.push_back(br_id);
    loop_body_idx.push_back(0);
}

void alloc_loop_body(){
    loop_body_idx[loop_body_idx.size() - 1]++;
}

void remove_loop(){
    loop_idx.pop_back();
    loop_body_idx.pop_back();
}

string loop_head_id(){
    return "%while_head" + to_string(loop_idx.back());
}

string loop_body_id(){
    return "%while_body" + to_string(loop_idx.back()) + "_" + to_string(loop_body_idx.back());
}
string loop_end_id(){
    return "%while_end" + to_string(loop_idx.back());
}
