#include "Interpreter.h"

#include "TuringSimulator.h"

Interpreter::Interpreter(const Program& programa, IO& io) : programa(programa), io(io) {
    stats.ejecutadosMaquina.assign(programa.maquinas.size(), 0);
}

const Estadisticas& Interpreter::estadisticas() const { return stats; }
const std::string& Interpreter::mensajeError() const { return mensaje; }
int Interpreter::lineaError() const { return linea; }

bool Interpreter::fallar(int lineaFallo, const std::string& texto) {
    mensaje = texto;
    linea = lineaFallo;
    return false;
}

// Guarda un valor; si el destino es una variable del programa, avisa a la pantalla.
void Interpreter::guardar(const Operando& destino, const Valor& valor) {
    memoria.guardar(destino.texto, valor);
    if (destino.clase == ClaseOperando::Var) {
        io.variableCambio(destino.texto, aTexto(valor));
    }
}

bool Interpreter::llamar(const Quad& q) {
    const std::string& funcion = q.arg1.texto;
    std::vector<Valor> args;
    args.swap(argumentos);   // la lista de pendientes queda vacía

    if (funcion == "print") {
        io.imprimir(args.empty() ? "" : aTexto(args[0]));
        return true;
    }
    if (funcion == "input") {
        guardar(q.res, valorCadena(io.leer()));
        return true;
    }
    if (args.size() < 2) {
        return fallar(q.linea, "faltan argumentos para " + funcion);
    }
    if (funcion == "repeat") {
        const std::string& base = args[0].cadena;
        int32_t veces = args[1].entero;
        std::string resultado;
        if (veces > 0 && !base.empty()) {
            if (static_cast<int64_t>(base.size()) * veces > LIMITE_TEXTO) {
                return fallar(q.linea, "cadena demasiado larga");
            }
            for (int32_t i = 0; i < veces; ++i) resultado += base;
        }
        guardar(q.res, valorCadena(resultado));
        return true;
    }

    // Una máquina.
    for (size_t i = 0; i < programa.maquinas.size(); ++i) {
        if (programa.maquinas[i].nombre != funcion) continue;
        TuringSimulator simulador(programa.maquinas[i], io);
        bool acepto = simulador.ejecutar(args[0].cadena, args[1].entero);
        stats.ejecutadosMaquina[i] += simulador.ejecutados();
        if (simulador.huboError()) {
            return fallar(simulador.lineaError(), simulador.mensajeError());
        }
        ultimaCinta = simulador.cintaFinal();
        guardar(q.res, valorBool(acepto));
        return true;
    }
    return fallar(q.linea, "función desconocida: " + funcion);
}

bool Interpreter::ejecutar() {
    const std::vector<Quad>& quads = programa.principal.quads;
    int lineaAnterior = 0;
    size_t pc = 0;

    while (pc < quads.size()) {
        const Quad& q = quads[pc];
        if (io.detenido()) return fallar(q.linea, "ejecución detenida");
        if (q.linea > 0 && q.linea != lineaAnterior) {
            io.lineaActual(q.linea);
        }
        lineaAnterior = q.linea;
        ++stats.ejecutadosPrincipal;
        std::string texto;

        switch (q.op) {
            case Op::IfFalse: {
                ++stats.saltosPrincipal;
                Valor c;
                if (!memoria.valorDe(q.arg1, c, texto)) return fallar(q.linea, texto);
                pc = c.entero == 0 ? static_cast<size_t>(renglonSalto(q.res) - 1) : pc + 1;
                continue;
            }
            case Op::Jump:
                ++stats.saltosPrincipal;
                pc = static_cast<size_t>(renglonSalto(q.res) - 1);
                continue;
            case Op::Param: {
                Valor v;
                if (!memoria.valorDe(q.arg1, v, texto)) return fallar(q.linea, texto);
                argumentos.push_back(v);
                ++pc;
                continue;
            }
            case Op::Call:
                if (!llamar(q)) return false;
                ++pc;
                continue;
            case Op::Ret:
                return true;
            case Op::Asignar:
                if (q.arg1.clase == ClaseOperando::Cinta) {
                    // (=, cinta, ---, T): la cinta final de la última máquina ejecutada.
                    guardar(q.res, valorCadena(ultimaCinta));
                    ++pc;
                    continue;
                }
                break;
            default:
                break;
        }

        if (!esCalculo(q.op)) {
            return fallar(q.linea, std::string("operación inválida: ") + nombreOp(q.op));
        }
        Valor r;
        if (!calcular(q, memoria, r, texto)) return fallar(q.linea, texto);
        guardar(q.res, r);
        ++pc;
    }
    return true;
}
