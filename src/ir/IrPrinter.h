#pragma once

#include <ostream>

#include "runtime/Program.h"

// Cuádruplos y tripletas de cada bloque: las máquinas y al final la sección de control.
void imprimirCodigoIntermedio(const Program& programa, std::ostream& salida);
