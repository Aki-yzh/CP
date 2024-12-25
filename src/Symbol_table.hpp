#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <stack>
#include <sstream>

using std::string;
using std::shared_ptr;
using std::unordered_map;

// 符号表中符号的类型
enum symbol_type
{
    SYM_TYPE_CONST, // 常量
    SYM_TYPE_VAR,   // 变量
    SYM_TYPE_UND    // 符号不存在
};

// 符号表中符号的值
struct symbol_value
{
    symbol_type type; // 符号的类型
    int value;        // 符号的值
};

// 符号表类型
using symbol_table_t = unordered_map<string, shared_ptr<symbol_value>>;

namespace SymbolTableNamespace
{
    // 作用域栈
    inline std::stack<symbol_table_t> scope_stack;
    inline int scope_counter = 0;

    // 进入新的作用域
    inline void enter_code_block()
    {
        scope_stack.push(symbol_table_t());
        scope_counter++;
    }

    // 离开当前作用域
    inline void exit_code_block()
    {
        if (!scope_stack.empty())
        {
            scope_stack.pop();
        }
    }

    // 返回当前符号表的表号(作用域号), 格式形如 "SYM_TABLE_233_"
    inline std::string current_code_block()
    {
        std::ostringstream oss;
        oss << "SYM_TABLE_" << scope_counter << "_";
        return oss.str();
    }

    // 插入符号定义
    inline void insert_symbol(const string &symbol, symbol_type type, int value)
    {
        if (!scope_stack.empty())
        {
            auto &current_scope = scope_stack.top();
            auto symval = std::make_shared<symbol_value>(symbol_value{ type, value });
            current_scope[symbol] = symval;
        }
    }

    // 确认符号定义是否存在, 若存在返回1, 否则返回0
    inline int exist_symbol(const string &symbol)
    {
        std::stack<symbol_table_t> temp_stack = scope_stack;
        while (!temp_stack.empty())
        {
            auto &current_scope = temp_stack.top();
            if (current_scope.find(symbol) != current_scope.end())
            {
                return 1;
            }
            temp_stack.pop();
        }
        return 0;
    }

    // 查询符号定义, 返回该符号所在符号表表号和指向这个符号的值的指针.
    // 符号表表号格式形如 "SYM_TABLE_233_"
    // 若符号不存在, 返回的表号为-1, symbol_type为UND
    inline std::pair<std::string, std::shared_ptr<const symbol_value>> query_symbol(const string &symbol)
    {
        std::stack<symbol_table_t> temp_stack = scope_stack;
        int temp_counter = scope_counter;
        while (!temp_stack.empty())
        {
            auto &current_scope = temp_stack.top();
            auto it = current_scope.find(symbol);
            if (it != current_scope.end())
            {
                std::ostringstream oss;
                oss << "SYM_TABLE_" << temp_counter << "_";
                return {oss.str(), it->second};
            }
            temp_stack.pop();
            temp_counter--;
        }
        return {"-1", std::make_shared<symbol_value>(symbol_value{SYM_TYPE_UND, 0})};
    }
}

// 全局函数接口
inline void enter_code_block()
{
    SymbolTableNamespace::enter_code_block();
}

inline void exit_code_block()
{
    SymbolTableNamespace::exit_code_block();
}

inline std::string current_code_block()
{
    return SymbolTableNamespace::current_code_block();
}

inline void insert_symbol(const string &symbol, symbol_type type, int value)
{
    SymbolTableNamespace::insert_symbol(symbol, type, value);
}

inline int exist_symbol(const string &symbol)
{
    return SymbolTableNamespace::exist_symbol(symbol);
}

inline std::pair<std::string, std::shared_ptr<const symbol_value>> query_symbol(const string &symbol)
{
    return SymbolTableNamespace::query_symbol(symbol);
}