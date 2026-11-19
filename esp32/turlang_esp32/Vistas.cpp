#include "Vistas.h"

#include "Pantalla.h"

namespace {

// Barra superior
const int BARRA_ALTO = 24;
const int BOTON_Y = 1, BOTON_H = 22;
const int PAUSA_X = 128, PAUSA_W = 52;
const int VEL_X = 184, VEL_W = 84;
const int SALIR_X = 272, SALIR_W = 46;
const int REPETIR_X = 184, REPETIR_W = 64;
const int MENU_X = 252, MENU_W = 66;

// Esperas según la velocidad: Lento, Normal, Rápido
const char* const NOMBRE_VELOCIDAD[3] = {"Vel: Lento", "Vel: Normal", "Vel: Rapido"};
const int ESPERA_LINEA[3] = {800, 350, 60};
const int ESPERA_PASO[3] = {500, 180, 30};

// Vista de programa
const int CODIGO_X = 0, CODIGO_W = 210, CODIGO_Y = 26;
const int RENGLON = 12, RENGLONES = 12;          // 12 renglones de 12 px: y 26 a 169
const int CODIGO_CARACTERES = 30;                // texto desde x = 30
const int CONTEXTO = 2;
const int VARS_X = 212, VARS_W = 108, VARS_Y = 26;
const int VARS_PRIMER_Y = 40, VARS_RENGLON = 11, VARS_RENGLONES = 11;
const int VARS_CARACTERES = 17;
const int CONSOLA_Y = 172, CONSOLA_RENGLON = 11, CONSOLA_LINEAS = 6;
const int CONSOLA_CARACTERES = 53;

// Vista de máquina
const int ENCABEZADO_Y = 30;
const int REGLA_Y = 48;
const int CASILLAS = 11, CASILLA_W = 28, CASILLA_H = 36;
const int CINTA_X = (ANCHO - CASILLAS * CASILLA_W) / 2, CINTA_Y = 80;
const int CABEZAL_Y = 122;
const int RESULTADO_Y = 170;
const char SIN_DIBUJAR = '\1';

std::string recortarEspacios(const std::string& t) {
    size_t inicio = t.find_first_not_of(' ');
    if (inicio == std::string::npos) return "";
    size_t fin = t.find_last_not_of(' ');
    return t.substr(inicio, fin - inicio + 1);
}

// Línea del fuente sin comentario y sin espacios a los lados.
std::string limpia(const std::string& linea) {
    size_t comentario = linea.find("//");
    return recortarEspacios(comentario == std::string::npos ? linea : linea.substr(0, comentario));
}

bool empiezaCon(const std::string& t, const char* prefijo) {
    return t.compare(0, strlen(prefijo), prefijo) == 0;
}

}  // namespace

// ============================================================ barra superior

void BarraSuperior::reiniciar() {
    pausa = false;
    salir = false;
}

void BarraSuperior::dibujar(const std::string& nombrePrograma) {
    nombre = nombrePrograma;
    tft.fillRect(0, 0, ANCHO, BARRA_ALTO, COLOR_FONDO);
    escribir(4, 8, recortar(aAscii(nombre), 20), COLOR_TEXTO, 1);
    tft.drawFastHLine(0, BARRA_ALTO, ANCHO, COLOR_GRIS);
    dibujarBotones();
}

void BarraSuperior::dibujarBotones() {
    boton(PAUSA_X, BOTON_Y, PAUSA_W, BOTON_H, pausa ? "Seguir" : "Pausa",
          pausa ? COLOR_AMBAR_OSCURO : COLOR_GRIS_OSCURO, 1);
    boton(VEL_X, BOTON_Y, VEL_W, BOTON_H, NOMBRE_VELOCIDAD[velocidad], COLOR_GRIS_OSCURO, 1);
    boton(SALIR_X, BOTON_Y, SALIR_W, BOTON_H, "Salir", COLOR_GRIS_OSCURO, 1);
}

void BarraSuperior::dibujarFin() {
    tft.fillRect(PAUSA_X, 0, ANCHO - PAUSA_X, BARRA_ALTO, COLOR_FONDO);
    boton(REPETIR_X, BOTON_Y, REPETIR_W, BOTON_H, "Repetir", COLOR_AZUL, 1);
    boton(MENU_X, BOTON_Y, MENU_W, BOTON_H, "Menu", COLOR_AZUL, 1);
}

bool BarraSuperior::atender(int x, int y, bool conPausa) {
    if (y >= BARRA_ALTO) return false;
    if (tocado(PAUSA_X, BOTON_Y, PAUSA_W, BOTON_H, x, y)) {
        if (conPausa) {
            pausa = !pausa;
            dibujarBotones();
        }
    } else if (tocado(VEL_X, BOTON_Y, VEL_W, BOTON_H, x, y)) {
        velocidad = (velocidad + 1) % 3;
        dibujarBotones();
    } else if (tocado(SALIR_X, BOTON_Y, SALIR_W, BOTON_H, x, y)) {
        salir = true;
        pausa = false;
    }
    return true;
}

void BarraSuperior::esperar(int ms) {
    unsigned long inicio = millis();
    while (!salir) {
        int x, y;
        if (leerToque(x, y)) {
            atender(x, y, true);
        }
        if (salir) return;
        if (!pausa && (long)(millis() - inicio) >= ms) return;
        delay(15);
    }
}

void BarraSuperior::revisar() {
    if (millis() - ultimaRevision < 15) return;
    ultimaRevision = millis();
    int x, y;
    if (leerToque(x, y)) {
        atender(x, y, true);
    }
    if (pausa) {
        esperar(0);   // no regresa hasta Seguir o Salir
    }
}

bool BarraSuperior::esperarFin() {
    while (true) {
        int x, y;
        if (leerToque(x, y)) {
            if (tocado(REPETIR_X, BOTON_Y, REPETIR_W, BOTON_H, x, y)) return true;
            if (tocado(MENU_X, BOTON_Y, MENU_W, BOTON_H, x, y)) return false;
        }
        delay(15);
    }
}

int BarraSuperior::esperaLinea() const { return ESPERA_LINEA[velocidad]; }
int BarraSuperior::esperaPaso() const { return ESPERA_PASO[velocidad]; }
bool BarraSuperior::salirPedido() const { return salir; }

// ========================================================= vista de programa

void VistaPrograma::cargar(const Compilado& compilado) {
    lineas.clear();
    for (const std::string& linea : compilado.fuente) {
        lineas.push_back(aAscii(linea));
    }
    nombres.clear();
    valores.clear();
    for (const VariableInfo& v : compilado.variables) {
        nombres.push_back(v.nombre);
        valores.push_back("---");
    }
    consola.clear();
    ultimaImpresa.clear();
    actual = 0;
    primera = 1;
    cambiada = -1;
    visible = false;
}

const std::vector<std::string>& VistaPrograma::fuente() const { return lineas; }

std::string VistaPrograma::ultimaLineaConsola() const { return ultimaImpresa; }

void VistaPrograma::mostrar() {
    visible = true;
    tft.fillRect(0, BARRA_ALTO + 1, ANCHO, ALTO - BARRA_ALTO - 1, COLOR_FONDO);
    tft.drawFastVLine(CODIGO_W, CODIGO_Y, RENGLONES * RENGLON, COLOR_GRIS);
    tft.drawFastHLine(0, CONSOLA_Y - 2, ANCHO, COLOR_GRIS);
    dibujarCodigo();
    dibujarVariables();
    dibujarConsola();
}

void VistaPrograma::ocultar() {
    visible = false;
}

// ---------------------------------------------------------------- código

// Deja la línea actual visible con CONTEXTO líneas arriba y abajo.
bool VistaPrograma::ajustarVentana() {
    int anterior = primera;
    int total = (int)lineas.size();
    if (actual > 0) {
        if (actual - CONTEXTO < primera) primera = actual - CONTEXTO;
        if (actual + CONTEXTO > primera + RENGLONES - 1) primera = actual + CONTEXTO - RENGLONES + 1;
    }
    int ultimaPrimera = total - RENGLONES + 1;
    if (primera > ultimaPrimera) primera = ultimaPrimera;
    if (primera < 1) primera = 1;
    return primera != anterior;
}

void VistaPrograma::dibujarCodigo() {
    ajustarVentana();
    for (int i = 0; i < RENGLONES; ++i) {
        dibujarLineaCodigo(primera + i);
    }
}

void VistaPrograma::dibujarLineaCodigo(int linea) {
    int fila = linea - primera;
    if (fila < 0 || fila >= RENGLONES) return;
    int y = CODIGO_Y + fila * RENGLON;
    bool esActual = linea == actual;
    tft.fillRect(CODIGO_X, y, CODIGO_W, RENGLON, esActual ? COLOR_AMBAR_OSCURO : COLOR_FONDO);
    if (linea < 1 || linea > (int)lineas.size()) return;

    char numero[12];
    snprintf(numero, sizeof numero, "%3d", linea);
    escribir(CODIGO_X + 2, y + 2, numero, COLOR_GRIS, 1);
    if (esActual) {
        escribir(CODIGO_X + 22, y + 2, ">", COLOR_AMBAR, 1);
    }
    escribir(CODIGO_X + 30, y + 2, recortar(lineas[linea - 1], CODIGO_CARACTERES), COLOR_TEXTO, 1);
}

void VistaPrograma::lineaActual(int linea) {
    int anterior = actual;
    actual = linea;
    if (!visible) return;
    if (ajustarVentana()) {
        dibujarCodigo();
    } else {
        dibujarLineaCodigo(anterior);
        dibujarLineaCodigo(actual);
    }
}

// ------------------------------------------------------------- variables

void VistaPrograma::dibujarVariables() {
    tft.fillRect(VARS_X, VARS_Y, VARS_W, RENGLONES * RENGLON, COLOR_FONDO);
    escribir(VARS_X + 2, VARS_Y + 2, "Variables", COLOR_GRIS, 1);
    for (int i = 0; i < (int)nombres.size(); ++i) {
        dibujarVariable(i);
    }
}

void VistaPrograma::dibujarVariable(int indice) {
    if (indice < 0 || indice >= VARS_RENGLONES || indice >= (int)nombres.size()) return;
    int y = VARS_PRIMER_Y + indice * VARS_RENGLON;
    tft.fillRect(VARS_X, y, VARS_W, VARS_RENGLON, COLOR_FONDO);

    std::string izquierda = recortar(aAscii(nombres[indice]) + " = ", VARS_CARACTERES - 1);
    std::string valor = recortar(aAscii(valores[indice]), VARS_CARACTERES - izquierda.size());
    escribir(VARS_X + 2, y + 1, izquierda, COLOR_TEXTO, 1);
    escribir(VARS_X + 2 + 6 * (int)izquierda.size(), y + 1, valor,
             indice == cambiada ? COLOR_AZUL : COLOR_TEXTO, 1);
}

void VistaPrograma::variableCambio(const std::string& nombre, const std::string& valor) {
    for (int i = 0; i < (int)nombres.size(); ++i) {
        if (nombres[i] != nombre) continue;
        int anterior = cambiada;
        valores[i] = valor;
        cambiada = i;
        if (visible) {
            if (anterior != i) dibujarVariable(anterior);
            dibujarVariable(i);
        }
        return;
    }
}

// --------------------------------------------------------------- consola

void VistaPrograma::imprimir(const std::string& texto, uint16_t color) {
    Serial.println(texto.c_str());
    ultimaImpresa = texto;

    // Se parte en renglones del ancho de la pantalla.
    std::string ascii = aAscii(texto);
    size_t inicio = 0;
    do {
        consola.push_back({ascii.substr(inicio, CONSOLA_CARACTERES), color});
        inicio += CONSOLA_CARACTERES;
    } while (inicio < ascii.size());
    if (consola.size() > (size_t)CONSOLA_LINEAS) {
        consola.erase(consola.begin(), consola.end() - CONSOLA_LINEAS);
    }
    if (visible) dibujarConsola();
}

void VistaPrograma::dibujarConsola() {
    tft.fillRect(0, CONSOLA_Y, ANCHO, ALTO - CONSOLA_Y, COLOR_FONDO);
    for (size_t i = 0; i < consola.size(); ++i) {
        escribir(1, CONSOLA_Y + 1 + (int)i * CONSOLA_RENGLON, consola[i].texto, consola[i].color, 1);
    }
}

// ========================================================== vista de máquina

namespace {

// Estado de la línea dada o de la primera línea hacia arriba que empieza con state.
std::string buscarEstado(const std::vector<std::string>& fuente, int linea) {
    for (int i = linea - 1; i >= 0 && i < (int)fuente.size(); --i) {
        std::string t = limpia(fuente[i]);
        if (!empiezaCon(t, "state")) continue;
        std::string resto = recortarEspacios(t.substr(5));
        size_t fin = resto.find_first_of(" {");
        return fin == std::string::npos ? resto : resto.substr(0, fin);
    }
    return "?";
}

// Regla del estado (que empieza en la línea dada) que se aplica al símbolo leído:
// primero la de su propio símbolo; si no hay, la del comodín (excepto con blank).
// Vacía si ninguna se aplica: la máquina rechaza.
std::string buscarRegla(const std::vector<std::string>& fuente, int lineaEstado, char simbolo) {
    std::string comodin;
    int desde = lineaEstado;   // índice de la línea siguiente a la del estado
    for (int i = lineaEstado - 1; i >= 0 && i < (int)fuente.size(); --i) {
        if (empiezaCon(limpia(fuente[i]), "state")) {
            desde = i + 1;
            break;
        }
    }
    for (int i = desde; i < (int)fuente.size(); ++i) {
        std::string t = limpia(fuente[i]);
        if (t.empty()) continue;
        if (t[0] == '}' || empiezaCon(t, "state")) break;
        size_t flecha = t.find("->");
        if (flecha == std::string::npos) continue;
        std::string lee = recortarEspacios(t.substr(0, flecha));
        if (lee == "*") {
            if (simbolo != ' ' && comodin.empty()) comodin = t;
        } else if ((lee == "blank" && simbolo == ' ') || (lee.size() == 1 && lee[0] == simbolo)) {
            return t;
        }
    }
    return comodin;
}

}  // namespace

void VistaMaquina::inicio(const std::string& nombreMaquina, const std::string& cinta, int cabezal,
                          int limiteMaquina, const std::vector<std::string>& lineasFuente) {
    fuente = &lineasFuente;
    nombre = aAscii(nombreMaquina);
    estado = "-";
    regla.clear();
    limite = limiteMaquina;
    pasos = 0;
    for (int k = 0; k < CASILLAS; ++k) mostrado[k] = SIN_DIBUJAR;

    tft.fillRect(0, BARRA_ALTO + 1, ANCHO, ALTO - BARRA_ALTO - 1, COLOR_FONDO);
    dibujarEncabezado();
    dibujarRegla();
    dibujarCinta(aAscii(cinta), cabezal);

    // Cabezal: triángulo bajo la casilla central.
    int centro = CINTA_X + (CASILLAS / 2) * CASILLA_W + CASILLA_W / 2;
    tft.fillTriangle(centro, CABEZAL_Y, centro - 9, CABEZAL_Y + 14, centro + 9, CABEZAL_Y + 14,
                     COLOR_AMBAR);
}

void VistaMaquina::paso(const std::string& cinta, int cabezal, int linea) {
    ++pasos;
    char simbolo = cabezal >= 0 && cabezal < (int)cinta.size() ? cinta[cabezal] : ' ';
    std::string nuevaRegla = "(sin regla: rechaza)";
    if (fuente != nullptr) {
        estado = buscarEstado(*fuente, linea);
        std::string encontrada = buscarRegla(*fuente, linea, simbolo);
        if (!encontrada.empty()) nuevaRegla = encontrada;
    }
    dibujarEncabezado();
    if (nuevaRegla != regla) {
        regla = nuevaRegla;
        dibujarRegla();
    }
    dibujarCinta(aAscii(cinta), cabezal);
}

void VistaMaquina::dibujarEncabezado() {
    char pasoTexto[32];
    snprintf(pasoTexto, sizeof pasoTexto, "paso %d/%d", pasos, limite);
    std::string texto = "Maquina " + nombre + "   estado " + estado + "   " + pasoTexto;
    tft.fillRect(0, ENCABEZADO_Y - 2, ANCHO, 12, COLOR_FONDO);
    escribir(2, ENCABEZADO_Y, recortar(texto, 53), COLOR_TEXTO, 1);
}

void VistaMaquina::dibujarRegla() {
    tft.fillRect(0, REGLA_Y - 2, ANCHO, 20, COLOR_FONDO);
    if (regla.empty()) return;
    escribir(2, REGLA_Y, recortar(regla, 26), COLOR_TEXTO, 2);
}

void VistaMaquina::dibujarCinta(const std::string& cinta, int cabezal) {
    for (int k = 0; k < CASILLAS; ++k) {
        long i = (long)cabezal - CASILLAS / 2 + k;
        char simbolo = i >= 0 && i < (long)cinta.size() ? cinta[i] : ' ';
        if (simbolo != mostrado[k]) {
            dibujarCasilla(k, simbolo);
        }
    }
}

void VistaMaquina::dibujarCasilla(int k, char simbolo) {
    mostrado[k] = simbolo;
    int x = CINTA_X + k * CASILLA_W;
    tft.fillRect(x, CINTA_Y, CASILLA_W, CASILLA_H, COLOR_FONDO);
    if (k == CASILLAS / 2) {
        for (int g = 0; g < 3; ++g) {
            tft.drawRect(x + g, CINTA_Y + g, CASILLA_W - 2 * g, CASILLA_H - 2 * g, COLOR_AMBAR);
        }
    } else {
        tft.drawRect(x, CINTA_Y, CASILLA_W, CASILLA_H, COLOR_GRIS);
    }
    if (simbolo != ' ') {
        escribir(x + (CASILLA_W - 12) / 2, CINTA_Y + (CASILLA_H - 16) / 2, std::string(1, simbolo),
                 COLOR_TEXTO, 2);
    }
}

void VistaMaquina::fin(bool acepto, const std::string& cinta) {
    tft.fillRect(0, RESULTADO_Y - 4, ANCHO, ALTO - RESULTADO_Y + 4, COLOR_FONDO);
    escribir(8, RESULTADO_Y, acepto ? "ACEPTADA" : "RECHAZADA", acepto ? COLOR_AZUL : COLOR_GRIS, 2);
    escribir(8, RESULTADO_Y + 26, "Cinta final:", COLOR_GRIS, 1);
    std::string final = cinta.empty() ? "(vacia)" : recortar(aAscii(cinta), 25);
    escribir(8, RESULTADO_Y + 38, final, COLOR_AZUL, 2);
}

void VistaMaquina::esperarResultado(BarraSuperior& barra, int ms) {
    unsigned long inicio = millis();
    while ((long)(millis() - inicio) < ms) {
        int x, y;
        if (leerToque(x, y)) {
            barra.atender(x, y, true);
            return;
        }
        delay(15);
    }
}
