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
%token EQ NEQ LE GE CONST VOID
%token IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT
%token <int_val> INT_CONST 
%type <ast_val> FuncDefList FuncDef FuncType FuncFParams FuncFParam FuncFParamInt FuncFParamArr FuncRParams
%type <ast_val> Block BlockItem BlockItems Stmt IfStmt Number Decl
%type <ast_val> ConstDecl ConstDefs ConstDef ConstDefIDENT ConstDefArr
%type <ast_val> ConstInitVal ConstInitValArr ConstInitValList ConstInitValElem ConstExp
%type <ast_val> VarDecl VarDefs VarDef VarDefIDENT VarDefArr
%type <ast_val> ArrSizeList ArrSize ArrParamList ArrParam
%type <ast_val> InitVal InitValArr InitValList InitValElem BType 
%type <ast_val> LVal LValArr LValIDENT
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp
%type <ast_val> LOrExp LAndExp RelExp EqExp
%type <chr_val> UnaryOp AddOp MulOp
%type <rel_op_val> RelOp
%type <eq_op_val> EqOp

%%

CompUnit
  : FuncDefList {
    auto temp = make_unique<CompUnitAST>();
    temp -> func_defs = unique_ptr<BaseAST>($1);
    ast = move(temp);
  }

FuncDefList: 
  Decl {
    auto ast = new FuncDefListAST();
    ast -> next = nullptr;
    ast -> func_def = nullptr;
    ast -> decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  FuncDef {
    auto ast = new FuncDefListAST();
    ast -> next = nullptr;
    ast -> func_def = unique_ptr<BaseAST>($1);
    ast -> decl = nullptr;
    $$ = ast;
  }|
  FuncDefList Decl {
    auto ast = new FuncDefListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> func_def = nullptr;
    ast -> decl = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|
  FuncDefList FuncDef {
    auto ast = new FuncDefListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> func_def = unique_ptr<BaseAST>($2);
    ast -> decl = nullptr;
    $$ = ast;
  }


FuncDef
  : FuncType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast -> func_type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> func_f_params = unique_ptr<BaseAST>($4);
    ast -> block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }|
  FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast -> func_type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> func_f_params = nullptr;
    ast -> block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }|
  BType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast -> func_type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> func_f_params = unique_ptr<BaseAST>($4);
    ast -> block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }|
  BType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast -> func_type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> func_f_params = nullptr;
    ast -> block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }

FuncType
  : VOID {
    auto ast = new FuncTypeAST();
    ast -> type = string("void");
    $$ = ast;
  }

FuncFParams
  : FuncFParam {
    auto ast = new FuncFParamListAST();
    ast -> next = nullptr;
    ast -> func_f_param = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  FuncFParams ',' FuncFParam {
    auto ast = new FuncFParamListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> func_f_param = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

FuncFParam:
  FuncFParamInt{
    auto ast = new FuncFParamAST();
    ast -> param = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  FuncFParamArr{
    auto ast = new FuncFParamAST();
    ast -> param = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

FuncFParamInt:
  BType IDENT{
    auto ast = new FuncFParamIntAST();
    ast -> type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    $$ = ast;
  }

FuncFParamArr:
  BType IDENT '[' ']'{
    auto ast = new FuncFParamArrAST();
    ast -> type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> size = nullptr;
    $$ = ast;
  }|
  BType IDENT '[' ']' ArrSizeList{
    auto ast = new FuncFParamArrAST();
    ast -> type = unique_ptr<BaseAST>($1);
    ast -> ident = *unique_ptr<string>($2);
    ast -> size = unique_ptr<BaseAST>($5);
    $$ = ast;
  }

FuncRParams
  : Exp {
    auto ast = new FuncFParamListAST();
    ast -> next = nullptr;
    ast -> func_f_param = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  FuncRParams ',' Exp {
    auto ast = new FuncFParamListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> func_f_param = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

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
  RETURN ';' {
    auto ast = new StmtAST2();
    ast -> exp = nullptr;
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
  : ConstDefIDENT {
    auto ast = new ConstDefAST();
    ast -> def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  ConstDefArr {
    auto ast = new ConstDefAST();
    ast -> def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstDefArr
  : IDENT ArrSizeList '=' ConstInitValArr{
    auto ast = new ConstDefArrAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> arr_size = unique_ptr<BaseAST>($2);
    ast -> const_init_arr_val = unique_ptr<BaseAST>($4);
    ast -> var_id = var_num ++;
    $$ = ast;
  }

ConstDefIDENT
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefIDENTAST();
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

  
ConstInitValArr
  : '{' '}' {
    auto ast = new ConstInitValArrAST();
    ast -> const_init_val_list = nullptr;
    $$ = ast;
  }|
  '{' ConstInitValList '}' {
    auto ast = new ConstInitValArrAST();
    ast -> const_init_val_list = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

ConstInitValList
  : ConstInitValElem {
    auto ast = new ConstInitValListAST();
    ast -> next = nullptr;
    ast -> const_init_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  ConstInitValList ',' ConstInitValElem {
    auto ast = new ConstInitValListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

ConstInitValElem
  : ConstInitVal {
    auto ast = new ConstInitValElemAST();
    ast -> val = unique_ptr<BaseAST>($1);
    ast -> is_arr = false;
    $$ = ast;
  }|
  ConstInitValArr {
    auto ast = new ConstInitValElemAST();
    ast -> val = unique_ptr<BaseAST>($1);
    ast -> is_arr = true;
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
  : VarDefIDENT {
    auto ast = new VarDefAST();
    ast -> def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  VarDefArr {
    auto ast = new VarDefAST();
    ast -> def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

VarDefArr
  : IDENT ArrSizeList '=' InitValArr{
    auto ast = new VarDefArrAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> arr_size = unique_ptr<BaseAST>($2);
    ast -> init_arr_val = unique_ptr<BaseAST>($4);
    $$ = ast;
  }|
  IDENT ArrSizeList{
    auto ast = new VarDefArrAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> arr_size = unique_ptr<BaseAST>($2);
    ast -> init_arr_val = nullptr;
    $$ = ast;
  }

VarDefIDENT
  : IDENT {
    auto ast = new VarDefIDENTAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> init_val = nullptr;
    ast -> var_id = var_num++;
    $$ = ast;
  }|
  IDENT '=' InitVal {
    auto ast = new VarDefIDENTAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> init_val = unique_ptr<BaseAST>($3);
    ast -> var_id = var_num++;
    $$ = ast;
  }

ArrSizeList
  : ArrSize {
    auto ast = new ArrSizesAST();
    ast -> next = nullptr;
    ast -> arr_size = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  ArrSizeList ArrSize {
    auto ast = new ArrSizesAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> arr_size = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

ArrSize
  : '[' ConstExp ']' {
    auto ast = new ArrSizeAST();
    ast -> exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast -> exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

InitValArr
  : '{' '}' {
    auto ast = new InitValArrAST();
    ast -> init_val_list = nullptr;
    $$ = ast;
  }|
  '{' InitValList '}' {
    auto ast = new InitValArrAST();
    ast -> init_val_list = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

InitValList
  : InitValElem {
    auto ast = new InitValListAST();
    ast -> next = nullptr;
    ast -> init_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  InitValList ',' InitValElem {
    auto ast = new InitValListAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

InitValElem
  : InitVal {
    auto ast = new InitValElemAST();
    ast -> val = unique_ptr<BaseAST>($1);
    ast -> is_arr = false;
    $$ = ast;
  }|
  InitValArr {
    auto ast = new InitValElemAST();
    ast -> val = unique_ptr<BaseAST>($1);
    ast -> is_arr = true;
    $$ = ast;
  }


LVal
  : LValIDENT {
    auto ast = new LValAST();
    ast -> l_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  LValArr {
    auto ast = new LValAST();
    ast -> l_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

LValIDENT
  : IDENT {
    auto ast = new LValIDENTAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

LValArr
  : IDENT ArrParamList{
    auto ast = new LValArrAST();
    ast -> ident = *unique_ptr<string>($1);
    ast -> arr_param = unique_ptr<BaseAST>($2);
    exp_num ++;
    ast -> exp_id = exp_num++;
    $$ = ast;
  }

ArrParamList
  : ArrParam{
    auto ast = new ArrParamsAST();
    ast -> next = nullptr;
    ast -> arr_param = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|
  ArrParamList ArrParam{
    auto ast = new ArrParamsAST();
    ast -> next = unique_ptr<BaseAST>($1);
    ast -> arr_param = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

ArrParam
  : '[' Exp ']' {
    auto ast = new ArrParamAST();
    ast -> exp = unique_ptr<BaseAST>($2);
    ast -> exp_id = exp_num ++;
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
  }|
  IDENT '(' FuncRParams ')'{
    auto ast = new UnaryExpAST3();
    ast -> ident = *unique_ptr<string>($1);
    ast -> func_r_params = unique_ptr<BaseAST>($3);
    ast -> exp_id = exp_num++;
    $$ = ast;
  }|
  IDENT '(' ')'{
    auto ast = new UnaryExpAST3();
    ast -> ident = *unique_ptr<string>($1);
    ast -> func_r_params = nullptr;
    ast -> exp_id = exp_num++;
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