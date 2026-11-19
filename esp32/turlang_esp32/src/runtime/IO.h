#pragma once

// Este archivo también se compila en el ESP32: sin iostream ni excepciones.

#include <string>

// Toda la entrada y salida de la ejecución pasa por esta interfaz.
class IO {
public:
    virtual ~IO() {}

    // Obligatorias
    virtual void imprimir(const std::string& texto) = 0;   // print: una línea completa
    virtual std::string leer() = 0;                        // input: una línea, sin el salto

    // Avisos para la pantalla del prototipo; por omisión no hacen nada
    virtual void lineaActual(int linea) { (void)linea; }
    virtual void variableCambio(const std::string& nombre, const std::string& valor) { (void)nombre; (void)valor; }
    // limite: el máximo de pasos que recibió la máquina (segundo argumento de run/output)
    virtual void maquinaInicio(const std::string& nombre, const std::string& cinta, int cabezal, int limite) {
        (void)nombre; (void)cinta; (void)cabezal; (void)limite;
    }
    virtual void maquinaPaso(const std::string& cinta, int cabezal, int linea) { (void)cinta; (void)cabezal; (void)linea; }
    virtual void maquinaFin(bool acepto, const std::string& cinta) { (void)acepto; (void)cinta; }

    // Se consulta antes de cada cuádruplo; true: el usuario pidió salir
    virtual bool detenido() { return false; }
};
