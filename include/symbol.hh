#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <set>
#include <string>
#include "ast.hh"


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::set<std::pair<std::string, std::string>> table;

    bool contains(std::string key);
    void insert(std::string key, std::string type);
    std::string get_type(std::string key);
};

#endif
