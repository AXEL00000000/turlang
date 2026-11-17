#include "ir/IrPrinter.h"

#include <algorithm>
#include <string>
#include <vector>

#include "ir/Triples.h"

namespace {

using Fila = std::vector<std::string>;

// Cuenta caracteres UTF-8, no bytes.
size_t anchoTexto(const std::string& texto) {
    size_t ancho = 0;
    for (unsigned char c : texto) {
        if ((c & 0xC0) != 0x80) {
            ++ancho;
        }
    }
    return ancho;
}

// Columnas del ancho del contenido más largo más 2, alineadas a la izquierda; la
// última sin relleno y sin espacios al final.
void imprimirTabla(const std::vector<Fila>& filas, std::ostream& salida) {
    std::vector<size_t> anchos(filas.front().size(), 0);
    for (const Fila& fila : filas) {
        for (size_t i = 0; i < fila.size(); ++i) {
            anchos[i] = std::max(anchos[i], anchoTexto(fila[i]));
        }
    }
    for (const Fila& fila : filas) {
        std::string linea;
        for (size_t i = 0; i < fila.size(); ++i) {
            linea += fila[i];
            if (i + 1 < fila.size()) {
                linea += std::string(anchos[i] + 2 - anchoTexto(fila[i]), ' ');
            }
        }
        size_t fin = linea.find_last_not_of(' ');
        linea.erase(fin == std::string::npos ? 0 : fin + 1);
        salida << linea << '\n';
    }
}

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
