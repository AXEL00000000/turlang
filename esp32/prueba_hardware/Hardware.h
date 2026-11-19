#pragma once

// Pines del prototipo. Para cambiar la conexión, edita solo este archivo.
//
// Formas de conectar el módulo (pantalla ILI9341 con táctil XPT2046 y ranura SD):
//
//   Señal del módulo     Directo (un cable por pin)   Protoboard (bus compartido)
//   -------------------  ---------------------------  ---------------------------
//   SCK                  18                           18
//   SDI(MOSI)            23                           23
//   SDO(MISO)            sin conectar (-1)            19 (PIN_TFT_MISO = 19)
//   CS                   5                            5
//   D/C                  16                           16
//   RESET                17                           17
//   T_CLK                25                           18
//   T_DIN                26                           23
//   T_OUT                27                           19
//   T_CS                 4                            4
//   T_IRQ                21                           21
//   SD_SCK               14                           18
//   SD_MOSI              13                           23
//   SD_MISO              33                           19
//   SD_CS                22                           22
//   VCC / LED / GND      5V / 3.3 V / GND             5V / 3.3 V / GND
//
// Cómo elige los buses el programa:
//   - La pantalla usa el bus SPI principal (VSPI) con sus pines SCK, MOSI y MISO.
//   - Si el táctil tiene los mismos SCK, MOSI y MISO que la pantalla, comparte ese
//     bus; si son distintos, usa un segundo bus (HSPI) con sus propios pines.
//   - La SD comparte el bus de la pantalla o el del táctil si sus SCK, MOSI y MISO
//     coinciden con los de alguno; si no coinciden, usa HSPI solo si el táctil no
//     lo ocupa.
//   El ESP32 solo tiene dos buses SPI libres (VSPI y HSPI). En la conexión directa
//   la pantalla y el táctil ya los ocupan, así que la SD no arranca ("SD no
//   detectada") hasta que se conecte compartiendo los pines de la pantalla o los
//   del táctil. En la conexión con protoboard los tres comparten un solo bus.
//
// Para pasar de "directo" a "protoboard": pon PIN_TFT_MISO = 19 y copia los pines
// SCK, MOSI y MISO de la pantalla (18, 23, 19) en los del táctil y los de la SD.
// Los CS, D/C, RESET y T_IRQ no cambian.

// Pantalla ILI9341
// Módulo -> GPIO: SCK -> 18, SDI(MOSI) -> 23, SDO(MISO) -> sin conectar,
//                 CS -> 5, D/C -> 16, RESET -> 17
const int PIN_TFT_SCK  = 18;
const int PIN_TFT_MOSI = 23;
const int PIN_TFT_MISO = -1;   // -1: sin conectar
const int PIN_TFT_CS   = 5;
const int PIN_TFT_DC   = 16;
const int PIN_TFT_RST  = 17;

// Táctil XPT2046
// Módulo -> GPIO: T_CLK -> 25, T_DIN -> 26, T_OUT -> 27, T_CS -> 4, T_IRQ -> 21
const int PIN_TOUCH_SCK  = 25;
const int PIN_TOUCH_MOSI = 26;
const int PIN_TOUCH_MISO = 27;
const int PIN_TOUCH_CS   = 4;
const int PIN_TOUCH_IRQ  = 21;

// Tarjeta SD (todavía sin conectar)
// Módulo -> GPIO: SD_SCK -> 14, SD_MOSI -> 13, SD_MISO -> 33, SD_CS -> 22
const int PIN_SD_SCK  = 14;
const int PIN_SD_MOSI = 13;
const int PIN_SD_MISO = 33;
const int PIN_SD_CS   = 22;

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
