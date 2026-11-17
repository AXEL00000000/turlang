#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/Tipos.h"

enum class ClaseExpr {
    Num, Str, Bool, Var, Binaria,
    Run, Output, Repeat, Input, ConvInt, ConvString
};

struct Expr {
    ClaseExpr clase;
    int linea = 0;
    int columna = 0;
    std::string texto;
    std::vector<std::unique_ptr<Expr>> hijos;
    TipoDato tipo = TipoDato::Desconocido;   // lo llena el análisis semántico (etapa 3)
    bool implicita = false;   // conversión que agregó el traductor, no el programa
};

enum class ClaseStmt { Decl, Asignacion, If, While, Print };

struct Stmt {
    ClaseStmt clase;
    int linea = 0;            // primer token de la sentencia
    int columna = 0;
    TipoDato tipoDecl = TipoDato::Desconocido;      // solo Decl
    std::string nombre;                             // Decl y Asignacion
    int lineaNombre = 0;                            // posición del identificador
    int columnaNombre = 0;
    std::unique_ptr<Expr> expr;                     // valor, condición o argumento de print
    std::vector<std::unique_ptr<Stmt>> cuerpo;      // bloque de if o de while
    std::vector<std::unique_ptr<Stmt>> cuerpoElse;  // bloque de else
    bool tieneElse = false;
};

struct Regla {
    std::string lee;          // "1", "X", "blank" o "*"
    std::string escribe;      // igual
    char movimiento = 'S';    // 'R', 'L' o 'S'
    std::string destino;
    int linea = 0;            // posición del símbolo leído
    int columna = 0;
    int lineaDestino = 0;     // posición del estado destino
    int columnaDestino = 0;
};

struct Estado {
    std::string nombre;
    int linea = 0;            // posición del nombre
    int columna = 0;
    std::vector<Regla> reglas;
};

struct Maquina {
    std::string nombre;
    int linea = 0;            // posición del nombre
    int columna = 0;
    std::string inicial;
    int lineaInicial = 0;
    int columnaInicial = 0;
    std::string aceptacion;
    int lineaAceptacion = 0;
    int columnaAceptacion = 0;
    std::vector<Estado> estados;
};

struct Programa {
    std::vector<Maquina> maquinas;
    std::vector<std::unique_ptr<Stmt>> sentencias;
};
