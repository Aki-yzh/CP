#pragma once
#include <iostream>
#include <string>
#include <cassert>
#include <unordered_map>
#include <memory>
#include <vector>
#include "Symbol_table.hpp"

using namespace std;
static int koopacnt = 0;
// 所有 AST 的基类
class BaseAST 
{
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual int Calc() const = 0;
};

class CompUnitAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> func_def;
  void Dump() const override
  {
    func_def->Dump();
  }
  int Calc() const override
  {
    
    return 0;
  }
};

//---


class DeclAST : public BaseAST 
{
 public:
  int type;
  unique_ptr<BaseAST> const_decl1_var_decl2;
  void Dump() const override
  {
     const_decl1_var_decl2->Dump();
  }
  int Calc() const override
  {
    
    return 0;
  }
};

// ConstDecl ::= "const" BType ConstDefList ";";
// ConstDefList ::= ConstDef | ConstDefList "," ConstDef;
class ConstDeclAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> b_type;
  unique_ptr<vector<unique_ptr<BaseAST> > > const_def_list;
  void Dump() const override
  {
    for(auto& const_def: *const_def_list)
      const_def->Dump();
  }
  int Calc() const override
  {
    
    return 0;
  }
};

// BType ::= "int";
class BTypeAST : public BaseAST 
{
 public:
  void Dump() const override
  {
    
    return;
  }
  int Calc() const override
  {
    
    return 0;
  }
};

// ConstDef ::= IDENT "=" ConstInitVal;
class ConstDefAST : public BaseAST 
{
 public:
  string ident;
  unique_ptr<BaseAST> const_init_val;
  void Dump() const override
  {
     insert_symbol(ident, SYM_TYPE_CONST, const_init_val->Calc());
  }
  int Calc() const override
  {
    
    return 0;
  }
};

// ConstInitVal ::= ConstExp;
class ConstInitValAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> const_exp;
  void Dump() const override
  {
    
    return;
  }
  int Calc() const override
  {
     return const_exp->Calc();
  }
};

// VarDecl ::= BType VarDefList ";";
// VarDefList ::= VarDef | VarDefList "," VarDef;
class VarDeclAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> b_type;
  unique_ptr<vector<unique_ptr<BaseAST> > > var_def_list;
  void Dump() const override
  {
    for(auto& var_def : *var_def_list)
       var_def->Dump();
  }

  int Calc() const override
  {
    
    return 0;
  }
};

// VarDef ::= IDENT | IDENT "=" InitVal;
class VarDefAST : public BaseAST 
{
 public:
  int type;
  string ident;
  unique_ptr<BaseAST> init_val;
  void Dump() const override
  {
    // @x = alloc i32
    cout << "  @" << ident << " = alloc i32" << endl;
    if(type==2) {
      init_val->Dump();
      // store %1, @x
      cout << "  store %" << koopacnt-1 << ", @" << ident << endl;
    }
    insert_symbol(ident, SYM_TYPE_VAR, 0);
  }
  int Calc() const override
  {
    
    return 0;
  }
};

// InitVal ::= Exp;
class InitValAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> exp;
  void Dump() const override
  {
     exp->Dump();
  }
  int Calc() const override
  {
    
    return 0;
  }
};

class FuncDefAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;


  void Dump() const override 
  {
    cout << "fun @" << ident << "(): ";
    func_type->Dump();
    cout << " {" << endl;
    block->Dump();
    cout << endl << "}" << endl;
  }
  int Calc() const override
  {
     
     return 0;
  }

};

// 自己补充

class FuncTypeAST : public BaseAST 
{
 public:
  void Dump() const override 
  {
    cout << "i32";
  }
  int Calc() const override
  {
     
      return 0;
  }
};

class BlockAST : public BaseAST 
{
 public:
   unique_ptr<vector<unique_ptr<BaseAST> > > block_item_list;

  void Dump() const override 
  {
   cout << "%entry:" << endl;
    for(auto& block_item: *block_item_list)
    {
      block_item->Dump();
    }
  }
  int Calc() const override
  {
    
    return 0;
  }
};
// BlockItem ::= Decl | Stmt;
class BlockItemAST : public BaseAST 
{
 public:
  int type;
  unique_ptr<BaseAST> decl1_stmt2;
  void Dump() const override
  {
    decl1_stmt2->Dump();
  }
  int Calc() const override 
  {
    
    return 0;
  }

};


// LVal ::= IDENT;
class LValAST : public BaseAST 
{
 public:
  string ident;
  void Dump() const override
  {
      auto val = query_symbol(ident);
      assert(val->type != SYM_TYPE_UND);
      // %0 = add 0, 233   // %0 = load @x
      string instruction = (val->type == SYM_TYPE_CONST) 
                                ? "add 0, " + to_string(val->value) 
                                : "load @" + ident;

      cout << "  %" << koopacnt++ << " = " << instruction << endl;

      return;
  }
  int Calc() const override
  {
    auto val = query_symbol(ident);
    assert(val->type == SYM_TYPE_CONST);
    return val->value;
  }
};

//lv3从这里开始需要修改

// Stmt ::= "return" Exp ";";
class StmtAST : public BaseAST 
{
  public:
  int type;
  unique_ptr<BaseAST> lval;
  unique_ptr<BaseAST> exp;
  void Dump() const override
  {
    exp->Dump();

    switch (type) 
    {
        case 1: 
        {
            // 存储刚刚计算出的值
            // store %1, @x
            auto lval_ast = dynamic_cast<LValAST*>(lval.get());
            assert(lval_ast && "Expected LValAST");
            cout << "  store %" << (koopacnt - 1) << ", @" << lval_ast->ident << endl;
            break;
        }
        case 2: 
        {
            // 返回表达式结果
            // ret %0
            cout << "  ret %" << (koopacnt - 1) << endl;
            break;
        }
        default:
            break;
    }
    }
    int Calc() const override
    {
      
      return 0;
    }
};

// Exp ::= LOrExp;
class ExpAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> lorexp;

  void Dump() const override 
  {
    lorexp->Dump();
  }
  int Calc() const override
  {
    return lorexp->Calc();
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

  void Dump() const override 
  {
    switch (type) 
    {
      case 1:
      case 2:
        exp->Dump();
        break;
      case 3:
        cout << "  %" << koopacnt ++ << " = add 0, " << number << endl;   
        break;
    }
  }
  int Calc() const override
  {
    switch (type) 
    {
          case 1:
          case 2:
            return exp->Calc();
            break;
          case 3:
            return number;
            break;
    }
    
    return 0;
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

  void Dump() const override 
  { 
    primaryexp1_unaryexp2->Dump();
    if (type == 2) 
    {
        cout << "  %" << koopacnt++ << " = "
         << (unaryop == '-' ? "sub" : "eq")
         << " 0, %" << (koopacnt - 2) << endl;
    }
  }
  int Calc() const override
  {
      switch (type) 
      {
          case 1:
              return primaryexp1_unaryexp2->Calc();
          case 2: 
          {
              int tmp = primaryexp1_unaryexp2->Calc();
              switch (unaryop) 
              {
                  case '+':
                      return tmp;
                  case '-':
                      return -tmp;
                  case '!':
                      return !tmp;
                  default:
                      return 0;
              }
              break;
          }
          default:
              return 0;
      }
  }


};




// MulExp ::= UnaryExp | MulExp MulOp UnaryExp;
// MulOp ::= "*" | "/" | "%"
// MulExp ::= UnaryExp | MulExp MulOp UnaryExp;
// MulOp ::= "*" | "/" | "%"
class MulExpAST : public BaseAST 
{
 public:
  int type;
  char mulop;
  unique_ptr<BaseAST> mulexp;
  unique_ptr<BaseAST> unaryexp;

  void Dump() const override 
  {
    switch(type) 
    {
        case 1:
            unaryexp->Dump();
            break;
        case 2: 
            mulexp->Dump();
            int left = koopacnt - 1;
            unaryexp->Dump();
            int right = koopacnt - 1;
            string op;
            switch(mulop)
            {
                case '*': op = "mul"; break;
                case '/': op = "div"; break;
                case '%': op = "mod"; break;
            }
            cout << "  %" << koopacnt++ << " = " << op << " %" << left << ", %" << right << endl;
            break;

    }
  }

  int Calc() const override 
  {
    switch(type) 
        {
            case 1:
                return unaryexp->Calc();
            case 2: 
            {
                int left = mulexp->Calc();
                int right = unaryexp->Calc();
                switch(mulop) 
                {
                    case '*':
                        // %2 = mul %0, %1
                        return left * right;
                    case '/':
                        // %2 = div %0, %1
                        return left / right;
                    case '%':
                        // %2 = mod %0, %1
                        return left % right;
                    default:
                        return 0;
                }
            }
            default:            
                return 0;
        }
  }
};



// AddExp ::= MulExp | AddExp AddOp MulExp;
// AddOp ::= "+" | "-"
class AddExpAST : public BaseAST 
{
 public:
  int type;
  char addop;
  unique_ptr<BaseAST> addexp;
  unique_ptr<BaseAST> mulexp;

  void Dump() const override 
  {
    switch(type) 
    {
        case 1:
            mulexp->Dump();
            break;
        case 2:
            addexp->Dump();
            int left = koopacnt - 1;
            mulexp->Dump();
            int right = koopacnt - 1;
            string op;
            switch(addop) 
            {
                case '+':
                    // %2 = add %0, %1
                    op = "add";
                    break;
                case '-':
                    // %2 = sub %0, %1
                    op = "sub";
                    break;
            }
            cout << "  %" << koopacnt++ << " = " << op << " %" << left << ", %" << right << endl;
            break;
    }
  }

  int Calc() const override
  {
      switch(type) 
      {
          case 1:
              return mulexp->Calc();
          case 2: 
          {
              int left = addexp->Calc();
              int right = mulexp->Calc();
              switch(addop) 
              {
                  case '+':
                      // %2 = add %0, %1
                      return left + right;
                  case '-':
                      // %2 = sub %0, %1
                      return left - right;
                  default:
                      // 未知的 addop，返回 0
                      return 0;
              }
          }
          default:
              // 未知的 type，返回 0
              return 0;
      }
  }
};




// RelExp ::= AddExp | RelExp RelOp AddExp;
// RelOp ::= "<" | ">" | "<=" | ">="
class RelExpAST : public BaseAST 
{
 public:
  int type;
  string relop;
  unique_ptr<BaseAST> relexp;
  unique_ptr<BaseAST> addexp;

  void Dump() const override
   
  {
    switch(type) 
    {
        case 1:
            addexp->Dump();
            break;
        case 2:
        {
            relexp->Dump();
            int left = koopacnt - 1;
            addexp->Dump();
            int right = koopacnt - 1;

            // Map relational operators to their corresponding instructions
            unordered_map<string, string> relop_map = {
                {"<", "lt"},
                {">", "gt"},
                {"<=", "le"},
                {">=", "ge"}
            };

            auto it = relop_map.find(relop);
            if (it != relop_map.end()) {
                // %2 = <op> %0, %1
                cout << "  %" << koopacnt << " = " << it->second 
                    << " %" << left << ", %" << right << endl;
                koopacnt++;
            }
            break;
        }

    }
  }

  int Calc() const override
  {
      switch(type) 
      {
          case 1:
              return addexp->Calc();
          case 2: 
          {
              int left = relexp->Calc();
              int right = addexp->Calc();
              return (relop == "<") ? (left < right) :
                    (relop == "<=") ? (left <= right) :
                    (relop == ">") ? (left > right) :
                    (relop == ">=") ? (left >= right) :
                    0; 
              
          }
          default:
              // 未知的 type，返回 0
              return 0;
      }
  }
};



// EqExp ::= RelExp | EqExp EqOp RelExp;
// EqOp ::= "==" | "!="
class EqExpAST : public BaseAST 
{
 public:
  int type;
  string eqop;
  unique_ptr<BaseAST> eqexp;
  unique_ptr<BaseAST> relexp;

  void Dump() const override 
  {
    switch(type) 
    {
        case 1:
            relexp->Dump();
            break;
        case 2:
            eqexp->Dump();
            int left = koopacnt - 1;
            relexp->Dump();
            int right = koopacnt - 1;
             // %2 = eq %0, %1  // %2 = ne %0, %1
            cout << "  %" << koopacnt++ << " = " 
                << ((eqop == "==") ? "eq" : "ne") 
                << " %" << left << ", %" << right << endl;
            
            break;
    }
  }

  int Calc() const override 
  {
    switch(type)
    {
        case 1:
            return relexp->Calc();
        case 2:
        {
            int left = eqexp->Calc();
            int right = relexp->Calc();
            return (eqop == "==") ? (left == right) :
                  (eqop == "!=") ? (left != right) :
                  0; 
        }
        default:
            return 0;
    }
  }
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST 
{
 public:
  int type;
  unique_ptr<BaseAST> landexp;
  unique_ptr<BaseAST> eqexp;

  void Dump() const override 
  {
  switch(type)
  {
      case 1:
          eqexp->Dump();
          break;
      case 2:
      {
        landexp->Dump();
        int left = koopacnt - 1;
        eqexp->Dump();
        int right = koopacnt - 1;
        // A&&B <==> (A!=0)&(B!=0)
        // %2 = ne %0, 0
        cout << "  %" << koopacnt << " = ne %" << left << ", 0" << endl;
        left = koopacnt;
        koopacnt++;
        // %3 = ne %1, 0
        cout << "  %" << koopacnt << " = ne %" << right << ", 0" << endl;
        right = koopacnt;
        koopacnt++;
        // %4 = and %2, %3
        cout << "  %" << koopacnt++ << " = and %" << left << ", %" << right << endl;
        break;
      }
  }
  }

  int Calc() const override 
  {
    switch(type)
    {
        case 1:
            return eqexp->Calc();
        case 2:
        {
            int left = landexp->Calc();
            int right = eqexp->Calc();
            return left && right;
        }
        default:
            return 0;
    }
  }
};

// LOrExp  ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST 
{
 public:
  int type;
  unique_ptr<BaseAST> lorexp;
  unique_ptr<BaseAST> landexp;

  void Dump() const override 
  {
    switch(type)
    {
        case 1:
          landexp->Dump();
          break;
        case 2:
        {
          lorexp->Dump();
          int left = koopacnt - 1;
          landexp->Dump();
          int right = koopacnt - 1;
          // A||B <==> (A!=0)|(B!=0)
          // %2 = ne %0, 0
          cout << "  %" << koopacnt << " = ne %" << left << ", 0" << endl;
          left = koopacnt;
          koopacnt++;
          // %3 = ne %1, 0
          cout << "  %" << koopacnt << " = ne %" << right << ", 0" << endl;
          right = koopacnt;
          koopacnt++;
          // %4 = or %2, %3
          cout << "  %" << koopacnt++ << " = or %" << left << ", %" << right << endl;
          break;
        }
      }   
  }

  int Calc() const override 
  {
      switch(type)
      {
          case 1:
              return landexp->Calc();
          case 2:
          {
              int left = lorexp->Calc();
              int right = landexp->Calc();
              return left || right;
          }
          default:
              
              return 0;
      }
  }
};
// ConstExp ::= Exp;
class ConstExpAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> exp;

  void Dump() const override 
  {
    
    return;
  }

  int Calc() const override 
  {
    return exp->Calc();
  }
};