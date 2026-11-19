#include "TuringSimulator.h"

TuringSimulator::TuringSimulator(const Bloque& maquina, IO& io) : maquina(maquina), io(io) {}

const std::string& TuringSimulator::cintaFinal() const { return final; }
long TuringSimulator::ejecutados() const { return cuenta; }
bool TuringSimulator::huboError() const { return error; }
const std::string& TuringSimulator::mensajeError() const { return mensaje; }
int TuringSimulator::lineaError() const { return linea; }

bool TuringSimulator::fallar(const Quad& q, const std::string& texto) {
    error = true;
    mensaje = texto;
    linea = q.linea;
    actualizarFinal();
    return false;
}

// La cinta sin los espacios del inicio y del final.
void TuringSimulator::actualizarFinal() {
    size_t inicio = cinta.find_first_not_of(' ');
    if (inicio == std::string::npos) {
        final.clear();
        return;
    }
    size_t fin = cinta.find_last_not_of(' ');
    final = cinta.substr(inicio, fin - inicio + 1);
}

// Índice dentro del texto de la cinta que corresponde al cabezal (arg2 del cuádruplo).
bool TuringSimulator::indiceCabezal(const Quad& q, long& indice) {
    Valor cabezal;
    std::string texto;
    if (!memoria.valorDe(q.arg2, cabezal, texto)) {
        return fallar(q, texto);
    }
    indice = static_cast<long>(cabezal.entero) + desplazamiento;
    return true;
}

bool TuringSimulator::ejecutar(const std::string& cadena, int maximo) {
    memoria = Memoria();
    cinta.clear();
    desplazamiento = 0;
    final.clear();
    cuenta = 0;
    error = false;
    mensaje.clear();
    linea = 0;

    memoria.guardar("cadena", valorCadena(cadena));
    memoria.guardar("maximo", valorEntero(maximo));

    const std::vector<Quad>& quads = maquina.quads;
    size_t pc = 0;
    while (pc < quads.size()) {
        const Quad& q = quads[pc];
        if (io.detenido()) return fallar(q, "ejecución detenida");
        ++cuenta;
        std::string texto;

        switch (q.op) {
            case Op::Asignar:
                if (q.res.clase == ClaseOperando::Cinta) {
                    // (=, cadena, ---, cinta): carga la cadena en la cinta.
                    Valor v;
                    if (!memoria.valorDe(q.arg1, v, texto)) return fallar(q, texto);
                    if (v.cadena.size() > static_cast<size_t>(LIMITE_TEXTO)) {
                        return fallar(q, "cinta demasiado larga");
                    }
                    cinta = v.cadena;
                    desplazamiento = 0;
                    io.maquinaInicio(maquina.nombre, cinta, 0, maximo);
                    ++pc;
                    continue;
                }
                break;
            case Op::LeerCinta: {
                long i = 0;
                if (!indiceCabezal(q, i)) return false;
                long tam = static_cast<long>(cinta.size());
                char c = (i >= 0 && i < tam) ? cinta[i] : ' ';
                // La pantalla recibe la cinta completa con el cabezal dentro del texto.
                if (i < 0) {
                    io.maquinaPaso(std::string(-i, ' ') + cinta, 0, q.linea);
                } else if (i >= tam) {
                    io.maquinaPaso(cinta + std::string(i - tam + 1, ' '), i, q.linea);
                } else {
                    io.maquinaPaso(cinta, i, q.linea);
                }
                memoria.guardar(q.res.texto, valorSimbolo(c));
                ++pc;
                continue;
            }
            case Op::EscribirCinta: {
                Valor s;
                if (!memoria.valorDe(q.arg1, s, texto)) return fallar(q, texto);
                long i = 0;
                if (!indiceCabezal(q, i)) return false;
                long tam = static_cast<long>(cinta.size());
                // La cinta crece con espacios hasta alcanzar el cabezal, en los dos sentidos.
                long nuevoTam = i < 0 ? tam - i : (i >= tam ? i + 1 : tam);
                if (nuevoTam > LIMITE_TEXTO) {
                    return fallar(q, "cinta demasiado larga");
                }
                if (i < 0) {
                    cinta.insert(0, std::string(-i, ' '));
                    desplazamiento += -i;
                    i = 0;
                } else if (i >= tam) {
                    cinta.append(static_cast<size_t>(i - tam + 1), ' ');
                }
                cinta[i] = static_cast<char>(s.entero);
                ++pc;
                continue;
            }
            case Op::IfFalse: {
                Valor c;
                if (!memoria.valorDe(q.arg1, c, texto)) return fallar(q, texto);
                pc = c.entero == 0 ? static_cast<size_t>(renglonSalto(q.res) - 1) : pc + 1;
                continue;
            }
            case Op::Jump:
                pc = static_cast<size_t>(renglonSalto(q.res) - 1);
                continue;
            case Op::Ret: {
                Valor r;
                if (!memoria.valorDe(q.arg1, r, texto)) return fallar(q, texto);
                actualizarFinal();
                io.maquinaFin(r.entero != 0, final);
                return r.entero != 0;
            }
            default:
                break;
        }

        if (!esCalculo(q.op)) {
            return fallar(q, std::string("operación inválida en una máquina: ") + nombreOp(q.op));
        }
        Valor r;
        if (!calcular(q, memoria, r, texto)) return fallar(q, texto);
        memoria.guardar(q.res.texto, r);
        ++pc;
    }

    // Sin ret: rechaza.
    actualizarFinal();
    io.maquinaFin(false, final);
    return false;
}
