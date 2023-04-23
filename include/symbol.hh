#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <set>
#include <string>
#include <vector>
#include "ast.hh"


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::set<std::pair<std::string, std::string>> table;

    bool contains(std::string key);
    void insert(std::string key, std::string type);
    std::string get_type(std::string key);
};

struct SymbolTableVector
{
    SymbolTableVector();
    std::vector <SymbolTable> vectorST;
    void create_scope();
    bool contains(std::string key, bool redefine);
    void insert(std::string key, std::string type);
    std::string get_type(std::string key);
    void destroy_scope();
};

struct SymbolTableStack
{
    SymbolTableStack();
    std::vector <SymbolTableVector> stackST;
    void create_scope();
    void create_context();
    bool contains(std::string key, bool redefine);
    void insert(std::string key, std::string type);
    std::string get_type(std::string key);
    void destroy_scope();
    void destroy_context();
};

#endif
