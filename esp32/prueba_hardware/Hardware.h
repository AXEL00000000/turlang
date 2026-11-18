#pragma once

// Bus SPI compartido (VSPI)
const int PIN_SCK  = 18;
const int PIN_MOSI = 23;
const int PIN_MISO = 19;

// Pantalla ILI9341
const int PIN_TFT_CS  = 5;
const int PIN_TFT_DC  = 16;
const int PIN_TFT_RST = 17;

// Táctil XPT2046
const int PIN_TOUCH_CS  = 4;
const int PIN_TOUCH_IRQ = 21;

// Tarjeta SD
const int PIN_SD_CS = 22;

// Orientación: horizontal, 320 x 240
const int ROTACION = 1;

// Calibración del táctil (valores crudos); se llenan con la prueba 3
const int TOUCH_X_MIN = 300;
const int TOUCH_X_MAX = 3800;
const int TOUCH_Y_MIN = 300;
const int TOUCH_Y_MAX = 3800;
const bool TOUCH_INVERTIR_X = false;
const bool TOUCH_INVERTIR_Y = false;
const bool TOUCH_INTERCAMBIAR_XY = false;
