#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "backend/CodeGenerator.h"
#include "common/ConsoleIO.h"
#include "common/ErrorHandler.h"
#include "common/IOSilenciosa.h"
#include "common/SymbolTable.h"
#include "common/Token.h"
#include "frontend/AstPrinter.h"
#include "frontend/Lexer.h"
#include "frontend/Parser.h"
#include "frontend/SemanticAnalyzer.h"
#include "ir/IntermediateCodeGenerator.h"
#include "ir/Costos.h"
#include "ir/IrPrinter.h"
#include "ir/Optimizer.h"
#include "runtime/Interpreter.h"
#include "runtime/Program.h"
#include "runtime/Turc.h"

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

// Escribe el archivo completo en modo binario (finales de línea \n).
bool escribirArchivo(const std::string& ruta, const std::string& contenido) {
    std::ofstream salida(ruta, std::ios::binary);
    if (!salida) {
        return false;
    }
    salida << contenido;
    salida.close();
    return static_cast<bool>(salida);
}

// Líneas del programa fuente, sin los saltos ni un '\r' final.
std::vector<std::string> lineasDe(const std::string& texto) {
    std::vector<std::string> lineas;
    size_t inicio = 0;
    while (inicio < texto.size()) {
        size_t fin = texto.find('\n', inicio);
        if (fin == std::string::npos) fin = texto.size();
        std::string linea = texto.substr(inicio, fin - inicio);
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();
        lineas.push_back(linea);
        inicio = fin + 1;
    }
    return lineas;
}

void imprimirErrorEjecucion(const std::string& archivo, const Interpreter& interprete) {
    std::cout << archivo << ':' << interprete.lineaError()
              << ": error de ejecución: " << interprete.mensajeError() << '\n';
}

// turlang programa.turc: carga el compilado y lo ejecuta en consola.
int ejecutarTurc(const std::string& texto) {
    Compilado compilado;
    std::string error;
    if (!leerTurc(texto, compilado, error)) {
        std::cerr << "turlang: " << error << '\n';
        return 2;
    }
    ConsoleIO io;
    Interpreter interprete(compilado.programa, io);
    if (!interprete.ejecutar()) {
        imprimirErrorEjecucion(compilado.nombre, interprete);
        return 1;
    }
    return 0;
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

    bool esTurc = terminaEn(ruta, ".turc");
    if (!esTurc && !terminaEn(ruta, ".tur")) {
        std::cerr << "turlang: el archivo debe tener extensión .tur o .turc\n";
        return 2;
    }
    if (esTurc && !bandera.empty()) {
        std::cerr << "turlang: un archivo .turc solo se puede ejecutar\n";
        return 2;
    }

    std::string fuente;
    if (!leerArchivo(ruta, fuente)) {
        std::cerr << "turlang: no se pudo abrir " << ruta << '\n';
        return 2;
    }

    if (esTurc) {
        return ejecutarTurc(fuente);
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

    if (bandera == "--esp32") {
        // El compilado lleva los cuádruplos sin optimizar.
        Compilado compilado;
        compilado.nombre = errores.nombreArchivo();
        compilado.fuente = lineasDe(fuente);
        for (const Simbolo& s : tabla.variablesPorDireccion()) {
            compilado.variables.push_back({s.nombre, nombreTipoDato(s.tipo)});
        }
        compilado.programa = codigo;
        std::string rutaTurc = ruta + "c";
        if (!escribirArchivo(rutaTurc, escribirTurc(compilado))) {
            std::cerr << "turlang: no se pudo escribir " << rutaTurc << '\n';
            return 2;
        }
        std::cout << "generado: " << rutaTurc << '\n';
        return 0;
    }

    Optimizer optimizador;
    Program optimizado = optimizador.optimizar(codigo);

    if (bandera == "--cuadruplos") {
        std::cout << "=== Antes de optimizar ===\n\n";
        imprimirCodigoIntermedio(codigo, std::cout);
        std::cout << "\n=== Técnicas aplicadas ===\n\n";
        imprimirTecnicas(optimizador, std::cout);
        std::cout << "\n=== Después de optimizar ===\n\n";
        imprimirCodigoIntermedio(optimizado, std::cout);
        return 0;
    }
    if (bandera == "--asm") {
        // <mismo nombre y carpeta que el .tur>.asm, a partir del programa optimizado.
        std::string rutaAsm = ruta.substr(0, ruta.size() - 4) + ".asm";
        std::ostringstream textoAsm;
        CodeGenerator generadorAsm(optimizado, tabla, errores.nombreArchivo());
        generadorAsm.generar(textoAsm);
        if (!escribirArchivo(rutaAsm, textoAsm.str())) {
            std::cerr << "turlang: no se pudo escribir " << rutaAsm << '\n';
            return 2;
        }
        std::cout << "generado: " << rutaAsm << '\n';
        return 0;
    }

    // Sin bandera o con --costos: se ejecuta el programa optimizado en consola.
    ConsoleIO io;
    Interpreter interprete(optimizado, io);
    if (!interprete.ejecutar()) {
        imprimirErrorEjecucion(errores.nombreArchivo(), interprete);
        return 1;
    }

    if (bandera == "--costos") {
        // El original se ejecuta en silencio con las mismas líneas de entrada.
        IOSilenciosa silenciosa(io.lineasLeidas());
        Interpreter original(codigo, silenciosa);
        original.ejecutar();
        imprimirCostos(codigo, original.estadisticas(), optimizado, interprete.estadisticas(),
                       std::cout);
    }
    return 0;
}
