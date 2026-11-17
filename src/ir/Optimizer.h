#pragma once

#include <map>
#include <string>
#include <vector>

#include "runtime/Program.h"

enum class Tecnica {
    Folding, SimplificacionAlgebraica, ReduccionFuerza,              // locales
    PropagacionConstantes, CodigoMuerto, DesenrolladoCiclos,
    EstadosInalcanzables,                                            // globales
    EscriturasRedundantes, SaltosInnecesarios,                       // mirilla
    Total
};

class Optimizer {
public:
    Program optimizar(const Program& original);     // no modifica el original
    int veces(Tecnica tecnica) const;               // cuántas veces se aplicó
private:
    // Valores conocidos: nombre → constante, o temporal → variable (copia).
    using Tabla = std::map<std::string, Operando>;

    // Lo que le pasó a un cuádruplo en la pasada de propagación.
    struct Efecto {
        bool sustituyo = false;
        bool plego = false;
        bool simplifico = false;
        bool redujo = false;
        bool eliminar = false;      // código muerto: se quita el cuádruplo
        bool aSalto = false;        // código muerto: if-false false → jump
    };

    int contadores[static_cast<int>(Tecnica::Total)] = {};

    void contar(Tecnica tecnica, int n = 1);

    // Pasos sobre un cuádruplo (no cuentan; los cuenta quien los llama)
    Efecto evaluar(Quad& q, Tabla& tabla);
    bool folding(Quad& q);
    bool simplificacionAlgebraica(Quad& q);
    bool reduccionFuerza(Quad& q);

    // Sección de control
    void optimizarControl(std::vector<Quad>& quads);
    bool propagacionConstantes(std::vector<Quad>& quads);
    bool codigoInalcanzable(std::vector<Quad>& quads);
    bool desenrolladoCiclos(std::vector<Quad>& quads);
    bool asignacionesSinUso(std::vector<Quad>& quads);
    Tabla tablaAntesDe(const std::vector<Quad>& quads, int renglon);
    bool simularCiclo(const std::vector<Quad>& quads, int h, int k, int j, int& vueltas);

    // Máquinas
    void optimizarMaquina(std::vector<Quad>& quads);
    bool estadosInalcanzables(std::vector<Quad>& quads);
    bool escriturasRedundantes(std::vector<Quad>& quads);

    // Todos los bloques
    bool saltosInnecesarios(std::vector<Quad>& quads);
};
