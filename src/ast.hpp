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

// 计数 if 语句，用于设置 entry
static int ifcnt = 0;
// 当前 entry 是否已经 ret, 若为 1 的话不应再生成任何语句
static int entry_returned = 0;

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


/**************************Decl***************************/

// Decl ::= ConstDecl | VarDecl;
class DeclAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> const_decl1_var_decl2;
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
class ConstDeclAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> b_type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > const_def_list;
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
class BTypeAST : public BaseAST {
 public:
  void Dump() const override
  {
     assert(0);
    return;
  }
   int Calc() const override
  {
    
    return 0;
  }
};

// ConstInitVal ::= ConstExp;
class ConstInitValAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> const_exp;
  void Dump() const override
  {
      assert(0);
      return;
  }
  int Calc() const override
  {
     return const_exp->Calc();
  }
};


// ConstDef ::= IDENT "=" ConstInitVal;
class ConstDefAST : public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> const_init_val;
  void Dump() const override
  {
    insert_symbol(ident, SYM_TYPE_CONST,
    dynamic_cast<ConstInitValAST*>(const_init_val.get())->Calc());
  }
   int Calc() const override
  {
    
    return 0;
  }
};


// VarDecl ::= BType VarDefList ";";
// VarDefList ::= VarDef | VarDefList "," VarDef;
class VarDeclAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> b_type;
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > var_def_list;
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
class VarDefAST : public BaseAST {
 public:
  int type;
  std::string ident;
  std::unique_ptr<BaseAST> init_val;
  void Dump() const override
  {
        // 先 alloc 一段内存
      // @x = alloc i32
      std::cout << "  @" << current_code_block() << ident << " = alloc i32" << std::endl;
      insert_symbol(ident, SYM_TYPE_VAR, 0);

      if(type==2) {
        init_val->Dump();
        // 存入 InitVal
        // store %1, @x
        std::cout << "  store %" << koopacnt-1 << ", @";
        std::cout << query_symbol(ident).first << ident << std::endl;
      }
  }
   int Calc() const override
  {
    
    return 0;
  }
};

// InitVal ::= Exp;
class InitValAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void Dump() const override
  {
      exp->Dump();
  }
   int Calc() const override
  {
    
    return 0;
  }
};

/**************************Func***************************/
// FuncType ::= "int";
class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  void Dump() const override
  {
     std::cout << type;

  }
   int Calc() const override
  {
    
    return 0;
  }
};

// FuncDef ::= FuncType IDENT "(" ")" Block;
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump() const override
  {
    // fun @main(): i32 {}
    std::cout << "fun @" << ident << "(): ";
    func_type->Dump();

    std::cout << " {" << std::endl;
    std::cout << "%entry:" << std::endl;
    entry_returned = 0;
    block->Dump();
    // 若函数还未返回, 补一个ret
    // 无返回值补 ret
    if (!entry_returned) {
      const std::string& type = dynamic_cast<FuncTypeAST*>(func_type.get())->type;
      if (type == "i32")
        std::cout << "  ret 0" << std::endl;
      else if (type == "void")
        std::cout << "  ret" << std::endl;
      else
        assert(0);
    }
    std::cout << "}" << std::endl;
  }
   int Calc() const override
  {
    
    return 0;
  }
};


/**************************Block***************************/

// Block ::= "{" BlockItemList "}";
// BlockItemList ::=  | BlockItemList BlockItem;
class BlockAST : public BaseAST {
 public:
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > block_item_list;
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
     int Calc() const override
  {
    
    return 0;
  }
};

// BlockItem ::= Decl | Stmt;
class BlockItemAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> decl1_stmt2;
  void Dump() const override
  {
    decl1_stmt2->Dump();
  }
   int Calc() const override
  {
    
    return 0;
  }
};
class LValAST : public BaseAST {
 public:
  std::string ident;
  void Dump() const override
  {
    auto val = query_symbol(ident);
    assert(val.second->type != SYM_TYPE_UND);

    if(val.second->type == SYM_TYPE_CONST)
    {
      // 此处有优化空间
      // %0 = add 0, 233
      std::cout << "  %" << koopacnt << " = add 0, ";
      std::cout<< val.second->value << std::endl;
      koopacnt++;
    }
    else if(val.second->type == SYM_TYPE_VAR)
    {
      // 从内存读取 LVal
      // %0 = load @x
      std::cout << "  %" << koopacnt << " = load @" << val.first << ident << std::endl;
      koopacnt++;
    }
    return;
  }
  int Calc() const override
  {
    auto val = query_symbol(ident);
    assert(val.second->type == SYM_TYPE_CONST);
    return val.second->value;
  }
};
// Stmt ::= LVal "=" Exp ";"
class StmtAssignAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> exp;
  void Dump() const override
  {
    exp->Dump();
    // 存入刚刚计算出的值
    // store %1, @x
    const std::string& ident = dynamic_cast<LValAST*>(lval.get())->ident;
    std::cout << "  store %" << koopacnt-1 << ", @";
    std::cout << query_symbol(ident).first << ident << std::endl;
  }
   int Calc() const override
  {
    
    return 0;
  }
};

//        | ";"
//        | Exp ";"
class StmtExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> exp;
  void Dump() const override
  {
    if(type==1) {
      // do nothing
    }
    else if(type==2) {
      exp->Dump();
    }
  }
   int Calc() const override
  {
    
    return 0;
  }
};

//        | Block
class StmtBlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> block;
  void Dump() const override
  {
    block->Dump();
  }
   int Calc() const override
  {
    
    return 0;
  }
};

//        | "if" "(" Exp ")" Stmt
//        | "if" "(" Exp ")" Stmt "else" Stmt
// 此处有移进/归约冲突, SysY 的语义规定了 else 必须和最近的 if 进行匹配
// 则此处应选择发生冲突时应选择移进，即选择第二条规则
// 在 sysy.y 中如下实现：
// %precedence IFX
// %precedence ELSE
// IF "(" Exp ")" Stmt %prec IFX
// IF "(" Exp ")" Stmt ELSE Stmt
class StmtIfAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> stmt_if;
  std::unique_ptr<BaseAST> stmt_else;
  void Dump() const override
  {
     assert(0);
  }
   int Calc() const override
  {
    
    return 0;
  }
};

//        | "return" ";";
//        | "return" Exp ";";
class StmtReturnAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> exp;
  void Dump() const override
  {
    if(type==1) {
      std::cout << "  ret" << std::endl;
      entry_returned = 1;
    }
    else if(type==2) {
      exp->Dump();
      // ret %0
      std::cout << "  ret %" << koopacnt-1 << std::endl;
      entry_returned = 1;
    }
  }
   int Calc() const override
  {
    
    return 0;
  }
};

/***************************Exp***************************/



// Exp ::= LOrExp;
class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> lorexp;
  void Dump() const override
  {
    lorexp->Dump();
  }
  int Calc() const override
  {
    return lorexp->Calc();
  }
};

// LVal ::= IDENT;


// PrimaryExp ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> exp1_lval2;
  int number;
  void Dump() const override
  {
    if(type==1) {
      exp1_lval2->Dump();
    }
    else if(type==2) {
      exp1_lval2->Dump();
    }
    else if(type==3) {
      // %0 = add 0, 233
      std::cout << "  %" << koopacnt << " = add 0, ";
      std::cout<< number << std::endl;
      koopacnt++;
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return exp1_lval2->Calc();
    }
    else if(type==2) {
      return exp1_lval2->Calc();
    }
    else if(type==3) {
      return number;
    }
    assert(0);
    return 0;
  }
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryOp ::= "+" | "-" | "!"
class UnaryExpAST : public BaseAST {
 public:
  int type;
  char unaryop;
  std::unique_ptr<BaseAST> primaryexp1_unaryexp2;
  void Dump() const override
  {
    if(type==1) {
      primaryexp1_unaryexp2->Dump();
    }
    else if(type==2) {
      primaryexp1_unaryexp2->Dump();
      if(unaryop=='-') {
        // %1 = sub 0, %0
        std::cout << "  %" << koopacnt << " = sub 0, %";
        std::cout << koopacnt-1 <<std::endl;
        koopacnt++;
      }
      else if(unaryop=='!') {
        // %1 = eq 0, %0
        std::cout << "  %" << koopacnt << " = eq 0, %";
        std::cout << koopacnt-1 <<std::endl;
        koopacnt++;
      }
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return primaryexp1_unaryexp2->Calc();
    }
    else if(type==2) {
      int tmp = primaryexp1_unaryexp2->Calc();
      if(unaryop=='+') {
        return tmp;
      }
      else if(unaryop=='-') {
        return -tmp;
      }
      else if(unaryop=='!') {
        return !tmp;
      }
    }
    assert(0);
    return 0;
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
  void Dump() const override
  {
    if(type==1) {
      unaryexp->Dump();
    }
    else if(type==2) {
      mulexp->Dump();
      int left = koopacnt-1;
      unaryexp->Dump();
      int right = koopacnt-1;
      if(mulop=='*') {
        // %2 = mul %0, %1
        std::cout << "  %" << koopacnt << " = mul %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
      else if(mulop=='/') {
        // %2 = div %0, %1
        std::cout << "  %" << koopacnt << " = div %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
      else if(mulop=='%') {
        // %2 = mod %0, %1
        std::cout << "  %" << koopacnt << " = mod %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return unaryexp->Calc();
    }
    else if(type==2) {
      int left = mulexp->Calc();
      int right = unaryexp->Calc();
      if(mulop=='*') {
        return left * right;
      }
      else if(mulop=='/') {
        return left / right;
      }
      else if(mulop=='%') {
        return left % right;
      }
    }
    assert(0);
    return 0;
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
  void Dump() const override
  {
    if(type==1) {
      mulexp->Dump();
    }
    else if(type==2) {
      addexp->Dump();
      int left = koopacnt-1;
      mulexp->Dump();
      int right = koopacnt-1;
      if(addop=='+') {
        // %2 = add %0, %1
        std::cout << "  %" << koopacnt << " = add %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
      else if(addop=='-') {
        // %2 = sub %0, %1
        std::cout << "  %" << koopacnt << " = sub %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return mulexp->Calc();
    }
    else if(type==2) {
      int left = addexp->Calc();
      int right = mulexp->Calc();
      if(addop=='+') {
        return left + right;
      }
      else if(addop=='-') {
        return left - right;
      }
    }
    assert(0);
    return 0;
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
  void Dump() const override
  {
     if(type==1) {
      addexp->Dump();
      }
      else if(type==2) {
        relexp->Dump();
        int left = koopacnt-1;
        addexp->Dump();
        int right = koopacnt-1;
        if(relop=="<") {
          // %2 = lt %0, %1
          std::cout << "  %" << koopacnt << " = lt %";
          std::cout << left << ", %" << right << std::endl;
          koopacnt++;
        }
        else if(relop==">") {
          // %2 = gt %0, %1
          std::cout << "  %" << koopacnt << " = gt %";
          std::cout << left << ", %" << right << std::endl;
          koopacnt++;
        }
        else if(relop=="<=") {
          // %2 = le %0, %1
          std::cout << "  %" << koopacnt << " = le %";
          std::cout << left << ", %" << right << std::endl;
          koopacnt++;
        }
        else if(relop==">=") {
          // %2 = ge %0, %1
          std::cout << "  %" << koopacnt << " = ge %";
          std::cout << left << ", %" << right << std::endl;
          koopacnt++;
        }
      }
  }
  int Calc() const override
  {
    if(type==1) {
      return addexp->Calc();
    }
    else if(type==2) {
      int left = relexp->Calc();
      int right = addexp->Calc();
      if(relop=="<") {
        return left < right;
      }
      else if(relop==">") {
        return left > right;
      }
      else if(relop=="<=") {
        return left <= right;
      }
      else if(relop==">=") {
        return left >= right;
      }
    }
    assert(0);
    return 0;
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
  void Dump() const override
  {
    if(type==1) {
      relexp->Dump();
    }
    else if(type==2) {
      eqexp->Dump();
      int left = koopacnt-1;
      relexp->Dump();
      int right = koopacnt-1;
      if(eqop=="==") {
        // %2 = eq %0, %1
        std::cout << "  %" << koopacnt << " = eq %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
      else if(eqop=="!=") {
        // %2 = ne %0, %1
        std::cout << "  %" << koopacnt << " = ne %";
        std::cout << left << ", %" << right << std::endl;
        koopacnt++;
      }
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return relexp->Calc();
    }
    else if(type==2) {
      int left = eqexp->Calc();
      int right = relexp->Calc();
      if(eqop=="==") {
        return left == right;
      }
      else if(eqop=="!=") {
        return left != right;
      }
    }
    assert(0);
    return 0;
  }
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> landexp;
  std::unique_ptr<BaseAST> eqexp;
  void Dump() const override
  {
    if(type==1) {
      eqexp->Dump();
    }
    else if(type==2) {
      landexp->Dump();
      int left = koopacnt-1;
      eqexp->Dump();
      int right = koopacnt-1;
      // A&&B <==> (A!=0)&(B!=0)
      // %2 = ne %0, 0
      std::cout << "  %" << koopacnt << " = ne %";
      std::cout << left << ", 0" << std::endl;
      left = koopacnt;
      koopacnt++;
      // %3 = ne %1, 0
      std::cout << "  %" << koopacnt << " = ne %";
      std::cout << right << ", 0" << std::endl;
      right = koopacnt;
      koopacnt++;
      // %4 = and %2, %3
      std::cout << "  %" << koopacnt << " = and %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return eqexp->Calc();
    }
    else if(type==2) {
      int left = landexp->Calc();
      int right = eqexp->Calc();
      return left && right;
    }
    assert(0);
    return 0;
  }
};

// LOrExp  ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> lorexp;
  std::unique_ptr<BaseAST> landexp;
  void Dump() const override
  {
    if(type==1) {
      landexp->Dump();
    }
    else if(type==2) {
      lorexp->Dump();
      int left = koopacnt-1;
      landexp->Dump();
      int right = koopacnt-1;
      // A||B <==> (A!=0)|(B!=0)
      // %2 = ne %0, 0
      std::cout << "  %" << koopacnt << " = ne %";
      std::cout << left << ", 0" << std::endl;
      left = koopacnt;
      koopacnt++;
      // %3 = ne %1, 0
      std::cout << "  %" << koopacnt << " = ne %";
      std::cout << right << ", 0" << std::endl;
      right = koopacnt;
      koopacnt++;
      // %4 = or %2, %3
      std::cout << "  %" << koopacnt << " = or %";
      std::cout << left << ", %" << right << std::endl;
      koopacnt++;
    }
  }
  int Calc() const override
  {
    if(type==1) {
      return landexp->Calc();
    }
    else if(type==2) {
      int left = lorexp->Calc();
      int right = landexp->Calc();
      return left || right;
    }
    assert(0);
    return 0;
  }
};

// ConstExp ::= Exp;
class ConstExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void Dump() const override
  {
    assert(0);
    return;
  }
  int Calc() const override
  {
    return exp->Calc();
  }
};