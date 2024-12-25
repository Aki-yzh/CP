#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>

using std::string;
using std::shared_ptr;
using std::unordered_map;
using std::vector;
using std::pair;
using std::optional;

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
    // 作用域计数, 已经使用了多少个符号表
    inline int symbol_table_cnt = 0;

    // 作用域栈, 内容物为 pair<符号表标号, 符号表>
    inline vector<pair<int, symbol_table_t>> symbol_table_stack;

    // 进入新的作用域
    inline void enter_code_block()
    {
        symbol_table_stack.emplace_back(symbol_table_cnt++, symbol_table_t{});
    }

    // 离开当前作用域
    inline void exit_code_block()
    {
        symbol_table_stack.pop_back();
    }

    // 返回当前作用域的标号, 格式形如 "SYM_TABLE_233"
    inline string current_code_block()
    {
        return "SYM_TABLE_" + std::to_string(symbol_table_stack.back().first) + "_";
    }

    // 在符号表中寻找符号, 返回其所在符号表的 标号 和其本身的 iterator
    inline optional<pair<int, symbol_table_t::iterator>> find_iter(const string &symbol)
    {
        for(auto rit = symbol_table_stack.rbegin(); rit != symbol_table_stack.rend(); ++rit)
        {
            auto it = rit->second.find(symbol);
            if(it != rit->second.end())
            {
                return std::make_pair(rit->first, it);
            }
        }
        // 没找到
        return std::nullopt;
    }

    // 插入符号定义
    inline void insert_symbol(const string &symbol, symbol_type type, int value)
    {
        auto symval = std::make_shared<symbol_value>(symbol_value{ type, value });
        symbol_table_stack.back().second[symbol] = symval;
    }

    // 确认符号定义是否存在, 若存在返回true, 否则返回false
    inline bool exist_symbol(const string &symbol)
    {
        return find_iter(symbol).has_value();
    }

    // 查询符号定义, 返回该符号所在符号表表号和指向这个符号的值的指针.
    // 若符号不存在, 返回的表号为-1, symbol_type为UND
    inline pair<string, shared_ptr<const symbol_value>> query_symbol(const string &symbol)
    {
        auto result = find_iter(symbol);
        if(result)
        {
            return { "SYM_TABLE_" + std::to_string(result->first) + "_", result->second->second };
        }
        else
        {
            auto symval = std::make_shared<symbol_value>(symbol_value{ SYM_TYPE_UND, -1 });
            return { "SYM_TABLE_-1_", symval };
        }
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

inline string current_code_block()
{
    return SymbolTableNamespace::current_code_block();
}

inline void insert_symbol(const string &symbol, symbol_type type, int value)
{
    SymbolTableNamespace::insert_symbol(symbol, type, value);
}

inline bool exist_symbol(const string &symbol)
{
    return SymbolTableNamespace::exist_symbol(symbol);
}

inline pair<string, shared_ptr<const symbol_value>> query_symbol(const string &symbol)
{
    return SymbolTableNamespace::query_symbol(symbol);
}