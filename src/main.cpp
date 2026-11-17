#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "common/ConsoleIO.h"
#include "common/ErrorHandler.h"
#include "common/SymbolTable.h"
#include "common/Token.h"
#include "frontend/AstPrinter.h"
#include "frontend/Lexer.h"
#include "frontend/Parser.h"
#include "frontend/SemanticAnalyzer.h"
#include "ir/IntermediateCodeGenerator.h"
#include "ir/Costos.h"
#include "ir/IrPrinter.h"
#include "runtime/Interpreter.h"
#include "runtime/Program.h"

namespace {

const char* const LINEA_USO =
    "uso: turlang <archivo.tur> [--tokens | --arbol | --tabla | --cuadruplos | --costos | --asm | --esp32]";

bool terminaEn(const std::string& texto, const std::string& sufijo) {
    return texto.size() >= sufijo.size() &&
           texto.compare(texto.size() - sufijo.size(), sufijo.size(), sufijo) == 0;
}

bool esBanderaConocida(const std::string& bandera) {
    return bandera == "--tokens" || bandera == "--arbol" || bandera == "--tabla" ||
           bandera == "--cuadruplos" || bandera == "--costos" || bandera == "--asm" ||
           bandera == "--esp32";
}

// Lee el archivo completo en modo binario y quita la marca BOM de UTF-8.
bool leerArchivo(const std::string& ruta, std::string& contenido) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) {
        return false;
    }
    contenido.assign(std::istreambuf_iterator<char>(entrada),
                     std::istreambuf_iterator<char>());
    if (contenido.size() >= 3 &&
        static_cast<unsigned char>(contenido[0]) == 0xEF &&
        static_cast<unsigned char>(contenido[1]) == 0xBB &&
        static_cast<unsigned char>(contenido[2]) == 0xBF) {
        contenido.erase(0, 3);
    }
    return true;
}

// Tabla de tokens para --tokens, con una línea final con el total.
void imprimirTokens(const std::vector<Token>& tokens, std::ostream& salida) {
    salida << "Línea  Col  Tipo         Lexema\n";
    for (const Token& t : tokens) {
        salida << std::right << std::setw(5) << t.linea
               << std::setw(5) << t.columna << "  ";
        if (t.lexema.empty()) {
            salida << nombreTipo(t.tipo) << '\n';
        } else {
            salida << std::left << std::setw(13) << nombreTipo(t.tipo) << t.lexema << '\n';
        }
    }
    salida << tokens.size() << " tokens\n";
}

}  // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2 || argc > 3) {
        std::cerr << LINEA_USO << '\n';
        return 2;
    }

    std::string ruta = argv[1];
    std::string bandera = (argc == 3) ? argv[2] : "";

    if (!bandera.empty() && !esBanderaConocida(bandera)) {
        std::cerr << "turlang: opción desconocida " << bandera << '\n';
        std::cerr << LINEA_USO << '\n';
        return 2;
    }

    if (!terminaEn(ruta, ".tur")) {
        std::cerr << "turlang: el archivo debe tener extensión .tur\n";
        return 2;
    }

    std::string fuente;
    if (!leerArchivo(ruta, fuente)) {
        std::cerr << "turlang: no se pudo abrir " << ruta << '\n';
        return 2;
    }

    ErrorHandler errores(ruta);

    Lexer lexer(fuente, errores);
    std::vector<Token> tokens = lexer.analizar();

    if (bandera == "--tokens") {
        imprimirTokens(tokens, std::cout);
    }

    // Con errores léxicos no se ejecuta el Parser.
    if (errores.hayErrores()) {
        errores.imprimir(std::cout);
        return 1;
    }

    if (bandera == "--tokens") {
        return 0;
    }

    Parser parser(tokens, errores);
    Programa programa = parser.analizar();

    if (errores.hayErrores()) {
        errores.imprimir(std::cout);
        return 1;
    }

    // --arbol muestra el árbol sin análisis semántico.
    if (bandera == "--arbol") {
        imprimirArbol(programa, std::cout);
        return 0;
    }

    SymbolTable tabla;
    SemanticAnalyzer semantico(programa, tabla, errores);
    semantico.analizar();

    if (errores.hayErrores()) {
        errores.imprimir(std::cout);
        return 1;
    }

    if (bandera == "--tabla") {
        tabla.imprimir(std::cout);
        return 0;
    }

    IntermediateCodeGenerator generador(programa);
    Program codigo = generador.generar();

    if (bandera == "--cuadruplos") {
        imprimirCodigoIntermedio(codigo, std::cout);
        return 0;
    }
    if (bandera == "--asm" || bandera == "--esp32") {
        std::cout << bandera << ": pendiente\n";
        return 0;
    }

    // Sin bandera o con --costos: ejecutar en consola.
    ConsoleIO io;
    Interpreter interprete(codigo, io);
    if (!interprete.ejecutar()) {
        std::cout << errores.nombreArchivo() << ':' << interprete.lineaError()
                  << ": error de ejecución: " << interprete.mensajeError() << '\n';
        return 1;
    }

    if (bandera == "--costos") {
        imprimirCostos(codigo, interprete.estadisticas(), std::cout);
    }
    return 0;
}
