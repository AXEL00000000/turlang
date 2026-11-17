#pragma once

#include <ostream>

#include "ir/Optimizer.h"
#include "runtime/Program.h"

// Cuádruplos y tripletas de cada bloque: las máquinas y al final la sección de control.
void imprimirCodigoIntermedio(const Program& programa, std::ostream& salida);

// Tabla Técnica / Tipo / Veces con las 9 técnicas del optimizador.
void imprimirTecnicas(const Optimizer& optimizador, std::ostream& salida);
