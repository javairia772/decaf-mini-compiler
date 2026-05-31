#include "symbol_table.h"
#include <iomanip>

// ---- ScopeTable ----

bool ScopeTable::insert(const SymbolEntry& entry) {
    if (table.count(entry.name)) return false;
    table[entry.name] = entry;
    return true;
}

SymbolEntry* ScopeTable::lookup(const std::string& name) {
    auto it = table.find(name);
    if (it != table.end()) return &it->second;
    return nullptr;
}

static std::string kindStr(SymbolKind k) {
    switch (k) {
        case SymbolKind::VARIABLE:  return "variable";
        case SymbolKind::CONSTANT:  return "constant";
        case SymbolKind::FUNCTION:  return "function";
        case SymbolKind::ARRAY:     return "array";
        case SymbolKind::CLASS:     return "class";
        case SymbolKind::PARAMETER: return "parameter";
        default: return "?";
    }
}

void ScopeTable::print() const {
    std::cout << "  --- Scope Level " << level << " ---\n";
    std::cout << "  " << std::left
              << std::setw(16) << "NAME"
              << std::setw(12) << "KIND"
              << std::setw(12) << "TYPE"
              << std::setw(8)  << "LINE"
              << "EXTRA\n";
    std::cout << "  " << std::string(56, '-') << "\n";
    for (const auto& [name, e] : table) {
        std::cout << "  " << std::left
                  << std::setw(16) << e.name
                  << std::setw(12) << kindStr(e.kind)
                  << std::setw(12) << e.type
                  << std::setw(8)  << e.line
                  << e.extra << "\n";
    }
}

// ---- SymbolTableManager ----

SymbolTableManager::SymbolTableManager(ErrorHandler& eh) : errHandler(eh) {
    scopeStack.emplace_back(0); // global scope
}

void SymbolTableManager::enterScope() {
    int newLevel = (int)scopeStack.size();
    scopeStack.emplace_back(newLevel);
}

void SymbolTableManager::exitScope() {
    if (scopeStack.size() > 1)
        scopeStack.pop_back();
}

bool SymbolTableManager::declare(const SymbolEntry& entry) {
    auto& currentScope = scopeStack.back();
    if (!currentScope.insert(entry)) {
        errHandler.reportError(ErrorType::SEMANTIC, entry.line, 0,
            "Redeclaration of '" + entry.name + "' in same scope");
        return false;
    }
    return true;
}

SymbolEntry* SymbolTableManager::resolve(const std::string& name) {
    for (int i = (int)scopeStack.size() - 1; i >= 0; i--) {
        SymbolEntry* e = scopeStack[i].lookup(name);
        if (e) return e;
    }
    return nullptr;
}

void SymbolTableManager::printAll() const {
    std::cout << "\n========== SYMBOL TABLE ==========\n";
    for (const auto& scope : scopeStack)
        scope.print();
    std::cout << "==================================\n";
}