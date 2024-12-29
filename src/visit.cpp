#include <cstring>
#include "koopa.h"
#include "visit.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

// 类型为 koopa_raw_value 的有返回值的语句的存储位置
static unordered_map<koopa_raw_value_t, string> loc;
// 栈帧长度
static int stack_frame_length = 0;
// 已经使用的栈帧长度
static int stack_frame_used = 0;

// 访问 raw program
void Visit(const koopa_raw_program_t &program) 
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) 
{
  for (size_t i = 0; i < slice.len; ++i) 
  {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) 
    {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) 
{
  // 执行一些其他的必要操作
  // ...
   // 输出函数头部的汇编指令
  cout << "  .text" << endl<< "  .globl " << func->name + 1 << endl << func->name + 1 << ":" << endl;
  // 重置栈帧相关的变量
  stack_frame_length = 0;
  stack_frame_used = 0;

  // 计算栈帧长度
  int var_cnt = 0;

  // 遍历基本块
  for (size_t i = 0; i < func->bbs.len; ++i)
  {
    const auto& insts = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i])->insts;
    var_cnt += insts.len;
    for (size_t j = 0; j < insts.len; ++j)
    {
      auto inst = reinterpret_cast<koopa_raw_value_t>(insts.buffer[j]);
      if(inst->ty->tag == KOOPA_RTT_UNIT)
          var_cnt--;
    }
  }
  // 每个变量占用4字节空间
  stack_frame_length = var_cnt << 2;
  // 将栈帧长度对齐到 16
  stack_frame_length = (stack_frame_length + 16 - 1) & (~(16 - 1));
  //分配栈空间
  if (stack_frame_length != 0) 
  {
    if (stack_frame_length <= 2047) 
    {
        cout << "  addi sp, sp, -" << stack_frame_length << endl;
    } 
    else 
    {
        int high = (stack_frame_length >> 12) & 0xFFFFF;
        int low = stack_frame_length & 0xFFF;
        cout << "  lui t0, " << high << endl;
        cout << "  addi t0, t0, -" << low << endl;
        cout << "  sub sp, sp, t0" << endl;
    }
}

  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) 
{
  // 执行一些其他的必要操作
  // ...
  if(strncmp(bb->name+1, "entry", 5))
    cout << bb->name+1 << ":" << endl;
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) 
{
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) 
  {
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
       // 访问 alloc 指令
    case KOOPA_RVT_ALLOC:
      loc[value] = to_string(stack_frame_used) + "(sp)";
      stack_frame_used += 4;
      break;
      // 访问 load 指令
    case KOOPA_RVT_LOAD:
      Visit(kind.data.load, value);
      break;
       // 访问 store 指令
    case KOOPA_RVT_STORE:
      Visit(kind.data.store);
      break;
      // 访问 binary 指令
    case KOOPA_RVT_BINARY:
      Visit(kind.data.binary, value);
      break;
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
        // 访问 br 指令
    case KOOPA_RVT_BRANCH:
      Visit(kind.data.branch);
      break;
      // 访问 jump 指令
    case KOOPA_RVT_JUMP:
      Visit(kind.data.jump);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
      break;
  }
}
// new

void Visit(const koopa_raw_return_t &ret) 
{
  // 将返回值放置在 a0 寄存器中
  if (ret.value->kind.tag == KOOPA_RVT_INTEGER) 
  {
    cout << "  li a0, " << ret.value->kind.data.integer.value << endl;
  }
  else
  {
    cout << "  lw a0, " << loc[ret.value] << endl;
  }
  // 恢复栈帧
if (stack_frame_length != 0) 
{
    if (stack_frame_length <= 2047) 
    {
        cout << "  addi sp, sp, " << stack_frame_length << endl;
    } 
    else 
    {
        int high = (stack_frame_length >> 12) & 0xFFFFF;
        int low = stack_frame_length & 0xFFF;
        cout << "  lui t0, " << high << endl;
        cout << "  addi t0, t0, " << low << endl;
        cout << "  add sp, sp, t0" << endl;
    }
}
  cout << "  ret" << endl;
}

// 处理 integer 指令，加载整数常量到 a0 寄存器
void Visit(const koopa_raw_integer_t &integer) 
{
  cout << "  li a0, " << integer.value << endl;
}

// 处理 load 指令，将源操作数加载到 t0 寄存器，并存储结果到栈中
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value) 
{
   // 将 load.src 的值放置在 t0 寄存器中
  if (load.src->kind.tag == KOOPA_RVT_INTEGER) 
  {
    cout << "  li t0, " << load.src->kind.data.integer.value << endl;
  }
  else
  {
    int offset = stoi(loc[load.src].substr(0, loc[load.src].find("(sp)")));
    if (offset <= 2047 && offset >= -2048) 
    {
        cout << "  lw t0, " << loc[load.src] << endl;
    } 
    else 
    {
        cout << "  li t6, " << offset << endl;
        cout << "  add t6, t6, sp" << endl;
        cout << "  lw t0, 0(t6)" << endl;
    }
    }
  loc[value] = to_string(stack_frame_used) + "(sp)";
  stack_frame_used += 4;
  cout << "  sw t0, " << loc[value] << endl;
}

// 处理 store 指令，将源操作数存储到目标地址
void Visit(const koopa_raw_store_t &store) 
{
   // 将 store.value 的值放置在 t0 寄存器中
  if (store.value->kind.tag == KOOPA_RVT_INTEGER) 
  {
    cout << "  li t0, " << store.value->kind.data.integer.value << endl;
  }
  else
  {
    cout << "  lw t0, " << loc[store.value] << endl;
  }
  int offset = stoi(loc[store.dest].substr(0, loc[store.dest].find("(sp)")));
    if (offset <= 2047 && offset >= -2048) 
    {
        cout << "  sw t0, " << loc[store.dest] << endl;
    } 
    else 
    {
        cout << "  li t6, " << offset << endl;
        cout << "  add t6, t6, sp" << endl;
        cout << "  sw t0, 0(t6)" << endl;
    }
}

// 处理 binary 指令，执行二元运算并将结果存储到栈中
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value) 
{
  // 将运算数存入 t0 和 t1
   if (binary.lhs->kind.tag == KOOPA_RVT_INTEGER) 
  {
    cout << "  li t0, " << binary.lhs->kind.data.integer.value << endl;
  }
  else
  {
    cout << "  lw t0, " << loc[binary.lhs] << endl;
  }
  if (binary.rhs->kind.tag == KOOPA_RVT_INTEGER) 
  {
    cout << "  li t1, " << binary.rhs->kind.data.integer.value << endl;
  }
  else
  {
    cout << "  lw t1, " << loc[binary.rhs] << endl;
  }
  // 进行运算，结果存入t0
  static const unordered_map<koopa_raw_binary_op_t, vector<string>> opInstructions = 
  {
      {KOOPA_RBO_NOT_EQ, {"xor t0, t0, t1", "snez t0, t0"}},
      {KOOPA_RBO_EQ, {"xor t0, t0, t1", "seqz t0, t0"}},
      {KOOPA_RBO_GT, {"sgt t0, t0, t1"}},
      {KOOPA_RBO_LT, {"slt t0, t0, t1"}},
      {KOOPA_RBO_GE, {"slt t0, t0, t1", "xori t0, t0, 1"}},
      {KOOPA_RBO_LE, {"sgt t0, t0, t1", "xori t0, t0, 1"}},
      {KOOPA_RBO_ADD, {"add t0, t0, t1"}},
      {KOOPA_RBO_SUB, {"sub t0, t0, t1"}},
      {KOOPA_RBO_MUL, {"mul t0, t0, t1"}},
      {KOOPA_RBO_DIV, {"div t0, t0, t1"}},
      {KOOPA_RBO_MOD, {"rem t0, t0, t1"}},
      {KOOPA_RBO_AND, {"and t0, t0, t1"}},
      {KOOPA_RBO_OR, {"or t0, t0, t1"}},
      {KOOPA_RBO_XOR, {"xor t0, t0, t1"}},
      {KOOPA_RBO_SHL, {"sll t0, t0, t1"}},
      {KOOPA_RBO_SHR, {"srl t0, t0, t1"}},
      {KOOPA_RBO_SAR, {"sra t0, t0, t1"}}
  };

  auto it = opInstructions.find(binary.op);
  if (it != opInstructions.end()) 
  {
      for (const auto &instr : it->second)
      {
          cout << "  " << instr << endl;
      }
  } 
  // 将 t0 中的结果存入栈
  loc[value] = to_string(stack_frame_used) + "(sp)";
  stack_frame_used += 4;
  cout << "  sw t0, " << loc[value] << endl;
}
// 访问 br 指令
void Visit(const koopa_raw_branch_t &branch) 
{
    // 将条件值加载到寄存器 t0 中
    if (branch.cond->kind.tag == KOOPA_RVT_INTEGER) 
    {
        cout << "  li t0, " << branch.cond->kind.data.integer.value << endl;
    } 
    else 
    {
        cout << "  lw t0, " << loc[branch.cond] << endl;
    }
    // 根据条件跳转到相应的基本块
    cout << "  bnez t0, DOUBLE_JUMP_" << branch.true_bb->name + 1 << endl << "  j " << branch.false_bb->name + 1 << endl;
    // 生成 true 分支的标签，跳转到 true 分支的基本块
    cout << "DOUBLE_JUMP_" << branch.true_bb->name + 1 << ":" << endl << "  j " << branch.true_bb->name + 1 << endl;

}
// 访问 jump 指令
void Visit(const koopa_raw_jump_t &jump) 
{
  cout << "  j " << jump.target->name+1 << endl;
}
// 视需求自行实现
// ...