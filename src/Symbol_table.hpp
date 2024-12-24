#pragma once
#include <string>
#include <memory>
#include <unordered_map>

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
    // 符号表
    inline symbol_table_t symbol_table;

    // 在符号表中寻找符号，返回其iterator
    inline symbol_table_t::iterator find_iter(const string &symbol)
    {
        return symbol_table.find(symbol);
    }

    // 插入符号定义, 若成功插入返回0, 否则返回-1
    inline int insert_symbol(const string &symbol, symbol_type type, int value)
    {
        auto it = find_iter(symbol);

        // 已经存在的符号
        if(it != symbol_table.end())
            return -1;

        // 插入该符号
        auto symval = std::make_shared<symbol_value>();
        symval->type = type;
        symval->value = value;
        symbol_table[symbol] = symval;
        return 0;
    }

    // 确认符号定义是否存在, 若存在返回1, 否则返回0
    inline int exist_symbol(const string &symbol)
    {
        return (find_iter(symbol) != symbol_table.end()) ? 1 : 0;
    }

    // 查询符号定义, 返回指向这个符号的值的指针. 若符号不存在，返回nullptr
    inline shared_ptr<const symbol_value> query_symbol(const string &symbol)
    {
        auto it = find_iter(symbol);

        // 若符号不存在
        if(it == symbol_table.end())
        {
            return nullptr;
        }

        // 符号存在，返回其value
        return it->second;
    }
}

// 全局函数接口
inline int insert_symbol(const string &symbol, symbol_type type, int value)
{
    return SymbolTableNamespace::insert_symbol(symbol, type, value);
}

inline int exist_symbol(const string &symbol)
{
    return SymbolTableNamespace::exist_symbol(symbol);
}

inline shared_ptr<const symbol_value> query_symbol(const string &symbol)
{
    return SymbolTableNamespace::query_symbol(symbol);
}