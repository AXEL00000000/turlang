CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc
SRCS     = src/main.cpp \
           src/common/ErrorHandler.cpp \
           src/common/SymbolTable.cpp \
           src/frontend/Lexer.cpp \
           src/frontend/Parser.cpp \
           src/frontend/AstPrinter.cpp \
           src/frontend/SemanticAnalyzer.cpp \
           src/ir/IntermediateCodeGenerator.cpp \
           src/ir/Triples.cpp \
           src/ir/IrPrinter.cpp

ifeq ($(OS),Windows_NT)
  CXXFLAGS += -static
endif

.PHONY: turlang
turlang:
	$(CXX) $(CXXFLAGS) $(SRCS) -o turlang
