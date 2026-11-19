#pragma once

// Vista de teclado: lee una línea de texto con un teclado en pantalla.

#include <string>

#include "Vistas.h"

class Teclado {
public:
    // Dibuja el teclado debajo de la barra y espera hasta OK. Muestra arriba el
    // mensaje dado (la última línea de la consola). Si se toca Salir en la barra,
    // regresa lo escrito hasta ese momento.
    std::string leer(const std::string& mensaje, BarraSuperior& barra);

private:
    enum Modo { MINUSCULAS, MAYUSCULAS, SIMBOLOS };
    enum Especial { NINGUNA, BORRAR, MOD, ESPACIO, OK };

    struct Tecla {
        char caracter;      // si no es especial
        Especial especial;
    };

    Modo modo = MINUSCULAS;
    std::string texto;

    Tecla tecla(int fila, int columna) const;
    void dibujarTeclas();
    void dibujarTecla(int fila, int columna, bool presionada);
    void dibujarCampo();
};
