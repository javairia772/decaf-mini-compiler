FROM python:3.11-slim

# Install g++
RUN apt-get update && apt-get install -y g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all source files
COPY . .

# Compile C++ compiler
RUN g++ -std=c++17 -O2 -o decaf_compiler \
    src/main.cpp \
    src/token.cpp \
    src/lexer.cpp \
    src/error_handler.cpp \
    src/symbol_table.cpp \
    src/rd_parser.cpp \
    src/ll1_parser.cpp \
    src/lr_parser.cpp

# Install Python deps
RUN pip install flask flask-cors

# Environment
ENV COMPILER_PATH=/app/decaf_compiler

EXPOSE 5000

CMD ["python", "api/app.py"]
