#include "common/SymbolTable.h"

#include <algorithm>

#include "common/Tabla.h"

namespace {

using Fila = std::vector<std::string>;

int tamano(TipoDato tipo) {
    switch (tipo) {
        case TipoDato::Int:    return 4;
        case TipoDato::String: return 4;   // referencia
        case TipoDato::Bool:   return 1;
        default:               return 0;
    }
}

}  // namespace

SymbolTable::SymbolTable() {
    registrarIntegrada("run", TipoDato::Bool, 2);
    registrarIntegrada("output", TipoDato::String, 2);
    registrarIntegrada("repeat", TipoDato::String, 2);
    registrarIntegrada("input", TipoDato::String, 0);
    registrarIntegrada("print", TipoDato::Void, 1);
}

void SymbolTable::registrarIntegrada(const std::string& nombre, TipoDato tipo, int numArgs) {
    Simbolo s;
    s.nombre = nombre;
    s.tipo = tipo;
    s.scope = 0;
    s.funcion = true;
    s.numArgs = numArgs;
    s.direccion = -1;
    insertar(nombre, s);
}

bool SymbolTable::insertar(const std::string& clave, const Simbolo& simbolo) {
    if (simbolos.count(clave) != 0) {
        return false;
    }
    simbolos.emplace(clave, simbolo);
    orden.push_back(clave);
    return true;
}

const Simbolo* SymbolTable::buscar(const std::string& clave) const {
    auto it = simbolos.find(clave);
    return it == simbolos.end() ? nullptr : &it->second;
}

int SymbolTable::reservar(TipoDato tipo) {
    int direccion = siguienteDireccion;
    siguienteDireccion += tamano(tipo);
    return direccion;
}

std::vector<Simbolo> SymbolTable::variablesPorDireccion() const {
    std::vector<Simbolo> variables;
    for (const std::string& clave : orden) {
        const Simbolo& s = simbolos.find(clave)->second;
        if (s.direccion >= 0) variables.push_back(s);
    }
    std::stable_sort(variables.begin(), variables.end(),
                     [](const Simbolo& a, const Simbolo& b) { return a.direccion < b.direccion; });
    return variables;
}

void SymbolTable::imprimir(std::ostream& salida) const {
    std::vector<Fila> filas;
    filas.push_back({"Nombre", "Tipo", "Valor", "Scope", "Función", "# Args", "Dirección"});

    auto agregar = [&](const Simbolo& s) {
        filas.push_back({
            s.nombre,
            nombreTipoDato(s.tipo),
            s.valor.empty() ? "---" : s.valor,
            std::to_string(s.scope),
            s.funcion ? "✓" : "",
            s.funcion ? std::to_string(s.numArgs) : "",
            s.direccion < 0 ? "---" : std::to_string(s.direccion),
        });
    };

    // Primero los símbolos del programa y al final las integradas, cada grupo
    // en orden de inserción.
    for (const std::string& clave : orden) {
        const Simbolo& s = simbolos.at(clave);
        if (!s.funcion) agregar(s);
    }
    for (const std::string& clave : orden) {
        const Simbolo& s = simbolos.at(clave);
        if (s.funcion) agregar(s);
    }

    imprimirTabla(filas, salida);
}
