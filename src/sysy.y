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
#include <vector>

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

static int exp_num = 0;
static int var_num = 0;
static int br_num = 0;
%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  char chr_val;
  BaseAST *ast_val;
  vector<unique_ptr<BaseAST>> *vector_ast_vals;
  rel_op_type rel_op_val;
  eq_op_type eq_op_val;
}


%token INT RETURN LAND LOR
%token EQ NEQ LE GE CONST
%token IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST 
%type <ast_val> FuncDef FuncType Block BlockItem BlockItems Stmt Number
%type <ast_val> Decl ConstDecl ConstDef ConstDefs ConstInitVal LVal ConstExp
%type <ast_val> VarDecl VarDefs VarDef InitVal BType IfStmt
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
  : '{' BlockItems '}' {
    auto ast = new BlockAST();
    ast -> block_items = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|
  '{' '}' {
    auto ast = new BlockAST();
    ast -> block_items = nullptr;
    $$ = ast;
  }

BlockItems
  : BlockItem {
    auto ast = new BlockItemListAST();
    ast -> block_item = unique_ptr<BaseAST>($1);
    ast -> next = nullptr;
    $$ = ast;
  }|
  BlockItems BlockItem {
    auto ast = new BlockItemListAST();
    ast -> block_item = unique_ptr<BaseAST>($2);
    ast -> next = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast -> decl_or_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  Stmt {
    auto ast = new BlockItemAST();
    ast -> decl_or_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  
Stmt
  : LVal '=' Exp ';'{
    auto ast = new StmtAST1();
    ast -> l_val = unique_ptr<BaseAST>($1);
    ast -> exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }|
  RETURN Exp ';' {
    auto ast = new StmtAST2();
    ast -> exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|
  ';' {
    auto ast = new StmtAST3();
    ast -> exp = nullptr;
    $$ = ast;
  }|
  Exp ';' {
    auto ast = new StmtAST3();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  Block {
    auto ast = new StmtAST4();
    ast -> block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  IfStmt {
    auto ast = new StmtAST5();
    ast -> if_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST6();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> stmt = unique_ptr<BaseAST>($5);
    ast -> br_id = br_num++;
    $$ = ast;
  }|
  CONTINUE ';' {
    auto ast = new StmtAST7();
    $$ = ast;
  }|
  BREAK ';' {
    auto ast = new StmtAST8();
    $$ = ast;
  }

IfStmt :
  IF '(' Exp ')' Stmt {
    auto ast = new IfStmtAST1();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> then_stmt = unique_ptr<BaseAST>($5);
    ast -> br_id = br_num++;
    $$ = ast;
  }|
  IF '(' Exp ')' Stmt ELSE Stmt{
    auto ast = new IfStmtAST2();
    ast -> exp = unique_ptr<BaseAST>($3);
    ast -> then_stmt = unique_ptr<BaseAST>($5);
    ast -> else_stmt = unique_ptr<BaseAST>($7);
    ast -> br_id = br_num++;
    $$ = ast;
  }

Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast -> const_or_var_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  VarDecl {
    auto ast = new DeclAST();
    ast -> const_or_var_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstDecl
  : CONST BType ConstDefs ';' {
    auto ast = new ConstDeclAST();
    ast -> b_type = unique_ptr<BaseAST>($2);
    ast -> const_defs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

BType
  : INT {
    auto ast = new BTypeAST();
    ast -> type = string("i32");
    $$ = ast;
  }

ConstDefs
  : ConstDef {
    auto ast = new ConstDefListAST();
    ast -> const_def = unique_ptr<BaseAST>($1);
    ast -> next = nullptr;
    $$ = ast;
  }|
  ConstDefs ',' ConstDef {
    auto ast = new ConstDefListAST();
    ast -> const_def = unique_ptr<BaseAST>($3);
    ast -> next = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast -> const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

VarDecl
  : BType VarDefs ';' {
    auto ast = new VarDeclAST();
    ast -> b_type = unique_ptr<BaseAST>($1);
    ast -> var_defs = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

VarDefs
  : VarDef {
    auto ast = new VarDefListAST();
    ast -> next = nullptr;
    ast -> var_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  VarDefs ',' VarDef {
    auto ast = new VarDefListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> var_def = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> init_val = nullptr;
    ast -> var_id = var_num++;
    $$ = ast;
  }|
  IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> init_val = unique_ptr<BaseAST>($3);
    ast -> var_id = var_num++;
    $$ = ast;
  }

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

LVal
  : IDENT {
    auto ast = new LValAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast -> val = $1;
    $$ = ast;
  }

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
  }|
  LVal {
    auto ast = new PrimaryExpAST3();
    ast -> l_val = unique_ptr<BaseAST>($1);
    ast -> exp_id = exp_num++;
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
    exp_num ++;
    ast -> exp_id = exp_num++;
    ast -> br_id = br_num++;
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
    exp_num ++;
    ast -> exp_id = exp_num++;
    ast -> br_id = br_num++;
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