#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorHandler.h"
#include "common/Token.h"
#include "frontend/Ast.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens, ErrorHandler& errores);
    Programa analizar();
private:
    const std::vector<Token>& tokens;
    ErrorHandler& errores;
    size_t pos = 0;

    // Auxiliares
    const Token& actual() const;
    bool es(TipoToken tipo) const;
    void avanzar();
    bool coincide(TipoToken tipo);
    const Token& esperar(TipoToken tipo, const std::string& descripcion);
    void reportarEsperado(const std::string& descripcion);
    [[noreturn]] void errorEsperado(const std::string& descripcion);
    std::string describir(const Token& token) const;

    // Recuperación de errores
    void sincronizarEncabezado(size_t inicio);
    void sincronizarRegla(size_t inicio);
    void sincronizarSentencia(size_t inicio);

    // Una función por no terminal
    Programa programa();
    bool maquina(Maquina& maquina);
    bool estado(Estado& estado);
    void regla(Regla& regla);
    std::string simbolo();
    char movimiento();
    std::vector<std::unique_ptr<Stmt>> bloque();
    std::unique_ptr<Stmt> sentenciaRecuperable();
    std::unique_ptr<Stmt> sentencia();
    std::unique_ptr<Expr> condicion();
    std::unique_ptr<Expr> expresion();
    std::unique_ptr<Expr> termino();
    std::unique_ptr<Expr> factor();
};
