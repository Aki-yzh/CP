%code requires
{
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param {std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
// 定义 yylval

%union 
{
  std::string *str_val;
  int int_val;
  char char_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST> > *vec_val;
}

// lexer 返回的所有 token 种类的声明
%token INT RETURN CONST IF ELSE
%token LAND LOR
%token <str_val> IDENT RELOP EQOP
%token <int_val> INT_CONST
%token <char_val> MULOP

// 非终结符的类型定义

// lv3.3参考语法规范，新添加的有Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <int_val> Number
%type <char_val> UnaryOp AddOp 

//lv4新增语法规范
%type <ast_val> Decl ConstDecl BType ConstDef ConstInitVal VarDecl VarDef InitVal
%type <ast_val> BlockItem 
%type <ast_val> LVal ConstExp
%type <vec_val> ConstDefList BlockItemList VarDefList

// 用于解决 dangling else 的优先级设置
%precedence IFX
%precedence ELSE



// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值

%%
//对 ::= 右侧的每个规则都设计一种 AST,
//格式均仿照开始给出的FuncDef实现

//CompUnit      ::= FuncDef;
CompUnit
  : FuncDef 
  {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

//Decl          ::= ConstDecl | VarDecl;
Decl
  : ConstDecl 
  {
    auto ast = new DeclAST();
    ast->type = 1;
    ast->const_decl1_var_decl2 = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl 
  {
    auto ast = new DeclAST();
    ast->type = 2;
    ast->const_decl1_var_decl2 = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

//ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
ConstDecl
  : CONST BType ConstDefList ';' 
  {
    auto ast = new ConstDeclAST();
    ast->b_type = unique_ptr<BaseAST>($2);
    ast->const_def_list = unique_ptr<vector<unique_ptr<BaseAST> > >($3);
    $$ = ast;
  }
  ;

//BType         ::= "int";
BType
  : INT 
  {
    auto ast = new BTypeAST();
    $$ = ast;
  }
  ;

//NEW,常量表(把constdecl再拆分)
ConstDefList
  : ConstDef 
  {
    auto vec = new vector<unique_ptr<BaseAST> >();
    vec->push_back(unique_ptr<BaseAST>($1));
    $$ = vec;
  }
  | ConstDefList ',' ConstDef 
  {
    auto vec = $1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$ = vec;
  }
  ;

//ConstDef      ::= IDENT "=" ConstInitVal;
ConstDef
  : IDENT '=' ConstInitVal 
  {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->const_init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

//ConstInitVal  ::= ConstExp;
ConstInitVal
  : ConstExp 
  {
    auto ast = new ConstInitValAST();
    ast->const_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

//VarDecl       ::= BType VarDef {"," VarDef} ";";
VarDecl
  : BType VarDefList ';' 
  {
    auto ast = new VarDeclAST();
    ast->b_type = unique_ptr<BaseAST>($1);
    ast->var_def_list = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$ = ast;
  }
  ;
//NEW,变量表(把vardecl再拆分)
VarDefList
  : VarDef 
  {
    auto vec = new vector<unique_ptr<BaseAST> >();
    vec->push_back(unique_ptr<BaseAST>($1));
    $$ = vec;
  }
  | VarDefList ',' VarDef 
  {
    auto vec = $1;
    vec->push_back(unique_ptr<BaseAST>($3));
    $$ = vec;
  }
  ;
//VarDef        ::= IDENT | IDENT "=" InitVal;
VarDef
  : IDENT 
  {
    auto ast = new VarDefAST();
    ast->type = 1;
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal 
  {
    auto ast = new VarDefAST();
    ast->type = 2;
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
//InitVal       ::= Exp;
InitVal
  : Exp 
  {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;



// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担

FuncDef
  : FuncType IDENT '(' ')' Block 
  {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;
// 同上, 不再解释
//FuncType    ::= "int";
FuncType
  : INT 
  {
    auto ast = new FuncTypeAST();
    ast->type = "i32";
    $$ = ast;
  }
  ;
//Block         ::= "{" {BlockItem} "}";
Block
  : '{' BlockItemList '}' 
  {
    auto ast = new BlockAST();
    ast->block_item_list = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$ = ast;
  }
  ;

BlockItemList
  : 
  {
    auto vec = new vector<unique_ptr<BaseAST> >();
    $$ = vec;
  }
  | BlockItemList BlockItem 
  {
    auto vec = $1;
    vec->push_back(unique_ptr<BaseAST>($2));
    $$ = vec;
  }
  ;
//BlockItem     ::= Decl | Stmt;
BlockItem
  : Decl 
  {
    auto ast=new BlockItemAST();
    ast->type = 1;
    ast->decl1_stmt2 = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  | Stmt {
    auto ast=new BlockItemAST();
    ast->type = 2;
    ast->decl1_stmt2 = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

//Stmt            ::= LVal "=" Exp ";"
 //               | [Exp] ";"
  //              | Block
   //             | "if" "(" Exp ")" Stmt ["else" Stmt]
    //            | "return" [Exp] ";";

Stmt
 : LVal '=' Exp ';' 
 {
    auto ast = new StmtAST();
    ast->type = 1;
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | ';' 
  {
    auto ast = new StmtAST();
    ast->type = 2;
    $$ = ast;
  }
  | Exp ';' 
  {
    auto ast = new StmtAST();
    ast->type = 3;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Block 
  {
    auto ast = new StmtAST();
    ast->type = 4;
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt %prec IFX 
  {
    auto ast = new StmtAST();
    ast->type = 5;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt_if = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt 
  {
    auto ast = new StmtAST();
    ast->type = 6;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt_if = unique_ptr<BaseAST>($5);
    ast->stmt_else = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | RETURN ';' 
  {
    auto ast = new StmtAST();
    ast->type = 7;
    $$ = ast;
  }
  | RETURN Exp ';' 
  {
    auto ast = new StmtAST();
    ast->type = 8;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;
//Exp           ::= LOrExp;
Exp
  : LOrExp 
  {
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;
  //LVal          ::= IDENT;
LVal
  : IDENT 
  {
    auto ast=new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$=ast;
  }
  ;
//PrimaryExp    ::= '(' Exp ')' | LVal | Number;
PrimaryExp
  : '(' Exp ')' 
  {
    auto ast = new PrimaryExpAST();
    ast->type = 1;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal 
  {
    auto ast = new PrimaryExpAST();
    ast->type = 2;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number 
  {
    auto ast = new PrimaryExpAST();
    ast->type = 3;
    ast->number = $1;
    $$ = ast;
  }
  ;
//Number        ::= INT_CONST;
Number
  : INT_CONST 
  {
    $$ = $1;
  }
  ;
//UnaryExp      ::= PrimaryExp | UnaryOp UnaryExp;
UnaryExp
  : PrimaryExp 
  {
    auto ast = new UnaryExpAST();
    ast->type = 1;
    ast->primaryexp1_unaryexp2 = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp 
  {
    auto ast = new UnaryExpAST();
    ast->type = 2;
    ast->unaryop = $1;
    ast->primaryexp1_unaryexp2 = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;
//UnaryOp       ::= "+" | "-" | "!";
UnaryOp
  : '+' 
  {
    $$ = '+';
  }
  | '-' 
  {
    $$ = '-';
  }
  | '!' 
  {
    $$ = '!';
  }
  ;
//MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->type = 1;
    ast->unaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp MULOP UnaryExp {
    auto ast = new MulExpAST();
    ast->type = 2;
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->mulop = $2;
    ast->unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;


//AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
AddExp
  : MulExp 
  {
    auto ast = new AddExpAST();
    ast->type = 1;
    ast->mulexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp AddOp MulExp 
  {
    auto ast = new AddExpAST();
    ast->type = 2;
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->addop = $2;
    ast->mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
//不知道为什么写token里过不了样例，所以写这了
AddOp
  : '+' 
  {
    $$ = '+';
  }
  | '-' 
  {
    $$ = '-';
  }
  ;
//RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
RelExp
  : AddExp 
  {
    auto ast = new RelExpAST();
    ast->type = 1;
    ast->addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp RELOP AddExp 
  {
    auto ast = new RelExpAST();
    ast->type = 2;
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->relop = *unique_ptr<string>($2);
    ast->addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
//EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
EqExp
  : RelExp 
  {
    auto ast = new EqExpAST();
    ast->type = 1;
    ast->relexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQOP RelExp 
  {
    auto ast = new EqExpAST();
    ast->type = 2;
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->eqop = *unique_ptr<string>($2);
    ast->relexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
//LAndExp       ::= EqExp | LAndExp "&&" EqExp;
LAndExp
  : EqExp 
  {
    auto ast = new LAndExpAST();
    ast->type = 1;
    ast->eqexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp LAND EqExp 
  {
    auto ast = new LAndExpAST();
    ast->type = 2;
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->eqexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
//LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
LOrExp
  : LAndExp 
  {
    auto ast = new LOrExpAST();
    ast->type = 1;
    ast->landexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp LOR LAndExp 
  {
    auto ast = new LOrExpAST();
    ast->type = 2;
    ast->lorexp = unique_ptr<BaseAST>($1);
    ast->landexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;
//ConstExp      ::= Exp;
ConstExp
  : Exp 
  {
    auto ast=new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$=ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) 
{
  cerr << "error: " << s << endl;
}