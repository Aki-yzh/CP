#pragma once
#include <iostream>
#include <string>

using namespace std;
static int koopacnt = 0;
// 所有 AST 的基类
class BaseAST 
{
 public:
  virtual ~BaseAST() = default;
  virtual void KoopaIR() const = 0;
};

class CompUnitAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> func_def;
  void KoopaIR() const override
  {
    func_def->KoopaIR();
  }


};

class FuncDefAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;


  void KoopaIR() const override 
  {
    cout << "fun @" << ident << "(): ";
    func_type->KoopaIR();
    cout << " {" << endl;
    block->KoopaIR();
    cout << endl << "}" << endl;
  }

};

// 自己补充

class FuncTypeAST : public BaseAST 
{
 public:
  void KoopaIR() const override 
  {
    cout << "i32";
  }
};

class BlockAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> stmt;

  void KoopaIR() const override 
  {
    cout << "%entry:" << endl<< "  ";
    stmt->KoopaIR();
  }
};
//从这里开始需要修改

// Stmt ::= "return" Exp ";";
class StmtAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> exp;

  void KoopaIR() const override 
  {
    exp->KoopaIR();
    cout << "  ret %" << koopacnt - 1;
  }
};

// Exp ::= LOrExp;
class ExpAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> lorexp;

  void KoopaIR() const override 
  {
    lorexp->KoopaIR();
  }
};

// PrimaryExp ::= "(" Exp ")" | Number;
class PrimaryExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 "(" Exp ")"
  // type 为 2 时为 Number
  int type;
  unique_ptr<BaseAST> exp;
  int32_t number;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      exp->KoopaIR();
    } 
    else if (type == 2) 
    {
      cout << "  %" << koopacnt << " = add 0, " << number << endl;
      koopacnt++;
    }
  }
};


// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryOp ::= "+" | "-" | "!"
class UnaryExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 PrimaryExp
  // 在 type 为 2 时 为 UnaryOp UnaryExp
  int type;
  char unaryop;
  unique_ptr<BaseAST> primaryexp1_unaryexp2;

  void KoopaIR() const override 
  {
    primaryexp1_unaryexp2->KoopaIR();
    if (type == 2) 
    {
      if (unaryop == '-') 
      {
        cout << "  %" << koopacnt << " = sub 0, %" << koopacnt - 1 << endl;
        koopacnt++;
      } 
      else if (unaryop == '!') 
      {
        cout << "  %" << koopacnt << " = eq 0, %" << koopacnt - 1 << endl;
        koopacnt++;
      }
     
    }
  }
};
// MulExp ::= UnaryExp | MulExp MulOp UnaryExp;
// MulOp ::= "*" | "/" | "%"
class MulExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 UnaryExp
  // 在 type 为 2 时 为 MulExp MulOp UnaryExp
  int type;
  char mulop;
  unique_ptr<BaseAST> mulexp;
  unique_ptr<BaseAST> unaryexp;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      unaryexp->KoopaIR();
    } 
    else if (type == 2) 
    {
      mulexp->KoopaIR();
      int left = koopacnt - 1;
      unaryexp->KoopaIR();
      int right = koopacnt - 1;
      if (mulop == '*') 
      {
        cout << "  %" << koopacnt << " = mul %" << left << ", %" << right << endl;
        koopacnt++;
      } 
      else if (mulop == '/') 
      {
        cout << "  %" << koopacnt << " = div %" << left << ", %" << right << endl;
        koopacnt++;
      } 
      else if (mulop == '%') 
      {
        cout << "  %" << koopacnt << " = mod %" << left << ", %" << right << endl;
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
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> mulexp;
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
      cout << "  %" << koopacnt << " = add %";
      cout << left << ", %" << right << endl;
      koopacnt++;
    }
    else if(addop=='-') {
      cout << "  %" << koopacnt << " = sub %";
      cout << left << ", %" << right << endl;
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
  string relop;
  unique_ptr<BaseAST> relexp;
  unique_ptr<BaseAST> addexp;
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
      cout << "  %" << koopacnt << " = lt %";
      cout << left << ", %" << right << endl;
      koopacnt++;
    }
    else if(relop==">") {
      cout << "  %" << koopacnt << " = gt %";
      cout << left << ", %" << right << endl;
      koopacnt++;
    }
    else if(relop=="<=") {
      cout << "  %" << koopacnt << " = le %";
      cout << left << ", %" << right << endl;
      koopacnt++;
    }
    else if(relop==">=") {
      cout << "  %" << koopacnt << " = ge %";
      cout << left << ", %" << right << endl;
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
  string eqop;
  unique_ptr<BaseAST> eqexp;
  unique_ptr<BaseAST> relexp;
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
      cout << "  %" << koopacnt << " = eq %";
      cout << left << ", %" << right << endl;
      koopacnt++;
    }
    else if(eqop=="!=") {
      cout << "  %" << koopacnt << " = ne %";
      cout << left << ", %" << right << endl;
      koopacnt++;
    }
  }
}
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
 public:
  int type;
  unique_ptr<BaseAST> landexp;
  unique_ptr<BaseAST> eqexp;
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
    cout << "  %" << koopacnt << " = ne %";
    cout << left << ", 0" << endl;
    left = koopacnt;
    koopacnt++;
    cout << "  %" << koopacnt << " = ne %";
    cout << right << ", 0" << endl;
    right = koopacnt;
    koopacnt++;
    cout << "  %" << koopacnt << " = and %";
    cout << left << ", %" << right << endl;
    koopacnt++;
  }
}

};

// LOrExp  ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
 public:
  int type;
  unique_ptr<BaseAST> lorexp;
  unique_ptr<BaseAST> landexp;
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
    cout << "  %" << koopacnt << " = ne %";
    cout << left << ", 0" << endl;
    left = koopacnt;
    koopacnt++;
    cout << "  %" << koopacnt << " = ne %";
    cout << right << ", 0" << endl;
    right = koopacnt;
    koopacnt++;
    cout << "  %" << koopacnt << " = or %";
    cout << left << ", %" << right << endl;
    koopacnt++;
  }
}
};