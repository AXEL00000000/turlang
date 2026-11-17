#pragma once

// Este archivo también se compila en el ESP32: sin iostream ni excepciones.
// Lo comparten Interpreter y TuringSimulator.

#include <cstdint>
#include <map>
#include <string>

#include "runtime/Quad.h"

// Largo máximo de una cadena y de la cinta.
const int LIMITE_TEXTO = 2000;

struct Valor {
    enum Tipo { Entero, Cadena, Bool, Simbolo } tipo = Entero;
    int32_t entero = 0;        // Entero; Bool (0 o 1); Simbolo (código del carácter)
    std::string cadena;        // Cadena
};

Valor valorEntero(int32_t n);
Valor valorCadena(const std::string& texto);
Valor valorBool(bool b);
Valor valorSimbolo(char c);

// Como lo muestra print: Entero → dígitos; Cadena → el texto; Bool → true/false.
std::string aTexto(const Valor& valor);

// Renglón (desde 1) de un operando Salto.
int renglonSalto(const Operando& salto);

// Variables y temporales de un bloque, por nombre.
class Memoria {
public:
    void guardar(const std::string& nombre, const Valor& valor);
    // Valor de un operando: literal, variable o temporal. Con error regresa false.
    bool valorDe(const Operando& o, Valor& valor, std::string& error) const;
private:
    std::map<std::string, Valor> valores;
};

// Cuádruplos que solo calculan un valor: =, + - * /, relacionales, int y string.
bool esCalculo(Op op);

// Calcula el valor de un cuádruplo de cálculo (sin guardarlo). Con error regresa false.
bool calcular(const Quad& q, const Memoria& memoria, Valor& resultado, std::string& error);
