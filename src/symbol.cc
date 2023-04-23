#include "symbol.hh"
#include <iostream>

bool SymbolTable::contains(std::string key) {
    // return table.find(key) != table.end();

    // iterate through pairs in table and check if key is in the second element
    for (auto it = table.begin(); it != table.end(); it++)
        if (it->first == key)
            return true;
    return false;
}

void SymbolTable::insert(std::string key, std::string type) {
    table.insert(std::make_pair(key, type));
}

std::string SymbolTable::get_type(std::string key) {
    for (auto it = table.begin(); it != table.end(); it++)
        if (it->first == key)
            return it->second;
    return "";
}

SymbolTableVector::SymbolTableVector()
{
    this->create_scope();
}

std::string SymbolTableVector::get_type(std::string key)
{
    for (int i = vectorST.size() - 1; i >= 0; i--)
    {
        auto st = vectorST[i];
        auto type = st.get_type(key);

        if(type != "")
            return type;
    }

    return "";
}
void SymbolTableVector::create_scope() {
    SymbolTable st;
    vectorST.push_back(st);
}

void SymbolTableVector::insert(std::string key, std::string type) {
    vectorST[vectorST.size() - 1].insert(key, type);
}

void SymbolTableVector::destroy_scope() {
    vectorST.pop_back();
}

bool SymbolTableVector::contains(std::string key, bool redefine) {
    for (int i = vectorST.size() - 1; i >= 0; i--)
    {
        auto st = vectorST[i];

        if (redefine)
        {
            if (st.contains(key) && i == vectorST.size() - 1)
                return true;
        }
        else
        {
            if (st.contains(key))
                return true;
        }
    }

    return false;
}

SymbolTableStack::SymbolTableStack()
{
    this->create_context();
}

std::string SymbolTableStack::get_type(std::string key)
{
    return stackST[stackST.size() - 1].get_type(key);
}

void SymbolTableStack::create_context() {
    SymbolTableVector st;
    stackST.push_back(st);
}

void SymbolTableStack::create_scope() {
    stackST[stackST.size() - 1].create_scope();
}

void SymbolTableStack::insert(std::string key, std::string type) {
    stackST[stackST.size() - 1].insert(key, type);
}

void SymbolTableStack::destroy_context() {
    stackST.pop_back();
}

void SymbolTableStack::destroy_scope() {
    stackST[stackST.size() - 1].destroy_scope();
}

bool SymbolTableStack::contains(std::string key, bool redefine) {
    return stackST[stackST.size() - 1].contains(key, redefine);
}