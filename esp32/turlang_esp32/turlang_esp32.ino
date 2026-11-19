// turlang_esp32.ino — prototipo de TurLang en el ESP32
//
// Los programas van incrustados en Programas.h (generado con mingw32-make esp32).
// Por ahora ejecuta el primer programa incrustado con la pantalla.

#include "Pantalla.h"
#include "Programas.h"
#include "TftIO.h"
#include "src/runtime/Interpreter.h"
#include "src/runtime/Turc.h"

BarraSuperior barra;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("TurLang");
    iniciarPantalla();

    Compilado compilado;
    std::string error;
    if (!leerTurc(PROGRAMAS[0].turc, compilado, error)) {
        Serial.println(error.c_str());
        return;
    }
    TftIO io(compilado, PROGRAMAS[0].nombre, barra);
    io.iniciar();
    Interpreter interprete(compilado.programa, io);
    bool correcto = interprete.ejecutar();
    io.terminar(correcto, compilado.nombre + ":" + std::to_string(interprete.lineaError()) +
                              ": error de ejecución: " + interprete.mensajeError());
}

void loop() {
    delay(1000);
}
