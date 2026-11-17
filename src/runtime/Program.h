#pragma once

// Este archivo también se compila en el ESP32: sin iostream ni excepciones.

#include <string>
#include <vector>

#include "runtime/Quad.h"

// Cada bloque se numera desde 1, tiene sus propios temporales (desde T1) y sus
// saltos apuntan a renglones del mismo bloque.
struct Bloque {
    std::string nombre;          // nombre de la máquina; vacío en la sección de control
    std::vector<Quad> quads;
};

struct Program {
    std::vector<Bloque> maquinas;    // una función por máquina, en el orden del fuente
    Bloque principal;                // la sección de control
};
