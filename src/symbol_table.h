#pragma once
#include "error_handler.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

enum class SymbolKind { VARIABLE, CONSTANT, FUNCTION, ARRAY, CLASS, PARAMETER };

struct SymbolEntry {
    std::string name;
    SymbolKind  kind;
    std::string type;
    int         scopeLevel;
    int         line;
    std::string extra; // e.g. array size, return type hint
};

class ScopeTable {
public:
    explicit ScopeTable(int level) : level(level) {}
    bool insert(const SymbolEntry& entry);
    SymbolEntry* lookup(const std::string& name);
    void print() const;
    int getLevel() const { return level; }

private:
    int level;
    std::unordered_map<std::string, SymbolEntry> table;
};

class SymbolTableManager {
public:
    explicit SymbolTableManager(ErrorHandler& eh);
    void enterScope();
    void exitScope();
    bool declare(const SymbolEntry& entry);
    SymbolEntry* resolve(const std::string& name);
    void printAll() const;
    int currentLevel() const { return (int)scopeStack.size() - 1; }

private:
    std::vector<ScopeTable> scopeStack;
    ErrorHandler& errHandler;
};