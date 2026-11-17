#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorHandler.h"
#include "common/SymbolTable.h"
#include "frontend/Ast.h"

class SemanticAnalyzer {
public:
    SemanticAnalyzer(Programa& programa, SymbolTable& tabla, ErrorHandler& errores);
    void analizar();
private:
    Programa& programa;
    SymbolTable& tabla;
    ErrorHandler& errores;

    void reportar(int linea, int columna, const std::string& mensaje);

    void analizarMaquina(const Maquina& maquina);
    void analizarSentencias(std::vector<std::unique_ptr<Stmt>>& sentencias);
    void analizarSentencia(Stmt& stmt);
    void analizarDecl(Stmt& stmt);
    void analizarAsignacion(Stmt& stmt);
    void analizarCondicion(Stmt& stmt);
    // Analiza el valor de una Decl o Asignación, con el caso especial de input().
    TipoDato analizarValor(std::unique_ptr<Expr>& expr, TipoDato tipoDestino);
    // Puede reemplazar el nodo (conversiones implícitas); regresa su tipo.
    TipoDato analizarExpr(std::unique_ptr<Expr>& expr);
    TipoDato analizarBinaria(std::unique_ptr<Expr>& expr);
    TipoDato analizarLlamada(Expr& expr);
};
