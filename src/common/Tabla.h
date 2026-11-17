#pragma once

#include <ostream>
#include <string>
#include <vector>

// Imprime una tabla: la primera fila es el encabezado. Cada columna mide el
// contenido más largo más 2 espacios, alineada a la izquierda; la última columna
// va sin relleno y ninguna línea termina en espacios. Cuenta caracteres, no bytes.
void imprimirTabla(const std::vector<std::vector<std::string>>& filas, std::ostream& salida);
