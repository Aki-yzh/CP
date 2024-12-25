#include <iostream>
#include <cassert>
#include "ast.hpp"

static int koopacnt = 0;

/************************CompUnit*************************/

// CompUnit ::= FuncDef
void CompUnitAST::Dump() const {
  func_def->Dump();
}

/**************************Decl***************************/

// Decl ::= ConstDecl | VarDecl;
void DeclAST::Dump() const {
  const_decl1_var_decl2->Dump();
}

// ConstDecl ::= "const" BType ConstDefList ";";
// ConstDefList ::= ConstDef | ConstDefList "," ConstDef;
void ConstDeclAST::Dump() const {
  for(auto& const_def: *const_def_list)
    const_def->Dump();
}

// BType ::= "int";
void BTypeAST::Dump() const {
  assert(0);
  return;
}

// ConstDef ::= IDENT "=" ConstInitVal;
void ConstDefAST::Dump() const {
  insert_symbol(ident, SYM_TYPE_CONST,
    dynamic_cast<ConstInitValAST*>(const_init_val.get())->Calc());
}

// ConstInitVal ::= ConstExp;
void ConstInitValAST::Dump() const {
  assert(0);
  return;
}

int ConstInitValAST::Calc() const {
  return dynamic_cast<ExpBaseAST*>(const_exp.get())->Calc();
}

// VarDecl ::= BType VarDefList ";";
// VarDefList ::= VarDef | VarDefList "," VarDef;
void VarDeclAST::Dump() const {
  for(auto& var_def : *var_def_list)
    var_def->Dump();
}

// VarDef ::= IDENT | IDENT "=" InitVal;
void VarDefAST::Dump() const {
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

// InitVal ::= Exp;
void InitValAST::Dump() const {
  exp->Dump();
}

/**************************Func***************************/

// FuncDef ::= FuncType IDENT "(" ")" Block;
void FuncDefAST::Dump() const {
  // fun @main(): i32 {}
  std::cout << "fun @" << ident << "(): ";
  func_type->Dump();
  std::cout << " {" << std::endl;
  std::cout << "%entry:" << std::endl;
  block->Dump();
  std::cout << "}" << std::endl;
}

// FuncType ::= "int";
void FuncTypeAST::Dump() const {
  std::cout << "i32";
}

/**************************Block***************************/

// Block ::= "{" BlockItemList "}";
// BlockItemList ::=  | BlockItemList BlockItem;
void BlockAST::Dump() const {
  enter_code_block();
  for(auto& block_item: *block_item_list)
  {
    block_item->Dump();
  }
  exit_code_block();
}

// BlockItem ::= Decl | Stmt;
void BlockItemAST::Dump() const {
  decl1_stmt2->Dump();
}

// Stmt ::= LVal "=" Exp ";"
//        | Exp ";"
//        | ";"
//        | Block
//        | "return" Exp ";";
//        | "return" ";";
void StmtAST::Dump() const {
  if(type==1) {
    exp->Dump();
    // 存入刚刚计算出的值
    // store %1, @x
    const std::string& ident = dynamic_cast<LValAST*>(lval1_block4.get())->ident;
    std::cout << "  store %" << koopacnt-1 << ", @";
    std::cout << query_symbol(ident).first << ident << std::endl;
  }
  else if(type==2) {
    exp->Dump();
  }
  else if(type==3) {
    // do nothing
  }
  else if(type==4) {
    lval1_block4->Dump();
  }
  else if(type==5) {
    exp->Dump();
    // ret %0
    std::cout << "  ret %" << koopacnt-1 << std::endl;
  }
  else if(type==6) {
    std::cout << "  ret" << std::endl;
  }
}

/***************************Exp***************************/

// Exp ::= LOrExp;
void ExpAST::Dump() const {
  lorexp->Dump();
}

int ExpAST::Calc() const {
  return dynamic_cast<ExpBaseAST*>(lorexp.get())->Calc();
}

// LVal ::= IDENT;
// 只有 LVal 出现在表达式中时会调用该 Dump
// 如果 LVal 作为左值出现，则在父节点读取其 ident
void LValAST::Dump() const {
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

int LValAST::Calc() const {
  auto val = query_symbol(ident);
  assert(val.second->type == SYM_TYPE_CONST);
  return val.second->value;
}

// PrimaryExp ::= "(" Exp ")" | LVal | Number;
void PrimaryExpAST::Dump() const {
  if(type==1) {
    exp->Dump();
  }
  else if(type==2) {
    exp->Dump();
  }
  else if(type==3) {
    // %0 = add 0, 233
    std::cout << "  %" << koopacnt << " = add 0, ";
    std::cout<< number << std::endl;
    koopacnt++;
  }
}

int PrimaryExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(exp.get())->Calc();
  }
  else if(type==2) {
    return dynamic_cast<ExpBaseAST*>(exp.get())->Calc();
  }
  else if(type==3) {
    return number;
  }
  assert(0);
  return 0;
}

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp;
// UnaryOp ::= "+" | "-" | "!"
void UnaryExpAST::Dump() const {
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

int UnaryExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(primaryexp1_unaryexp2.get())->Calc();
  }
  else if(type==2) {
    int tmp = dynamic_cast<ExpBaseAST*>(primaryexp1_unaryexp2.get())->Calc();
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

// MulExp ::= UnaryExp | MulExp MulOp UnaryExp;
// MulOp ::= "*" | "/" | "%"
void MulExpAST::Dump() const {
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

int MulExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(unaryexp.get())->Calc();
  }
  else if(type==2) {
    int left = dynamic_cast<ExpBaseAST*>(mulexp.get())->Calc();
    int right = dynamic_cast<ExpBaseAST*>(unaryexp.get())->Calc();
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

// AddExp ::= MulExp | AddExp AddOp MulExp;
// AddOp ::= "+" | "-"
void AddExpAST::Dump() const {
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

int AddExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(mulexp.get())->Calc();
  }
  else if(type==2) {
    int left = dynamic_cast<ExpBaseAST*>(addexp.get())->Calc();
    int right = dynamic_cast<ExpBaseAST*>(mulexp.get())->Calc();
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

// RelExp ::= AddExp | RelExp RelOp AddExp;
// RelOp ::= "<" | ">" | "<=" | ">="
void RelExpAST::Dump() const {
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

int RelExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(addexp.get())->Calc();
  }
  else if(type==2) {
    int left = dynamic_cast<ExpBaseAST*>(relexp.get())->Calc();
    int right = dynamic_cast<ExpBaseAST*>(addexp.get())->Calc();
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

// EqExp ::= RelExp | EqExp EqOp RelExp;
// EqOp ::= "==" | "!="
void EqExpAST::Dump() const {
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

int EqExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(relexp.get())->Calc();
  }
  else if(type==2) {
    int left = dynamic_cast<ExpBaseAST*>(eqexp.get())->Calc();
    int right = dynamic_cast<ExpBaseAST*>(relexp.get())->Calc();
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

// LAndExp ::= EqExp | LAndExp "&&" EqExp;
void LAndExpAST::Dump() const {
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

int LAndExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(eqexp.get())->Calc();
  }
  else if(type==2) {
    int left = dynamic_cast<ExpBaseAST*>(landexp.get())->Calc();
    int right = dynamic_cast<ExpBaseAST*>(eqexp.get())->Calc();
    return left && right;
  }
  assert(0);
  return 0;
}

// LOrExp  ::= LAndExp | LOrExp "||" LAndExp;
void LOrExpAST::Dump() const {
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

int LOrExpAST::Calc() const {
  if(type==1) {
    return dynamic_cast<ExpBaseAST*>(landexp.get())->Calc();
  }
  else if(type==2) {
    int left = dynamic_cast<ExpBaseAST*>(lorexp.get())->Calc();
    int right = dynamic_cast<ExpBaseAST*>(landexp.get())->Calc();
    return left || right;
  }
  assert(0);
  return 0;
}

// ConstExp ::= Exp;
void ConstExpAST::Dump() const {
  assert(0);
  return;
}

int ConstExpAST::Calc() const {
  return dynamic_cast<ExpBaseAST*>(exp.get())->Calc();
}