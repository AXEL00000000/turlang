#pragma once

#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "common/SymbolTable.h"
#include "runtime/Program.h"

// Traduce los cuádruplos optimizados a ensamblador x86 de 32 bits (NASM, cdecl).
class CodeGenerator {
public:
    CodeGenerator(const Program& programa, const SymbolTable& tabla,
                  const std::string& nombreArchivo);     // nombre del .tur, sin ruta
    void generar(std::ostream& salida);
private:
    enum class Tipo { Int, Cadena, Bool };

    const Program& programa;
    const SymbolTable& tabla;
    std::string nombreArchivo;

    // Cadenas literales: texto → etiqueta cad_k, en orden de aparición.
    std::map<std::string, std::string> etiquetaCadena;
    std::vector<std::string> ordenCadenas;

    // Rutinas de apoyo y textos que usa el archivo.
    std::set<std::string> rutinasUsadas;

    // Estado del bloque que se está traduciendo.
    const Bloque* bloque = nullptr;
    bool enMaquina = false;
    std::map<std::string, Tipo> tipoTemp;
    std::vector<std::string> lugarDef;           // por renglón: lugar del temporal que define
    std::map<std::string, std::string> lugarActual;
    std::set<int> destinos;                      // renglones (desde 1) destino de salto
    int casillas = 0;                            // casillas de temporales en la pila
    std::vector<Tipo> tiposParam;

    void recolectarCadenas();
    void traducirBloque(const Bloque& b, bool maquina, std::ostream& salida);
    std::string asignarRegistros();
    void inferirTipos();
    void traducir(int renglon, const Quad& q, std::ostream& o);

    Tipo tipoDe(const Operando& o) const;
    std::string lugarVariable(const std::string& nombre) const;
    void cargar(const Operando& o, const std::string& registro, std::ostream& o2);
    std::string fuente(const Operando& o, std::ostream& salida);
    void guardar(int renglon, const Operando& destino, std::ostream& o);
    void revisarCinta(int renglon, const Quad& q, std::ostream& o);
    void usar(const std::string& rutina);

    void escribirEncabezado(std::ostream& salida);
    void escribirDatos(std::ostream& salida);
    void escribirRutinas(std::ostream& salida);
};
