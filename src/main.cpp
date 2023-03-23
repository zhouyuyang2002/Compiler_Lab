#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h>
#include "type.h"
#include "riscv.h"

using namespace std;
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int file_size(char* fname){  
    struct stat statbuf;  
    stat(fname, &statbuf);  
    int size=statbuf.st_size;  
    return size;  
}  

int main(int argc, const char *argv[]){
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  yyin = fopen(input, "r");
  assert(yyin);

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  if (string(mode) == "-koopa"){
    freopen(output, "w", stdout);
    ast -> DumpIR();
    cout << endl;
    fclose(stdout);
  }
  if (string(mode) == "-riscv" || string(mode) == "-perf"){
    char buf[512] = {0};
    sprintf(buf, "%s.koopa", input);
    freopen(buf, "w", stdout);
    ast -> DumpIR();
    cout << endl;
    fclose(stdout);

    FILE* fin = fopen(buf, "r");
    int fsize = file_size(buf);
    char* koopa = new char[fsize + 105];
    fread(koopa, 1, fsize + 100, fin);
    freopen(output, "w", stdout);
    parse_koopa(koopa);
    delete[] koopa;
  }
  return 0;
}
