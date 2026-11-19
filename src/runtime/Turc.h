#pragma once

// Este archivo también se compila en el ESP32: sin iostream, archivos ni
// excepciones. Trabaja sobre texto en memoria.

#include <string>
#include <vector>

#include "Program.h"

struct VariableInfo {
    std::string nombre;
    std::string tipo;                  // "int", "string" o "bool"
};

struct Compilado {
    std::string nombre;                // nombre del .tur
    std::vector<std::string> fuente;   // líneas del programa
    std::vector<VariableInfo> variables;
    Program programa;                  // cuádruplos sin optimizar
};

// Texto del archivo .turc: una línea por dato, campos separados por tabulador.
std::string escribirTurc(const Compilado& compilado);

// Carga un .turc. Si está dañado regresa false y un mensaje corto en error.
bool leerTurc(const std::string& texto, Compilado& compilado, std::string& error);
