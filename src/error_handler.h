#pragma once
#include <string>
#include <vector>

enum class ErrorType { LEXICAL, SYNTACTIC, SEMANTIC };

struct CompilerError {
    ErrorType type;
    int line, col;
    std::string message;
};

class ErrorHandler {
public:
    void reportError(ErrorType type, int line, int col, const std::string& msg);
    void printSummary() const;
    bool hasErrors() const { return !errors.empty(); }
    int errorCount() const { return (int)errors.size(); }

private:
    std::vector<CompilerError> errors;
    std::string typeLabel(ErrorType t) const;
};