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
#include "block.h"
#include "loop.h"
#include "arr.h"

using namespace std;

/* is the decl in global, or in function*/
static bool is_global_decl = false;
/* pointer of the array, used in address calculation for array elements*/
static string arr_ptr;
/* size of each dimension */
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
    /*CompUnit  ::= [CompUnit] FuncDef;*/
    public:
        std::unique_ptr<BaseAST> func_defs;
        void DumpIR() const override{
            cout << "decl @getint(): i32" << endl;
            cout << "decl @getch(): i32" << endl;
            cout << "decl @getarray(*i32): i32" << endl;
            cout << "decl @putint(i32)" << endl;
            cout << "decl @putch(i32)" << endl;
            cout << "decl @putarray(i32, *i32)" << endl;
            cout << "decl @starttime()" << endl;
            cout << "decl @stoptime()" << endl;
            set_symbol_func("getint", FUNC_INT);
            set_symbol_func("getch", FUNC_INT);
            set_symbol_func("getarray", FUNC_INT);
            set_symbol_func("putint", FUNC_VOID);
            set_symbol_func("putch", FUNC_VOID);
            set_symbol_func("putarray", FUNC_INT);
            set_symbol_func("starttime", FUNC_VOID);
            set_symbol_func("stoptime", FUNC_VOID);
            alloc_symbol_space();
            func_defs -> DumpIR();
            remove_symbol_space();
        }
};

class FuncDefListAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> next;
        std::unique_ptr<BaseAST> func_def;
        std::unique_ptr<BaseAST> decl;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            if (func_def != nullptr)
                func_def -> DumpIR();
            if (decl != nullptr){
                is_global_decl = true;
                decl -> DumpIR();
                is_global_decl = false;
            }
        }
};

class FuncDefAST: public BaseAST{
    
    /*FuncDef   ::= FuncType IDENT "(" ")" Block;*/
    public:
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> func_f_params;
        std::unique_ptr<BaseAST> block;
        void DumpIR() const override{
            if (func_type -> ExpId() == "")
                set_symbol_func(ident, FUNC_VOID);
            else
                set_symbol_func(ident, FUNC_INT);
            
            alloc_symbol_space();
            cout << "fun @" << ident << "(";
            if (func_f_params != nullptr)
                cout << func_f_params -> ExpId();
            cout << ")";
            if (func_type -> ExpId() != "")
                cout << ": i32";
            cout << "{" << endl;

            cout << "%entry_" << ident << ":" << endl;
            if (func_f_params != nullptr)
                func_f_params -> DumpIR();
            alloc_block();
            block -> DumpIR();
            if (!block_back().fin){
                assert(func_type->ExpId() == ""); //void type
                cout << "  ret" << endl;
                block_back().fin = true;
            }
            remove_block();
            cout << "}" << endl;
            remove_symbol_space();
        }
};

class FuncFParamListAST: public BaseAST{
    /*FuncFParams   ::= FuncFParam {"," FuncFParam};*/
    public:
        std::unique_ptr<BaseAST> next;
        std::unique_ptr<BaseAST> func_f_param;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            func_f_param -> DumpIR();
        }
        
        string ExpId() const override{
            if (next != nullptr)
                return next -> ExpId() + ", " + func_f_param -> ExpId();
            return func_f_param -> ExpId();
        }

};

class FuncFParamAST : public BaseAST{
    /*FuncFParam    ::= BType IDENT;*/
    public:
        std::unique_ptr<BaseAST> type;
        std::string ident;
        int var_id;
        void DumpIR() const override{
            set_symbol_var_id(ident, var_id);
            string var_name = get_symbol_name(ident);
            string param_name = get_symbol_param_name(ident);
            cout << "  @" << var_name << " = alloc i32" << endl;
            cout << "  store %" << param_name << ", @" << var_name << endl;
        }

        
        string ExpId() const override{
            return "%" + get_symbol_param_name(ident) + ": " + type -> ExpId();
        }
};

class FuncRParamListAST: public BaseAST{
    /*FuncRParams   ::= Exp {"," Exp};*/
    public:
        std::unique_ptr<BaseAST> next;
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            exp -> DumpIR();
        }
        string ExpId() const override{
            if (next != nullptr)
                return next -> ExpId() + ", " + exp -> ExpId();
            return exp -> ExpId();
        }
};

class FuncTypeAST: public BaseAST{
    /*FuncType   ::= "void" */
    public:
        std::string type;
        void DumpIR() const override{
        }
        string ExpId() const override{
            return "";
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
            if (!block_back().fin)
            decl_or_stmt -> DumpIR();
        }
};


class StmtAST1: public BaseAST{
    /*Stmt      ::= LVal "=" Exp ";" */
    public:
        std::unique_ptr<BaseAST> l_val;
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            l_val -> DumpIR();
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
            if (exp != nullptr){
                exp -> DumpIR();
                cout << "  ret " << exp -> ExpId() << endl;
            }
            else
                cout << "  ret" << endl;
            block_back().fin = true;
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
    /*Stmt      ::= Block ";"*/
    public:
        std::unique_ptr<BaseAST> block;
        void DumpIR() const override{
            block -> DumpIR();
        }
};

class StmtAST5: public BaseAST{
    /*Stmt      ::= IfStmt ";"*/
    public:
        std::unique_ptr<BaseAST> if_stmt;
        void DumpIR() const override{
            if_stmt -> DumpIR();
        }
};

class StmtAST6: public BaseAST{
    /*Stmt      ::= "while" "(" Exp ")" Stmt */
    public:
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> stmt;
        int br_id;
        void DumpIR() const override{
            alloc_loop(br_id);
            if (!block_back().fin){
                cout << "  jump " << loop_head_id() << endl;
                block_back().fin = true;
            }
            remove_block();

            alloc_block();
            alloc_symbol_space();
            cout <<  loop_head_id() << ":" << endl;
            exp -> DumpIR();
            string cur = exp -> ExpId();
            cout << "  br " << cur << ", " <<  loop_body_id() << ", " << loop_end_id() << endl;
            block_back().fin = true;
            remove_block();

            alloc_block();
            cout << loop_body_id() << ":" << endl;
            stmt -> DumpIR();
            if (!block_back().fin){
                cout << "  jump " << loop_head_id() << endl;
                block_back().fin = true;
            }
            remove_block();
            remove_symbol_space();
            alloc_block();
            cout << loop_end_id() <<  ":" << endl;
            remove_loop();
        }
};

class StmtAST7: public BaseAST{
    /*Stmt      ::= "continue" */
    public:
        void DumpIR() const override{
            if (!block_back().fin){
                cout << "  jump " << loop_head_id() << endl;
                block_back().fin = true;
            }
            remove_block();

            alloc_loop_body();
            alloc_block();
            cout << loop_body_id() << ":" << endl;
        }
};

class StmtAST8: public BaseAST{
    /*Stmt      ::= "break" */
    public:
        void DumpIR() const override{
            if (!block_back().fin){
                cout << "  jump " << loop_end_id() << endl;
                block_back().fin = true;
            }
            remove_block();

            alloc_loop_body();
            alloc_block();
            cout << loop_body_id() << ":" << endl;
        }
};

class IfStmtAST1: public BaseAST{
    /*Stmt      ::="if" "(" Exp ")" Stmt*/
    public:
        int br_id;
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> then_stmt;
        void DumpIR() const override{
            exp -> DumpIR();
            string cur = exp -> ExpId();
            cout << "  br " << cur << ", %then" + to_string(br_id) << ", %endif" + to_string(br_id) << endl;
            block_back().fin = true;
            remove_block();
            alloc_block();
            alloc_symbol_space();
            cout << "%then" + to_string(br_id) + ":" << endl;
            then_stmt -> DumpIR();
            if (!block_back().fin){
                block_back().fin = true;
                cout << "  jump %endif" + to_string(br_id) << endl;
            }
            remove_block();
            remove_symbol_space();
            alloc_block();
            cout << "%endif" + to_string(br_id) + ":" << endl;
        }
};

class IfStmtAST2: public BaseAST{
    /*Stmt      ::="if" "(" Exp ")" Stmt*/
    public:
        int br_id;
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> then_stmt;
        std::unique_ptr<BaseAST> else_stmt;
        void DumpIR() const override{
            exp -> DumpIR();
            string cur = exp -> ExpId();
            cout << "  br " << cur << ", %then" + to_string(br_id) << ", %else" + to_string(br_id) << endl;
            block_back().fin = true;
            remove_block();
            alloc_block();
            alloc_symbol_space();
            cout << "%then" + to_string(br_id) + ":" << endl;
            then_stmt -> DumpIR();
            if (!block_back().fin){
                block_back().fin = true;
                cout << "  jump %endif" + to_string(br_id) << endl;
            }
            remove_block();
            remove_symbol_space();
            alloc_block();
            alloc_symbol_space();
            cout << "%else" + to_string(br_id) + ":" << endl;
            else_stmt -> DumpIR();
            if (!block_back().fin){
                block_back().fin = true;
                cout << "  jump %endif" + to_string(br_id) << endl;
            }
            remove_block();
            remove_symbol_space();
            alloc_block();
            cout << "%endif" + to_string(br_id) + ":" << endl;
        }
};

/* ------------------- Const Definitions ---------------- */

class DeclAST: public BaseAST{
    /*Decl          ::= ConstDecl | VarDecl; */
    public:
        std::unique_ptr<BaseAST> const_or_var_decl;
        void DumpIR() const override{
            //cout << "Decl" << endl;
            const_or_var_decl -> DumpIR();
        }
};

/* -------------------- Const Decls --------------------- */

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
        string ExpId() const override{
            assert(type == "i32");
            return type;
        }
};

class ConstDefAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> def;
        void DumpIR() const override{
            def -> DumpIR();
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
class ConstDefIDENTAST: public BaseAST{
    /*ConstDef      ::= IDENT "=" ConstInitVal;*/
    public:
        std::string ident;
        std::unique_ptr<BaseAST> const_init_val;
        void DumpIR() const override{
            set_symbol_value(ident, const_init_val -> ExpVal());
        }
};

class ConstDefArrAST: public BaseAST{
    /* ConstDef      ::= IDENT ArrSizeList "=" ConstInitArrVal; */
    public:
        std::string ident;
        std::unique_ptr<BaseAST> arr_size;
        std::unique_ptr<BaseAST> const_init_arr_val;
        int var_id;
        void DumpIR() const override{
            clear_arr_info();
            arr_size -> DumpIR();
            pre_arr_initialize(CONST_ARR);
            const_init_arr_val -> DumpIR();
            set_symbol_var_id(ident, var_id);
            if (is_global_decl){
                string exp_val = get_arr_val();
                cout << "global @" << get_symbol_name(ident) << " = alloc " << arr_size -> ExpId() << ", " << exp_val << endl;
            }
            else{
                cout << "  @" << get_symbol_name(ident) << " = alloc " << arr_size -> ExpId() << endl;
                cout << "  store zeroinit, @" << get_symbol_name(ident) << endl;
                fit_local_arr(get_symbol_name(ident));
            }
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

class ConstInitValArrAST: public BaseAST{
    /* InitValArr             ::= '{' {ConstInitValList} '}' */
    public:
        std::unique_ptr<BaseAST> const_init_val_list;
        void DumpIR() const override{
            left_bracket();
            if (const_init_val_list != nullptr)
                const_init_val_list -> DumpIR();
            right_bracket();
        }
};

class ConstInitValListAST: public BaseAST{
    /* ConstInitValList             ::= ConstInitValElem [',' ConstInitValElem] */
    public:
        std::unique_ptr<BaseAST> next;
        std::unique_ptr<BaseAST> const_init_val;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            const_init_val -> DumpIR();
        }
};

class ConstInitValElemAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> val;
        bool is_arr;
        void DumpIR() const override{
            val -> DumpIR();
            if (!is_arr)
                insert_arr_val(to_string(val -> ExpVal()));
        }
};


/* -------------------- Var Decls --------------------- */

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
    public:
        std::unique_ptr<BaseAST> def;
        void DumpIR() const override{
            def -> DumpIR();
        }
};

class VarDefIDENTAST: public BaseAST{
    /*VarDef        ::= IDENT | IDENT "=" InitVal;*/
    public:
        std::string ident;
        int var_id;
        std::unique_ptr<BaseAST> init_val;
        void DumpIR() const override{
            if (!is_global_decl){
                set_symbol_var_id(ident, var_id);
                cout << "  @" << get_symbol_name(ident) << " = alloc i32" << endl;
                if (init_val != nullptr){
                    init_val -> DumpIR();
                    string exp_val = init_val -> ExpId();
                    cout << "  store " << exp_val << ", @" << get_symbol_name(ident) << endl;
                }
            }
            else{
                set_symbol_var_id(ident, var_id);
                cout << "global @" << get_symbol_name(ident) << " = alloc i32, ";
                if (init_val != nullptr)
                    cout << init_val -> ExpVal() << endl;
                else
                    cout << "0" << endl;
            }
        }
};

class VarDefArrAST: public BaseAST{
    /* VarDef        ::= IDENT ArrSizeList ['=' InitArrVal] */
    public:
        std::string ident;
        std::unique_ptr<BaseAST> arr_size;
        std::unique_ptr<BaseAST> init_arr_val;
        int var_id;
        void DumpIR() const override{
            string exp_val = "zeroinit";
            if (init_arr_val != nullptr){
                clear_arr_info();
                arr_size -> DumpIR();
                pre_arr_initialize(VAR_ARR);
                init_arr_val -> DumpIR();
                exp_val = get_arr_val();
            }
            else{
                clear_arr_info();
                arr_size -> DumpIR();
            }
            set_symbol_var_id(ident, var_id);
            if (is_global_decl){
                cout << "global @" << get_symbol_name(ident) << " = alloc " << arr_size -> ExpId() << ", " << exp_val << endl;
            }
            else{
                cout << "  @" << get_symbol_name(ident) << " = alloc " << arr_size -> ExpId() << endl;
                cout << "  store zeroinit, @" << get_symbol_name(ident) << endl;
                if (init_arr_val != nullptr)
                    fit_local_arr(get_symbol_name(ident));
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

class InitValArrAST: public BaseAST{
    /* InitValArr             ::= '{' {InitValList} '}' */
    public:
        std::unique_ptr<BaseAST> init_val_list;
        void DumpIR() const override{
            //cout << "InitValArr" << endl;
            left_bracket();
            if (init_val_list != nullptr)
                init_val_list -> DumpIR();
            right_bracket();
        }
};

class InitValListAST: public BaseAST{
    /* InitValList             ::= InitValElem [',' InitValElem] */
    public:
        std::unique_ptr<BaseAST> next;
        std::unique_ptr<BaseAST> init_val;
        void DumpIR() const override{
            //cout << "InitValList" << endl;
            if (next != nullptr)
                next -> DumpIR();
            init_val -> DumpIR();
        }
};

class InitValElemAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> val;
        bool is_arr;
        void DumpIR() const override{
            //cout << "InitValElem" << endl;
            val -> DumpIR();
            if (!is_arr)
                insert_arr_val(val -> ExpId());
        }
};
/* ------------------ Array Params --------------- */

class ArrSizesAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> arr_size; 
        std::unique_ptr<BaseAST> next;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            arr_size -> DumpIR();
        }
        string ExpId() const override{
            return get_arr_exp_id();
        }
};

class ArrSizeAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> exp;
        void DumpIR() const override{
            int exp_val = exp -> ExpVal();
            assert(exp_val >= 1);
            alloc_dim(exp_val);
        }
        int ExpVal() const override{
            return exp -> ExpVal();
        }
};

class ArrParamsAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> arr_param; 
        std::unique_ptr<BaseAST> next;
        void DumpIR() const override{
            if (next != nullptr)
                next -> DumpIR();
            arr_param -> DumpIR();
        }
        string ExpId() const override{
            return arr_param -> ExpId();
        }
};

class ArrParamAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> exp;
        int exp_id;
        void DumpIR() const override{
            string cur_arr_ptr = arr_ptr;
            exp -> DumpIR();
            arr_ptr = "@ptr_" + to_string(exp_id) + "_arr";
            cout << "  " << arr_ptr << " = getelemptr " << cur_arr_ptr << ", " << exp -> ExpId() << endl;
        }
        string ExpId() const override{
            return "@ptr_" + to_string(exp_id) + "_arr";
        }
};

/* ------------------ Expressions ---------------- */

class LValAST: public BaseAST{
    public:
        std::unique_ptr<BaseAST> l_val;
        void DumpIR() const override{
            l_val -> DumpIR();
        }
        string ExpId() const override{
            return l_val -> ExpId();
        }
        int ExpVal() const override{
            return l_val -> ExpVal();
        }
};

class LValArrAST: public BaseAST{
    /*LVal          ::= IDENT "[" Exp "]";*/
    public:
        std::string ident;
        std::unique_ptr<BaseAST> arr_param; 
        int exp_id;
        void DumpIR() const override{
            arr_ptr = "@" + get_symbol_name(ident);
            arr_param -> DumpIR();
        }
        string ExpId() const override{
            return arr_param -> ExpId();
        }
};

class LValIDENTAST: public BaseAST{
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
            l_val -> DumpIR();
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

class UnaryExpAST3: public BaseAST{
    /* UnaryExp    ::= IDENT "(" [FuncRParams] ")" */
    public:
        std::string ident;
        std::unique_ptr<BaseAST> func_r_params;
        int exp_id;
        void DumpIR() const override{
            assert(is_symbol_func_set(ident));
            if (func_r_params != nullptr)
                func_r_params -> DumpIR();
            if (get_symbol_func(ident) == FUNC_INT)
                cout << "  %" << exp_id << " = ";
            else
                cout << "  ";
            if (func_r_params != nullptr)
                cout << "call @" << ident << "(" << func_r_params -> ExpId() << ")" << endl;
            else
                cout << "call @" << ident << "()" << endl;
        }
        string ExpId() const override{
            assert(is_symbol_func_set(ident));
            assert(get_symbol_func(ident) == FUNC_INT);
            assert(exp_id != -1);
            return "%" + to_string(exp_id);
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
        int br_id;
        void DumpIR() const override{
            l_and_exp -> DumpIR();
            cout << "  @br" << br_id << " = alloc i32" << endl;
            cout << "  br " << l_and_exp -> ExpId() << ", %then_exp" << br_id << ", %else_exp" << br_id << endl;
            block_back().fin = true;
            remove_block();
            alloc_block();
            cout << "%then_exp" << br_id << ":" << endl;
            eq_exp -> DumpIR();
            cout << "  %" << exp_id - 1 << " = ne " << eq_exp -> ExpId() << ", " << 0 << endl;
            cout << "  store %" << exp_id - 1 << ", @br" << br_id << endl;
            if (!block_back().fin){
                block_back().fin = true;
                cout << "  jump %endif_exp" << br_id << endl; 
            }
            remove_block();
            alloc_block();
            cout << "%else_exp" << br_id << ":" << endl;
            cout << "  store " << 0 << ", @br" << br_id << endl;
            cout << "  jump %endif_exp" << br_id << endl; 
            block_back().fin = true;
            remove_block();
            alloc_block();
            cout << "%endif_exp" << br_id << ":" << endl;
            cout << "  %" << exp_id << " = load @br" << br_id << endl;
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
        int br_id;
        void DumpIR() const override{
            l_or_exp -> DumpIR();
            cout << "  @br" << br_id << " = alloc i32" << endl;
            cout << "  br " << l_or_exp -> ExpId() << ", %then_exp" << br_id << ", %else_exp" << br_id << endl;
            block_back().fin = true;
            remove_block();
            alloc_block();
            cout << "%then_exp" << br_id << ":" << endl;
            cout << "  store " << 1 << ", @br" << br_id << endl;
            cout << "  jump %endif_exp" << br_id << endl; 
            block_back().fin = true;
            remove_block();
            alloc_block();
            cout << "%else_exp" << br_id << ":" << endl;
            l_and_exp -> DumpIR();
            cout << "  %" << exp_id - 1 << " = ne " << l_and_exp -> ExpId() << ", " << 0 << endl;
            cout << "  store %" << exp_id - 1 << ", @br" << br_id << endl;
            if (!block_back().fin){
                block_back().fin = true;
                cout << "  jump %endif_exp" << br_id << endl; 
            }
            remove_block();
            alloc_block();
            cout << "%endif_exp" << br_id << ":" << endl;
            cout << "  %" << exp_id << " = load @br" << br_id << endl;
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