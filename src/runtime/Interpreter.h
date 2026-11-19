#pragma once

// Este archivo también se compila en el ESP32: sin iostream ni excepciones.

#include <string>
#include <vector>

#include "IO.h"
#include "Program.h"
#include "Valor.h"

struct Estadisticas {
    long ejecutadosPrincipal = 0;          // cuádruplos ejecutados de la sección de control
    long saltosPrincipal = 0;              // if-false y jump ejecutados (salten o no)
    std::vector<long> ejecutadosMaquina;   // acumulado por máquina, en el orden de Program::maquinas
};

class Interpreter {
public:
    Interpreter(const Program& programa, IO& io);
    bool ejecutar();                               // false si hubo un error de ejecución
    const Estadisticas& estadisticas() const;
    const std::string& mensajeError() const;
    int lineaError() const;                        // línea del cuádruplo que falló
private:
    const Program& programa;
    IO& io;
    Memoria memoria;
    std::vector<Valor> argumentos;     // param pendientes
    std::string ultimaCinta;           // cinta final de la última máquina ejecutada
    Estadisticas stats;
    std::string mensaje;
    int linea = 0;

    bool fallar(int lineaFallo, const std::string& texto);
    void guardar(const Operando& destino, const Valor& valor);
    bool llamar(const Quad& q);
};
