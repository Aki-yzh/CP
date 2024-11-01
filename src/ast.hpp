#pragma once
#include <iostream>
#include <string>

using namespace std;
static int koopacnt = 0;
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  
  virtual void KoopaIR() const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_def;


  void KoopaIR() const override {
    func_def->KoopaIR();
  }


};

class FuncDefAST : public BaseAST {
 public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;


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

  void KoopaIR() const override {
    cout << "i32";
  }
};
class BlockAST : public BaseAST {
 public:
  unique_ptr<BaseAST> stmt;

  void KoopaIR() const override {
    cout << "%entry:" << endl;
    cout << "  ";
    stmt->KoopaIR();
  }
};
//从这里开始需要修改
/*
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
*/



// Stmt ::= "return" Exp ";";
class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void KoopaIR() const override
  {
    exp->KoopaIR();
    cout << "  ret %" << koopacnt-1;
  }
};

// Exp ::= LOrExp;
class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> lorexp;
  void KoopaIR() const override {
  lorexp->KoopaIR();
}
};

// PrimaryExp ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> exp;
  std::int32_t number;
  void KoopaIR() const override {
  if(type==1) {
    exp->KoopaIR();
  }
  else if(type==2) {
    std::cout << "  %" << koopacnt << " = add 0, ";
    std::cout<< number << std::endl;
    koopacnt++;
  }
}
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryOp ::= "+" | "-" | "!"
class UnaryExpAST : public BaseAST {
 public:
  int type;
  char unaryop;
  // primaryexp1_unaryexp2 表示在 type 为 1 时为 PrimaryExp
  // 在 type 为 2 时 为 UnaryExp
  std::unique_ptr<BaseAST> primaryexp1_unaryexp2;
  void KoopaIR() const override{
  if(type==1) {
    primaryexp1_unaryexp2->KoopaIR();
  }
  else if(type==2) {
    primaryexp1_unaryexp2->KoopaIR();
    if(unaryop=='-') {
      std::cout << "  %" << koopacnt << " = sub 0, %";
      std::cout << koopacnt-1 <<std::endl;
      koopacnt++;
    }
    else if(unaryop=='!') {
      std::cout << "  %" << koopacnt << " = eq 0, %";
      std::cout << koopacnt-1 <<std::endl;
      koopacnt++;
    }
  }
}
};

// MulExp ::= UnaryExp | MulExp MulOp UnaryExp;
// MulOp ::= "*" | "/" | "%"
class MulExpAST : public BaseAST {
 public:
  int type;
  char mulop;
  std::unique_ptr<BaseAST> mulexp;
  std::unique_ptr<BaseAST> unaryexp;
  void KoopaIR() const override {
  if(type==1) {
    unaryexp->KoopaIR();
  }
  else if(type==2) {
    mulexp->KoopaIR();
    int left = koopacnt-1;
    unaryexp->KoopaIR();
    int right = koopacnt-1;
    if(mulop=='*') {
      std::cout << "  %" << koopacnt << " = mul %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(mulop=='/') {
      std::cout << "  %" << koopacnt << " = div %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(mulop=='%') {
      std::cout << "  %" << koopacnt << " = mod %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
  }
}
};

// AddExp ::= MulExp | AddExp AddOp MulExp;
// AddOp ::= "+" | "-"
class AddExpAST : public BaseAST {
 public:
  int type;
  char addop;
  std::unique_ptr<BaseAST> addexp;
  std::unique_ptr<BaseAST> mulexp;
  void KoopaIR() const override{
  if(type==1) {
    mulexp->KoopaIR();
  }
  else if(type==2) {
    addexp->KoopaIR();
    int left = koopacnt-1;
    mulexp->KoopaIR();
    int right = koopacnt-1;
    if(addop=='+') {
      std::cout << "  %" << koopacnt << " = add %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(addop=='-') {
      std::cout << "  %" << koopacnt << " = sub %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
  }
}
};

// RelExp ::= AddExp | RelExp RelOp AddExp;
// RelOp ::= "<" | ">" | "<=" | ">="
class RelExpAST : public BaseAST {
 public:
  int type;
  std::string relop;
  std::unique_ptr<BaseAST> relexp;
  std::unique_ptr<BaseAST> addexp;
  void KoopaIR() const override{
  if(type==1) {
    addexp->KoopaIR();
  }
  else if(type==2) {
    relexp->KoopaIR();
    int left = koopacnt-1;
    addexp->KoopaIR();
    int right = koopacnt-1;
    if(relop=="<") {
      std::cout << "  %" << koopacnt << " = lt %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(relop==">") {
      std::cout << "  %" << koopacnt << " = gt %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(relop=="<=") {
      std::cout << "  %" << koopacnt << " = le %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(relop==">=") {
      std::cout << "  %" << koopacnt << " = ge %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
  }
}
};

// EqExp ::= RelExp | EqExp EqOp RelExp;
// EqOp ::= "==" | "!="
class EqExpAST : public BaseAST {
 public:
  int type;
  std::string eqop;
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> relexp;
  void KoopaIR() const override{
  if(type==1) {
    relexp->KoopaIR();
  }
  else if(type==2) {
    eqexp->KoopaIR();
    int left = koopacnt-1;
    relexp->KoopaIR();
    int right = koopacnt-1;
    if(eqop=="==") {
      std::cout << "  %" << koopacnt << " = eq %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
    else if(eqop=="!=") {
      std::cout << "  %" << koopacnt << " = ne %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
  }
}
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> landexp;
  std::unique_ptr<BaseAST> eqexp;
  void KoopaIR() const override{
  if(type==1) {
    eqexp->KoopaIR();
  }
  else if(type==2) {
    landexp->KoopaIR();
    int left = koopacnt-1;
    eqexp->KoopaIR();
    int right = koopacnt-1;
    // A&&B <==> (A!=0)&(B!=0)
    std::cout << "  %" << koopacnt << " = ne %";
    std::cout << left << ", 0" << std::endl;
    left = koopacnt;
    koopacnt++;
    std::cout << "  %" << koopacnt << " = ne %";
    std::cout << right << ", 0" << std::endl;
    right = koopacnt;
    koopacnt++;
    std::cout << "  %" << koopacnt << " = and %";
    std::cout << left << ", %" << right << std::endl;
    koopacnt++;
  }
}

};

// LOrExp  ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> lorexp;
  std::unique_ptr<BaseAST> landexp;
  void KoopaIR() const override {
  if(type==1) {
    landexp->KoopaIR();
  }
  else if(type==2) {
    lorexp->KoopaIR();
    int left = koopacnt-1;
    landexp->KoopaIR();
    int right = koopacnt-1;
    // A||B <==> (A!=0)|(B!=0)
    std::cout << "  %" << koopacnt << " = ne %";
    std::cout << left << ", 0" << std::endl;
    left = koopacnt;
    koopacnt++;
    std::cout << "  %" << koopacnt << " = ne %";
    std::cout << right << ", 0" << std::endl;
    right = koopacnt;
    koopacnt++;
    std::cout << "  %" << koopacnt << " = or %";
    std::cout << left << ", %" << right << std::endl;
    koopacnt++;
  }
}
};