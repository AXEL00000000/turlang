#pragma once

// Este archivo también se compila en el ESP32: sin iostream ni excepciones.

#include <string>

#include "runtime/IO.h"
#include "runtime/Program.h"
#include "runtime/Valor.h"

class TuringSimulator {
public:
    TuringSimulator(const Bloque& maquina, IO& io);
    bool ejecutar(const std::string& cadena, int maximo);   // true si la máquina aceptó
    const std::string& cintaFinal() const;                  // recortada
    long ejecutados() const;                                // cuádruplos ejecutados en esta llamada
    bool huboError() const;
    const std::string& mensajeError() const;
    int lineaError() const;
private:
    const Bloque& maquina;
    IO& io;
    Memoria memoria;
    std::string cinta;          // casilla vacía = espacio
    long desplazamiento = 0;    // índice en cinta = cabezal + desplazamiento
    std::string final;
    long cuenta = 0;
    bool error = false;
    std::string mensaje;
    int linea = 0;

    bool fallar(const Quad& q, const std::string& texto);
    bool indiceCabezal(const Quad& q, long& indice);
    void actualizarFinal();
};
