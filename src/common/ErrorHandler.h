#pragma once

#include <ostream>
#include <string>
#include <vector>

enum class Fase { Lexico, Sintactico, Semantico };

struct Error {
    int linea;
    int columna;
    Fase fase;
    std::string mensaje;
};

class ErrorHandler {
public:
    explicit ErrorHandler(const std::string& archivo);
    void reportar(int linea, int columna, Fase fase, const std::string& mensaje);
    bool hayErrores() const;
    int total() const;
    void imprimir(std::ostream& salida) const;
    const std::string& nombreArchivo() const;   // sin la ruta
private:
    std::string archivo;
    std::vector<Error> errores;
};
