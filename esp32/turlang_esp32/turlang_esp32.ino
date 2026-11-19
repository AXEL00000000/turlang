// turlang_esp32.ino — prototipo de TurLang en el ESP32
//
// Al encender muestra el menú de programas (hoy, los incrustados en Programas.h,
// generado con mingw32-make esp32). Al elegir uno lo carga, lo ejecuta con la
// pantalla y al terminar ofrece Repetir o Menu. El Monitor serie (115200) recibe lo
// mismo que la consola de la pantalla.

#include "Menu.h"
#include "Pantalla.h"
#include "TftIO.h"
#include "src/runtime/Interpreter.h"
#include "src/runtime/Turc.h"

BarraSuperior barra;

// Ejecuta un programa ya cargado hasta que el usuario elige Menu.
void ejecutar(const Compilado& compilado, const std::string& nombre) {
    bool repetir = true;
    while (repetir) {
        Serial.printf("== %s\n", nombre.c_str());
        TftIO io(compilado, nombre, barra);
        io.iniciar();
        Interpreter interprete(compilado.programa, io);
        bool correcto = interprete.ejecutar();
        io.terminar(correcto, compilado.nombre + ":" + std::to_string(interprete.lineaError()) +
                                  ": error de ejecución: " + interprete.mensajeError());
        barra.dibujarFin();
        repetir = barra.esperarFin();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("TurLang");
    iniciarPantalla();
}

void loop() {
    std::vector<ProgramaDisponible> programas = listarProgramas();
    const ProgramaDisponible& elegido = programas[elegirPrograma(programas)];

    std::string texto, error;
    Compilado compilado;
    if (!cargarPrograma(elegido, texto, error) || !leerTurc(texto, compilado, error)) {
        Serial.printf("%s: %s\n", elegido.nombre.c_str(), error.c_str());
        mostrarError(elegido.nombre, error);
        return;
    }
    ejecutar(compilado, elegido.nombre);
}
