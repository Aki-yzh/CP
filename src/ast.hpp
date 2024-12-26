#pragma once
#include <iostream>
#include <string>
#include <cassert>
#include <unordered_map>
#include <memory>
#include <vector>
#include "Symbol_table.hpp"
#include <stack>


using namespace std;
static int koopacnt = 0;

// 计数 if 语句，用于设置 entry
static int ifcnt = 0;
// 当前 entry 是否已经 结束, 若为 1 的话不应再生成任何语句
static int entry_returned = 0;

// 计数 while 语句，用于设置 entry
static int whilecnt = 0;
// 当前 while 语句的标号栈
static stack<int> whileStack;


// 所有 AST 的基类
class BaseAST 
{
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual int EVa() const = 0;
};


class CompUnitAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> func_def;
  void Dump() const override
  {
    func_def->Dump();
  }
  int EVa() const override
  {
    
    return 0;
  }
};


class DeclAST : public BaseAST 
{
 public:
  int type;
  unique_ptr<BaseAST> const_decl1_var_decl2;
  void Dump() const override
  {
     const_decl1_var_decl2->Dump();
  }
  int EVa() const override
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
  int EVa() const override
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
  int EVa() const override
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
  int EVa() const override
  {
     return const_exp->EVa();
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
    //===
     insert_symbol(ident, SYM_TYPE_CONST, const_init_val->EVa());
  }
  int EVa() const override
  {
    
    return 0;
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

  int EVa() const override
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
        // 先 alloc 一段内存
      // @x = alloc i32
      cout << "  @" << current_code_block() << ident << " = alloc i32" << endl;
      insert_symbol(ident, SYM_TYPE_VAR, 0);

      if(type==2) 
      {
        init_val->Dump();
        // 存入 InitVal
        // store %1, @x
        cout << "  store %" << koopacnt-1 << ", @"<< query_symbol(ident).first << ident << endl;
      }
  }
   int EVa() const override
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
   int EVa() const override
  {
    
    return 0;
  }
};


// FuncType ::= "int";
class FuncTypeAST : public BaseAST 
{
 public:
  string type;
  void Dump() const override
  {
     cout << type;

  }
   int EVa() const override
  {
    
    return 0;
  }
};

// FuncDef ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;
  void Dump() const override
  {
    // fun @main(): i32 {}
    cout << "fun @" << ident << "(): ";
    func_type->Dump();

    cout << " {" << endl<< "%entry:" << endl;
    entry_returned = 0;
    block->Dump();
    // 若函数还未返回, 补一个ret
    // 无返回值补 ret
    if (!entry_returned) 
    {
      const string& type = dynamic_cast<FuncTypeAST*>(func_type.get())->type;
      if (type == "i32")
        cout << "  ret 0" << endl;
      else if (type == "void")
        cout << "  ret" << endl;
      else
        assert(0);
    }
    cout << "}" << endl;
  }
   int EVa() const override
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
    enter_code_block();
    for(auto& block_item: *block_item_list)
    {
      if(entry_returned) break;
      block_item->Dump();
    }
    exit_code_block();
    }
     int EVa() const override
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
   int EVa() const override
  {
    
    return 0;
  }
};


//-----

// LVal ::= IDENT;
class LValAST : public BaseAST 
{
 public:
  string ident;
  void Dump() const override
  {


    auto val = query_symbol(ident);
    assert(val.second->type != SYM_TYPE_UND);

    string instruction = (val.second->type == SYM_TYPE_CONST) 
                          ? "add 0, " + to_string(val.second->value) 
                          : "load @" + val.first + ident;

    cout << "  %" << koopacnt++ << " = " << instruction << endl;
    return;
  }
  int EVa() const override
  {
    auto val = query_symbol(ident);
    assert(val.second->type == SYM_TYPE_CONST);
    return val.second->value;
  }
};


class StmtAST : public BaseAST 
{
 public:
  int type;
  unique_ptr<BaseAST> lval;
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> block;
  unique_ptr<BaseAST> stmt_if;
  unique_ptr<BaseAST> stmt_else;
   unique_ptr<BaseAST> stmt_while;

  void Dump() const override
  {
    switch (type) 
    {
      case 1: // LVal '=' Exp ';'
      {  exp->Dump();
        const string& ident = dynamic_cast<LValAST*>(lval.get())->ident;
        cout << "  store %" << koopacnt-1 << ", @"<< query_symbol(ident).first << ident << endl;
        break;
      }
      case 2: // ';'
        break;
      case 3: // Exp ';'
      {
        exp->Dump();
        break;
      }
      case 4: // Block
      {
        block->Dump();
        break;
      }
      case 5:
      case 6:
      { 
        
        // IF "(" Exp ")" Stmt
       
        // IF "(" Exp ")" Stmt ELSE Stmt
        if(entry_returned) return;
        int ifcur = ifcnt;
        ifcnt++;
        exp->Dump();
        if(type==5) {
          // br %0, %then, %end
          cout << "  br %" << koopacnt-1 << ", %STMTIF_THEN_" << ifcur;
          cout << ", %STMTIF_END_" << ifcur << endl;
          // %STMTIF_THEN_233: 创建新的entry
          cout << "%STMTIF_THEN_" << ifcur << ":" << endl;
          entry_returned = 0;
          stmt_if->Dump();
          if(!entry_returned) {
            // jump %STMTIF_END_233
            cout << "  jump %STMTIF_END_" << ifcur << endl;
          }
          // %STMTIF_END_233: 创建新的entry
          cout << "%STMTIF_END_" << ifcur << ":" << endl;
          entry_returned = 0;
        }
        else if(type==6) {
          // br %0, %then, %else
          cout << "  br %" << koopacnt-1 << ", %STMTIF_THEN_" << ifcur;
          cout << ", %STMTIF_ELSE_" << ifcur << endl;
          // %STMTIF_THEN_233: 创建新的entry
          cout << "%STMTIF_THEN_" << ifcur << ":" << endl;
          entry_returned = 0;
          stmt_if->Dump();
          if(!entry_returned) {
            // jump %STMTIF_END_233
            cout << "  jump %STMTIF_END_" << ifcur << endl;
          }
          // %STMTIF_ELSE_233: 创建新的entry
          cout << "%STMTIF_ELSE_" << ifcur << ":" << endl;
          entry_returned = 0;
          stmt_else->Dump();
          if(!entry_returned) {
            // jump %STMTIF_END_233
            cout << "  jump %STMTIF_END_" << ifcur << endl;
          }
          // %STMTIF_END_233: 创建新的entry
          cout << "%STMTIF_END_" << ifcur << ":" << endl;
          entry_returned = 0;
        }
        break;
      }
      case 7:
      {
         if (entry_returned) return;

        int currentWhile = whilecnt++;
        whileStack.push(currentWhile);

        // 生成 while 入口跳转指令
        cout << "  jump %WHILE_ENTRY_" << currentWhile << endl;
        // 生成 while 入口标签
        cout << "%WHILE_ENTRY_" << currentWhile << ":" << endl;

        entry_returned = 0;
        exp->Dump();

        // 生成条件跳转指令
        cout << "  br %" << koopacnt-1 << ", %WHILE_BODY_" << currentWhile;
        cout << ", %WHILE_END_" << currentWhile << endl;

        // 生成 while 主体标签
        cout << "%WHILE_BODY_" << currentWhile << ":" << endl;
        entry_returned = 0;
        stmt_while->Dump();

        if (!entry_returned) 
        {
            // 生成跳转回入口的指令
            cout << "  jump %WHILE_ENTRY_" << currentWhile << endl;
        }

        // 生成 while 结束标签
        cout << "%WHILE_END_" << currentWhile << ":" << endl;
        entry_returned = 0;

        // 恢复父 while 标号
        whileStack.pop();
        break;
      }
      case 8:
      {
        // BREAK ';' 
          // jump %while_end
        if (!whileStack.empty()) 
        {
          int currentWhile = whileStack.top();
          // 生成跳转到 while 结束标签的指令
          cout << "  jump %WHILE_END_" << currentWhile << endl;
          entry_returned = 1;
        }
        break;
      }
      case 9:
      {
        //CONTINUE ';' 
        // jump %while_entry
        if (!whileStack.empty()) 
        {
            int currentWhile = whileStack.top();
            // 生成跳转到 while 入口标签的指令
            cout << "  jump %WHILE_ENTRY_" << currentWhile << endl;
            entry_returned = 1;
        }
        break;
      }
      case 10:
      { // RETURN ';'
        cout << "  ret" << endl;
        entry_returned = 1;
        break;
      }
      case 11: 
      {// RETURN Exp ';'
        exp->Dump();
        cout << "  ret %" << koopacnt-1 << endl;
        entry_returned = 1;
        break;
      }
    }
  }

  int EVa() const override
  {
    return 0;
  }
};

//-----



// Exp ::= LOrExp;
class ExpAST : public BaseAST 
{
 public:
  unique_ptr<BaseAST> lorexp;
  void Dump() const override
  {
    lorexp->Dump();
  }
  int EVa() const override
  {
    return lorexp->EVa();
  }
};

// LVal ::= IDENT;


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
  int EVa() const override
  {
    switch (type) 
    {
          case 1:
          case 2:
            return exp->EVa();
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
      if (unaryop == '-' || unaryop == '!')
        cout << "  %" << koopacnt++ << " = "
         << (unaryop == '-' ? "sub" : "eq")
         << " 0, %" << (koopacnt - 2) << endl;
    }
  }
  int EVa() const override
  {
      switch (type) 
      {
          case 1:
              return primaryexp1_unaryexp2->EVa();
          case 2: 
          {
              int tmp = primaryexp1_unaryexp2->EVa();
              return (unaryop == '+') ? tmp :
                    (unaryop == '-') ? -tmp :
                    (unaryop == '!') ? !tmp :
                    0; 
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
                default: return;
            }
            cout << "  %" << koopacnt++ << " = " << op << " %" << left << ", %" << right << endl;
            break;

    }
  }

  int EVa() const override 
  {
    switch(type) 
        {
            case 1:
                return unaryexp->EVa();
            case 2: 
            {
                int left = mulexp->EVa();
                int right = unaryexp->EVa();
                // %2 = mul/div/mod %0, %1
                return (mulop == '*') ? left * right :
                      (mulop == '/') ? left / right :
                      (mulop == '%') ? left % right :
                      0; 
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
                default:
                    return;
            }
            cout << "  %" << koopacnt++ << " = " << op << " %" << left << ", %" << right << endl;
            break;
    }
  }

  int EVa() const override
  {
      switch(type) 
      {
          case 1:
              return mulexp->EVa();
          case 2: 
          {
              int left = addexp->EVa();
              int right = mulexp->EVa();
              // %2 = add/sub %0, %1
              return (addop == '+') ? left + right :
                    (addop == '-') ? left - right :
                    0; 
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
            unordered_map<string, string> relop_map = 
            {
                {"<", "lt"},
                {">", "gt"},
                {"<=", "le"},
                {">=", "ge"}
            };

            auto it = relop_map.find(relop);
            if (it != relop_map.end()) 
            {
                // %2 = <op> %0, %1
                cout << "  %" << koopacnt << " = " << it->second 
                    << " %" << left << ", %" << right << endl;
                koopacnt++;
            }
            break;
        }

    }
  }

  int EVa() const override
  {
      switch(type) 
      {
          case 1:
              return addexp->EVa();
          case 2: 
          {
              int left = relexp->EVa();
              int right = addexp->EVa();
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

  int EVa() const override 
  {
    switch(type)
    {
        case 1:
            return relexp->EVa();
        case 2:
        {
            int left = eqexp->EVa();
            int right = relexp->EVa();
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
      // A&&B <==> (A!=0)&(B!=0)
      landexp->Dump();
      // %2 = ne %0, 0
      cout << "  %" << koopacnt++ << " = ne %"<< koopacnt-2 << ", 0" << endl;

    
      int ifcur = ifcnt++;
      
      // @STMTIF_LAND_RESULT_233 = alloc i32
      cout << "  @" << "STMTIF_LAND_RESULT_" << ifcur << " = alloc i32" << endl;

      // br %0, %then, %else
      cout << "  br %" << koopacnt-1 << ", %STMTIF_THEN_" << ifcur<< ", %STMTIF_ELSE_" << ifcur << endl;


      cout << "%STMTIF_THEN_" << ifcur << ":" << endl;
      entry_returned = 0;

      eqexp->Dump();
      // %2 = ne %0, 0
      cout << "  %" << koopacnt++ << " = ne %" << koopacnt-2 << ", 0" << endl;
      
      cout << "  store %" << koopacnt-1 << ", @"<< "STMTIF_LAND_RESULT_" << ifcur << endl;

      if(!entry_returned) 
      {
        // jump %STMTIF_END_233
        cout << "  jump %STMTIF_END_" << ifcur << endl;
      }


      cout << "%STMTIF_ELSE_" << ifcur << ":" << endl;
      entry_returned = 0;

      cout << "  store 0, @"<< "STMTIF_LAND_RESULT_" << ifcur << endl;

      if(!entry_returned) 
      {
        // jump %STMTIF_END_233
        cout << "  jump %STMTIF_END_" << ifcur << endl;
      }

   
      cout << "%STMTIF_END_" << ifcur << ":" << endl;
      entry_returned = 0;
      cout << "  %" << koopacnt++ << " = load @"<< "STMTIF_LAND_RESULT_" << ifcur << endl;
      break;
    }
  }
  }
  int EVa() const override 
  {
    switch(type)
    {
        case 1:
            return eqexp->EVa();
        case 2:
        {
            int left = landexp->EVa();
            if(!left) return 0;
            int right = eqexp->EVa();
            return (right!=0);
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
      switch (type)
      {
      case 1:
          landexp->Dump();
          break;
      case 2:
      {
        // A||B <==> (A!=0)|(B!=0)
        lorexp->Dump();
        // %2 = ne %0, 0
        cout << "  %" << koopacnt++ << " = ne %"<< koopacnt-2 << ", 0" << endl;
        

        // 短路求值, 相当于一个if
        int ifcur = ifcnt++;
        
        // @STMTIF_LOR_RESULT_233 = alloc i32
        cout << "  @" << "STMTIF_LOR_RESULT_" << ifcur << " = alloc i32" << endl;

        // br %0, %then, %else
        cout << "  br %" << koopacnt-1 << ", %STMTIF_THEN_" << ifcur << ", %STMTIF_ELSE_" << ifcur << endl;

        // %STMTIF_THEN_233: 创建新的entry
        cout << "%STMTIF_THEN_" << ifcur << ":" << endl;
        entry_returned = 0;
        // || 左侧 LOrExp 为 1, 答案为 1, 即左侧 LOrExp 的值
        cout << "  store 1, @" << "STMTIF_LOR_RESULT_" << ifcur << endl;

        if(!entry_returned) {
          // jump %STMTIF_END_233
          cout << "  jump %STMTIF_END_" << ifcur << endl;
        }

        // %STMTIF_ELSE_233: 创建新的entry
        cout << "%STMTIF_ELSE_" << ifcur << ":" << endl;
        entry_returned = 0;
        // || 左侧 LOrExp 为 0, 答案为 LAndExp 的值
        landexp->Dump();
        // %2 = ne %0, 0
        cout << "  %" << koopacnt++ << " = ne %"<< koopacnt-2 << ", 0" << endl << "  store %" << koopacnt-1 << ", @"<< "STMTIF_LOR_RESULT_" << ifcur << endl;

        if(!entry_returned) {
          // jump %STMTIF_END_233
          cout << "  jump %STMTIF_END_" << ifcur << endl;
        }

        // %STMTIF_END_233: 创建新的entry
        cout << "%STMTIF_END_" << ifcur << ":" << endl;
        entry_returned = 0;
        cout << "  %" << koopacnt++ << " = load @" << "STMTIF_LOR_RESULT_" << ifcur << endl;
       break;
      }
  }
  }

  int EVa() const override 
  {
      switch(type)
      {
          case 1:
              return landexp->EVa();
          case 2:
          {
              int left = lorexp->EVa();
              if(left) return 1;
              int right = landexp->EVa();
              return (right!=0);
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

  int EVa() const override 
  {
    return exp->EVa();
  }
};