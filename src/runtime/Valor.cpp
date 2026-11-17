#include "runtime/Valor.h"

namespace {

const int64_t MIN_32 = -2147483648LL;
const int64_t MAX_32 = 2147483647LL;

// Recorta a 32 bits sin comportamiento indefinido.
int32_t recortar32(int64_t n) {
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(n)));
}

bool esDigito(char c) {
    return c >= '0' && c <= '9';
}

// Dígitos (con signo opcional ya quitado) dentro del límite dado.
bool leerDigitos(const std::string& texto, size_t desde, int64_t limite, int64_t& n) {
    if (desde >= texto.size()) {
        return false;
    }
    n = 0;
    for (size_t i = desde; i < texto.size(); ++i) {
        if (!esDigito(texto[i])) {
            return false;
        }
        n = n * 10 + (texto[i] - '0');
        if (n > limite) {
            return false;
        }
    }
    return true;
}

// int(): espacios al inicio y al final, un '-' opcional y dígitos, en 32 bits.
bool convertirEntero(const std::string& texto, int32_t& resultado) {
    size_t inicio = texto.find_first_not_of(' ');
    if (inicio == std::string::npos) {
        return false;
    }
    size_t fin = texto.find_last_not_of(' ');
    std::string recortado = texto.substr(inicio, fin - inicio + 1);

    bool negativo = !recortado.empty() && recortado[0] == '-';
    int64_t n = 0;
    if (!leerDigitos(recortado, negativo ? 1 : 0, negativo ? -MIN_32 : MAX_32, n)) {
        return false;
    }
    resultado = recortar32(negativo ? -n : n);
    return true;
}

bool esRelacional(Op op) {
    return op == Op::Menor || op == Op::Mayor || op == Op::MenorIgual ||
           op == Op::MayorIgual || op == Op::Igual || op == Op::Distinto;
}

}  // namespace

Valor valorEntero(int32_t n) {
    Valor v;
    v.tipo = Valor::Entero;
    v.entero = n;
    return v;
}

Valor valorCadena(const std::string& texto) {
    Valor v;
    v.tipo = Valor::Cadena;
    v.cadena = texto;
    return v;
}

Valor valorBool(bool b) {
    Valor v;
    v.tipo = Valor::Bool;
    v.entero = b ? 1 : 0;
    return v;
}

Valor valorSimbolo(char c) {
    Valor v;
    v.tipo = Valor::Simbolo;
    v.entero = static_cast<unsigned char>(c);
    return v;
}

std::string aTexto(const Valor& valor) {
    switch (valor.tipo) {
        case Valor::Entero:  return std::to_string(valor.entero);
        case Valor::Cadena:  return valor.cadena;
        case Valor::Bool:    return valor.entero != 0 ? "true" : "false";
        case Valor::Simbolo: return std::string(1, static_cast<char>(valor.entero));
    }
    return "";
}

int renglonSalto(const Operando& salto) {
    int n = 0;
    for (char c : salto.texto) {
        if (esDigito(c)) n = n * 10 + (c - '0');
    }
    return n;
}

// ------------------------------------------------------------------ Memoria

void Memoria::guardar(const std::string& nombre, const Valor& valor) {
    valores[nombre] = valor;
}

bool Memoria::valorDe(const Operando& o, Valor& valor, std::string& error) const {
    switch (o.clase) {
        case ClaseOperando::Entero: {
            // El optimizador puede dejar un entero negativo al plegar constantes.
            bool negativo = !o.texto.empty() && o.texto[0] == '-';
            int64_t n = 0;
            if (!leerDigitos(o.texto, negativo ? 1 : 0, negativo ? -MIN_32 : MAX_32, n)) {
                error = "número fuera de rango: " + o.texto;
                return false;
            }
            valor = valorEntero(recortar32(negativo ? -n : n));
            return true;
        }
        case ClaseOperando::Cadena:
            valor = valorCadena(o.texto);
            return true;
        case ClaseOperando::Bool:
            valor = valorBool(o.texto == "true");
            return true;
        case ClaseOperando::Simbolo:
            valor = valorSimbolo(o.texto == "blank" || o.texto.empty() ? ' ' : o.texto[0]);
            return true;
        case ClaseOperando::Var:
        case ClaseOperando::Temp: {
            auto it = valores.find(o.texto);
            if (it == valores.end()) {
                error = "variable sin valor: " + o.texto;
                return false;
            }
            valor = it->second;
            return true;
        }
        default:
            error = "operando inválido: " + textoOperando(o);
            return false;
    }
}

// --------------------------------------------------------------- operaciones

bool esCalculo(Op op) {
    switch (op) {
        case Op::Asignar:
        case Op::Suma: case Op::Resta: case Op::Mult: case Op::Div:
        case Op::Menor: case Op::Mayor: case Op::MenorIgual:
        case Op::MayorIgual: case Op::Igual: case Op::Distinto:
        case Op::ConvInt: case Op::ConvString:
            return true;
        default:
            return false;
    }
}

bool calcular(const Quad& q, const Memoria& memoria, Valor& r, std::string& error) {
    Valor a;
    if (!memoria.valorDe(q.arg1, a, error)) {
        return false;
    }

    switch (q.op) {
        case Op::Asignar:
            r = a;
            return true;
        case Op::ConvInt: {
            int32_t n = 0;
            if (!convertirEntero(a.cadena, n)) {
                error = "conversión inválida: \"" + a.cadena + "\" no es un número";
                return false;
            }
            r = valorEntero(n);
            return true;
        }
        case Op::ConvString:
            r = valorCadena(aTexto(a));
            return true;
        default:
            break;
    }

    Valor b;
    if (!memoria.valorDe(q.arg2, b, error)) {
        return false;
    }

    if (esRelacional(q.op)) {
        int32_t x = a.entero;
        int32_t y = b.entero;
        bool res = false;
        switch (q.op) {
            case Op::Menor:      res = x < y; break;
            case Op::Mayor:      res = x > y; break;
            case Op::MenorIgual: res = x <= y; break;
            case Op::MayorIgual: res = x >= y; break;
            case Op::Igual:      res = x == y; break;
            default:             res = x != y; break;
        }
        r = valorBool(res);
        return true;
    }

    if (q.op == Op::Suma && a.tipo == Valor::Cadena) {
        if (a.cadena.size() + b.cadena.size() > static_cast<size_t>(LIMITE_TEXTO)) {
            error = "cadena demasiado larga";
            return false;
        }
        r = valorCadena(a.cadena + b.cadena);
        return true;
    }

    int64_t x = a.entero;
    int64_t y = b.entero;
    switch (q.op) {
        case Op::Suma:  r = valorEntero(recortar32(x + y)); return true;
        case Op::Resta: r = valorEntero(recortar32(x - y)); return true;
        case Op::Mult:  r = valorEntero(recortar32(x * y)); return true;
        case Op::Div:
            if (y == 0) {
                error = "división entre cero";
                return false;
            }
            r = valorEntero(recortar32(x / y));   // hacia cero
            return true;
        default:
            error = std::string("operación inválida: ") + nombreOp(q.op);
            return false;
    }
}
