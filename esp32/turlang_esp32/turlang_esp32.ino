// turlang_esp32.ino — prototipo de TurLang en el ESP32
//
// Los programas van incrustados en Programas.h (generado con mingw32-make esp32).
// Por ahora solo comprueba que cada .turc incrustado se carga.

#include "Programas.h"
#include "src/runtime/Turc.h"

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("TurLang");
    for (int i = 0; i < NUM_PROGRAMAS; ++i) {
        Compilado compilado;
        std::string error;
        bool correcto = leerTurc(PROGRAMAS[i].turc, compilado, error);
        Serial.printf("  %s: %s\n", PROGRAMAS[i].nombre, correcto ? "cargado" : error.c_str());
    }
}

void loop() {
    delay(1000);
}
