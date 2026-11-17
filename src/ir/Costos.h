#pragma once

#include <ostream>

#include "runtime/Interpreter.h"
#include "runtime/Program.h"

// Una línea vacía y la tabla Medida / Valor: la sección de control y cada máquina.
void imprimirCostos(const Program& programa, const Estadisticas& est, std::ostream& salida);
