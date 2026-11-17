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

void imprimirTecnicas(const Optimizer& optimizador, std::ostream& salida) {
    struct Fila { Tecnica tecnica; const char* nombre; const char* tipo; };
    const Fila filas[] = {
        {Tecnica::Folding, "Folding", "Local"},
        {Tecnica::SimplificacionAlgebraica, "Simplificación algebraica", "Local"},
        {Tecnica::ReduccionFuerza, "Reducción de fuerza", "Local"},
        {Tecnica::PropagacionConstantes, "Propagación de constantes", "Global"},
        {Tecnica::CodigoMuerto, "Eliminación de código muerto", "Global"},
        {Tecnica::DesenrolladoCiclos, "Desenrollado de ciclos", "Global"},
        {Tecnica::EstadosInalcanzables, "Eliminación de estados inalcanzables", "Global"},
        {Tecnica::EscriturasRedundantes, "Eliminación de escrituras redundantes", "Mirilla"},
        {Tecnica::SaltosInnecesarios, "Eliminación de saltos innecesarios", "Mirilla"},
    };
    std::vector<std::vector<std::string>> tabla = {{"Técnica", "Tipo", "Veces"}};
    for (const Fila& f : filas) {
        tabla.push_back({f.nombre, f.tipo, std::to_string(optimizador.veces(f.tecnica))});
    }
    imprimirTabla(tabla, salida);
}

void imprimirCodigoIntermedio(const Program& programa, std::ostream& salida) {
    for (const Bloque& maquina : programa.maquinas) {
        imprimirBloque(maquina, salida);
        salida << '\n';
    }
    imprimirBloque(programa.principal, salida);
}
