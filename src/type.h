#pragma once

#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

enum ASTType{
    BaseASTType,
    CompUnitASTType,
    FuncDefASTType
};

class BaseAST{
    public:
        ASTType type;  
        virtual ~BaseAST() = default;
        virtual void Dump() const = 0;
        virtual void DumpIR() const = 0;
};

class CompUnitAST: public BaseAST{
    /*CompUnit  ::= FuncDef;*/
    public:
        std::unique_ptr<BaseAST> func_def;
        void Dump() const override{
            cout << "CompUnit :{ ";
            func_def -> Dump();
            cout << "} ";
        }
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
        void Dump() const override{
            cout << "FuncDet :{ ";
            func_type -> Dump();
            cout << ", " << ident << " , ";
            block -> Dump();
            cout << "} ";
        }
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
        void Dump() const override{
            cout << "FuncType :{ ";
            cout << type;
            cout << " } ";
        }
        void DumpIR() const override{
            cout << "i32";
        }
};


class BlockAST: public BaseAST{
    /*Block     ::= "{" Stmt "}"*/
    public:
        std::unique_ptr<BaseAST> stmt;
        void Dump() const override{
            cout << "Block :{ ";
            stmt -> Dump();
            cout << "} ";
        }
        void DumpIR() const override{
            cout << "\%entry:" << endl;
            stmt -> DumpIR();
        }
};

class StmtAST: public BaseAST{
    /*Stmt      ::= "return" Number ";"*/
    public:
        std::unique_ptr<BaseAST> number;
        void Dump() const override{
            cout << "Stmt :{ return ";
            number -> Dump();
            cout << "} ";
        }
        void DumpIR() const override{
            cout << "  ret ";
            number -> DumpIR();
            cout << endl;
        }
};

class NumberAST: public BaseAST{
    /*Number    ::= INT_CONST*/
    public:
        int val;
        void Dump() const override{
            cout << "Number :{ ";
            cout << val;
            cout << " } ";
        }
        void DumpIR() const override{
            cout << val;
        }
};