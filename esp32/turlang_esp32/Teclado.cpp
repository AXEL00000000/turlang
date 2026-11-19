#include "Teclado.h"

#include "Pantalla.h"

namespace {

const int MENSAJE_Y = 26;
const int CAMPO_X = 4, CAMPO_Y = 44, CAMPO_W = 312, CAMPO_H = 27;
const size_t MAXIMO = 40;                 // caracteres que se pueden escribir
const size_t VISIBLES = 25;               // caracteres de tamaño 2 que caben en el campo
const int FILAS = 4, COLUMNAS = 10;
const int TECLA_W = 31, TECLA_H = 38;
const int TECLADO_X = (ANCHO - COLUMNAS * TECLA_W) / 2, TECLADO_Y = 80, FILA_ALTO = 40;

// Caracteres por fila; en las filas 3 y 4 los lugares de las teclas especiales
// no se usan.
const char* const LETRAS[FILAS] = {"1234567890", "qwertyuiop", "asdfghjkl ", " zxcvbnm  "};
const char* const SIMBOLOS_FILA2 = "-+*/=<>!,.";
const char* const SIMBOLOS_FILA3 = "()_:?#@\"' ";
const char SIMBOLO_FILA4 = '%';           // en el lugar de la z

}  // namespace

Teclado::Tecla Teclado::tecla(int fila, int columna) const {
    if (fila == 2 && columna == 9) return {0, BORRAR};
    if (fila == 3 && columna == 0) return {0, MOD};
    if (fila == 3 && columna == 8) return {0, ESPACIO};
    if (fila == 3 && columna == 9) return {0, OK};

    char c = LETRAS[fila][columna];
    if (modo == MAYUSCULAS && c >= 'a' && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    } else if (modo == SIMBOLOS) {
        if (fila == 1) c = SIMBOLOS_FILA2[columna];
        if (fila == 2) c = SIMBOLOS_FILA3[columna];
        if (fila == 3 && columna == 1) c = SIMBOLO_FILA4;
    }
    return {c, NINGUNA};
}

void Teclado::dibujarTecla(int fila, int columna, bool presionada) {
    int x = TECLADO_X + columna * TECLA_W;
    int y = TECLADO_Y + fila * FILA_ALTO;
    Tecla t = tecla(fila, columna);
    uint16_t fondo = presionada ? COLOR_AMBAR : COLOR_GRIS_OSCURO;
    switch (t.especial) {
        case BORRAR:  boton(x + 1, y, TECLA_W - 2, TECLA_H, "Borr", fondo, 1); break;
        case MOD:     boton(x + 1, y, TECLA_W - 2, TECLA_H, "Mod", fondo, 1); break;
        case ESPACIO: boton(x + 1, y, TECLA_W - 2, TECLA_H, "Esp", fondo, 1); break;
        case OK:      boton(x + 1, y, TECLA_W - 2, TECLA_H, "OK", presionada ? COLOR_AMBAR : COLOR_AZUL, 1); break;
        case NINGUNA: {
            char etiqueta[2] = {t.caracter, '\0'};
            boton(x + 1, y, TECLA_W - 2, TECLA_H, etiqueta, fondo, 2);
            break;
        }
    }
}

void Teclado::dibujarTeclas() {
    for (int f = 0; f < FILAS; ++f) {
        for (int c = 0; c < COLUMNAS; ++c) {
            dibujarTecla(f, c, false);
        }
    }
}

void Teclado::dibujarCampo() {
    tft.fillRect(CAMPO_X + 1, CAMPO_Y + 1, CAMPO_W - 2, CAMPO_H - 2, COLOR_FONDO);
    std::string visible = texto.size() > VISIBLES ? texto.substr(texto.size() - VISIBLES) : texto;
    escribir(CAMPO_X + 4, CAMPO_Y + 6, visible, COLOR_TEXTO, 2);
    int cursorX = CAMPO_X + 4 + 12 * (int)visible.size();
    tft.fillRect(cursorX, CAMPO_Y + 5, 2, 17, COLOR_AMBAR);
}

std::string Teclado::leer(const std::string& mensaje, BarraSuperior& barra) {
    texto.clear();
    modo = MINUSCULAS;

    tft.fillRect(0, MENSAJE_Y - 1, ANCHO, ALTO - MENSAJE_Y + 1, COLOR_FONDO);
    escribir(4, MENSAJE_Y + 4, recortar(aAscii(mensaje), 52), COLOR_TEXTO, 1);
    tft.drawRect(CAMPO_X, CAMPO_Y, CAMPO_W, CAMPO_H, COLOR_GRIS);
    dibujarCampo();
    dibujarTeclas();

    while (!barra.salirPedido()) {
        int x, y;
        if (!leerToque(x, y)) {
            delay(10);
            continue;
        }
        if (barra.atender(x, y, false)) continue;   // Vel y Salir; Pausa no aplica aquí

        int columna = (x - TECLADO_X) / TECLA_W;
        int fila = (y - TECLADO_Y) / FILA_ALTO;
        if (x < TECLADO_X || y < TECLADO_Y || columna >= COLUMNAS || fila >= FILAS) continue;
        if ((y - TECLADO_Y) % FILA_ALTO >= TECLA_H) continue;   // entre dos filas

        // La tecla se marca en ámbar mientras se presiona.
        dibujarTecla(fila, columna, true);
        esperarSoltar();
        dibujarTecla(fila, columna, false);

        Tecla t = tecla(fila, columna);
        switch (t.especial) {
            case OK:
                return texto;
            case MOD:
                modo = modo == MINUSCULAS ? MAYUSCULAS : modo == MAYUSCULAS ? SIMBOLOS : MINUSCULAS;
                dibujarTeclas();
                break;
            case BORRAR:
                if (!texto.empty()) texto.erase(texto.size() - 1);
                dibujarCampo();
                break;
            case ESPACIO:
                if (texto.size() < MAXIMO) texto += ' ';
                dibujarCampo();
                break;
            case NINGUNA:
                if (texto.size() < MAXIMO) texto += t.caracter;
                dibujarCampo();
                break;
        }
    }
    return texto;
}
