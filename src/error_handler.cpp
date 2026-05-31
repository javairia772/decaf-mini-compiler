#include "error_handler.h"
#include <iostream>
#include <iomanip>

void ErrorHandler::reportError(ErrorType type, int line, int col, const std::string& msg) {
    errors.push_back({type, line, col, msg});
    std::cerr << "[" << typeLabel(type) << " ERROR] "
              << "Line " << line << ", Col " << col << ": " << msg << "\n";
}

void ErrorHandler::printSummary() const {
    std::cout << "\n========== ERROR SUMMARY ==========\n";
    if (errors.empty()) {
        std::cout << "  No errors detected. Compilation successful.\n";
    } else {
        std::cout << "  Total errors: " << errors.size() << "\n\n";
        for (const auto& e : errors) {
            std::cout << "  [" << typeLabel(e.type) << "] "
                      << "Line " << e.line << ", Col " << e.col
                      << " => " << e.message << "\n";
        }
    }
    std::cout << "====================================\n";
}

std::string ErrorHandler::typeLabel(ErrorType t) const {
    switch(t) {
        case ErrorType::LEXICAL:    return "LEXICAL";
        case ErrorType::SYNTACTIC:  return "SYNTACTIC";
        case ErrorType::SEMANTIC:   return "SEMANTIC";
        default:                    return "UNKNOWN";
    }
}