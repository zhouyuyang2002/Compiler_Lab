CompUnit      ::= FuncDef;
FuncDef       ::= FuncType IDENT "(" ")" Block;
FuncType      ::= "int";

Block         ::= "{" {BlockItem} "}";
BlockItem     ::= Decl | Stmt; 
Stmt          ::= LVal "=" Exp ";"
                | "return" Exp ";";

Decl          ::= ConstDecl | VarDecl; 
ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
BType         ::= "int";
ConstDef      ::= IDENT "=" ConstInitVal;
ConstInitVal  ::= ConstExp;
VarDecl       ::= BType VarDef {"," VarDef} ";";
VarDef        ::= IDENT | IDENT "=" InitVal;
InitVal       ::= Exp;

LVal          ::= IDENT;
ConstExp      ::= Exp;
Exp           ::= LOrExp;
PrimaryExp    ::= "(" Exp ")" | LVal | Number;
UnaryExp      ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp       ::= "+" | "-" | "!";
MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp       ::= EqExp | LAndExp "&&" EqExp;
LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
Number        ::= INT_CONST;
