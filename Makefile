CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc
SRCS     = src/main.cpp \
           src/common/ErrorHandler.cpp \
           src/common/SymbolTable.cpp \
           src/common/Tabla.cpp \
           src/common/ConsoleIO.cpp \
           src/common/IOSilenciosa.cpp \
           src/frontend/Lexer.cpp \
           src/frontend/Parser.cpp \
           src/frontend/AstPrinter.cpp \
           src/frontend/SemanticAnalyzer.cpp \
           src/ir/IntermediateCodeGenerator.cpp \
           src/ir/Triples.cpp \
           src/ir/IrPrinter.cpp \
           src/ir/Costos.cpp \
           src/ir/Optimizer.cpp \
           src/runtime/Valor.cpp \
           src/runtime/TuringSimulator.cpp \
           src/runtime/Interpreter.cpp \
           src/runtime/Turc.cpp \
           src/backend/CodeGenerator.cpp

ifeq ($(OS),Windows_NT)
  CXXFLAGS += -static
  POWERSHELL = powershell
else
  POWERSHELL = pwsh
endif

.PHONY: turlang test paquete
turlang:
	$(CXX) $(CXXFLAGS) $(SRCS) -o turlang

test: turlang
	$(POWERSHELL) -NoProfile -ExecutionPolicy Bypass -File scripts/probar.ps1

paquete: turlang
	$(POWERSHELL) -NoProfile -ExecutionPolicy Bypass -File scripts/paquete.ps1
