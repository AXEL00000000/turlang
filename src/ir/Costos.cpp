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

// Valores de la sección de control y de cada máquina, en el orden de las filas.
std::vector<std::string> medir(const Program& programa, const Estadisticas& est) {
    // Temporales distintos y su memoria: 1 byte si los produce un relacional o una
    // llamada a una máquina (bool); 4 bytes si no.
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

    std::vector<std::string> valores = {
        std::to_string(programa.principal.quads.size()),
        std::to_string(est.ejecutadosPrincipal),
        std::to_string(est.saltosPrincipal),
        std::to_string(temporales.size()),
        std::to_string(bytes) + " bytes",
    };
    for (size_t i = 0; i < programa.maquinas.size(); ++i) {
        long ejecutados = i < est.ejecutadosMaquina.size() ? est.ejecutadosMaquina[i] : 0;
        valores.push_back(std::to_string(programa.maquinas[i].quads.size()));
        valores.push_back(std::to_string(ejecutados));
    }
    return valores;
}

}  // namespace

void imprimirCostos(const Program& antes, const Estadisticas& estAntes,
                    const Program& despues, const Estadisticas& estDespues,
                    std::ostream& salida) {
    std::vector<std::string> medidas = {
        "Cuádruplos de la sección de control",
        "Cuádruplos ejecutados de la sección de control",
        "Saltos ejecutados de la sección de control",
        "Temporales de la sección de control",
        "Memoria de temporales",
    };
    for (const Bloque& b : antes.maquinas) {
        medidas.push_back("Cuádruplos de la máquina " + b.nombre);
        medidas.push_back("Cuádruplos ejecutados de la máquina " + b.nombre);
    }

    std::vector<std::string> valoresAntes = medir(antes, estAntes);
    std::vector<std::string> valoresDespues = medir(despues, estDespues);

    std::vector<std::vector<std::string>> filas = {{"Medida", "Antes", "Después"}};
    for (size_t i = 0; i < medidas.size(); ++i) {
        filas.push_back({medidas[i], valoresAntes[i], valoresDespues[i]});
    }

    salida << '\n';
    imprimirTabla(filas, salida);
}
