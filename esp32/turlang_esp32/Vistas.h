#pragma once

// Barra superior, vista de programa y vista de máquina (pantalla de 320 x 240).

#include <Arduino.h>
#include <string>
#include <vector>

#include "src/runtime/Turc.h"

// ------------------------------------------------------------ barra superior

// y = 0 a 23: nombre del programa y los botones Pausa/Seguir, Vel y Salir.
class BarraSuperior {
public:
    void reiniciar();                          // sin pausa ni salida; la velocidad se conserva
    void dibujar(const std::string& nombre);   // barra con Pausa, Vel y Salir
    void dibujarFin();                         // en lugar de los botones: Repetir y Menu

    // Espera ms milisegundos revisando el táctil cada ~15 ms. Con Pausa no regresa
    // hasta Seguir o Salir; con Salir regresa enseguida.
    void esperar(int ms);

    // Revisa el táctil sin esperar (como mucho cada 15 ms) y atiende los botones.
    void revisar();

    // Atiende un toque. conPausa = false: el botón Pausa no hace nada (teclado).
    // Regresa true si el toque cayó en la barra.
    bool atender(int x, int y, bool conPausa);

    // Después de dibujarFin: espera Repetir (true) o Menu (false).
    bool esperarFin();

    int esperaLinea() const;   // espera por línea del programa, según la velocidad
    int esperaPaso() const;    // espera por paso de máquina
    bool salirPedido() const;

private:
    std::string nombre;
    bool pausa = false;
    bool salir = false;
    int velocidad = 1;         // 0 Lento, 1 Normal, 2 Rápido
    unsigned long ultimaRevision = 0;

    void dibujarBotones();
};

// --------------------------------------------------------- vista de programa

class VistaPrograma {
public:
    // Prepara la vista para un programa: variables en ---, consola vacía, sin flecha.
    void cargar(const Compilado& compilado);

    void mostrar();    // dibuja las tres zonas completas y queda visible
    void ocultar();    // otra vista ocupa la pantalla; los cambios se guardan sin dibujar

    void lineaActual(int linea);
    void variableCambio(const std::string& nombre, const std::string& valor);
    void imprimir(const std::string& texto, uint16_t color);   // también al Monitor serie

    std::string ultimaLineaConsola() const;
    const std::vector<std::string>& fuente() const;            // líneas ya en ASCII

private:
    struct LineaConsola {
        std::string texto;
        uint16_t color;
    };

    std::vector<std::string> lineas;        // fuente en ASCII
    std::vector<std::string> nombres;       // variables en el orden de VARS
    std::vector<std::string> valores;       // "---" antes de recibir valor
    std::vector<LineaConsola> consola;      // últimas 6 líneas ya partidas
    std::string ultimaImpresa;
    int actual = 0;                         // línea de la flecha (0: ninguna)
    int primera = 1;                        // primera línea visible del código
    int cambiada = -1;                      // variable dibujada en azul
    bool visible = false;

    void dibujarCodigo();
    void dibujarLineaCodigo(int linea);
    void dibujarVariables();
    void dibujarVariable(int indice);
    void dibujarConsola();
    bool ajustarVentana();                  // true si la ventana del código se movió
};

// ---------------------------------------------------------- vista de máquina

class VistaMaquina {
public:
    void inicio(const std::string& nombre, const std::string& cinta, int cabezal, int limite,
                const std::vector<std::string>& fuente);
    void paso(const std::string& cinta, int cabezal, int linea);
    void fin(bool acepto, const std::string& cinta);

    // Espera ms milisegundos o hasta un toque (los botones de la barra se atienden).
    void esperarResultado(BarraSuperior& barra, int ms);

private:
    const std::vector<std::string>* fuente = nullptr;
    std::string nombre;
    std::string estado;
    std::string regla;
    int limite = 0;
    int pasos = 0;
    char mostrado[11];                      // símbolo dibujado en cada casilla

    void dibujarEncabezado();
    void dibujarRegla();
    void dibujarCinta(const std::string& cinta, int cabezal);
    void dibujarCasilla(int k, char simbolo);
};
