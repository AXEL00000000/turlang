#include "Pantalla.h"

#include <SPI.h>

#include "Hardware.h"

Adafruit_ILI9341 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);   // bus SPI principal
XPT2046_Touchscreen ts(PIN_TOUCH_CS, PIN_TOUCH_IRQ);

namespace {

// Segundo bus, para el táctil cuando no comparte los pines de la pantalla.
// Cuando se agregue la SD, se elige su bus aquí, como en prueba_hardware.
SPIClass busHSPI(HSPI);

const int PRESION_MINIMA = 300;   // táctil resistivo: hay que presionar firme

bool mismosPines(int sck1, int mosi1, int miso1, int sck2, int mosi2, int miso2) {
    return sck1 == sck2 && mosi1 == mosi2 && miso1 == miso2;
}

// Valores crudos de un toque nuevo (promedio de varias lecturas).
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

}  // namespace

void iniciarPantalla() {
    // Los CS en alto antes de iniciar cualquier dispositivo.
    pinMode(PIN_TFT_CS, OUTPUT);
    pinMode(PIN_TOUCH_CS, OUTPUT);
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_TFT_CS, HIGH);
    digitalWrite(PIN_TOUCH_CS, HIGH);
    digitalWrite(PIN_SD_CS, HIGH);

    SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI);
    SPIClass* busTactil = &SPI;
    if (!mismosPines(PIN_TOUCH_SCK, PIN_TOUCH_MOSI, PIN_TOUCH_MISO,
                     PIN_TFT_SCK, PIN_TFT_MOSI, PIN_TFT_MISO)) {
        busHSPI.begin(PIN_TOUCH_SCK, PIN_TOUCH_MISO, PIN_TOUCH_MOSI);
        busTactil = &busHSPI;
    }

    tft.begin();
    tft.setRotation(ROTACION);
    tft.fillScreen(COLOR_FONDO);

    ts.begin(*busTactil);
    ts.setRotation(ROTACION);
}

bool leerToque(int& x, int& y) {
    int crudoX, crudoY;
    if (!leerCrudo(crudoX, crudoY)) {
        return false;
    }
    if (TOUCH_INTERCAMBIAR_XY) {
        int t = crudoX;
        crudoX = crudoY;
        crudoY = t;
    }
    x = escalar(crudoX, TOUCH_X_MIN, TOUCH_X_MAX, ANCHO, TOUCH_INVERTIR_X);
    y = escalar(crudoY, TOUCH_Y_MIN, TOUCH_Y_MAX, ALTO, TOUCH_INVERTIR_Y);
    return true;
}

bool tocando() {
    return ts.touched();
}

void esperarSoltar() {
    unsigned long inicio = millis();
    while (tocando() && millis() - inicio < 2000) {
        delay(10);
    }
}

bool tocado(int x, int y, int w, int h, int tx, int ty) {
    return tx >= x && tx < x + w && ty >= y && ty < y + h;
}

void boton(int x, int y, int w, int h, const char* texto, uint16_t fondo, int tamano) {
    tft.fillRoundRect(x, y, w, h, 4, fondo);
    tft.drawRoundRect(x, y, w, h, 4, COLOR_GRIS);
    int anchoTexto = (int)strlen(texto) * 6 * tamano;
    tft.setTextSize(tamano);
    tft.setTextColor(COLOR_TEXTO);
    tft.setCursor(x + (w - anchoTexto) / 2, y + (h - 8 * tamano) / 2);
    tft.print(texto);
}

void escribir(int x, int y, const std::string& texto, uint16_t color, int tamano) {
    tft.setTextSize(tamano);
    tft.setTextColor(color);
    tft.setCursor(x, y);
    tft.print(texto.c_str());
}

std::string aAscii(const std::string& texto) {
    std::string r;
    r.reserve(texto.size());
    size_t i = 0;
    while (i < texto.size()) {
        unsigned char c = (unsigned char)texto[i];
        if (c < 0x80) {
            r += c == '\t' ? ' ' : (c < 32 || c == 127 ? '?' : (char)c);
            ++i;
            continue;
        }
        // Secuencia UTF-8: el byte inicial y sus bytes de continuación.
        size_t largo = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
        char letra = '?';
        if (c == 0xC3 && i + 1 < texto.size()) {
            switch ((unsigned char)texto[i + 1]) {
                case 0xA1: letra = 'a'; break;   // á
                case 0xA9: letra = 'e'; break;   // é
                case 0xAD: letra = 'i'; break;   // í
                case 0xB3: letra = 'o'; break;   // ó
                case 0xBA: case 0xBC: letra = 'u'; break;   // ú ü
                case 0x81: letra = 'A'; break;   // Á
                case 0x89: letra = 'E'; break;   // É
                case 0x8D: letra = 'I'; break;   // Í
                case 0x93: letra = 'O'; break;   // Ó
                case 0x9A: case 0x9C: letra = 'U'; break;   // Ú Ü
                case 0xB1: letra = 'n'; break;   // ñ
                case 0x91: letra = 'N'; break;   // Ñ
                default: break;
            }
        }
        r += letra;
        ++i;
        for (size_t k = 1; k < largo && i < texto.size() &&
                           ((unsigned char)texto[i] & 0xC0) == 0x80; ++k) {
            ++i;
        }
    }
    return r;
}

std::string recortar(const std::string& texto, size_t maximo) {
    if (texto.size() <= maximo) return texto;
    if (maximo == 0) return "";
    return texto.substr(0, maximo - 1) + "~";
}
