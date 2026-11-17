#pragma once

#include <ostream>

#include "runtime/Interpreter.h"
#include "runtime/Program.h"

// Una línea vacía y la tabla Medida / Antes / Después: la sección de control y
// cada máquina, del programa original (antes) y del optimizado (después).
void imprimirCostos(const Program& antes, const Estadisticas& estAntes,
                    const Program& despues, const Estadisticas& estDespues,
                    std::ostream& salida);
