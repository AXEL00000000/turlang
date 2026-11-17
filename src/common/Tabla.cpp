#include "common/Tabla.h"

#include <algorithm>

namespace {

// Cuenta caracteres UTF-8, no bytes: los bytes de continuación no cuentan.
size_t anchoTexto(const std::string& texto) {
    size_t ancho = 0;
    for (unsigned char c : texto) {
        if ((c & 0xC0) != 0x80) {
            ++ancho;
        }
    }
    return ancho;
}

}  // namespace

void imprimirTabla(const std::vector<std::vector<std::string>>& filas, std::ostream& salida) {
    if (filas.empty()) {
        return;
    }
    std::vector<size_t> anchos(filas.front().size(), 0);
    for (const auto& fila : filas) {
        for (size_t i = 0; i < fila.size() && i < anchos.size(); ++i) {
            anchos[i] = std::max(anchos[i], anchoTexto(fila[i]));
        }
    }
    for (const auto& fila : filas) {
        std::string linea;
        for (size_t i = 0; i < fila.size(); ++i) {
            linea += fila[i];
            if (i + 1 < fila.size()) {
                linea += std::string(anchos[i] + 2 - anchoTexto(fila[i]), ' ');
            }
        }
        size_t fin = linea.find_last_not_of(' ');
        linea.erase(fin == std::string::npos ? 0 : fin + 1);
        salida << linea << '\n';
    }
}
