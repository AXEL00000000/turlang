#include "Menu.h"

#include "Pantalla.h"
#include "Programas.h"

namespace {

const int POR_PAGINA = 6;
const int LISTA_X = 2, LISTA_W = ANCHO - 4, LISTA_Y = 26, BOTON_ALTO = 34;
const int SD_Y = LISTA_Y + POR_PAGINA * BOTON_ALTO + 1;
const int ANTERIOR_X = 150, SIGUIENTE_X = 236, PAGINA_W = 82, PAGINA_Y = 1, PAGINA_H = 22;

void dibujarMenu(const std::vector<ProgramaDisponible>& programas, int pagina, int paginas) {
    tft.fillScreen(COLOR_FONDO);
    escribir(6, 5, "TurLang", COLOR_TEXTO, 2);
    if (paginas > 1) {
        boton(ANTERIOR_X, PAGINA_Y, PAGINA_W, PAGINA_H, "Anterior",
              pagina > 0 ? COLOR_GRIS_OSCURO : COLOR_FONDO, 1);
        boton(SIGUIENTE_X, PAGINA_Y, PAGINA_W, PAGINA_H, "Siguiente",
              pagina < paginas - 1 ? COLOR_GRIS_OSCURO : COLOR_FONDO, 1);
    }
    for (int i = 0; i < POR_PAGINA; ++i) {
        int indice = pagina * POR_PAGINA + i;
        if (indice >= (int)programas.size()) break;
        int y = LISTA_Y + i * BOTON_ALTO;
        tft.fillRoundRect(LISTA_X, y, LISTA_W, BOTON_ALTO - 2, 4, COLOR_GRIS_OSCURO);
        tft.drawRoundRect(LISTA_X, y, LISTA_W, BOTON_ALTO - 2, 4, COLOR_GRIS);
        escribir(LISTA_X + 10, y + 9, recortar(aAscii(programas[indice].nombre), 25), COLOR_TEXTO, 2);
    }
    if (programas.empty()) {
        escribir(LISTA_X + 10, LISTA_Y + 9, "(no hay programas)", COLOR_GRIS, 2);
    }
    escribir(LISTA_X + 2, SD_Y, "SD: pendiente", COLOR_GRIS, 1);
}

}  // namespace

std::vector<ProgramaDisponible> listarProgramas() {
    std::vector<ProgramaDisponible> lista;
    for (int i = 0; i < NUM_PROGRAMAS; ++i) {
        lista.push_back({PROGRAMAS[i].nombre, PROGRAMAS[i].turc});
    }
    return lista;
}

bool cargarPrograma(const ProgramaDisponible& programa, std::string& texto, std::string& error) {
    if (programa.incrustado == nullptr) {
        error = "programa no disponible";
        return false;
    }
    texto = programa.incrustado;
    return true;
}

int elegirPrograma(const std::vector<ProgramaDisponible>& programas) {
    int paginas = ((int)programas.size() + POR_PAGINA - 1) / POR_PAGINA;
    if (paginas < 1) paginas = 1;
    int pagina = 0;
    dibujarMenu(programas, pagina, paginas);
    while (true) {
        int x, y;
        if (!leerToque(x, y)) {
            delay(15);
            continue;
        }
        if (paginas > 1 && tocado(ANTERIOR_X, PAGINA_Y, PAGINA_W, PAGINA_H, x, y) && pagina > 0) {
            dibujarMenu(programas, --pagina, paginas);
            continue;
        }
        if (paginas > 1 && tocado(SIGUIENTE_X, PAGINA_Y, PAGINA_W, PAGINA_H, x, y) &&
            pagina < paginas - 1) {
            dibujarMenu(programas, ++pagina, paginas);
            continue;
        }
        for (int i = 0; i < POR_PAGINA; ++i) {
            int indice = pagina * POR_PAGINA + i;
            if (indice >= (int)programas.size()) break;
            if (tocado(LISTA_X, LISTA_Y + i * BOTON_ALTO, LISTA_W, BOTON_ALTO - 2, x, y)) {
                return indice;
            }
        }
    }
}

void mostrarError(const std::string& titulo, const std::string& mensaje) {
    tft.fillScreen(COLOR_FONDO);
    escribir(6, 5, recortar(aAscii(titulo), 26), COLOR_TEXTO, 2);
    escribir(6, 40, recortar(aAscii(mensaje), 53), COLOR_ROJO, 1);
    escribir(6, 60, "Toca la pantalla para volver al menu", COLOR_GRIS, 1);
    int x, y;
    while (!leerToque(x, y)) {
        delay(15);
    }
}
