%code requires {
  #include <memory>
  #include <string>
}

%{

#include "type.h"

#include <iostream>
#include <memory>
#include <string>
#include <cassert>

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

static int exp_num = 0;
%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  char chr_val;
  BaseAST *ast_val;
  rel_op_type rel_op_val;
  eq_op_type eq_op_val;
}

%token INT RETURN LAND LOR
%token EQ NEQ LE GE
%token <str_val> IDENT
%token <int_val> INT_CONST 
%type <ast_val> FuncDef FuncType Block Stmt Number
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp
%type <ast_val> LOrExp LAndExp RelExp EqExp
%type <chr_val> UnaryOp AddOp MulOp
%type <rel_op_val> RelOp
%type <eq_op_val> EqOp

%%

CompUnit
  : FuncDef {
    auto temp = make_unique<CompUnitAST>();
    temp -> func_def = unique_ptr<BaseAST>($1);
    ast = move(temp);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast -> func_type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast -> type = string("int");
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast -> stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast -> exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast -> val = $1;
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast -> l_or_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST1();
    ast -> exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|
  Number {
    auto ast = new PrimaryExpAST2();
    ast -> number = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  
UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST1();
    ast -> primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  UnaryOp UnaryExp {
    auto ast = new UnaryExpAST2();
    ast -> unary_op = char($1);
    ast -> unary_exp = unique_ptr<BaseAST>($2);
    switch ($1){
      case '+':
        ast -> exp_id = -1;
        break;
      case '-':
      case '!':
        ast -> exp_id = exp_num++;
        break;
      default:
        assert(false);
    }
    $$ = ast;
  }

MulExp
  : UnaryExp{
    auto ast = new MulExpAST1();
    ast -> unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| MulExp MulOp UnaryExp{
    auto ast = new MulExpAST2();
    ast -> mul_op = char($2);
    ast -> mul_exp = unique_ptr<BaseAST>($1);
    ast -> unary_exp = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

AddExp
  : MulExp{
    auto ast = new AddExpAST1();
    ast -> mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| AddExp AddOp MulExp{
    auto ast = new AddExpAST2();
    ast -> add_op = char($2);
    ast -> add_exp = unique_ptr<BaseAST>($1);
    ast -> mul_exp = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

RelExp
  : AddExp{
    auto ast = new RelExpAST1();
    ast -> add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| RelExp RelOp AddExp{
    auto ast = new RelExpAST2();
    ast -> rel_op = $2;
    ast -> rel_exp = unique_ptr<BaseAST>($1);
    ast -> add_exp = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

EqExp
  : RelExp{
    auto ast = new EqExpAST1();
    ast -> rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| EqExp EqOp RelExp{
    auto ast = new EqExpAST2();
    ast -> eq_op = $2;
    ast -> eq_exp = unique_ptr<BaseAST>($1);
    ast -> rel_exp = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

LAndExp
  : EqExp{
    auto ast = new LAndExpAST1();
    ast -> eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| LAndExp LAND EqExp{
    auto ast = new LAndExpAST2();
    ast -> l_and_exp = unique_ptr<BaseAST>($1);
    ast -> eq_exp = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

LOrExp
  : LAndExp{
    auto ast = new LOrExpAST1();
    ast -> l_and_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| LOrExp LOR LAndExp{
    auto ast = new LOrExpAST2();
    ast -> l_or_exp = unique_ptr<BaseAST>($1);
    ast -> l_and_exp = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

UnaryOp
  : '+' {
    $$ = '+';
  }|
  '-' {
    $$ = '-';
  }|
  '!' {
    $$ = '!';
  }

MulOp
  : '*' {
    $$ = '*';
  }|
  '/' {
    $$ = '/';
  }|
  '%' {
    $$ = '%';
  }

AddOp
  : '+' {
    $$ = '+';
  }|
  '-' {
    $$ = '-';
  }

RelOp
  : '<' {
    $$ = REL_OP_LT;
  }|
  '>' {
    $$ = REL_OP_GT;
  }|
  LE {
    $$ = REL_OP_LE;
  }|
  GE {
    $$ = REL_OP_GE;
  }

EqOp
  : EQ {
    $$ = EQ_OP_EQ;
  }|
  NEQ {
    $$ = EQ_OP_NEQ;
  }
  
%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  extern int yylineno;
  extern char *yytext;

  int len = strlen(yytext);
  char buf[512] = {0};
  for (int i = 0; i < len; i++)
    sprintf(buf, "%s,%d", buf, (int)yytext[i]);
  fprintf(stderr, "Error, %s at line %d with ASCII code (%s), %s\n", s, yylineno, buf, yytext);
}