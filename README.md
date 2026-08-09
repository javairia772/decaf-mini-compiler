# Decaf Compiler

A full compiler front-end for the **Decaf programming language**, built from scratch in C++. Implements all classical compiler construction phases with a live web interface for visualization.

**Live Demo → [javairia772.github.io/decaf-mini-compiler](https://javairia772.github.io/decaf-mini-compiler)**

---

## Overview

This project implements a complete compiler front-end pipeline — from raw source text to fully parsed and semantically analyzed output. Every phase is built manually without using parser generators like YACC or Bison.

```
Source Code → Lexer → RD Parser → LL(1) Parser → LALR(1) Parser → Symbol Table
```

The web interface lets anyone paste Decaf code and watch each compiler phase run in real time.

---

## Modules

| Module | Algorithm | Details |
|--------|-----------|---------|
| Lexer | Character-by-character scan | Handles keywords, identifiers, operators, strings, comments, line/col tracking |
| RD Parser | Recursive descent | One function per grammar rule, full precedence climbing for expressions |
| LL(1) Parser | Table-driven predictive | FIRST/FOLLOW sets computed algorithmically, explicit stack, parse trace |
| LALR(1) Parser | LR(0) automaton + SLR lookaheads | 133 states, 1500+ ACTION entries, shift-reduce with full trace |
| Symbol Table | Scoped hash map | Nested scopes, O(1) lookup, redeclaration and undeclared variable detection |
| Error Handler | Panic-mode recovery | Lexical, syntactic, semantic errors with line/col, compilation continues after errors |

---

## Tech Stack

**Compiler core** — C++17, built with g++  
**API layer** — Python Flask, deployed on Render  
**Frontend** — Vanilla HTML/CSS/JavaScript, single file, no framework  
**Hosting** — GitHub Pages (frontend) + Render (backend)

---

## Live Demo

Open the web interface, paste any Decaf code, and click Run:

- Token stream shown as color-coded chips by token type
- LL(1) parse trace with every stack operation visible
- LALR(1) shift-reduce trace with state stack at each step
- FIRST and FOLLOW sets displayed as a searchable card grid
- Symbol table with scope levels and entry kinds
- Error report with exact line and column numbers

**[Try it live →](https://javairia772.github.io/decaf-mini-compiler)**

---

## Build and Run Locally

**Requirements:** g++ with C++17 support, Python 3.8+

**Compile:**

```bash
g++ -std=c++17 -O2 -o decaf_compiler \
  src/main.cpp src/token.cpp src/lexer.cpp \
  src/error_handler.cpp src/symbol_table.cpp \
  src/rd_parser.cpp src/ll1_parser.cpp src/lr_parser.cpp
```

**Run on a file:**

```bash
./decaf_compiler test/test1.decaf
```

**Start the API:**

```bash
cd api
pip install flask flask-cors
export COMPILER_PATH=../decaf_compiler
python app.py
```

**Open the UI:**  
Open `docs/index.html` in your browser. Change `const API` to `http://localhost:5000`.

---

## Sample Input

```decaf
class Main {
    int x;
    void main(int argc) {
        int a;
        a = 5 + 3;
        if (a < 10) {
            return a;
        }
        while (a > 0) {
            a = a - 1;
        }
    }
}
```

**Output:** 48 tokens, RD / LL(1) / LALR all PASS, symbol table with 5 entries, 0 errors.

---

## Project Structure

```
src/              C++ source — all compiler modules
api/              Flask API wrapping the compiler binary
docs/             Web UI (index.html) served by GitHub Pages
test/             Sample Decaf programs including error cases
Dockerfile        Container config for Render deployment
```

---

## Grammar

Decaf supports classes, interfaces, inheritance, methods, arrays,
all standard control flow (if/else, while, for, break, return),
expressions with full operator precedence, and Print statements.

The LL(1) parser uses a left-recursion-free left-factored grammar.  
The LALR(1) parser uses the original left-recursive grammar directly.  
Full BNF, FIRST/FOLLOW sets, and parsing tables are in the
[project report](docs/report.md).

---

## What I Learned

Building this project required understanding compiler theory at
implementation depth — not just reading about LR items but actually
computing closure and GOTO operations, propagating lookaheads,
resolving shift-reduce conflicts, and debugging automaton states when
the table produced wrong actions. The gap between textbook description
and working code is significant and this project closed that gap.

---

## Reference

Aho, Lam, Sethi, Ullman — *Compilers: Principles, Techniques, and Tools* (Dragon Book)
