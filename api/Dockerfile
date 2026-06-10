# Base image with C++ and Python
FROM python:3.11-slim

# Install g++
RUN apt-get update && apt-get install -y \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy everything
COPY . .

# Compile the C++ compiler
RUN g++ -std=c++17 -Wall -O2 -o decaf_compiler \
    src/main.cpp \
    src/token.cpp \
    src/lexer.cpp \
    src/error_handler.cpp \
    src/symbol_table.cpp \
    src/rd_parser.cpp \
    src/ll1_parser.cpp \
    src/lr_parser.cpp

# Copy compiler binary to api folder
RUN cp decaf_compiler api/decaf_compiler

# Install Python dependencies
RUN pip install -r api/requirements.txt

# Set compiler path
ENV COMPILER_PATH=/app/api/decaf_compiler

# Expose port
EXPOSE 5000

# Start Flask
CMD ["python", "api/app.py"]