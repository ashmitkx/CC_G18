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
