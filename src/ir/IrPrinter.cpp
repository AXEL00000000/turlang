#include "ir/IrPrinter.h"

#include <string>
#include <vector>

#include "common/Tabla.h"
#include "ir/Triples.h"

namespace {

using Fila = std::vector<std::string>;

void imprimirBloque(const Bloque& bloque, std::ostream& salida) {
    salida << (bloque.nombre.empty() ? "Sección de control" : "Máquina " + bloque.nombre)
           << "\n\n";

    salida << "Cuádruplos\n";
    std::vector<Fila> cuadruplos = {{"#", "Op", "Arg1", "Arg2", "Res"}};
    for (size_t i = 0; i < bloque.quads.size(); ++i) {
        const Quad& q = bloque.quads[i];
        cuadruplos.push_back({std::to_string(i + 1), nombreOp(q.op), textoOperando(q.arg1),
                              textoOperando(q.arg2), textoOperando(q.res)});
    }
    imprimirTabla(cuadruplos, salida);

    salida << "\nTripletas\n";
    std::vector<Fila> tripletas = {{"#", "Op", "Arg1", "Arg2"}};
    std::vector<Tripleta> lista = aTripletas(bloque.quads);
    for (size_t i = 0; i < lista.size(); ++i) {
        tripletas.push_back({std::to_string(i + 1), lista[i].op, lista[i].arg1, lista[i].arg2});
    }
    imprimirTabla(tripletas, salida);
}

}  // namespace

void imprimirCodigoIntermedio(const Program& programa, std::ostream& salida) {
    for (const Bloque& maquina : programa.maquinas) {
        imprimirBloque(maquina, salida);
        salida << '\n';
    }
    imprimirBloque(programa.principal, salida);
}
