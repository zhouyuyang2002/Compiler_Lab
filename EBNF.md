CompUnit      ::= [CompUnit] (Decl | FuncDef);

FuncDef       ::= [FuncType | BType] IDENT "(" [FuncFParams] ")" Block;
FuncType      ::= "void";
FuncFParams   ::= FuncFParam {"," FuncFParam};
FuncFParam    ::= BType IDENT;
FuncRParams   ::= Exp {"," Exp};

Block         ::= "{" {BlockItem} "}";
BlockItem     ::= Decl | Stmt; 
Stmt          ::= LVal "=" Exp ";"
                | [Exp] ";"
                | Block
                | "if" "(" Exp ")" Stmt ["else" Stmt]
                | "return" [Exp] ";"
                | "while" "(" Exp ")" Stmt
                | "continue" ";"
                | "break" ";";


Decl          ::= ConstDecl | VarDecl; 
ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
BType         ::= "int";
ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
VarDecl       ::= BType VarDef {"," VarDef} ";";
VarDef        ::= IDENT {"[" ConstExp "]"}
                | IDENT {"[" ConstExp "]"} "=" InitVal;

InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}";

LVal          ::= IDENT {"[" Exp "]"};
ConstExp      ::= Exp;
Exp           ::= LOrExp;
PrimaryExp    ::= "(" Exp ")" | LVal | Number | IDENT "(" [FuncRParams] ")"
UnaryExp      ::= PrimaryExp | UnaryOp UnaryExp
UnaryOp       ::= "+" | "-" | "!";
MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp       ::= EqExp | LAndExp "&&" EqExp;
LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
Number        ::= INT_CONST;

/*Todo List*/

ConstDefArr *
ConstInitValArr
ConstInitList

VarDefArr *
InitValArr
InitList

ArrSizeList *
ArrSize *
ArrParamList *
ArrParam *

LValArr *