#pragma once
#include <iostream>
#include <string>

using namespace std;
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual void KoopaIR() const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_def;

  void Dump() const override {
    cout << "CompUnitAST { ";
    func_def->Dump();
    cout << " }";
  }
  void KoopaIR() const override {
    func_def->KoopaIR();
  }


};

class FuncDefAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;

  void Dump() const override {
    cout << "FuncDefAST { ";
    func_type->Dump();
    cout << ", " << ident << ", ";
    block->Dump();
    cout << " }";
  }
  void KoopaIR() const override {
    cout << "fun @" << ident << "(): ";
    func_type->KoopaIR();
    cout << " {" << endl;
    block->KoopaIR();
    cout << endl << "}" << endl;
  }

};

// 自己补充

class FuncTypeAST : public BaseAST {
 public:
  void Dump() const override {
     cout << "FuncTypeAST { int }";
  }
  void KoopaIR() const override {
    cout << "i32";
  }
};
class BlockAST : public BaseAST {
 public:
  unique_ptr<BaseAST> stmt;
  void Dump() const override {
    cout << "BlockAST { ";
    stmt->Dump();
    cout << " }";
  }
  void KoopaIR() const override {
    cout << "%entry:" << endl;
    cout << "  ";
    stmt->KoopaIR();
  }
};
class StmtAST : public BaseAST {
 public:
  int32_t number;
  void Dump() const override {
    cout << "StmtAST { "<<number<<" }";
  }
  void KoopaIR() const override {
    cout << "ret " << number;
  }
};