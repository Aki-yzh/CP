#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

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
//lv3从这里开始需要修改

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
      static const unordered_map<char, string> unaryop_map = 
      {
        {'-', "sub 0, %"},
        {'!', "eq 0, %"}
      };

      auto it = unaryop_map.find(unaryop);
      if (it != unaryop_map.end()) 
      {
        cout << "  %" << koopacnt << " = " << it->second << koopacnt - 1 << endl;
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
      static const unordered_map<char, string> mulop_map = 
      {
        {'*', "mul"},
        {'/', "div"},
        {'%', "mod"}
      };

      auto it = mulop_map.find(mulop);
      if (it != mulop_map.end()) 
      {
        cout << "  %" << koopacnt << " = " << it->second << " %" << left << ", %" << right << endl;
        koopacnt++;
      }
     
    }
  }
};

// AddExp ::= MulExp | AddExp AddOp MulExp;
// AddOp ::= "+" | "-"
class AddExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 MulExp
  // 在 type 为 2 时 为 AddExp AddOp MulExp
  int type;
  char addop;
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> mulexp;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      mulexp->KoopaIR();
    } 
    else if (type == 2) 
    {
      addexp->KoopaIR();
      int left = koopacnt - 1;
      mulexp->KoopaIR();
      int right = koopacnt - 1;
      static const unordered_map<char, string> addop_map = 
      {
        {'+', "add"},
        {'-', "sub"}
      };

      auto it = addop_map.find(addop);
      if (it != addop_map.end()) 
      {
        cout << "  %" << koopacnt << " = " << it->second << " %" << left << ", %" << right << endl;
        koopacnt++;
      }
    }
  }
};


// RelExp ::= AddExp | RelExp RelOp AddExp;
// RelOp ::= "<" | ">" | "<=" | ">="
class RelExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 AddExp
  // 在 type 为 2 时 为 RelExp RelOp AddExp
  int type;
  string relop;
  unique_ptr<BaseAST> relexp;
  unique_ptr<BaseAST> addexp;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      addexp->KoopaIR();
    } 
    else if (type == 2) 
    {
      relexp->KoopaIR();
      int left = koopacnt - 1;
      addexp->KoopaIR();
      int right = koopacnt - 1;
      static const unordered_map<string, string> relop_map = 
      {
        {"<", "lt"},
        {">", "gt"},
        {"<=", "le"},
        {">=", "ge"}
      };

      auto it = relop_map.find(relop);
      if (it != relop_map.end()) 
      {
        cout << "  %" << koopacnt << " = " << it->second << " %" << left << ", %" << right << endl;
        koopacnt++;
      }
     
    }
  }
};


// EqExp ::= RelExp | EqExp EqOp RelExp;
// EqOp ::= "==" | "!="
class EqExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 RelExp
  // 在 type 为 2 时 为 EqExp EqOp RelExp
  int type;
  string eqop;
  unique_ptr<BaseAST> eqexp;
  unique_ptr<BaseAST> relexp;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      relexp->KoopaIR();
    } 
    else if (type == 2) 
    {
      eqexp->KoopaIR();
      int left = koopacnt - 1;
      relexp->KoopaIR();
      int right = koopacnt - 1;
      static const unordered_map<string, string> eqop_map = 
      {
        {"==", "eq"},
        {"!=", "ne"}
      };

      auto it = eqop_map.find(eqop);
      if (it != eqop_map.end()) 
      {
        cout << "  %" << koopacnt << " = " << it->second << " %" << left << ", %" << right << endl;
        koopacnt++;
      }
      
    }
  }
};
// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 EqExp
  // 在 type 为 2 时 为 LAndExp "&&" EqExp
  int type;
  unique_ptr<BaseAST> landexp;
  unique_ptr<BaseAST> eqexp;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      eqexp->KoopaIR();
    } 
    else if (type == 2) 
    {
      landexp->KoopaIR();
      int left = koopacnt - 1;
      eqexp->KoopaIR();
      int right = koopacnt - 1;
      // A&&B <==> (A!=0)&(B!=0)
      cout << "  %" << koopacnt << " = ne %" << left << ", 0" << endl;
      left = koopacnt++;
      cout << "  %" << koopacnt << " = ne %" << right << ", 0" << endl;
      right = koopacnt++;
      cout << "  %" << koopacnt << " = and %" << left << ", %" << right << endl;
      koopacnt++;
    }
  }
};

// LOrExp  ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST 
{
 public:
  // type 为 1 时为 LAndExp
  // 在 type 为 2 时 为 LOrExp "||" LAndExp
  int type;
  unique_ptr<BaseAST> lorexp;
  unique_ptr<BaseAST> landexp;

  void KoopaIR() const override 
  {
    if (type == 1) 
    {
      landexp->KoopaIR();
    } 
    else if (type == 2) 
    {
      lorexp->KoopaIR();
      int left = koopacnt - 1;
      landexp->KoopaIR();
      int right = koopacnt - 1;
      // A||B <==> (A!=0)|(B!=0)
      cout << "  %" << koopacnt << " = ne %" << left << ", 0" << endl;
      left = koopacnt++;
      cout << "  %" << koopacnt << " = ne %" << right << ", 0" << endl;
      right = koopacnt++;
      cout << "  %" << koopacnt << " = or %" << left << ", %" << right << endl;
      koopacnt++;
    }
  }
};