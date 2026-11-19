#pragma once

// Implementación de IO para la pantalla del prototipo: une la barra superior, la
// vista de programa, la vista de máquina y el teclado.

#include <string>

#include "Teclado.h"
#include "Vistas.h"
#include "src/runtime/IO.h"
#include "src/runtime/Turc.h"

class TftIO : public IO {
public:
    TftIO(const Compilado& compilado, const std::string& nombre, BarraSuperior& barra);

    // Barra superior y vista de programa con las variables en --- y la consola vacía.
    void iniciar();

    // Al terminar la ejecución: regresa a la vista de programa y escribe en la
    // consola "-- fin del programa --" o el error en rojo.
    void terminar(bool correcto, const std::string& error);

    void imprimir(const std::string& texto) override;
    std::string leer() override;
    void lineaActual(int linea) override;
    void variableCambio(const std::string& nombre, const std::string& valor) override;
    void maquinaInicio(const std::string& nombre, const std::string& cinta, int cabezal,
                       int limite) override;
    void maquinaPaso(const std::string& cinta, int cabezal, int linea) override;
    void maquinaFin(bool acepto, const std::string& cinta) override;
    bool detenido() override;

private:
    const Compilado& compilado;
    std::string nombre;
    BarraSuperior& barra;
    VistaPrograma programa;
    VistaMaquina maquina;
    Teclado teclado;
    bool enMaquina = false;

    void volverAPrograma();
};
