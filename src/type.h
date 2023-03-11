#pragma once

#include <memory>
#include <string>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;


class BaseAST{
    public:  
        virtual ~BaseAST() = default;
        virtual void DumpIR() const = 0;
        virtual string ExpVal() const{
            return "???";
            // It should not happen.
        }
};

class CompUnitAST: public BaseAST{
    /*CompUnit  ::= FuncDef;*/
    public:
        std::unique_ptr<BaseAST> func_def;
        void DumpIR() const override{
            func_def -> DumpIR();
        }
};

class FuncDefAST: public BaseAST{
    
    /*FuncDef   ::= FuncType IDENT "(" ")" Block;*/
    public:
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> block;
        void DumpIR() const override{
            cout << "fun @" << ident << "(): ";
            func_type -> DumpIR();
            cout << " {" << endl;
            block -> DumpIR();
            cout << "}" << endl;
        }
};


class FuncTypeAST: public BaseAST{
    /*FuncDef   ::= "int"*/
    public:
        std::string type;
        void DumpIR() const override{
            cout << "i32";
        }
};


class BlockAST: public BaseAST{
    /*Block     ::= "{" Stmt "}"*/
    public:
        std::unique_ptr<BaseAST> stmt;
        void DumpIR() const override{
            cout << "\%entry:" << endl;
            stmt -> DumpIR();
        }
};

class StmtAST: public BaseAST{
    /*Stmt      ::= "return" Exp ";"*/
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            exp -> DumpIR();
            cout << "  ret " << exp -> ExpVal() << endl;
        }
};

class ExpAST: public BaseAST{
    /* Exp         ::= LOrExp;*/
    public:
        std::unique_ptr<BaseAST> l_or_exp;
        void DumpIR() const override{
            l_or_exp -> DumpIR();
        }
        string ExpVal() const override{
            return l_or_exp -> ExpVal();
        }
};

class PrimaryExpAST1: public BaseAST{
    /* PrimaryExp  ::= "(" Exp ")" */
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            exp -> DumpIR();
        }
        string ExpVal() const override{
            return exp -> ExpVal();
        }
};


class PrimaryExpAST2: public BaseAST{
    /* PrimaryExp  ::= Number */
    public:
        std::unique_ptr<BaseAST> number;
        void DumpIR() const override{
            number -> DumpIR();
        }
        string ExpVal() const override{
            return number -> ExpVal();
        }
};

class UnaryExpAST1: public BaseAST{
    /* UnaryExp    ::= PrimaryExp */
    public:
        std::unique_ptr<BaseAST> primary_exp;
        void DumpIR() const override{
            primary_exp -> DumpIR();
        }
        string ExpVal() const override{
            return primary_exp -> ExpVal();
        }
};

class UnaryExpAST2: public BaseAST{
    /* UnaryExp    ::= UnaryOp UnaryExp; */
    public:
        char unary_op;
        std::unique_ptr<BaseAST> unary_exp;
        int exp_id;
        void DumpIR() const override{
            unary_exp -> DumpIR();
            switch (unary_op){
                case '+':
                    break;
                case '-':
                    cout << "  %" << exp_id << " = sub 0, " << unary_exp->ExpVal() << endl;
                    break;
                case '!':
                    cout << "  %" << exp_id << " = eq 0, " << unary_exp->ExpVal() << endl;
                    break;
                default:
                    assert(false);
            }
        }

        string ExpVal() const override{
            switch (unary_op){
                case '+':
                    return unary_exp -> ExpVal();
                case '-':
                case '!':
                    assert(exp_id != -1);
                    return "%" + to_string(exp_id);
                default:
                    assert(false);
            }
        }
};

class NumberAST: public BaseAST{
    /*Number    ::= INT_CONST*/
    public:
        int val;
        void DumpIR() const override{
        }
        string ExpVal() const override{
            return to_string(val);
        }
};

class MulExpAST1: public BaseAST{
    //MulExp      ::= UnaryExp
    public:
        std::unique_ptr<BaseAST> unary_exp;
        void DumpIR() const override{
            unary_exp -> DumpIR();
        }
        string ExpVal() const override{
            return unary_exp -> ExpVal();
        }
};


class MulExpAST2: public BaseAST{
    //MulExp      ::= MulExp ("*" | "/" | "%") UnaryExp
    public:
        char mul_op;
        std::unique_ptr<BaseAST> mul_exp;
        std::unique_ptr<BaseAST> unary_exp;
        int exp_id;
        void DumpIR() const override{
            mul_exp -> DumpIR();
            unary_exp -> DumpIR();
            switch (mul_op){
                case '*':
                    cout << "  %" << exp_id << " = mul " << mul_exp -> ExpVal() << ", " << unary_exp->ExpVal() << endl;
                    break;
                case '/':
                    cout << "  %" << exp_id << " = div " << mul_exp -> ExpVal() << ", " << unary_exp->ExpVal() << endl;
                    break;
                case '%':
                    cout << "  %" << exp_id << " = mod " << mul_exp -> ExpVal() << ", " << unary_exp->ExpVal() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpVal() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
};


class AddExpAST1: public BaseAST{
    //AddExp      ::= MulExp
    public:
        std::unique_ptr<BaseAST> mul_exp;
        void DumpIR() const override{
            mul_exp -> DumpIR();
        }
        string ExpVal() const override{
            return mul_exp -> ExpVal();
        }
};


class AddExpAST2: public BaseAST{
    //AddExp      ::= AddExp ("+" | "-") MulExp;
    public:
        char add_op;
        std::unique_ptr<BaseAST> add_exp;
        std::unique_ptr<BaseAST> mul_exp;
        int exp_id;
        void DumpIR() const override{
            add_exp -> DumpIR();
            mul_exp -> DumpIR();
            switch (add_op){
                case '+':
                    cout << "  %" << exp_id << " = add " << add_exp -> ExpVal() << ", " << mul_exp->ExpVal() << endl;
                    break;
                case '-':
                    cout << "  %" << exp_id << " = sub " << add_exp -> ExpVal() << ", " << mul_exp->ExpVal() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpVal() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
};


class RelExpAST1: public BaseAST{
    //RelExp      ::= AddExp
    public:
        std::unique_ptr<BaseAST> add_exp;
        void DumpIR() const override{
            add_exp -> DumpIR();
        }
        string ExpVal() const override{
            return add_exp -> ExpVal();
        }
};

enum rel_op_type{
    REL_OP_LT,
    REL_OP_GT,
    REL_OP_LE,
    REL_OP_GE
};

class RelExpAST2: public BaseAST{
    //RelExp      ::= RelExp ("<" | ">" | "<=" | ">=") AddExp;
    public:
        rel_op_type rel_op;
        std::unique_ptr<BaseAST> rel_exp;
        std::unique_ptr<BaseAST> add_exp;
        int exp_id;
        void DumpIR() const override{
            rel_exp -> DumpIR();
            add_exp -> DumpIR();
            switch (rel_op){
                case REL_OP_LT:
                    cout << "  %" << exp_id << " = lt " << rel_exp -> ExpVal() << ", " << add_exp->ExpVal() << endl;
                    break;
                case REL_OP_LE:
                    cout << "  %" << exp_id << " = le " << rel_exp -> ExpVal() << ", " << add_exp->ExpVal() << endl;
                    break;
                case REL_OP_GT:
                    cout << "  %" << exp_id << " = gt " << rel_exp -> ExpVal() << ", " << add_exp->ExpVal() << endl;
                    break;
                case REL_OP_GE:
                    cout << "  %" << exp_id << " = ge " << rel_exp -> ExpVal() << ", " << add_exp->ExpVal() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpVal() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
};

class EqExpAST1: public BaseAST{
    //EqExp       ::= RelExp
    public:
        std::unique_ptr<BaseAST> rel_exp;
        void DumpIR() const override{
            rel_exp -> DumpIR();
        }
        string ExpVal() const override{
            return rel_exp -> ExpVal();
        }
};

enum eq_op_type{
    EQ_OP_EQ,
    EQ_OP_NEQ
};

class EqExpAST2: public BaseAST{
    //EqExp       ::= EqExp ("==" | "!=") RelExp;
    public:
        eq_op_type eq_op;
        std::unique_ptr<BaseAST> eq_exp;
        std::unique_ptr<BaseAST> rel_exp;
        int exp_id;
        void DumpIR() const override{
            eq_exp -> DumpIR();
            rel_exp -> DumpIR();
            switch (eq_op){
                case EQ_OP_EQ:
                    cout << "  %" << exp_id << " = eq " << eq_exp -> ExpVal() << ", " << rel_exp->ExpVal() << endl;
                    break;
                case EQ_OP_NEQ:
                    cout << "  %" << exp_id << " = ne " << eq_exp -> ExpVal() << ", " << rel_exp->ExpVal() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpVal() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
};


class LAndExpAST1: public BaseAST{
    //LAndExp     ::= EqExp
    public:
        std::unique_ptr<BaseAST> eq_exp;
        void DumpIR() const override{
            eq_exp -> DumpIR();
        }
        string ExpVal() const override{
            return eq_exp -> ExpVal();
        }
};

class LAndExpAST2: public BaseAST{
    //LAndExp     ::= LAndExp "&&" EqExp;
    public:
        std::unique_ptr<BaseAST> l_and_exp;
        std::unique_ptr<BaseAST> eq_exp;
        int exp_id;
        void DumpIR() const override{
            l_and_exp -> DumpIR();
            eq_exp -> DumpIR();
            cout << "  %" << exp_id - 2 << " = ne " << l_and_exp -> ExpVal() << ", " << 0 << endl;
            cout << "  %" << exp_id - 1 << " = ne " << eq_exp -> ExpVal() << ", " << 0 << endl;
            cout << "  %" << exp_id - 0 << " = and %" <<  exp_id - 2 << ", %" << exp_id - 1 << endl;
        }
        string ExpVal() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
};


class LOrExpAST1: public BaseAST{
    //LOrExp      ::= LAndExp;
    public:
        std::unique_ptr<BaseAST> l_and_exp;
        void DumpIR() const override{
            l_and_exp -> DumpIR();
        }
        string ExpVal() const override{
            return l_and_exp -> ExpVal();
        }
};

class LOrExpAST2: public BaseAST{
    //LOrExp      ::= LOrExp "||" LAndExp;
    public:
        std::unique_ptr<BaseAST> l_or_exp;
        std::unique_ptr<BaseAST> l_and_exp;
        int exp_id;
        void DumpIR() const override{
            l_or_exp -> DumpIR();
            l_and_exp -> DumpIR();
            cout << "  %" << exp_id - 2 << " = ne " << l_or_exp -> ExpVal() << ", " << 0 << endl;
            cout << "  %" << exp_id - 1 << " = ne " << l_and_exp -> ExpVal() << ", " << 0 << endl;
            cout << "  %" << exp_id - 0 << " = or %" <<  exp_id - 2 << ", %" << exp_id - 1 << endl;
        }
        string ExpVal() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
};