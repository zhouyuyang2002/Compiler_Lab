#ifndef __MY_TYPE_H
#define __MY_TYPE_H

#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

#include "symbol.h"

using namespace std;

class BaseAST{
    public:  
        virtual ~BaseAST() = default;
        virtual void DumpIR() const = 0;
        virtual string ExpId() const{
            return "??? BadExpId call";
            // It should not happen.
        }
        virtual int ExpVal() const{
            cout << "??? BadExpVal call" << endl;
            return -19260817;
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
            cout << "%entry:" << endl;
            //alloc_block();
            block -> DumpIR();
            //assert(block_stack() -> ret);
            //remove_block();
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

/* ------------------------ Statement ------------------- */

class BlockAST: public BaseAST{
    /*Block     ::= "{" {" {BlockItem} "}" "}"*/
    public:
        std::unique_ptr<BaseAST> block_items;
        void DumpIR() const override{
            if (block_items != nullptr){
                alloc_symbol_space();
                block_items -> DumpIR();
                remove_symbol_space();
            }
        }
};

class BlockItemListAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> block_item;
        std::unique_ptr<BaseAST> next;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            block_item -> DumpIR();
        }
};

class BlockItemAST: public BaseAST{
    /* BlockItem     ::= Decl | Stmt; */
    public:
        std::unique_ptr<BaseAST> decl_or_stmt;
        void DumpIR() const override{
            decl_or_stmt -> DumpIR();
        }
};


class StmtAST1: public BaseAST{
    /*Stmt      ::= LVal "=" Exp ";" */
    public:
        std::unique_ptr<BaseAST> l_val;
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            string l_val_id = l_val -> ExpId();
            if (l_val_id[0] != '@')
                cout << " ??? LVal is a const variable" << endl;
            exp -> DumpIR();
            string exp_val = exp -> ExpId();
            cout << "  store " << exp_val << ", " << l_val_id << endl;
        }
};

class StmtAST2: public BaseAST{
    /*Stmt      ::= "return" Exp ";"*/
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            exp -> DumpIR();
            cout << "  ret " << exp -> ExpId() << endl;
        }
};

class StmtAST3: public BaseAST{
    /*Stmt      ::= [Exp] ";"*/
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            if (exp != nullptr)
                exp -> DumpIR();
        }
};

class StmtAST4: public BaseAST{
    /*Stmt      ::= [Exp] ";"*/
    public:
        std::unique_ptr<BaseAST> block;
        void DumpIR() const override{
            block -> DumpIR();
        }
};

/* ------------------- Const Definitions ---------------- */

class DeclAST: public BaseAST{
    /*Decl          ::= ConstDecl | VarDecl; */
    public:
        std::unique_ptr<BaseAST> const_or_var_decl;
        void DumpIR() const override{
            const_or_var_decl -> DumpIR();
        }
};

class ConstDeclAST: public BaseAST{
    /*ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";*/
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> const_defs;
        void DumpIR() const override{
            const_defs -> DumpIR();
        }
};

class BTypeAST: public BaseAST{
    /*BType            ::= "int"*/
    public:
        std::string type; /*i32*/
        void DumpIR() const override{
            cout << type;
        }
};

class ConstDefListAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> const_def;
        std::unique_ptr<BaseAST> next;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            const_def -> DumpIR();
        }
};

// IgNore BType for only "int" type is used
class ConstDefAST: public BaseAST{
    /*ConstDef      ::= IDENT "=" ConstInitVal;*/
    public:
        std::string ident;
        std::unique_ptr<BaseAST> const_init_val;
        void DumpIR() const override{
            set_symbol_value(ident, const_init_val -> ExpVal());
        }
};

class ConstInitValAST: public BaseAST{
    /*ConstDef      ::= ConstExp;*/
    public:
        std::unique_ptr<BaseAST> const_exp;
        void DumpIR() const override{
        }
        int ExpVal() const override{
            return const_exp -> ExpVal();
        }
};


class VarDeclAST: public BaseAST{
    /*VarDecl       ::= BType VarDef {"," VarDef} ";"*/
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> var_defs;
        void DumpIR() const override{
            var_defs -> DumpIR();
        }
};

class VarDefListAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> var_def;
        std::unique_ptr<BaseAST> next;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            var_def -> DumpIR();
        }
};

class VarDefAST: public BaseAST{
    /*VarDef        ::= IDENT | IDENT "=" InitVal;*/
    public:
        std::string ident;
        int var_id;
        std::unique_ptr<BaseAST> init_val;
        void DumpIR() const override{
            set_symbol_var_id(ident, var_id);
            cout << "  @" << get_symbol_name(ident) << " = alloc i32" << endl;
            if (init_val != nullptr){
                init_val -> DumpIR();
                string exp_val = init_val -> ExpId();
                cout << "  store " << exp_val << ", @" << get_symbol_name(ident) << endl;
            }
        }
};

class InitValAST: public BaseAST{
    /*InitVal       ::= Exp;*/
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            exp -> DumpIR();
        }
        string ExpId() const override{
            return exp -> ExpId();
        }
        int ExpVal() const override{
            return exp -> ExpVal();
        }
};

/* ------------------ Expressions ---------------- */

class LValAST: public BaseAST{
    /*LVal          ::= IDENT;*/
    public:
        std::string ident;
        int exp_id;
        void DumpIR() const override{
        }
        string ExpId() const override{
            if (is_symbol_value_set(ident))
                return to_string(get_symbol_value(ident));
            if (is_symbol_var_id_set(ident))
                return "@" + get_symbol_name(ident);
            return "??? Bad LVal defination";
        }
        int ExpVal() const override{
            if (is_symbol_value_set(ident))
                return get_symbol_value(ident);
            cout << "??? Bad LVal calculation" << endl;
            return -19260817;
        }
};

class ConstExpAST: public BaseAST{
    /*ConstExp      ::= Exp;*/
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            exp -> DumpIR();
        }
        string ExpId() const override{
            return exp -> ExpId();
        }
        int ExpVal() const override{
            return exp -> ExpVal();
        }
};

class ExpAST: public BaseAST{
    /* Exp         ::= LOrExp;*/
    public:
        std::unique_ptr<BaseAST> l_or_exp;
        void DumpIR() const override{
            l_or_exp -> DumpIR();
        }
        string ExpId() const override{
            return l_or_exp -> ExpId();
        }
        int ExpVal() const override{
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
        string ExpId() const override{
            return exp -> ExpId();
        }
        int ExpVal() const override{
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
        string ExpId() const override{
            return number -> ExpId();
        }
        int ExpVal() const override{
            return number -> ExpVal();
        }
};


class PrimaryExpAST3: public BaseAST{
    /* PrimaryExp  ::= LVal */
    public:
        std::unique_ptr<BaseAST> l_val;
        int exp_id;
        void DumpIR() const override{
            string l_val_id = l_val -> ExpId();
            if (l_val_id[0] == '@')
                cout << "  %" << exp_id << " = load " << l_val_id << endl;
        }
        string ExpId() const override{
            string l_val_id = l_val -> ExpId();
            if (l_val_id[0] == '@')
                return "%" + to_string(exp_id);
            else
                return l_val_id;
        }
        int ExpVal() const override{
            return l_val -> ExpVal();
            // only for constant used
        }
};

class UnaryExpAST1: public BaseAST{
    /* UnaryExp    ::= PrimaryExp */
    public:
        std::unique_ptr<BaseAST> primary_exp;
        void DumpIR() const override{
            primary_exp -> DumpIR();
        }
        string ExpId() const override{
            return primary_exp -> ExpId();
        }
        int ExpVal() const override{
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
                    cout << "  %" << exp_id << " = sub 0, " << unary_exp->ExpId() << endl;
                    break;
                case '!':
                    cout << "  %" << exp_id << " = eq 0, " << unary_exp->ExpId() << endl;
                    break;
                default:
                    assert(false);
            }
        }

        string ExpId() const override{
            switch (unary_op){
                case '+':
                    return unary_exp -> ExpId();
                case '-':
                case '!':
                    assert(exp_id != -1);
                    return "%" + to_string(exp_id);
                default:
                    assert(false);
            }
        }

        
        int ExpVal() const override{
            switch (unary_op){
                case '+':
                    return unary_exp -> ExpVal();
                case '-':
                    return -(unary_exp -> ExpVal());
                case '!':
                    return !(unary_exp -> ExpVal());
                default:
                    assert(false);
            }
            return -1;
        }
};

class NumberAST: public BaseAST{
    /*Number    ::= INT_CONST*/
    public:
        int val;
        void DumpIR() const override{
        }
        string ExpId() const override{
            return to_string(val);
        }
        int ExpVal() const override{
            return val;
        }
};

class MulExpAST1: public BaseAST{
    //MulExp      ::= UnaryExp
    public:
        std::unique_ptr<BaseAST> unary_exp;
        void DumpIR() const override{
            unary_exp -> DumpIR();
        }
        string ExpId() const override{
            return unary_exp -> ExpId();
        }
        int ExpVal() const override{
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
                    cout << "  %" << exp_id << " = mul " << mul_exp -> ExpId() << ", " << unary_exp->ExpId() << endl;
                    break;
                case '/':
                    cout << "  %" << exp_id << " = div " << mul_exp -> ExpId() << ", " << unary_exp->ExpId() << endl;
                    break;
                case '%':
                    cout << "  %" << exp_id << " = mod " << mul_exp -> ExpId() << ", " << unary_exp->ExpId() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpId() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
        int ExpVal() const override{
            switch (mul_op){
                case '*':
                    return mul_exp -> ExpVal() * unary_exp -> ExpVal();
                case '/':
                    return mul_exp -> ExpVal() / unary_exp -> ExpVal();
                case '%':
                    return mul_exp -> ExpVal() % unary_exp -> ExpVal();
                default:
                    assert(false);
            }
            return -1;
        }
};


class AddExpAST1: public BaseAST{
    //AddExp      ::= MulExp
    public:
        std::unique_ptr<BaseAST> mul_exp;
        void DumpIR() const override{
            mul_exp -> DumpIR();
        }
        string ExpId() const override{
            return mul_exp -> ExpId();
        }
        int ExpVal() const override{
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
                    cout << "  %" << exp_id << " = add " << add_exp -> ExpId() << ", " << mul_exp->ExpId() << endl;
                    break;
                case '-':
                    cout << "  %" << exp_id << " = sub " << add_exp -> ExpId() << ", " << mul_exp->ExpId() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpId() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
        int ExpVal() const override{
            switch (add_op){
                case '+':
                    return add_exp -> ExpVal() + mul_exp -> ExpVal();
                case '-':
                    return add_exp -> ExpVal() - mul_exp -> ExpVal();
                default:
                    assert(false);
            }
            return -1;
        }
};


class RelExpAST1: public BaseAST{
    //RelExp      ::= AddExp
    public:
        std::unique_ptr<BaseAST> add_exp;
        void DumpIR() const override{
            add_exp -> DumpIR();
        }
        string ExpId() const override{
            return add_exp -> ExpId();
        }
        int ExpVal() const override{
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
                    cout << "  %" << exp_id << " = lt " << rel_exp -> ExpId() << ", " << add_exp->ExpId() << endl;
                    break;
                case REL_OP_LE:
                    cout << "  %" << exp_id << " = le " << rel_exp -> ExpId() << ", " << add_exp->ExpId() << endl;
                    break;
                case REL_OP_GT:
                    cout << "  %" << exp_id << " = gt " << rel_exp -> ExpId() << ", " << add_exp->ExpId() << endl;
                    break;
                case REL_OP_GE:
                    cout << "  %" << exp_id << " = ge " << rel_exp -> ExpId() << ", " << add_exp->ExpId() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpId() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
        int ExpVal() const override{
            switch (rel_op){
                case REL_OP_LT:
                    return rel_exp -> ExpVal() <  add_exp -> ExpVal();
                case REL_OP_LE:
                    return rel_exp -> ExpVal() <= add_exp -> ExpVal();
                case REL_OP_GT:
                    return rel_exp -> ExpVal() >  add_exp -> ExpVal();
                case REL_OP_GE:
                    return rel_exp -> ExpVal() >= add_exp -> ExpVal();
                default:
                    assert(false);
            }
            return -1;
        }
};

class EqExpAST1: public BaseAST{
    //EqExp       ::= RelExp
    public:
        std::unique_ptr<BaseAST> rel_exp;
        void DumpIR() const override{
            rel_exp -> DumpIR();
        }
        string ExpId() const override{
            return rel_exp -> ExpId();
        }
        int ExpVal() const override{
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
                    cout << "  %" << exp_id << " = eq " << eq_exp -> ExpId() << ", " << rel_exp->ExpId() << endl;
                    break;
                case EQ_OP_NEQ:
                    cout << "  %" << exp_id << " = ne " << eq_exp -> ExpId() << ", " << rel_exp->ExpId() << endl;
                    break;
                default:
                    assert(false);
            }
        }
        string ExpId() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
        int ExpVal() const override{
            switch (eq_op){
                case EQ_OP_EQ:
                    return eq_exp -> ExpVal() == rel_exp -> ExpVal();
                case EQ_OP_NEQ:
                    return eq_exp -> ExpVal() != rel_exp -> ExpVal();
                default:
                    assert(false);
            }
            return -1;
        }
};


class LAndExpAST1: public BaseAST{
    //LAndExp     ::= EqExp
    public:
        std::unique_ptr<BaseAST> eq_exp;
        void DumpIR() const override{
            eq_exp -> DumpIR();
        }
        string ExpId() const override{
            return eq_exp -> ExpId();
        }
        int ExpVal() const override{
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
            cout << "  %" << exp_id - 2 << " = ne " << l_and_exp -> ExpId() << ", " << 0 << endl;
            cout << "  %" << exp_id - 1 << " = ne " << eq_exp -> ExpId() << ", " << 0 << endl;
            cout << "  %" << exp_id - 0 << " = and %" <<  exp_id - 2 << ", %" << exp_id - 1 << endl;
        }
        string ExpId() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
        int ExpVal() const override{
            return l_and_exp -> ExpVal() && eq_exp -> ExpVal();
        }
};


class LOrExpAST1: public BaseAST{
    //LOrExp      ::= LAndExp;
    public:
        std::unique_ptr<BaseAST> l_and_exp;
        void DumpIR() const override{
            l_and_exp -> DumpIR();
        }
        string ExpId() const override{
            return l_and_exp -> ExpId();
        }
        int ExpVal() const override{
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
            cout << "  %" << exp_id - 2 << " = ne " << l_or_exp -> ExpId() << ", " << 0 << endl;
            cout << "  %" << exp_id - 1 << " = ne " << l_and_exp -> ExpId() << ", " << 0 << endl;
            cout << "  %" << exp_id - 0 << " = or %" <<  exp_id - 2 << ", %" << exp_id - 1 << endl;
        }
        string ExpId() const override{
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
        }
        int ExpVal() const override{
            return l_or_exp -> ExpVal() && l_and_exp -> ExpVal();
        }
};

#endif