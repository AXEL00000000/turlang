#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/Tipos.h"

struct Simbolo {
    std::string nombre;       // el que se muestra
    TipoDato tipo = TipoDato::Desconocido;
    std::string valor;        // valor inicial si se conoce al compilar; vacío si no
    int scope = 0;            // 0 programa, 1 interior de una máquina
    bool funcion = false;     // función integrada
    int numArgs = 0;
    int direccion = -1;       // -1 si no ocupa memoria
};

class SymbolTable {
public:
    SymbolTable();                                          // registra las integradas
    bool insertar(const std::string& clave, const Simbolo& simbolo);   // false si la clave ya existe
    const Simbolo* buscar(const std::string& clave) const;  // nullptr si no existe
    int reservar(TipoDato tipo);                            // regresa la dirección y avanza
    void imprimir(std::ostream& salida) const;
private:
    std::unordered_map<std::string, Simbolo> simbolos;
    std::vector<std::string> orden;   // claves en orden de inserción
    int siguienteDireccion = 0;

    void registrarIntegrada(const std::string& nombre, TipoDato tipo, int numArgs);
};
