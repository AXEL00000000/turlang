#include "ir/Optimizer.h"

#include <set>

#include "runtime/Valor.h"

namespace {

const int MAX_REPETICIONES = 20;
const int MAX_VUELTAS = 8;
const int MAX_DESENROLLADO = 64;

bool esSalto(Op op) {
    return op == Op::Jump || op == Op::IfFalse;
}

bool esOperacion(Op op) {
    switch (op) {
        case Op::Suma: case Op::Resta: case Op::Mult: case Op::Div:
        case Op::Menor: case Op::Mayor: case Op::MenorIgual:
        case Op::MayorIgual: case Op::Igual: case Op::Distinto:
        case Op::ConvInt: case Op::ConvString:
            return true;
        default:
            return false;
    }
}

bool esConversion(Op op) {
    return op == Op::ConvInt || op == Op::ConvString;
}

bool esConstante(const Operando& o) {
    return o.clase == ClaseOperando::Entero || o.clase == ClaseOperando::Cadena ||
           o.clase == ClaseOperando::Bool;
}

bool esNombre(const Operando& o) {
    return o.clase == ClaseOperando::Var || o.clase == ClaseOperando::Temp;
}

bool iguales(const Operando& a, const Operando& b) {
    return a.clase == b.clase && a.texto == b.texto;
}

bool esEntero(const Operando& o, const char* texto) {
    return o.clase == ClaseOperando::Entero && o.texto == texto;
}

Operando operando(ClaseOperando clase, const std::string& texto) {
    Operando o;
    o.clase = clase;
    o.texto = texto;
    return o;
}

Operando aOperando(const Valor& v) {
    switch (v.tipo) {
        case Valor::Cadena: return operando(ClaseOperando::Cadena, v.cadena);
        case Valor::Bool:   return operando(ClaseOperando::Bool, v.entero != 0 ? "true" : "false");
        default:            return operando(ClaseOperando::Entero, std::to_string(v.entero));
    }
}

int destino(const Quad& q) {
    return renglonSalto(q.res);
}

void ponerDestino(Quad& q, int renglon) {
    q.res = operando(ClaseOperando::Salto, std::to_string(renglon));
}

void convertirEnAsignacion(Quad& q, const Operando& valor) {
    q.op = Op::Asignar;
    q.arg1 = valor;
    q.arg2 = Operando();
}

// Renglones (desde 1) a los que apunta algún if-false o jump.
std::set<int> destinosDeSalto(const std::vector<Quad>& quads) {
    std::set<int> destinos;
    for (const Quad& q : quads) {
        if (esSalto(q.op)) destinos.insert(destino(q));
    }
    return destinos;
}

// Renglones a los que se llega desde el renglón 1 siguiendo el flujo.
std::vector<bool> alcanzables(const std::vector<Quad>& quads) {
    std::vector<bool> visto(quads.size(), false);
    std::vector<int> pendientes;
    if (!quads.empty()) pendientes.push_back(0);
    while (!pendientes.empty()) {
        int i = pendientes.back();
        pendientes.pop_back();
        if (i < 0 || i >= static_cast<int>(quads.size()) || visto[i]) continue;
        visto[i] = true;
        const Quad& q = quads[i];
        if (q.op == Op::Ret) continue;
        if (q.op == Op::Jump) {
            pendientes.push_back(destino(q) - 1);
            continue;
        }
        if (q.op == Op::IfFalse) {
            pendientes.push_back(destino(q) - 1);
        }
        pendientes.push_back(i + 1);
    }
    return visto;
}

// Elimina los renglones marcados y renumera los saltos: un salto que apuntaba a
// un renglón eliminado pasa a apuntar al siguiente que queda. El último ret nunca
// se elimina. Es la única función que elimina cuádruplos.
int eliminarRenglones(std::vector<Quad>& quads, std::vector<bool> borrar) {
    int n = static_cast<int>(quads.size());
    if (n > 0 && quads[n - 1].op == Op::Ret) {
        borrar[n - 1] = false;
    }
    // nuevo[i]: renglón (desde 1) que ocupa, o que ocuparía, el renglón viejo i+1.
    std::vector<int> nuevo(n + 1);
    int quedan = 0;
    for (int i = 0; i < n; ++i) {
        nuevo[i] = quedan + 1;
        if (!borrar[i]) ++quedan;
    }
    nuevo[n] = quedan + 1;
    if (quedan == n) return 0;

    std::vector<Quad> resultado;
    for (int i = 0; i < n; ++i) {
        if (borrar[i]) continue;
        Quad q = quads[i];
        if (esSalto(q.op)) {
            int d = destino(q);
            if (d >= 1 && d <= n + 1) ponerDestino(q, nuevo[d - 1]);
        }
        resultado.push_back(q);
    }
    quads.swap(resultado);
    return n - quedan;
}

// Inserta cuádruplos después del renglón j (desde 1); los saltos a renglones
// posteriores a j se recorren.
void insertarDespues(std::vector<Quad>& quads, int j, const std::vector<Quad>& nuevos) {
    int m = static_cast<int>(nuevos.size());
    for (Quad& q : quads) {
        if (esSalto(q.op) && destino(q) > j) ponerDestino(q, destino(q) + m);
    }
    quads.insert(quads.begin() + j, nuevos.begin(), nuevos.end());
}

}  // namespace

// ---------------------------------------------------------------- interfaz

int Optimizer::veces(Tecnica tecnica) const {
    return contadores[static_cast<int>(tecnica)];
}

void Optimizer::contar(Tecnica tecnica, int n) {
    contadores[static_cast<int>(tecnica)] += n;
}

Program Optimizer::optimizar(const Program& original) {
    for (int& c : contadores) c = 0;
    Program resultado = original;
    for (Bloque& maquina : resultado.maquinas) {
        optimizarMaquina(maquina.quads);
    }
    optimizarControl(resultado.principal.quads);
    return resultado;
}

// ------------------------------------------------- pasos sobre un cuádruplo

// Sustituye, intenta folding, simplificación algebraica y reducción de fuerza, y
// actualiza la tabla de valores conocidos.
Optimizer::Efecto Optimizer::evaluar(Quad& q, Tabla& tabla) {
    Efecto e;

    auto sustituir = [&](Operando& o) {
        if (!esNombre(o)) return;
        auto it = tabla.find(o.texto);
        if (it != tabla.end()) {
            o = it->second;
            e.sustituyo = true;
        }
    };

    if (esOperacion(q.op)) {
        sustituir(q.arg1);
        if (!esConversion(q.op)) sustituir(q.arg2);
    } else if (q.op == Op::Asignar || q.op == Op::Param || q.op == Op::Ret ||
               q.op == Op::IfFalse) {
        sustituir(q.arg1);
    }

    if (esOperacion(q.op)) {
        if (folding(q)) {
            e.plego = true;
        } else if (simplificacionAlgebraica(q)) {
            e.simplifico = true;
        } else if (reduccionFuerza(q)) {
            e.redujo = true;
        }
    }

    if (q.op == Op::IfFalse) {
        if (q.arg1.clase == ClaseOperando::Bool) {
            if (q.arg1.texto == "true") {
                e.eliminar = true;
            } else {
                q.op = Op::Jump;
                q.arg1 = Operando();
                e.aSalto = true;
            }
        }
        return e;
    }
    if (q.op == Op::Jump || q.op == Op::Param || q.op == Op::Ret || !esNombre(q.res)) {
        return e;
    }

    const std::string& destinoValor = q.res.texto;

    // (=, c, ---, x) con x que ya vale c: sobra.
    if (q.op == Op::Asignar && esConstante(q.arg1) && q.res.clase == ClaseOperando::Var) {
        auto it = tabla.find(destinoValor);
        if (it != tabla.end() && iguales(it->second, q.arg1)) {
            e.eliminar = true;
            return e;
        }
    }

    // Se olvida el valor anterior y toda copia T → destino.
    tabla.erase(destinoValor);
    for (auto it = tabla.begin(); it != tabla.end();) {
        if (it->second.clase == ClaseOperando::Var && it->second.texto == destinoValor) {
            it = tabla.erase(it);
        } else {
            ++it;
        }
    }

    if (q.op == Op::Asignar) {
        if (esConstante(q.arg1)) {
            tabla[destinoValor] = q.arg1;
        } else if (q.arg1.clase == ClaseOperando::Var && q.res.clase == ClaseOperando::Temp) {
            tabla[destinoValor] = q.arg1;   // copia
        }
    }
    return e;
}

// Operación con todos sus argumentos constantes → (=, resultado, ---, Res).
// Con la misma función calcular() del intérprete; si da error no se pliega.
bool Optimizer::folding(Quad& q) {
    if (!esConstante(q.arg1)) return false;
    if (!esConversion(q.op) && !esConstante(q.arg2)) return false;
    Memoria vacia;
    Valor resultado;
    std::string error;
    if (!calcular(q, vacia, resultado, error)) return false;
    convertirEnAsignacion(q, aOperando(resultado));
    return true;
}

// n*1, 1*n, n+0, 0+n, n-0, n/1 → n;  n*0, 0*n → 0.
bool Optimizer::simplificacionAlgebraica(Quad& q) {
    switch (q.op) {
        case Op::Mult:
            if (esEntero(q.arg2, "1")) { convertirEnAsignacion(q, q.arg1); return true; }
            if (esEntero(q.arg1, "1")) { convertirEnAsignacion(q, q.arg2); return true; }
            if (esEntero(q.arg1, "0") || esEntero(q.arg2, "0")) {
                convertirEnAsignacion(q, operando(ClaseOperando::Entero, "0"));
                return true;
            }
            return false;
        case Op::Suma:
            if (esEntero(q.arg2, "0")) { convertirEnAsignacion(q, q.arg1); return true; }
            if (esEntero(q.arg1, "0")) { convertirEnAsignacion(q, q.arg2); return true; }
            return false;
        case Op::Resta:
            if (esEntero(q.arg2, "0")) { convertirEnAsignacion(q, q.arg1); return true; }
            return false;
        case Op::Div:
            if (esEntero(q.arg2, "1")) { convertirEnAsignacion(q, q.arg1); return true; }
            return false;
        default:
            return false;
    }
}

// n*2, 2*n (n no constante) → n + n.
bool Optimizer::reduccionFuerza(Quad& q) {
    if (q.op != Op::Mult) return false;
    if (esEntero(q.arg2, "2") && !esConstante(q.arg1)) {
        q.op = Op::Suma;
        q.arg2 = q.arg1;
        return true;
    }
    if (esEntero(q.arg1, "2") && !esConstante(q.arg2)) {
        q.op = Op::Suma;
        q.arg1 = q.arg2;
        return true;
    }
    return false;
}

// -------------------------------------------------------- sección de control

void Optimizer::optimizarControl(std::vector<Quad>& quads) {
    for (int externa = 0; externa < MAX_REPETICIONES; ++externa) {
        bool cambio = false;

        // Pasadas 1 a 4 hasta que una vuelta completa no cambie nada.
        for (int vuelta = 0; vuelta < MAX_REPETICIONES; ++vuelta) {
            bool cambioVuelta = false;
            if (propagacionConstantes(quads)) cambioVuelta = true;
            if (codigoInalcanzable(quads)) cambioVuelta = true;
            if (saltosInnecesarios(quads)) cambioVuelta = true;
            if (desenrolladoCiclos(quads)) cambioVuelta = true;
            if (!cambioVuelta) break;
            cambio = true;
        }

        // Pasada 5: asignaciones sin uso.
        for (int vuelta = 0; vuelta < MAX_REPETICIONES; ++vuelta) {
            if (!asignacionesSinUso(quads)) break;
            cambio = true;
        }

        if (!cambio) break;
    }
}

bool Optimizer::propagacionConstantes(std::vector<Quad>& quads) {
    std::set<int> destinos = destinosDeSalto(quads);
    std::vector<bool> borrar(quads.size(), false);
    Tabla tabla;
    bool cambio = false;

    for (size_t i = 0; i < quads.size(); ++i) {
        // A un destino de salto se puede llegar por otro camino.
        if (destinos.count(static_cast<int>(i) + 1)) tabla.clear();

        Efecto e = evaluar(quads[i], tabla);
        if (e.sustituyo)  contar(Tecnica::PropagacionConstantes);
        if (e.plego)      contar(Tecnica::Folding);
        if (e.simplifico) contar(Tecnica::SimplificacionAlgebraica);
        if (e.redujo)     contar(Tecnica::ReduccionFuerza);
        if (e.eliminar || e.aSalto) contar(Tecnica::CodigoMuerto);
        if (e.eliminar) borrar[i] = true;
        if (e.sustituyo || e.plego || e.simplifico || e.redujo || e.eliminar || e.aSalto) {
            cambio = true;
        }
    }
    eliminarRenglones(quads, borrar);
    return cambio;
}

bool Optimizer::codigoInalcanzable(std::vector<Quad>& quads) {
    std::vector<bool> visto = alcanzables(quads);
    std::vector<bool> borrar(quads.size());
    for (size_t i = 0; i < quads.size(); ++i) borrar[i] = !visto[i];
    int eliminados = eliminarRenglones(quads, borrar);
    contar(Tecnica::CodigoMuerto, eliminados);
    return eliminados > 0;
}

// Tabla de valores conocidos justo antes del renglón dado (solo lectura).
Optimizer::Tabla Optimizer::tablaAntesDe(const std::vector<Quad>& quads, int renglon) {
    std::set<int> destinos = destinosDeSalto(quads);
    Tabla tabla;
    for (int r = 1; r < renglon; ++r) {
        if (destinos.count(r)) tabla.clear();
        Quad copia = quads[r - 1];
        evaluar(copia, tabla);
    }
    return tabla;
}

// Simula el ciclo h..j (condición h..k) con los valores conocidos. Regresa false
// si la condición no es constante o se exceden los límites.
bool Optimizer::simularCiclo(const std::vector<Quad>& quads, int h, int k, int j, int& vueltas) {
    Tabla tabla = tablaAntesDe(quads, h);
    int tamCuerpo = j - 1 - k;
    vueltas = 0;
    while (true) {
        for (int r = h; r < k; ++r) {
            Quad copia = quads[r - 1];
            evaluar(copia, tabla);
        }
        Quad condicion = quads[k - 1];
        Efecto e = evaluar(condicion, tabla);
        if (e.aSalto) return true;          // false: termina
        if (!e.eliminar) return false;      // no es constante
        ++vueltas;
        if (vueltas > MAX_VUELTAS || vueltas * tamCuerpo > MAX_DESENROLLADO) return false;
        for (int r = k + 1; r < j; ++r) {
            Quad copia = quads[r - 1];
            evaluar(copia, tabla);
        }
    }
}

bool Optimizer::desenrolladoCiclos(std::vector<Quad>& quads) {
    int n = static_cast<int>(quads.size());
    for (int j = 1; j <= n; ++j) {
        // 1. El renglón j es (jump, ---, ---, (h)).
        if (quads[j - 1].op != Op::Jump) continue;
        int h = destino(quads[j - 1]);
        if (h < 1 || h >= j) continue;

        // 2. Entre h y j, un solo salto: un if-false hacia j + 1.
        int k = 0;
        bool valido = true;
        for (int r = h; r < j && valido; ++r) {
            Op op = quads[r - 1].op;
            if (op == Op::IfFalse) {
                if (k != 0) valido = false;
                k = r;
            } else if (op == Op::Jump || op == Op::Ret) {
                valido = false;
            }
        }
        if (!valido || k == 0 || destino(quads[k - 1]) != j + 1) continue;

        // 3. Ningún salto de fuera entra al ciclo; a h solo apunta j.
        for (int r = 1; r <= n && valido; ++r) {
            if (r >= h && r <= j) continue;
            if (esSalto(quads[r - 1].op)) {
                int d = destino(quads[r - 1]);
                if (d >= h && d <= j) valido = false;
            }
        }
        if (!valido) continue;

        // 4. A h se llega de corrido desde h - 1.
        if (h > 1) {
            Op anterior = quads[h - 2].op;
            if (anterior == Op::Jump || anterior == Op::Ret) continue;
        }

        int vueltas = 0;
        if (!simularCiclo(quads, h, k, j, vueltas)) continue;

        // El cuerpo repetido sustituye a h..j.
        std::vector<Quad> copias;
        for (int v = 0; v < vueltas; ++v) {
            for (int r = k + 1; r < j; ++r) copias.push_back(quads[r - 1]);
        }
        insertarDespues(quads, j, copias);
        std::vector<bool> borrar(quads.size(), false);
        for (int r = h; r <= j; ++r) borrar[r - 1] = true;
        eliminarRenglones(quads, borrar);
        contar(Tecnica::DesenrolladoCiclos);
        return true;   // uno por vuelta
    }
    return false;
}

// Análisis de variables vivas: en cada ret están vivas todas las variables del
// programa y ningún temporal. Se elimina lo que guarda en algo que no está vivo.
bool Optimizer::asignacionesSinUso(std::vector<Quad>& quads) {
    int n = static_cast<int>(quads.size());
    std::set<std::string> todasLasVariables;
    for (const Quad& q : quads) {
        for (const Operando* o : {&q.arg1, &q.arg2, &q.res}) {
            if (o->clase == ClaseOperando::Var) todasLasVariables.insert(o->texto);
        }
    }

    auto usos = [](const Quad& q) {
        std::set<std::string> u;
        if (esNombre(q.arg1)) u.insert(q.arg1.texto);
        if (esNombre(q.arg2)) u.insert(q.arg2.texto);
        return u;
    };
    auto define = [](const Quad& q) -> std::string {
        if (esSalto(q.op) || q.op == Op::Param || q.op == Op::Ret) return "";
        return esNombre(q.res) ? q.res.texto : "";
    };
    auto sucesores = [&](int i) {
        std::vector<int> s;
        const Quad& q = quads[i];
        if (q.op == Op::Ret) return s;
        if (esSalto(q.op)) s.push_back(destino(q) - 1);
        if (q.op != Op::Jump && i + 1 < n) s.push_back(i + 1);
        return s;
    };

    std::vector<std::set<std::string>> vivasEntrada(n), vivasSalida(n);
    bool cambio = true;
    while (cambio) {
        cambio = false;
        for (int i = n - 1; i >= 0; --i) {
            std::set<std::string> salida;
            std::set<std::string> entrada;
            if (quads[i].op == Op::Ret) {
                entrada = todasLasVariables;
            } else {
                for (int s : sucesores(i)) {
                    if (s >= 0 && s < n) salida.insert(vivasEntrada[s].begin(), vivasEntrada[s].end());
                }
                entrada = salida;
                std::string d = define(quads[i]);
                if (!d.empty()) entrada.erase(d);
            }
            std::set<std::string> u = usos(quads[i]);
            entrada.insert(u.begin(), u.end());
            if (entrada != vivasEntrada[i] || salida != vivasSalida[i]) {
                vivasEntrada[i] = entrada;
                vivasSalida[i] = salida;
                cambio = true;
            }
        }
    }

    std::vector<bool> borrar(n, false);
    for (int i = 0; i < n; ++i) {
        Op op = quads[i].op;
        if (op == Op::Call || op == Op::Param || op == Op::Ret || esSalto(op)) continue;
        std::string d = define(quads[i]);
        if (!d.empty() && vivasSalida[i].count(d) == 0) borrar[i] = true;
    }
    int eliminados = eliminarRenglones(quads, borrar);
    contar(Tecnica::CodigoMuerto, eliminados);
    return eliminados > 0;
}

// ------------------------------------------------------------------ máquinas

void Optimizer::optimizarMaquina(std::vector<Quad>& quads) {
    for (int vuelta = 0; vuelta < MAX_REPETICIONES; ++vuelta) {
        bool cambio = false;
        if (estadosInalcanzables(quads)) cambio = true;
        if (escriturasRedundantes(quads)) cambio = true;
        if (saltosInnecesarios(quads)) cambio = true;
        if (!cambio) break;
    }
}

// Cuenta 1 por cada bloque de estado eliminado (empieza con (<, pasos, limite, T)).
bool Optimizer::estadosInalcanzables(std::vector<Quad>& quads) {
    std::vector<bool> visto = alcanzables(quads);
    std::vector<bool> borrar(quads.size());
    int bloques = 0;
    for (size_t i = 0; i < quads.size(); ++i) {
        borrar[i] = !visto[i];
        if (borrar[i] && quads[i].op == Op::Menor && quads[i].arg1.texto == "pasos") ++bloques;
    }
    int eliminados = eliminarRenglones(quads, borrar);
    contar(Tecnica::EstadosInalcanzables, bloques);
    return eliminados > 0;
}

// (==, Tc, a, Td), (if-false, Td, ...), ([]=, a, cabezal, cinta) → sin el []=.
bool Optimizer::escriturasRedundantes(std::vector<Quad>& quads) {
    std::vector<bool> borrar(quads.size(), false);
    int eliminados = 0;
    for (size_t i = 0; i + 2 < quads.size(); ++i) {
        const Quad& comparar = quads[i];
        const Quad& saltar = quads[i + 1];
        const Quad& escribir = quads[i + 2];
        if (comparar.op == Op::Igual && comparar.arg2.clase == ClaseOperando::Simbolo &&
            saltar.op == Op::IfFalse && iguales(saltar.arg1, comparar.res) &&
            escribir.op == Op::EscribirCinta && iguales(escribir.arg1, comparar.arg2)) {
            borrar[i + 2] = true;
            ++eliminados;
        }
    }
    eliminarRenglones(quads, borrar);
    contar(Tecnica::EscriturasRedundantes, eliminados);
    return eliminados > 0;
}

// --------------------------------------------------------------- mirilla

bool Optimizer::saltosInnecesarios(std::vector<Quad>& quads) {
    int n = static_cast<int>(quads.size());
    bool cambio = false;

    // Un salto cuyo destino es un jump apunta al destino final.
    for (int i = 0; i < n; ++i) {
        if (!esSalto(quads[i].op)) continue;
        int original = destino(quads[i]);
        int final = original;
        std::set<int> vistos = {i + 1};
        bool ciclo = false;
        while (final >= 1 && final <= n && quads[final - 1].op == Op::Jump) {
            if (vistos.count(final)) {
                ciclo = true;
                break;
            }
            vistos.insert(final);
            final = destino(quads[final - 1]);
        }
        if (!ciclo && final != original) {
            ponerDestino(quads[i], final);
            contar(Tecnica::SaltosInnecesarios);
            cambio = true;
        }
    }

    // Un jump al renglón siguiente sobra.
    std::vector<bool> borrar(n, false);
    for (int i = 0; i < n; ++i) {
        if (quads[i].op == Op::Jump && destino(quads[i]) == i + 2) borrar[i] = true;
    }
    int eliminados = eliminarRenglones(quads, borrar);
    contar(Tecnica::SaltosInnecesarios, eliminados);
    return cambio || eliminados > 0;
}
