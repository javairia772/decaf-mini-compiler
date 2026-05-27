CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = decaf_compiler.exe
SRCS     = src/main.cpp src/token.cpp src/lexer.cpp       \
           src/error_handler.cpp src/symbol_table.cpp     \
           src/rd_parser.cpp src/ll1_parser.cpp           \
           src/lr_parser.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET) test/test1.decaf

clean:
	del $(TARGET)