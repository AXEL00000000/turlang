#pragma once

#include <string>
#include <vector>

#include "common/ErrorHandler.h"
#include "common/Token.h"

class Lexer {
public:
    Lexer(const std::string& fuente, ErrorHandler& errores);
    std::vector<Token> analizar();
private:
    const std::string& fuente;
    ErrorHandler& errores;
    size_t pos = 0;
    int linea = 1;
    int columna = 1;

    bool alFinal() const;
    unsigned char actual() const;
    unsigned char siguiente() const;
    void avanzar();
    void saltarComentario();
    Token leerIdentificador();
    Token leerNumero();
    // Regresa false si la cadena tuvo un error y no genera token.
    bool leerCadena(Token& token);
    // Regresa false si el carácter no inicia ningún operador o signo.
    bool leerOperador(Token& token);
    void reportarNoReconocido();
};
