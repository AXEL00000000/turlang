#include "common/ErrorHandler.h"

#include <algorithm>

namespace {

// Texto de la fase tal como aparece en los mensajes.
const char* textoFase(Fase fase) {
    switch (fase) {
        case Fase::Lexico:     return "léxico";
        case Fase::Sintactico: return "sintáctico";
        case Fase::Semantico:  return "semántico";
    }
    return "";
}

// Quita la ruta y deja solo el nombre del archivo.
std::string soloNombre(const std::string& ruta) {
    size_t corte = ruta.find_last_of("/\\");
    if (corte == std::string::npos) {
        return ruta;
    }
    return ruta.substr(corte + 1);
}

}  // namespace

ErrorHandler::ErrorHandler(const std::string& archivo)
    : archivo(soloNombre(archivo)) {}

void ErrorHandler::reportar(int linea, int columna, Fase fase, const std::string& mensaje) {
    errores.push_back({linea, columna, fase, mensaje});
}

bool ErrorHandler::hayErrores() const {
    return !errores.empty();
}

const std::string& ErrorHandler::nombreArchivo() const {
    return archivo;
}

int ErrorHandler::total() const {
    return static_cast<int>(errores.size());
}

void ErrorHandler::imprimir(std::ostream& salida) const {
    if (errores.empty()) {
        return;
    }

    std::vector<Error> ordenados = errores;
    std::stable_sort(ordenados.begin(), ordenados.end(),
                     [](const Error& a, const Error& b) {
                         if (a.linea != b.linea) {
                             return a.linea < b.linea;
                         }
                         return a.columna < b.columna;
                     });

    for (const Error& e : ordenados) {
        salida << archivo << ':' << e.linea << ':' << e.columna
               << ": error " << textoFase(e.fase) << ": " << e.mensaje << '\n';
    }

    if (total() == 1) {
        salida << "1 error encontrado; no se generó código.\n";
    } else {
        salida << total() << " errores encontrados; no se generó código.\n";
    }
}
