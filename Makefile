CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc
SRCS     = src/main.cpp \
           src/common/ErrorHandler.cpp \
           src/frontend/Lexer.cpp \
           src/frontend/Parser.cpp \
           src/frontend/AstPrinter.cpp

ifeq ($(OS),Windows_NT)
  CXXFLAGS += -static
endif

.PHONY: turlang
turlang:
	$(CXX) $(CXXFLAGS) $(SRCS) -o turlang
