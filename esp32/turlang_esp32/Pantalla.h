#pragma once

// Pantalla ILI9341 y táctil XPT2046: objetos, colores y funciones básicas.
// Los pines están en Hardware.h.

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <string>

extern Adafruit_ILI9341 tft;
extern XPT2046_Touchscreen ts;

const int ANCHO = 320;
const int ALTO = 240;

// Colores (RGB565)
const uint16_t COLOR_FONDO = ILI9341_BLACK;
const uint16_t COLOR_TEXTO = ILI9341_WHITE;
const uint16_t COLOR_AMBAR = 0xFD20;          // lo actual
const uint16_t COLOR_AMBAR_OSCURO = 0x5180;   // fondo de la línea actual
const uint16_t COLOR_AZUL = 0x041F;           // el resultado
const uint16_t COLOR_GRIS = 0x7BEF;           // bordes y textos secundarios
const uint16_t COLOR_GRIS_OSCURO = 0x2945;    // fondo de botones y teclas
const uint16_t COLOR_ROJO = ILI9341_RED;      // errores

// Inicia los buses SPI, la pantalla y el táctil. La pantalla usa el bus SPI
// principal; el táctil lo comparte si tiene sus mismos pines o usa HSPI.
void iniciarPantalla();

// Un toque nuevo en coordenadas de pantalla. No acepta otro hasta que se suelte.
bool leerToque(int& x, int& y);

// true mientras el dedo sigue sobre la pantalla.
bool tocando();

// Espera a que se suelte el dedo (máximo 2 s).
void esperarSoltar();

bool tocado(int x, int y, int w, int h, int tx, int ty);

// Botón con el texto centrado.
void boton(int x, int y, int w, int h, const char* texto, uint16_t fondo, int tamano);

// Texto en la posición dada, con fondo transparente.
void escribir(int x, int y, const std::string& texto, uint16_t color, int tamano);

// Solo ASCII: vocales con acento y ñ sin acento; otro carácter fuera de ASCII, '?'.
std::string aAscii(const std::string& texto);

// Si el texto pasa de maximo caracteres, se recorta y termina en '~'.
std::string recortar(const std::string& texto, size_t maximo);
