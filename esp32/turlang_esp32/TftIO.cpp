#include "TftIO.h"

#include "Pantalla.h"

namespace {
const int ESPERA_RESULTADO = 1500;   // ms que se ve el resultado de una máquina
}

TftIO::TftIO(const Compilado& compilado, const std::string& nombre, BarraSuperior& barra)
    : compilado(compilado), nombre(nombre), barra(barra) {}

void TftIO::iniciar() {
    barra.reiniciar();
    barra.dibujar(nombre);
    programa.cargar(compilado);
    programa.mostrar();
    enMaquina = false;
}

void TftIO::volverAPrograma() {
    enMaquina = false;
    programa.mostrar();
}

void TftIO::terminar(bool correcto, const std::string& error) {
    if (enMaquina) volverAPrograma();
    if (correcto) {
        programa.imprimir("-- fin del programa --", COLOR_GRIS);
    } else {
        programa.imprimir(error, COLOR_ROJO);
    }
}

void TftIO::imprimir(const std::string& texto) {
    programa.imprimir(texto, COLOR_TEXTO);
}

std::string TftIO::leer() {
    programa.ocultar();
    std::string texto = teclado.leer(programa.ultimaLineaConsola(), barra);
    programa.imprimir(texto, COLOR_TEXTO);
    programa.mostrar();
    return texto;
}

void TftIO::lineaActual(int linea) {
    programa.lineaActual(linea);
    barra.esperar(barra.esperaLinea());
}

void TftIO::variableCambio(const std::string& nombreVariable, const std::string& valor) {
    programa.variableCambio(nombreVariable, valor);
}

void TftIO::maquinaInicio(const std::string& nombreMaquina, const std::string& cinta, int cabezal,
                          int limite) {
    programa.ocultar();
    enMaquina = true;
    maquina.inicio(nombreMaquina, cinta, cabezal, limite, programa.fuente());
    barra.esperar(barra.esperaPaso());
}

void TftIO::maquinaPaso(const std::string& cinta, int cabezal, int linea) {
    maquina.paso(cinta, cabezal, linea);
    barra.esperar(barra.esperaPaso());
}

void TftIO::maquinaFin(bool acepto, const std::string& cinta) {
    maquina.fin(acepto, cinta);
    maquina.esperarResultado(barra, ESPERA_RESULTADO);
    volverAPrograma();
}

bool TftIO::detenido() {
    barra.revisar();
    return barra.salirPedido();
}
