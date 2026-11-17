#pragma once

#include <ostream>

#include "frontend/Ast.h"

// Un nodo por línea, con 2 espacios de sangría por nivel.
void imprimirArbol(const Programa& programa, std::ostream& salida);
