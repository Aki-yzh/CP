#include <cstring>
#include "koopa.h"
#include "visit.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;



// 栈帧信息结构体
struct StackFrame {
    int length; // 栈帧长度
    int used;   // 已经使用的栈帧长度
    bool saved_ra; // 当前正在访问的函数有没有保存ra
    // 类型为 koopa_raw_value 的有返回值的语句的存储位置
    unordered_map<koopa_raw_value_t, string> loc;
    StackFrame() : length(0), used(0), saved_ra(false) {}
};

static StackFrame stack_frame;



// 访问 raw program
void Visit(const koopa_raw_program_t &program) 
{
  // 执行一些其他的必要操作
  // ..
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

void Visit(const koopa_raw_function_t &func) 
{
    if(func->bbs.len == 0) return;

    // 输出函数头
    cout << "  .text\n"
         << "  .globl " << func->name + 1 << "\n" 
         << func->name + 1 << ":\n";

    // 初始化栈帧
    stack_frame = StackFrame();
    
    // 计算栈空间需求
   
    int locals = 0; // 局部变量空间
    bool need_ra = false;// 返回地址标记
    int max_args = 0; // 参数空间

    // 遍历统计
    for (size_t i = 0; i < func->bbs.len; ++i) 
    {
        const auto& bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        for (size_t j = 0; j < bb->insts.len; ++j) 
        {
            auto inst = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            locals += (inst->ty->tag != KOOPA_RTT_UNIT);
            
            if (inst->kind.tag == KOOPA_RVT_CALL) 
            {
                need_ra = true;
                max_args = max(max_args, int(inst->kind.data.call.args.len) - 8);
            }
        }
    }

    // 计算栈帧大小 (16字节对齐)
    int total_words = locals + need_ra + max(0, max_args);
    stack_frame.length = (total_words * 4 + 15) & ~15;
    stack_frame.used = max(0, max_args) * 4;
    stack_frame.saved_ra = need_ra;

    // 分配栈空间
    if (stack_frame.length > 0) 
    {
        cout << "  addi sp, sp, " << -stack_frame.length << "\n";
    }
    // 保存返回地址
    if (need_ra) 
    {
        cout << "  sw ra, " << stack_frame.length - 4 << "(sp)\n";
    }
    // 访问基本块
    Visit(func->bbs);
    cout << "\n";
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
      stack_frame.loc[value] = to_string(stack_frame.used);
      stack_frame.used += 4;
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
    case KOOPA_RVT_CALL:
      // 访问 call 指令
      Visit(kind.data.call, value);
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      // 访问 global alloc 指令
      Visit(value->kind.data.global_alloc, value);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
      break;
  }
}
// new

// 处理 integer 指令，加载整数常量到 a0 寄存器
void Visit(const koopa_raw_integer_t &integer) 
{
  cout << "  li a0, " << integer.value << endl;
}
// 访问 jump 指令
void Visit(const koopa_raw_jump_t &jump) 
{
  cout << "  j " << jump.target->name+1 << endl;
}

// 访问 return 指令
void Visit(const koopa_raw_return_t &ret) 
{
    // 处理返回值
    if(ret.value) 
    {
        if (ret.value->kind.tag == KOOPA_RVT_INTEGER) 
        {
            cout << "  li a0, " << ret.value->kind.data.integer.value;
        } 
        else 
        {
            auto offset = atoi(stack_frame.loc[ret.value].c_str());
            cout << "  lw a0, " << offset << "(sp)";
        }
        cout << "\n";
    }

    // 栈帧恢复操作
    if (stack_frame.length) 
    {
        if (stack_frame.saved_ra) 
        {
            cout << "  lw ra, " << (stack_frame.length - 4) << "(sp)\n";
        }
        cout << "  addi sp, sp, " << stack_frame.length;
    }
    cout << "\n  ret\n";
}

// 访问 global alloc 指令
void Visit(const koopa_raw_global_alloc_t &global_alloc, const koopa_raw_value_t &value) 
{
     // 输出全局变量声明
    cout << "  .data\n"
         << "  .globl " << value->name + 1 << "\n"
         << value->name + 1 << ":\n";
  switch (global_alloc.init->kind.tag) 
  {
    case KOOPA_RVT_ZERO_INIT:
      // 初始化为 0
     cout << "  .zero 4\n" ;
     break;
    case KOOPA_RVT_INTEGER:
      // 初始化为 int
     cout << "  .word " << global_alloc.init->kind.data.integer.value <<endl;
     break;
    default:
      // 处理其他可能的初始化类型
     cerr << "Unsupported initialization type\n";
     break;
  }

 cout <<endl;
}

//---

// 处理 load 指令，将源操作数加载到 t0 寄存器，并存储结果到栈中

void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value) 
{
  // 将源操作数加载到 t0 寄存器
  switch (load.src->kind.tag) 
  {
    case KOOPA_RVT_INTEGER:
      cout << "  li t0, " << load.src->kind.data.integer.value << endl;
      break;
    case KOOPA_RVT_FUNC_ARG_REF:
      {
        const auto& index = load.src->kind.data.func_arg_ref.index;
        cout << (index < 8 
                ? "  mv t0, a" + to_string(index) 
                : "  lw t0, " + to_string(stack_frame.length + (index - 8) * 4) + "(sp)") << "\n";
      }
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
       cout << "  la t6, " << load.src->name + 1 << endl
                 << "  lw t0, 0(t6)\n";
      break;
    default:
      cout << "  lw t0, " << stack_frame.loc[load.src] << "(sp)\n";
      break;
  }

    // 若有返回值则保存到栈里
    if (value->ty->tag != KOOPA_RTT_UNIT) 
    {
        stack_frame.loc[value] = to_string(stack_frame.used);
        stack_frame.used += 4;    
        cout << (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC
        ? "  la t6, " + string(value->name + 1) + "\n  sw t0, 0(t6)"
        : "  sw t0, " + stack_frame.loc[value] + "(sp)") << "\n";
    }
}

// 处理 store 指令，将源操作数存储到目标地址
void Visit(const koopa_raw_store_t &store) 
{
    // 源操作数加载到 t0
    switch (store.value->kind.tag) 
    {
        case KOOPA_RVT_INTEGER:
            cout << "  li t0, " << store.value->kind.data.integer.value << "\n";
            break;
            
        case KOOPA_RVT_FUNC_ARG_REF: 
        {
            const auto& index = store.value->kind.data.func_arg_ref.index;
            cout << (index < 8 
                          ? "  mv t0, a" + to_string(index)
                          : "  lw t0, " + to_string(stack_frame.length + (index - 8) * 4) + "(sp)") << "\n";
            
            break;
        }
            
        case KOOPA_RVT_GLOBAL_ALLOC:
            cout << "  la t6, " << store.value->name + 1 << "\n"
                 << "  lw t0, 0(t6)\n";
            break;
            
        default:
            cout << "  lw t0, " << stack_frame.loc[store.value] << "(sp)\n";
    }

    // 存储 t0 到目标地址
    cout << (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC
        ? "  la t6, " + string(store.dest->name + 1) + "\n  sw t0, 0(t6)"
        : "  sw t0, " + stack_frame.loc[store.dest] + "(sp)") << "\n";
}

// 处理 binary 指令，执行二元运算并将结果存储到栈中
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value) 
{
  // 将运算数存入 t0 和 t1
    // 处理 binary.lhs
    switch (binary.lhs->kind.tag) 
    {
        case KOOPA_RVT_INTEGER:
            cout << "  li t0, " << binary.lhs->kind.data.integer.value << "\n";
            break;
        case KOOPA_RVT_FUNC_ARG_REF: 
        {
            const auto& index = binary.lhs->kind.data.func_arg_ref.index;
            cout << (index < 8 
                ? "  mv t0, a" + to_string(index)
                : "  lw t0, " + to_string(stack_frame.length + (index - 8) * 4) + "(sp)") << "\n";
            break;
        }
        case KOOPA_RVT_GLOBAL_ALLOC:
            cout << "  la t6, " << binary.lhs->name + 1 << "\n  lw t0, 0(t6)\n";
            break;
        default:
            cout << "  lw t0, " << stack_frame.loc[binary.lhs] << "(sp)\n";
    }

    // 处理 binary.rhs
    switch (binary.rhs->kind.tag) 
    {
        case KOOPA_RVT_INTEGER:
            cout << "  li t1, " << binary.rhs->kind.data.integer.value << "\n";
            break;
        case KOOPA_RVT_FUNC_ARG_REF: 
        {
            const auto& index = binary.rhs->kind.data.func_arg_ref.index;
            cout << (index < 8 
                ? "  mv t1, a" + to_string(index)
                : "  lw t1, " + to_string(stack_frame.length + (index - 8) * 4) + "(sp)") << "\n";
            break;
        }
        case KOOPA_RVT_GLOBAL_ALLOC:
            cout << "  la t6, " << binary.rhs->name + 1 << "\n  lw t1, 0(t6)\n";
            break;
        default:
            cout << "  lw t1, " << stack_frame.loc[binary.rhs] << "(sp)\n";
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
  // 若有返回值则将 t0 中的结果存入栈
     // 存储结果
  if(value->ty->tag != KOOPA_RTT_UNIT) 
  {
      stack_frame.loc[value] = to_string(stack_frame.used);
      stack_frame.used += 4;
      cout << (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC
          ? "  la t6, " + string(value->name + 1) + "\n  sw t0, 0(t6)"
          : "  sw t0, " + stack_frame.loc[value] + "(sp)") << "\n";
  }
}

// 访问 br 指令
void Visit(const koopa_raw_branch_t &branch) 
{
    // 加载条件值到 t0
    switch (branch.cond->kind.tag) 
    {
        case KOOPA_RVT_INTEGER:
            cout << "  li t0, " << branch.cond->kind.data.integer.value << "\n";
            break;
            
        case KOOPA_RVT_FUNC_ARG_REF: 
        {
            const auto& index = branch.cond->kind.data.func_arg_ref.index;
            cout << (index < 8 
                ? "  mv t0, a" + to_string(index)
                : "  lw t0, " + to_string(stack_frame.length + (index - 8) * 4) + "(sp)") << "\n";
            break;
        }
            
        case KOOPA_RVT_GLOBAL_ALLOC:
            cout << "  la t6, " << branch.cond->name + 1 << "\n  lw t0, 0(t6)\n";
            break;
            
        default:
            cout << "  lw t0, " << stack_frame.loc[branch.cond] << "(sp)\n";
            break;
    }

    // 条件跳转
    cout << "  bnez t0, " << branch.true_bb->name + 1 
         << "\n  j " << branch.false_bb->name + 1 << "\n";
}

// 视需求自行实现
// ...


// 访问 call 指令
void Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value) 
{
  // 处理参数
  for (size_t i = 0; i < call.args.len; ++i) 
  {
    auto arg = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);
    string reg = (i < 8) ? "a" + to_string(i) : "t0";
    switch (arg->kind.tag) 
    {
      case KOOPA_RVT_INTEGER:
        cout << "  li " << reg << ", " << arg->kind.data.integer.value << endl;
        break;
      case KOOPA_RVT_FUNC_ARG_REF:
        {
          const auto& index = arg->kind.data.func_arg_ref.index;
          if (index < 8) 
          {
            cout << "  mv " << reg << ", a" << index << endl;
          } 
          else 
          {
            cout << "  li t6, " << stack_frame.length + (index - 8) * 4 << endl << "  add t6, t6, sp" << endl << "  lw " << reg << ", 0(t6)" << endl;
          }
        }
        break;
      case KOOPA_RVT_GLOBAL_ALLOC:
        cout << "  la t6, " << arg->name+1 << endl << "  lw " << reg << ", 0(t6)" << endl;
        break;
      default:
        cout << "  li t6, " << stack_frame.loc[arg] << endl << "  add t6, t6, sp" << endl << "  lw " << reg << ", 0(t6)" << endl;
        break;
    }
    if (i >= 8) 
    {
      cout << "  li t6, " << (i - 8) * 4 << endl << "  add t6, t6, sp" << endl << "  sw t0, 0(t6)" << endl;
    }
  }

  // call half
  cout << "  call " << call.callee->name+1 << endl;

  // 若有返回值则将 a0 中的结果存入栈
  if (value->ty->tag != KOOPA_RTT_UNIT) 
  {
    stack_frame.loc[value] = to_string(stack_frame.used);
    stack_frame.used += 4;
    if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) 
    {
      cout << "  la t6, " << value->name+1 << endl << "  sw a0, 0(t6)" << endl;
    } 
    else 
    {
      cout << "  li t6, " << stack_frame.loc[value] << endl << "  add t6, t6, sp" << endl << "  sw a0, 0(t6)" << endl;
    }
  }
}