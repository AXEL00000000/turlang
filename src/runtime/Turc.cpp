#include "runtime/Turc.h"

namespace {

const char* const VERSION = "1";

// ---------------------------------------------------------------- escritura

char letraClase(ClaseOperando clase) {
    switch (clase) {
        case ClaseOperando::Nada:    return '-';
        case ClaseOperando::Var:     return 'V';
        case ClaseOperando::Temp:    return 'T';
        case ClaseOperando::Entero:  return 'E';
        case ClaseOperando::Cadena:  return 'C';
        case ClaseOperando::Bool:    return 'B';
        case ClaseOperando::Simbolo: return 'S';
        case ClaseOperando::Salto:   return 'J';
        case ClaseOperando::Funcion: return 'F';
        case ClaseOperando::Cinta:   return 'K';
    }
    return '-';
}

std::string codificar(const Operando& o) {
    switch (o.clase) {
        case ClaseOperando::Nada:  return "-";
        case ClaseOperando::Cinta: return "K";
        case ClaseOperando::Temp:  return o.texto;   // "T1": la letra y el nombre coinciden
        default:                   return std::string(1, letraClase(o.clase)) + o.texto;
    }
}

void escribirBloque(const std::vector<Quad>& quads, std::string& salida) {
    for (const Quad& q : quads) {
        salida += nombreOp(q.op);
        salida += '\t' + std::to_string(q.linea);
        salida += '\t' + codificar(q.arg1);
        salida += '\t' + codificar(q.arg2);
        salida += '\t' + codificar(q.res);
        salida += '\n';
    }
}

// ------------------------------------------------------------------ lectura

// Recorre el texto línea por línea sin salirse de él.
class Lector {
public:
    explicit Lector(const std::string& texto) : texto(texto) {}

    // Siguiente línea sin el salto (ni un '\r' final). false al terminar el texto.
    bool siguiente(std::string& linea) {
        if (pos >= texto.size()) return false;
        size_t fin = texto.find('\n', pos);
        if (fin == std::string::npos) fin = texto.size();
        linea = texto.substr(pos, fin - pos);
        if (!linea.empty() && linea[linea.size() - 1] == '\r') linea.erase(linea.size() - 1);
        pos = fin + 1;
        ++numero;
        return true;
    }

    int linea() const { return numero; }

private:
    const std::string& texto;
    size_t pos = 0;
    int numero = 0;
};

std::vector<std::string> separar(const std::string& linea) {
    std::vector<std::string> campos;
    size_t inicio = 0;
    while (true) {
        size_t tab = linea.find('\t', inicio);
        if (tab == std::string::npos) {
            campos.push_back(linea.substr(inicio));
            return campos;
        }
        campos.push_back(linea.substr(inicio, tab - inicio));
        inicio = tab + 1;
    }
}

// Número sin signo de 1 a 9 dígitos.
bool numero(const std::string& texto, int& n) {
    if (texto.empty() || texto.size() > 9) return false;
    n = 0;
    for (char c : texto) {
        if (c < '0' || c > '9') return false;
        n = n * 10 + (c - '0');
    }
    return true;
}

bool decodificar(const std::string& campo, Operando& o) {
    if (campo.empty()) return false;
    std::string resto = campo.substr(1);
    switch (campo[0]) {
        case '-': o.clase = ClaseOperando::Nada;    break;
        case 'V': o.clase = ClaseOperando::Var;     break;
        case 'T': o.clase = ClaseOperando::Temp;    resto = campo; break;
        case 'E': o.clase = ClaseOperando::Entero;  break;
        case 'C': o.clase = ClaseOperando::Cadena;  break;
        case 'B': o.clase = ClaseOperando::Bool;    break;
        case 'S': o.clase = ClaseOperando::Simbolo; break;
        case 'J': o.clase = ClaseOperando::Salto;   break;
        case 'F': o.clase = ClaseOperando::Funcion; break;
        case 'K': o.clase = ClaseOperando::Cinta;   resto = "cinta"; break;
        default:  return false;
    }
    if ((o.clase == ClaseOperando::Nada || o.clase == ClaseOperando::Cinta) && campo.size() != 1) {
        return false;
    }
    o.texto = resto;
    return true;
}

class Analizador {
public:
    Analizador(const std::string& texto, std::string& error) : lector(texto), error(error) {}

    bool cargar(Compilado& c) {
        std::vector<std::string> campos;

        if (!encabezado("TURC", 2, campos) || campos[1] != VERSION) return fallar();
        if (!encabezado("NOMBRE", 2, campos)) return fallar();
        c.nombre = campos[1];

        int n = 0;
        if (!encabezadoConConteo("FUENTE", n)) return fallar();
        c.fuente.clear();
        for (int i = 0; i < n; ++i) {
            std::string linea;
            if (!lector.siguiente(linea)) {
                faltaTexto = true;
                return fallar();
            }
            c.fuente.push_back(linea);
        }

        if (!encabezadoConConteo("VARS", n)) return fallar();
        c.variables.clear();
        for (int i = 0; i < n; ++i) {
            if (!leerCampos(campos, 2)) return fallar();
            if (campos[1] != "int" && campos[1] != "string" && campos[1] != "bool") return fallar();
            c.variables.push_back({campos[0], campos[1]});
        }

        if (!encabezadoConConteo("MAQUINAS", n)) return fallar();
        c.programa = Program();
        for (int i = 0; i < n; ++i) {
            int cuantos = 0;
            if (!encabezado("MAQUINA", 3, campos) || campos[1].empty() ||
                !numero(campos[2], cuantos)) {
                return fallar();
            }
            Bloque b;
            b.nombre = campos[1];
            if (!bloque(cuantos, b)) return fallar();
            c.programa.maquinas.push_back(b);
        }

        int cuantos = 0;
        if (!encabezadoConConteo("PRINCIPAL", cuantos)) return fallar();
        if (!bloque(cuantos, c.programa.principal)) return fallar();

        if (!encabezado("FIN", 1, campos)) return fallar();
        return true;
    }

private:
    Lector lector;
    std::string& error;

    bool faltaTexto = false;

    // Si el texto se acabó, el error apunta a la línea que se esperaba.
    bool fallar() {
        int linea = lector.linea() + (faltaTexto ? 1 : 0);
        error = "archivo .turc dañado (línea " + std::to_string(linea) + ")";
        return false;
    }

    // Una línea con exactamente k campos separados por tabulador.
    bool leerCampos(std::vector<std::string>& campos, size_t k) {
        std::string linea;
        if (!lector.siguiente(linea)) {
            faltaTexto = true;
            return false;
        }
        campos = separar(linea);
        return campos.size() == k;
    }

    bool encabezado(const char* palabra, size_t k, std::vector<std::string>& campos) {
        return leerCampos(campos, k) && campos[0] == palabra;
    }

    bool encabezadoConConteo(const char* palabra, int& n) {
        std::vector<std::string> campos;
        return encabezado(palabra, 2, campos) && numero(campos[1], n);
    }

    bool bloque(int cuantos, Bloque& b) {
        b.quads.clear();
        for (int i = 0; i < cuantos; ++i) {
            std::vector<std::string> campos;
            if (!leerCampos(campos, 5)) return false;
            Quad q;
            int linea = 0;
            if (!opDesdeTexto(campos[0], q.op) || !numero(campos[1], linea) ||
                !decodificar(campos[2], q.arg1) || !decodificar(campos[3], q.arg2) ||
                !decodificar(campos[4], q.res)) {
                return false;
            }
            // Los saltos apuntan a un renglón del mismo bloque.
            if (q.op == Op::Jump || q.op == Op::IfFalse) {
                int destino = 0;
                if (q.res.clase != ClaseOperando::Salto || !numero(q.res.texto, destino) ||
                    destino < 1 || destino > cuantos) {
                    return false;
                }
            }
            q.linea = linea;
            b.quads.push_back(q);
        }
        return true;
    }
};

}  // namespace

std::string escribirTurc(const Compilado& c) {
    std::string salida;
    salida += std::string("TURC\t") + VERSION + "\n";
    salida += "NOMBRE\t" + c.nombre + "\n";
    salida += "FUENTE\t" + std::to_string(c.fuente.size()) + "\n";
    for (const std::string& linea : c.fuente) salida += linea + "\n";
    salida += "VARS\t" + std::to_string(c.variables.size()) + "\n";
    for (const VariableInfo& v : c.variables) salida += v.nombre + "\t" + v.tipo + "\n";
    salida += "MAQUINAS\t" + std::to_string(c.programa.maquinas.size()) + "\n";
    for (const Bloque& b : c.programa.maquinas) {
        salida += "MAQUINA\t" + b.nombre + "\t" + std::to_string(b.quads.size()) + "\n";
        escribirBloque(b.quads, salida);
    }
    salida += "PRINCIPAL\t" + std::to_string(c.programa.principal.quads.size()) + "\n";
    escribirBloque(c.programa.principal.quads, salida);
    salida += "FIN\n";
    return salida;
}

bool leerTurc(const std::string& texto, Compilado& compilado, std::string& error) {
    Analizador analizador(texto, error);
    return analizador.cargar(compilado);
}
