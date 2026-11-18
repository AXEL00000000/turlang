// prueba_hardware.ino — prueba de pantalla, táctil y SD en el mismo bus SPI
//
// Menú con tres pruebas: 1. Pantalla, 2. SD, 3. Táctil (calibración y dibujo).
// Cada resultado se escribe también en el Monitor serie (115200).
// Los textos de la pantalla van sin acentos: la fuente de Adafruit GFX no los tiene.

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#include "Hardware.h"

Adafruit_ILI9341 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
XPT2046_Touchscreen ts(PIN_TOUCH_CS, PIN_TOUCH_IRQ);

const int ANCHO = 320;
const int ALTO = 240;

// Colores (RGB565): ámbar para lo actual, azul para el resultado.
const uint16_t COLOR_AMBAR = 0xFD20;
const uint16_t COLOR_AZUL = 0x041F;
const uint16_t COLOR_GRIS = 0x7BEF;

const int PRESION_MINIMA = 300;

// Botones del menú y botón Volver (esquina superior derecha).
const int BOTON_X = 20, BOTON_W = 280, BOTON_H = 50;
const int BOTON1_Y = 45, BOTON2_Y = 110, BOTON3_Y = 175;
const int VOLVER_X = 240, VOLVER_Y = 0, VOLVER_W = 80, VOLVER_H = 40;

bool sdLista = false;

// Calibración en uso: empieza con la de Hardware.h; la prueba 3 la reemplaza.
int calXMin = TOUCH_X_MIN;
int calXMax = TOUCH_X_MAX;
int calYMin = TOUCH_Y_MIN;
int calYMax = TOUCH_Y_MAX;
bool calInvertirX = TOUCH_INVERTIR_X;
bool calInvertirY = TOUCH_INVERTIR_Y;
bool calIntercambiar = TOUCH_INTERCAMBIAR_XY;

// ------------------------------------------------------------------- táctil

// Valores crudos de un toque nuevo (promedio de varias lecturas). Ignora presión
// baja y no acepta otro toque hasta que se suelte el dedo.
bool leerCrudo(int& crudoX, int& crudoY) {
    static bool presionado = false;
    if (!ts.touched()) {
        presionado = false;
        return false;
    }
    if (presionado) {
        return false;
    }
    long sumaX = 0, sumaY = 0;
    int validas = 0;
    for (int i = 0; i < 8; ++i) {
        TS_Point p = ts.getPoint();
        if (p.z >= PRESION_MINIMA) {
            sumaX += p.x;
            sumaY += p.y;
            ++validas;
        }
        delay(5);
    }
    if (validas == 0) {
        return false;
    }
    presionado = true;
    crudoX = sumaX / validas;
    crudoY = sumaY / validas;
    return true;
}

int escalar(int valor, int minimo, int maximo, int tamano, bool invertir) {
    if (maximo <= minimo) return 0;
    long r = (long)(valor - minimo) * (tamano - 1) / (maximo - minimo);
    if (r < 0) r = 0;
    if (r > tamano - 1) r = tamano - 1;
    return invertir ? (tamano - 1 - (int)r) : (int)r;
}

// Toque en coordenadas de pantalla, con la calibración en uso.
bool leerToque(int& x, int& y) {
    int crudoX, crudoY;
    if (!leerCrudo(crudoX, crudoY)) {
        return false;
    }
    if (calIntercambiar) {
        int t = crudoX;
        crudoX = crudoY;
        crudoY = t;
    }
    x = escalar(crudoX, calXMin, calXMax, ANCHO, calInvertirX);
    y = escalar(crudoY, calYMin, calYMax, ALTO, calInvertirY);
    return true;
}

// ------------------------------------------------------------------ botones

void boton(int x, int y, int w, int h, const char* texto) {
    tft.fillRoundRect(x, y, w, h, 6, COLOR_AZUL);
    tft.drawRoundRect(x, y, w, h, 6, ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    int anchoTexto = strlen(texto) * 12;
    tft.setCursor(x + (w - anchoTexto) / 2, y + (h - 16) / 2);
    tft.print(texto);
}

bool tocado(int x, int y, int w, int h, int tx, int ty) {
    return tx >= x && tx < x + w && ty >= y && ty < y + h;
}

void botonVolver() {
    boton(VOLVER_X, VOLVER_Y, VOLVER_W, VOLVER_H, "Volver");
}

// Espera hasta que se toque el botón Volver.
void esperarVolver() {
    int x, y;
    while (true) {
        if (leerToque(x, y) && tocado(VOLVER_X, VOLVER_Y, VOLVER_W, VOLVER_H, x, y)) {
            return;
        }
        delay(10);
    }
}

void titulo(const char* texto) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(8, 12);
    tft.print(texto);
}

// ------------------------------------------------------- prueba 1: pantalla

void pruebaPantalla() {
    Serial.println("Prueba 1: pantalla");
    titulo("1. Pantalla");
    botonVolver();

    // Colores de lo actual y del resultado.
    tft.fillRect(10, 48, 140, 30, COLOR_AMBAR);
    tft.fillRect(170, 48, 140, 30, COLOR_AZUL);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(16, 59);
    tft.print("Actual (ambar)");
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(176, 59);
    tft.print("Resultado (azul)");

    // Juego de caracteres en tamaño 1 y 2.
    const char* caracteres = "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 +-*/<>=!(){};,\"";
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(1, 86);
    tft.print(caracteres);
    tft.setTextSize(2);
    tft.setCursor(0, 100);
    tft.print(caracteres);

    // Cinta de 10 casillas con 1 1 0 X _ y el cabezal bajo la tercera.
    const char simbolos[10] = {'1', '1', '0', 'X', '_', ' ', ' ', ' ', ' ', ' '};
    const int casilla = 28;
    const int cintaX = (ANCHO - 10 * casilla) / 2;
    const int cintaY = 158;
    tft.setTextSize(2);
    for (int i = 0; i < 10; ++i) {
        int x = cintaX + i * casilla;
        tft.drawRect(x, cintaY, casilla, casilla, ILI9341_WHITE);
        tft.setCursor(x + (casilla - 12) / 2 + 1, cintaY + (casilla - 16) / 2 + 1);
        tft.print(simbolos[i]);
    }
    int centro = cintaX + 2 * casilla + casilla / 2;
    int puntaY = cintaY + casilla + 4;
    tft.fillTriangle(centro, puntaY, centro - 9, puntaY + 14, centro + 9, puntaY + 14, COLOR_AMBAR);

    Serial.println("  Dibujado: titulo, colores ambar y azul, texto en tamano 1 y 2, cinta y cabezal");
    esperarVolver();
}

// ------------------------------------------------------------- prueba 2: SD

bool terminaEnTurc(String nombre) {
    nombre.toLowerCase();
    return nombre.endsWith(".turc");
}

void pruebaSD() {
    Serial.println("Prueba 2: SD");
    titulo("2. SD");
    botonVolver();
    tft.setTextSize(2);
    tft.setCursor(8, 50);

    if (!sdLista) {
        tft.setTextColor(ILI9341_RED);
        tft.print("SD no detectada");
        Serial.println("  SD no detectada");
        esperarVolver();
        return;
    }

    uint8_t tipo = SD.cardType();
    const char* nombreTipo = tipo == CARD_MMC ? "MMC" : tipo == CARD_SD ? "SDSC"
                           : tipo == CARD_SDHC ? "SDHC" : "desconocido";
    uint32_t megas = (uint32_t)(SD.cardSize() / (1024ULL * 1024ULL));

    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(8, 50);
    tft.printf("Tipo: %s   Tamano: %lu MB", nombreTipo, (unsigned long)megas);
    Serial.printf("  Tipo: %s, tamaño: %lu MB\n", nombreTipo, (unsigned long)megas);

    // Archivos .turc de la raíz (hasta 10).
    tft.setCursor(8, 66);
    tft.print("Archivos .turc:");
    Serial.println("  Archivos .turc:");
    String primero = "";
    int cuantos = 0;
    File raiz = SD.open("/");
    if (raiz) {
        File archivo = raiz.openNextFile();
        while (archivo && cuantos < 10) {
            String nombre = archivo.name();
            if (nombre.startsWith("/")) nombre = nombre.substring(1);
            if (!archivo.isDirectory() && terminaEnTurc(nombre)) {
                tft.setCursor(20, 80 + cuantos * 11);
                tft.print(nombre);
                Serial.println("    " + nombre);
                if (primero.length() == 0) primero = nombre;
                ++cuantos;
            }
            archivo.close();
            archivo = raiz.openNextFile();
        }
        raiz.close();
    }
    if (cuantos == 0) {
        tft.setCursor(20, 80);
        tft.print("(ninguno)");
        Serial.println("    (ninguno)");
    }

    // Primera línea del primer .turc: debe ser TURC<tab>1.
    if (primero.length() > 0) {
        String linea = "";
        File f = SD.open("/" + primero);
        if (f) {
            while (f.available()) {
                char c = f.read();
                if (c == '\n') break;
                if (c != '\r') linea += c;
            }
            f.close();
        }
        bool correcta = linea == "TURC\t1";
        String visible = linea;
        visible.replace("\t", " <tab> ");
        tft.setCursor(8, 200);
        tft.setTextColor(ILI9341_WHITE);
        tft.print("Primera linea de " + primero + ":");
        tft.setCursor(8, 214);
        tft.setTextSize(2);
        tft.setTextColor(correcta ? ILI9341_GREEN : ILI9341_RED);
        tft.print(visible);
        Serial.println("  Primera línea de " + primero + ": " + visible +
                       (correcta ? "  (correcta)" : "  (no es TURC 1)"));
    }
    esperarVolver();
}

// ---------------------------------------------------------- prueba 3: táctil

void cruz(int x, int y, uint16_t color) {
    tft.drawLine(x - 10, y, x + 10, y, color);
    tft.drawLine(x, y - 10, x, y + 10, color);
}

// Espera un toque y regresa sus valores crudos.
void esperarCrudo(int& crudoX, int& crudoY) {
    while (!leerCrudo(crudoX, crudoY)) {
        delay(10);
    }
}

void calibrar() {
    // Cruces a 20 píxeles del borde: arriba-izq, arriba-der, abajo-der, abajo-izq.
    const int px[4] = {20, ANCHO - 21, ANCHO - 21, 20};
    const int py[4] = {20, 20, ALTO - 21, ALTO - 21};
    int rx[4], ry[4];

    for (int i = 0; i < 4; ++i) {
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(60, 100);
        tft.print("Toca la cruz");
        tft.setCursor(60, 124);
        tft.printf("(%d de 4)", i + 1);
        cruz(px[i], py[i], COLOR_AMBAR);
        esperarCrudo(rx[i], ry[i]);
        Serial.printf("  Cruz %d (%d, %d): crudo x=%d y=%d\n", i + 1, px[i], py[i], rx[i], ry[i]);
    }

    // ¿Qué eje crudo sigue al eje horizontal de la pantalla?
    long izqX = (rx[0] + rx[3]) / 2, derX = (rx[1] + rx[2]) / 2;
    long izqY = (ry[0] + ry[3]) / 2, derY = (ry[1] + ry[2]) / 2;
    bool intercambiar = labs(derY - izqY) > labs(derX - izqX);

    // a: eje crudo horizontal; b: vertical (después del intercambio).
    int a[4], b[4];
    for (int i = 0; i < 4; ++i) {
        a[i] = intercambiar ? ry[i] : rx[i];
        b[i] = intercambiar ? rx[i] : ry[i];
    }
    float izquierda = (a[0] + a[3]) / 2.0f, derecha = (a[1] + a[2]) / 2.0f;
    float arriba = (b[0] + b[1]) / 2.0f, abajo = (b[2] + b[3]) / 2.0f;

    // Se extrapola de las cruces (a 20 px) a los bordes de la pantalla.
    float porPixelX = (derecha - izquierda) / (ANCHO - 1 - 40);
    float porPixelY = (abajo - arriba) / (ALTO - 1 - 40);
    float bordeIzq = izquierda - 20 * porPixelX, bordeDer = derecha + 20 * porPixelX;
    float bordeArr = arriba - 20 * porPixelY, bordeAba = abajo + 20 * porPixelY;

    calIntercambiar = intercambiar;
    calInvertirX = izquierda > derecha;
    calInvertirY = arriba > abajo;
    calXMin = (int)min(bordeIzq, bordeDer);
    calXMax = (int)max(bordeIzq, bordeDer);
    calYMin = (int)min(bordeArr, bordeAba);
    calYMax = (int)max(bordeArr, bordeAba);
}

void mostrarConstantes() {
    char lineas[7][48];
    snprintf(lineas[0], sizeof lineas[0], "const int TOUCH_X_MIN = %d;", calXMin);
    snprintf(lineas[1], sizeof lineas[1], "const int TOUCH_X_MAX = %d;", calXMax);
    snprintf(lineas[2], sizeof lineas[2], "const int TOUCH_Y_MIN = %d;", calYMin);
    snprintf(lineas[3], sizeof lineas[3], "const int TOUCH_Y_MAX = %d;", calYMax);
    snprintf(lineas[4], sizeof lineas[4], "const bool TOUCH_INVERTIR_X = %s;", calInvertirX ? "true" : "false");
    snprintf(lineas[5], sizeof lineas[5], "const bool TOUCH_INVERTIR_Y = %s;", calInvertirY ? "true" : "false");
    snprintf(lineas[6], sizeof lineas[6], "const bool TOUCH_INTERCAMBIAR_XY = %s;", calIntercambiar ? "true" : "false");

    titulo("Calibracion");
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    Serial.println("  Constantes para Hardware.h:");
    for (int i = 0; i < 7; ++i) {
        tft.setCursor(8, 50 + i * 14);
        tft.print(lineas[i]);
        Serial.println(lineas[i]);
    }
    tft.setCursor(8, 156);
    tft.print("Tambien estan en el Monitor serie.");
    boton(100, 180, 120, 50, "Dibujar");

    int x, y;
    while (!(leerToque(x, y) && tocado(100, 180, 120, 50, x, y))) {
        delay(10);
    }
}

// En modo dibujo, Volver va arriba al centro para dejar libres las cuatro esquinas.
const int DIBUJO_VOLVER_X = (ANCHO - VOLVER_W) / 2;

void modoDibujo() {
    tft.fillScreen(ILI9341_BLACK);
    boton(DIBUJO_VOLVER_X, VOLVER_Y, VOLVER_W, VOLVER_H, "Volver");
    // Marcas de referencia: las cuatro esquinas (a 20 px del borde) y el centro.
    cruz(20, 20, COLOR_GRIS);
    cruz(ANCHO - 21, 20, COLOR_GRIS);
    cruz(ANCHO - 21, ALTO - 21, COLOR_GRIS);
    cruz(20, ALTO - 21, COLOR_GRIS);
    cruz(ANCHO / 2, ALTO / 2, COLOR_GRIS);
    Serial.println("  Modo dibujo (toca Volver para salir)");

    int x, y;
    while (true) {
        if (leerToque(x, y)) {
            if (tocado(DIBUJO_VOLVER_X, VOLVER_Y, VOLVER_W, VOLVER_H, x, y)) return;
            tft.fillCircle(x, y, 3, ILI9341_GREEN);
            Serial.printf("  Toque en (%d, %d)\n", x, y);
        }
        delay(10);
    }
}

void pruebaTactil() {
    Serial.println("Prueba 3: táctil");
    calibrar();
    mostrarConstantes();
    modoDibujo();
}

// --------------------------------------------------------------------- menú

void dibujarMenu() {
    titulo("Prueba de hardware");
    boton(BOTON_X, BOTON1_Y, BOTON_W, BOTON_H, "1. Pantalla");
    boton(BOTON_X, BOTON2_Y, BOTON_W, BOTON_H, "2. SD");
    boton(BOTON_X, BOTON3_Y, BOTON_W, BOTON_H, "3. Tactil");
}

void setup() {
    // 1. Monitor serie (solo para depurar).
    Serial.begin(115200);

    // 2. Los tres CS en alto antes de iniciar cualquier dispositivo.
    pinMode(PIN_TFT_CS, OUTPUT);
    pinMode(PIN_TOUCH_CS, OUTPUT);
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_TFT_CS, HIGH);
    digitalWrite(PIN_TOUCH_CS, HIGH);
    digitalWrite(PIN_SD_CS, HIGH);

    // 3. Bus SPI compartido.
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);

    // 4. Pantalla.
    tft.begin();
    tft.setRotation(ROTACION);

    // 5. Táctil.
    ts.begin();
    ts.setRotation(ROTACION);

    // 6. SD (si falla, la prueba sigue sin SD).
    sdLista = SD.begin(PIN_SD_CS, SPI, 4000000);

    Serial.println();
    Serial.println("Prueba de hardware TurLang");
    Serial.println(sdLista ? "SD lista" : "SD no detectada");

    dibujarMenu();
}

void loop() {
    int x, y;
    if (!leerToque(x, y)) {
        delay(10);
        return;
    }
    if (tocado(BOTON_X, BOTON1_Y, BOTON_W, BOTON_H, x, y)) {
        pruebaPantalla();
    } else if (tocado(BOTON_X, BOTON2_Y, BOTON_W, BOTON_H, x, y)) {
        pruebaSD();
    } else if (tocado(BOTON_X, BOTON3_Y, BOTON_W, BOTON_H, x, y)) {
        pruebaTactil();
    } else {
        return;
    }
    dibujarMenu();
}
