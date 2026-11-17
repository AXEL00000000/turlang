#include "common/ConsoleIO.h"

#include <iostream>

void ConsoleIO::imprimir(const std::string& texto) {
    std::cout << texto << '\n';
}

// Una línea sin el salto; quita un '\r' final. Sin más entrada regresa cadena vacía.
std::string ConsoleIO::leer() {
    std::string linea;
    if (!std::getline(std::cin, linea)) {
        linea.clear();
    } else if (!linea.empty() && linea.back() == '\r') {
        linea.pop_back();
    }
    leidas.push_back(linea);
    return linea;
}

const std::vector<std::string>& ConsoleIO::lineasLeidas() const {
    return leidas;
}
