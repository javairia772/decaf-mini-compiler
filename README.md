# Decaf Mini Compiler — CS-471L Spring 2026
**UET Lahore | Group: [Your Name] | Roll No: [Your Roll No]**

## Build
```bash
make
```

## Run
```bash
./decaf_compiler test/test1.decaf
```

## Modules
1. **Lexer** — tokenizes source with double-buffer style, line/col tracking
2. **RD Parser** — recursive descent, calls symbol table on every declaration/use
3. **LL(1) Parser** — table-driven, prints FIRST/FOLLOW sets and parse table
4. **LR Parser** — SLR(1), prints action/goto tables and full parse trace
5. **Symbol Table** — hash-based, nested scopes, insert/lookup/delete
6. **Error Handler (Bonus)** — panic-mode recovery, line/col messages, end summary

## Grammar
Decaf Language (full subset with classes, methods, arrays, control flow)