#include "frontend/SemanticAnalyzer.h"

#include <set>
#include <unordered_set>
#include <utility>

namespace {

bool esRelacional(const std::string& op) {
    return op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=";
}

// Envuelve un nodo en una conversión que agrega el traductor.
void envolverImplicita(std::unique_ptr<Expr>& expr, ClaseExpr clase, TipoDato tipo) {
    auto conversion = std::make_unique<Expr>();
    conversion->clase = clase;
    conversion->linea = expr->linea;
    conversion->columna = expr->columna;
    conversion->tipo = tipo;
    conversion->implicita = true;
    conversion->hijos.push_back(std::move(expr));
    expr = std::move(conversion);
}

// Valor inicial que se conoce al compilar: solo si la expresión es un literal.
std::string valorLiteral(const Expr& expr) {
    switch (expr.clase) {
        case ClaseExpr::Num:
        case ClaseExpr::Bool: return expr.texto;
        case ClaseExpr::Str:  return "\"" + expr.texto + "\"";
        default:              return "";
    }
}

std::string mensajeIncompatibles(TipoDato esperado, TipoDato encontrado) {
    return std::string("tipos incompatibles: se esperaba ") + nombreTipoDato(esperado) +
           " y se encontró " + nombreTipoDato(encontrado);
}

}  // namespace

SemanticAnalyzer::SemanticAnalyzer(Programa& programa, SymbolTable& tabla, ErrorHandler& errores)
    : programa(programa), tabla(tabla), errores(errores) {}

void SemanticAnalyzer::reportar(int linea, int columna, const std::string& mensaje) {
    errores.reportar(linea, columna, Fase::Semantico, mensaje);
}

void SemanticAnalyzer::analizar() {
    for (const Maquina& maquina : programa.maquinas) {
        analizarMaquina(maquina);
    }
    analizarSentencias(programa.sentencias);
}

// ------------------------------------------------------------------ máquinas

void SemanticAnalyzer::analizarMaquina(const Maquina& maq) {
    // 1. La máquina.
    Simbolo simMaquina;
    simMaquina.nombre = maq.nombre;
    simMaquina.tipo = TipoDato::Machine;
    simMaquina.scope = 0;
    if (!tabla.insertar(maq.nombre, simMaquina)) {
        reportar(maq.linea, maq.columna, "máquina duplicada: " + maq.nombre);
        return;
    }

    // 2 y 3. Bloques state; se guardan los que son válidos para revisar sus reglas.
    std::set<std::string> bloques;
    std::vector<const Estado*> validos;
    for (const Estado& est : maq.estados) {
        if (est.nombre == maq.aceptacion) {
            reportar(est.linea, est.columna,
                     "el estado de aceptación no lleva reglas: " + est.nombre);
            continue;
        }
        Simbolo simEstado;
        simEstado.nombre = est.nombre;
        simEstado.tipo = TipoDato::State;
        simEstado.scope = 1;
        if (!tabla.insertar(maq.nombre + "." + est.nombre, simEstado)) {
            reportar(est.linea, est.columna, "estado duplicado: " + est.nombre);
            continue;
        }
        bloques.insert(est.nombre);
        validos.push_back(&est);
    }

    // 4. Estado de aceptación, después de todos los bloques state.
    Simbolo simAceptacion;
    simAceptacion.nombre = maq.aceptacion;
    simAceptacion.tipo = TipoDato::State;
    simAceptacion.scope = 1;
    tabla.insertar(maq.nombre + "." + maq.aceptacion, simAceptacion);

    // 5. Estado inicial.
    if (bloques.count(maq.inicial) == 0) {
        reportar(maq.lineaInicial, maq.columnaInicial,
                 "estado inicial no declarado: " + maq.inicial);
    }

    // 6 y 7. Reglas de los estados válidos, ya con todos los estados conocidos.
    for (const Estado* est : validos) {
        std::unordered_set<std::string> leidos;
        for (const Regla& r : est->reglas) {
            if (bloques.count(r.destino) == 0 && r.destino != maq.aceptacion) {
                reportar(r.lineaDestino, r.columnaDestino, "estado no declarado: " + r.destino);
            }
            if (!leidos.insert(r.lee).second) {
                reportar(r.linea, r.columna, "regla duplicada para el símbolo " + r.lee);
            }
        }
    }
}

// --------------------------------------------------------------- sentencias

void SemanticAnalyzer::analizarSentencias(std::vector<std::unique_ptr<Stmt>>& sentencias) {
    for (auto& stmt : sentencias) {
        analizarSentencia(*stmt);
    }
}

void SemanticAnalyzer::analizarSentencia(Stmt& stmt) {
    switch (stmt.clase) {
        case ClaseStmt::Decl:
            analizarDecl(stmt);
            break;
        case ClaseStmt::Asignacion:
            analizarAsignacion(stmt);
            break;
        case ClaseStmt::If:
            analizarCondicion(stmt);
            analizarSentencias(stmt.cuerpo);
            if (stmt.tieneElse) {
                analizarSentencias(stmt.cuerpoElse);
            }
            break;
        case ClaseStmt::While:
            analizarCondicion(stmt);
            analizarSentencias(stmt.cuerpo);
            break;
        case ClaseStmt::Print: {
            TipoDato tipo = analizarExpr(stmt.expr);
            bool valido = tipo == TipoDato::Int || tipo == TipoDato::String ||
                          tipo == TipoDato::Bool || tipo == TipoDato::Desconocido;
            if (!valido) {
                reportar(stmt.linea, stmt.columna,
                         "argumentos inválidos: print espera int, string o bool");
            }
            break;
        }
    }
}

void SemanticAnalyzer::analizarDecl(Stmt& stmt) {
    // La variable se inserta después de analizar su expresión.
    TipoDato tipo = analizarValor(stmt.expr, stmt.tipoDecl);
    bool tiposCorrectos = tipo == stmt.tipoDecl;
    if (!tiposCorrectos && tipo != TipoDato::Desconocido) {
        reportar(stmt.expr->linea, stmt.expr->columna, mensajeIncompatibles(stmt.tipoDecl, tipo));
    }

    if (tabla.buscar(stmt.nombre) != nullptr) {
        reportar(stmt.lineaNombre, stmt.columnaNombre, "variable duplicada: " + stmt.nombre);
        return;
    }

    // Se inserta aunque la expresión tenga error, para evitar errores en cascada.
    Simbolo simbolo;
    simbolo.nombre = stmt.nombre;
    simbolo.tipo = stmt.tipoDecl;
    simbolo.scope = 0;   // todas las variables son globales
    simbolo.valor = tiposCorrectos ? valorLiteral(*stmt.expr) : "";
    simbolo.direccion = tabla.reservar(stmt.tipoDecl);
    tabla.insertar(stmt.nombre, simbolo);
}

void SemanticAnalyzer::analizarAsignacion(Stmt& stmt) {
    const Simbolo* variable = tabla.buscar(stmt.nombre);
    if (variable == nullptr) {
        reportar(stmt.lineaNombre, stmt.columnaNombre, "variable no declarada: " + stmt.nombre);
        analizarExpr(stmt.expr);
        return;
    }
    TipoDato tipoVariable = variable->tipo;
    TipoDato tipo = analizarValor(stmt.expr, tipoVariable);
    if (tipo != tipoVariable && tipo != TipoDato::Desconocido) {
        reportar(stmt.expr->linea, stmt.expr->columna, mensajeIncompatibles(tipoVariable, tipo));
    }
}

void SemanticAnalyzer::analizarCondicion(Stmt& stmt) {
    TipoDato tipo = analizarExpr(stmt.expr);
    if (tipo != TipoDato::Bool && tipo != TipoDato::Desconocido) {
        reportar(stmt.expr->linea, stmt.expr->columna, "la condición debe ser bool");
    }
}

TipoDato SemanticAnalyzer::analizarValor(std::unique_ptr<Expr>& expr, TipoDato tipoDestino) {
    // Caso especial: input() asignado directo a un int lleva conversión implícita.
    if (expr->clase == ClaseExpr::Input && tipoDestino == TipoDato::Int) {
        analizarExpr(expr);
        envolverImplicita(expr, ClaseExpr::ConvInt, TipoDato::Int);
        return TipoDato::Int;
    }
    return analizarExpr(expr);
}

// -------------------------------------------------------------- expresiones

TipoDato SemanticAnalyzer::analizarExpr(std::unique_ptr<Expr>& expr) {
    Expr& e = *expr;
    switch (e.clase) {
        case ClaseExpr::Num:
            e.tipo = TipoDato::Int;
            break;
        case ClaseExpr::Str:
            e.tipo = TipoDato::String;
            break;
        case ClaseExpr::Bool:
            e.tipo = TipoDato::Bool;
            break;
        case ClaseExpr::Var: {
            const Simbolo* simbolo = tabla.buscar(e.texto);
            if (simbolo == nullptr) {
                reportar(e.linea, e.columna, "variable no declarada: " + e.texto);
                e.tipo = TipoDato::Desconocido;
            } else {
                e.tipo = simbolo->tipo;
            }
            break;
        }
        case ClaseExpr::Binaria:
            return analizarBinaria(expr);
        default:
            e.tipo = analizarLlamada(e);
            break;
    }
    return e.tipo;
}

TipoDato SemanticAnalyzer::analizarBinaria(std::unique_ptr<Expr>& expr) {
    Expr& e = *expr;
    TipoDato izq = analizarExpr(e.hijos[0]);
    TipoDato der = analizarExpr(e.hijos[1]);
    const std::string& op = e.texto;

    e.tipo = TipoDato::Desconocido;

    // Sin cascadas: un operando con error ya se reportó.
    if (izq == TipoDato::Desconocido || der == TipoDato::Desconocido) {
        return e.tipo;
    }

    if (izq == TipoDato::Int && der == TipoDato::Int) {
        e.tipo = esRelacional(op) ? TipoDato::Bool : TipoDato::Int;
    } else if (op == "+" && izq == TipoDato::String && der == TipoDato::String) {
        e.tipo = TipoDato::String;
    } else if (op == "+" && izq == TipoDato::String &&
               (der == TipoDato::Int || der == TipoDato::Bool)) {
        envolverImplicita(e.hijos[1], ClaseExpr::ConvString, TipoDato::String);
        e.tipo = TipoDato::String;
    } else if (op == "+" && der == TipoDato::String &&
               (izq == TipoDato::Int || izq == TipoDato::Bool)) {
        envolverImplicita(e.hijos[0], ClaseExpr::ConvString, TipoDato::String);
        e.tipo = TipoDato::String;
    } else {
        reportar(e.linea, e.columna, std::string("tipos incompatibles: ") +
                                         nombreTipoDato(izq) + " " + op + " " +
                                         nombreTipoDato(der));
    }
    return e.tipo;
}

// Run, Output, Repeat, Input, ConvInt y ConvString conservan su tipo de resultado
// aunque sus argumentos tengan error.
TipoDato SemanticAnalyzer::analizarLlamada(Expr& e) {
    std::vector<TipoDato> args;
    for (auto& hijo : e.hijos) {
        args.push_back(analizarExpr(hijo));
    }
    bool hayDesconocido = false;
    for (TipoDato t : args) {
        if (t == TipoDato::Desconocido) hayDesconocido = true;
    }
    auto cadenaYEntero = [&]() {
        return args.size() == 2 && args[0] == TipoDato::String && args[1] == TipoDato::Int;
    };

    switch (e.clase) {
        case ClaseExpr::Run:
        case ClaseExpr::Output: {
            const char* nombre = e.clase == ClaseExpr::Run ? "run" : "output";
            const Simbolo* simbolo = tabla.buscar(e.texto);
            if (simbolo == nullptr || simbolo->tipo != TipoDato::Machine) {
                reportar(e.linea, e.columna, "argumentos inválidos: " + e.texto + " no es una máquina");
            } else if (!hayDesconocido && !cadenaYEntero()) {
                reportar(e.linea, e.columna,
                         std::string("argumentos inválidos: ") + nombre + " espera (string, int)");
            }
            return e.clase == ClaseExpr::Run ? TipoDato::Bool : TipoDato::String;
        }
        case ClaseExpr::Repeat:
            if (!hayDesconocido && !cadenaYEntero()) {
                reportar(e.linea, e.columna, "argumentos inválidos: repeat espera (string, int)");
            }
            return TipoDato::String;
        case ClaseExpr::Input:
            return TipoDato::String;
        case ClaseExpr::ConvInt:
            if (!hayDesconocido && args[0] != TipoDato::String) {
                reportar(e.linea, e.columna, "conversión inválida: int() espera string");
            }
            return TipoDato::Int;
        case ClaseExpr::ConvString:
            if (!hayDesconocido && args[0] != TipoDato::Int && args[0] != TipoDato::Bool) {
                reportar(e.linea, e.columna, "conversión inválida: string() espera int o bool");
            }
            return TipoDato::String;
        default:
            return TipoDato::Desconocido;
    }
}
