#include "ir/Costos.h"

#include <set>
#include <string>
#include <vector>

#include "common/Tabla.h"

namespace {

bool esRelacional(Op op) {
    return op == Op::Menor || op == Op::Mayor || op == Op::MenorIgual ||
           op == Op::MayorIgual || op == Op::Igual || op == Op::Distinto;
}

bool esMaquina(const Program& programa, const std::string& nombre) {
    for (const Bloque& b : programa.maquinas) {
        if (b.nombre == nombre) return true;
    }
    return false;
}

}  // namespace

void imprimirCostos(const Program& programa, const Estadisticas& est, std::ostream& salida) {
    // Temporales distintos de la sección de control y su memoria: 1 byte si los
    // produce un relacional o una llamada a una máquina (bool); 4 bytes si no.
    std::set<std::string> temporales;
    int bytes = 0;
    for (const Quad& q : programa.principal.quads) {
        if (q.res.clase != ClaseOperando::Temp || !temporales.insert(q.res.texto).second) {
            continue;
        }
        bool esBool = esRelacional(q.op) ||
                      (q.op == Op::Call && esMaquina(programa, q.arg1.texto));
        bytes += esBool ? 1 : 4;
    }

    std::vector<std::vector<std::string>> filas = {
        {"Medida", "Valor"},
        {"Cuádruplos de la sección de control", std::to_string(programa.principal.quads.size())},
        {"Cuádruplos ejecutados de la sección de control", std::to_string(est.ejecutadosPrincipal)},
        {"Saltos ejecutados de la sección de control", std::to_string(est.saltosPrincipal)},
        {"Temporales de la sección de control", std::to_string(temporales.size())},
        {"Memoria de temporales", std::to_string(bytes) + " bytes"},
    };
    for (size_t i = 0; i < programa.maquinas.size(); ++i) {
        const Bloque& b = programa.maquinas[i];
        long ejecutados = i < est.ejecutadosMaquina.size() ? est.ejecutadosMaquina[i] : 0;
        filas.push_back({"Cuádruplos de la máquina " + b.nombre, std::to_string(b.quads.size())});
        filas.push_back({"Cuádruplos ejecutados de la máquina " + b.nombre, std::to_string(ejecutados)});
    }

    salida << '\n';
    imprimirTabla(filas, salida);
}
