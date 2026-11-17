#include "frontend/Parser.h"

#include <utility>

namespace {

// Excepción interna para recuperarse de un error sintáctico.
struct ErrorSintactico {};

const char* const DESC_SIMBOLO = "un símbolo (letra, dígito, blank o *)";
const char* const DESC_MOVIMIENTO = "un movimiento (R, L o S)";
const char* const DESC_ID = "un identificador";

std::unique_ptr<Expr> nuevaExpr(ClaseExpr clase, const Token& token, const std::string& texto) {
    auto expr = std::make_unique<Expr>();
    expr->clase = clase;
    expr->linea = token.linea;
    expr->columna = token.columna;
    expr->texto = texto;
    return expr;
}

bool esRelacional(TipoToken tipo) {
    return tipo == TipoToken::Menor || tipo == TipoToken::Mayor ||
           tipo == TipoToken::MenorIgual || tipo == TipoToken::MayorIgual ||
           tipo == TipoToken::Igual || tipo == TipoToken::Distinto;
}

}  // namespace

Parser::Parser(const std::vector<Token>& tokens, ErrorHandler& errores)
    : tokens(tokens), errores(errores) {}

Programa Parser::analizar() {
    pos = 0;
    return programa();
}

// ---------------------------------------------------------------- auxiliares

const Token& Parser::actual() const {
    return tokens[pos];
}

bool Parser::es(TipoToken tipo) const {
    return actual().tipo == tipo;
}

// Nunca avanza más allá del token Fin.
void Parser::avanzar() {
    if (pos + 1 < tokens.size()) {
        ++pos;
    }
}

bool Parser::coincide(TipoToken tipo) {
    if (es(tipo)) {
        avanzar();
        return true;
    }
    return false;
}

const Token& Parser::esperar(TipoToken tipo, const std::string& descripcion) {
    if (!es(tipo)) {
        errorEsperado(descripcion);
    }
    const Token& token = actual();
    avanzar();
    return token;
}

void Parser::reportarEsperado(const std::string& descripcion) {
    errores.reportar(actual().linea, actual().columna, Fase::Sintactico,
                     "se esperaba " + descripcion + " y se encontró " + describir(actual()));
}

void Parser::errorEsperado(const std::string& descripcion) {
    reportarEsperado(descripcion);
    throw ErrorSintactico{};
}

std::string Parser::describir(const Token& token) const {
    if (token.tipo == TipoToken::Fin) {
        return "el fin del archivo";
    }
    if (token.tipo == TipoToken::Str) {
        return "\"" + token.lexema + "\"";
    }
    return "'" + token.lexema + "'";
}

// ------------------------------------------------------ recuperación de errores

// Encabezado de una máquina o de un estado: avanza hasta State, Machine o Fin.
void Parser::sincronizarEncabezado(size_t inicio) {
    while (!es(TipoToken::State) && !es(TipoToken::Machine) && !es(TipoToken::Fin)) {
        avanzar();
    }
    if (pos == inicio) {
        avanzar();
    }
}

// Regla: salta los tokens de la misma línea que el primer token de la regla.
void Parser::sincronizarRegla(size_t inicio) {
    int lineaRegla = tokens[inicio].linea;
    while (!es(TipoToken::Fin) && actual().linea == lineaRegla) {
        avanzar();
    }
    if (pos == inicio) {
        avanzar();
    }
}

// Sentencia: hasta consumir un ';' o quedar frente a If, While, Print, '}' o Fin.
void Parser::sincronizarSentencia(size_t inicio) {
    while (!es(TipoToken::Fin)) {
        if (es(TipoToken::PuntoComa)) {
            avanzar();
            break;
        }
        if (es(TipoToken::If) || es(TipoToken::While) || es(TipoToken::Print) ||
            es(TipoToken::LlaveDer)) {
            break;
        }
        avanzar();
    }
    if (pos == inicio) {
        avanzar();
    }
}

// ------------------------------------------------------------------ máquinas

Programa Parser::programa() {
    Programa prog;

    while (es(TipoToken::Machine)) {
        Maquina maq;
        if (maquina(maq)) {
            prog.maquinas.push_back(std::move(maq));
        }
    }

    // Al menos una sentencia.
    do {
        std::unique_ptr<Stmt> stmt = sentenciaRecuperable();
        if (stmt) {
            prog.sentencias.push_back(std::move(stmt));
        }
    } while (!es(TipoToken::Fin));

    return prog;
}

bool Parser::maquina(Maquina& maq) {
    size_t inicio = pos;
    bool encabezadoCorrecto = true;
    bool correcta = true;

    try {
        esperar(TipoToken::Machine, "'machine'");
        const Token& nombre = esperar(TipoToken::Id, DESC_ID);
        maq.nombre = nombre.lexema;
        maq.linea = nombre.linea;
        maq.columna = nombre.columna;
        esperar(TipoToken::LlaveIzq, "'{'");

        esperar(TipoToken::Start, "'start'");
        const Token& inicial = esperar(TipoToken::Id, DESC_ID);
        maq.inicial = inicial.lexema;
        maq.lineaInicial = inicial.linea;
        maq.columnaInicial = inicial.columna;

        esperar(TipoToken::Accept, "'accept'");
        const Token& aceptacion = esperar(TipoToken::Id, DESC_ID);
        maq.aceptacion = aceptacion.lexema;
        maq.lineaAceptacion = aceptacion.linea;
        maq.columnaAceptacion = aceptacion.columna;

        // Al menos un estado.
        if (!es(TipoToken::State)) {
            errorEsperado("'state'");
        }
    } catch (const ErrorSintactico&) {
        encabezadoCorrecto = false;
        correcta = false;
        sincronizarEncabezado(inicio);
    }

    while (es(TipoToken::State)) {
        Estado est;
        if (estado(est)) {
            maq.estados.push_back(std::move(est));
        } else {
            correcta = false;
        }
    }

    if (encabezadoCorrecto) {
        try {
            esperar(TipoToken::LlaveDer, "'}'");
        } catch (const ErrorSintactico&) {
            correcta = false;
            sincronizarEncabezado(inicio);
        }
    } else {
        // Tras un encabezado con error no se reporta la llave que cierra.
        coincide(TipoToken::LlaveDer);
    }

    return correcta;
}

bool Parser::estado(Estado& est) {
    size_t inicio = pos;

    try {
        esperar(TipoToken::State, "'state'");
        const Token& nombre = esperar(TipoToken::Id, DESC_ID);
        est.nombre = nombre.lexema;
        est.linea = nombre.linea;
        est.columna = nombre.columna;
        esperar(TipoToken::LlaveIzq, "'{'");
    } catch (const ErrorSintactico&) {
        sincronizarEncabezado(inicio);
        return false;
    }

    bool correcto = true;

    // Al menos una regla. Se reporta sin consumir la llave, que cierra el estado.
    if (es(TipoToken::LlaveDer)) {
        reportarEsperado(DESC_SIMBOLO);
        correcto = false;
    }

    while (!es(TipoToken::LlaveDer) && !es(TipoToken::State) &&
           !es(TipoToken::Machine) && !es(TipoToken::Fin)) {
        size_t inicioRegla = pos;
        try {
            Regla r;
            regla(r);
            est.reglas.push_back(std::move(r));
        } catch (const ErrorSintactico&) {
            correcto = false;
            sincronizarRegla(inicioRegla);
        }
    }

    try {
        esperar(TipoToken::LlaveDer, "'}'");
    } catch (const ErrorSintactico&) {
        sincronizarEncabezado(inicio);
        return false;
    }
    return correcto;
}

void Parser::regla(Regla& r) {
    r.linea = actual().linea;
    r.columna = actual().columna;
    r.lee = simbolo();
    esperar(TipoToken::Flecha, "'->'");
    r.escribe = simbolo();
    esperar(TipoToken::Coma, "','");
    r.movimiento = movimiento();
    esperar(TipoToken::Coma, "','");
    const Token& destino = esperar(TipoToken::Id, DESC_ID);
    r.destino = destino.lexema;
    r.lineaDestino = destino.linea;
    r.columnaDestino = destino.columna;
}

// Num de un dígito, Id de una letra, Blank o Por.
std::string Parser::simbolo() {
    const Token& t = actual();
    bool valido = ((t.tipo == TipoToken::Num || t.tipo == TipoToken::Id) && t.lexema.size() == 1) ||
                  t.tipo == TipoToken::Blank || t.tipo == TipoToken::Por;
    if (!valido) {
        errorEsperado(DESC_SIMBOLO);
    }
    avanzar();
    return t.lexema;
}

// Id con lexema exactamente R, L o S.
char Parser::movimiento() {
    const Token& t = actual();
    if (t.tipo != TipoToken::Id || (t.lexema != "R" && t.lexema != "L" && t.lexema != "S")) {
        errorEsperado(DESC_MOVIMIENTO);
    }
    avanzar();
    return t.lexema[0];
}

// --------------------------------------------------------------- sentencias

std::vector<std::unique_ptr<Stmt>> Parser::bloque() {
    std::vector<std::unique_ptr<Stmt>> sentencias;
    esperar(TipoToken::LlaveIzq, "'{'");

    // Al menos una sentencia. Se reporta sin consumir la llave, que cierra el bloque.
    if (es(TipoToken::LlaveDer)) {
        reportarEsperado("una sentencia");
    }

    while (!es(TipoToken::LlaveDer) && !es(TipoToken::Fin)) {
        std::unique_ptr<Stmt> stmt = sentenciaRecuperable();
        if (stmt) {
            sentencias.push_back(std::move(stmt));
        }
    }

    esperar(TipoToken::LlaveDer, "'}'");
    return sentencias;
}

// Punto de recuperación de una sentencia: con error regresa nullptr.
std::unique_ptr<Stmt> Parser::sentenciaRecuperable() {
    size_t inicio = pos;
    try {
        return sentencia();
    } catch (const ErrorSintactico&) {
        sincronizarSentencia(inicio);
        return nullptr;
    }
}

std::unique_ptr<Stmt> Parser::sentencia() {
    const Token& primero = actual();
    auto stmt = std::make_unique<Stmt>();
    stmt->linea = primero.linea;
    stmt->columna = primero.columna;

    switch (primero.tipo) {
        case TipoToken::Int:
        case TipoToken::String:
        case TipoToken::Bool: {
            stmt->clase = ClaseStmt::Decl;
            stmt->tipoDecl = primero.tipo == TipoToken::Int    ? TipoDato::Int
                           : primero.tipo == TipoToken::String ? TipoDato::String
                                                               : TipoDato::Bool;
            avanzar();
            const Token& nombre = esperar(TipoToken::Id, DESC_ID);
            stmt->nombre = nombre.lexema;
            stmt->lineaNombre = nombre.linea;
            stmt->columnaNombre = nombre.columna;
            esperar(TipoToken::Asignar, "'='");
            stmt->expr = expresion();
            esperar(TipoToken::PuntoComa, "';'");
            break;
        }
        case TipoToken::Id: {
            stmt->clase = ClaseStmt::Asignacion;
            stmt->nombre = primero.lexema;
            stmt->lineaNombre = primero.linea;
            stmt->columnaNombre = primero.columna;
            avanzar();
            esperar(TipoToken::Asignar, "'='");
            stmt->expr = expresion();
            esperar(TipoToken::PuntoComa, "';'");
            break;
        }
        case TipoToken::If: {
            stmt->clase = ClaseStmt::If;
            avanzar();
            esperar(TipoToken::ParIzq, "'('");
            stmt->expr = condicion();
            esperar(TipoToken::ParDer, "')'");
            stmt->cuerpo = bloque();
            if (coincide(TipoToken::Else)) {
                stmt->tieneElse = true;
                stmt->cuerpoElse = bloque();
            }
            break;
        }
        case TipoToken::While: {
            stmt->clase = ClaseStmt::While;
            avanzar();
            esperar(TipoToken::ParIzq, "'('");
            stmt->expr = condicion();
            esperar(TipoToken::ParDer, "')'");
            stmt->cuerpo = bloque();
            break;
        }
        case TipoToken::Print: {
            stmt->clase = ClaseStmt::Print;
            avanzar();
            esperar(TipoToken::ParIzq, "'('");
            stmt->expr = expresion();
            esperar(TipoToken::ParDer, "')'");
            esperar(TipoToken::PuntoComa, "';'");
            break;
        }
        default:
            errorEsperado("una sentencia");
    }
    return stmt;
}

// -------------------------------------------------------------- expresiones

std::unique_ptr<Expr> Parser::condicion() {
    std::unique_ptr<Expr> izquierda = expresion();
    if (esRelacional(actual().tipo)) {
        const Token& op = actual();
        avanzar();
        auto binaria = nuevaExpr(ClaseExpr::Binaria, op, op.lexema);
        binaria->hijos.push_back(std::move(izquierda));
        binaria->hijos.push_back(expresion());
        return binaria;
    }
    return izquierda;
}

std::unique_ptr<Expr> Parser::expresion() {
    std::unique_ptr<Expr> izquierda = termino();
    while (es(TipoToken::Mas) || es(TipoToken::Menos)) {
        const Token& op = actual();
        avanzar();
        auto binaria = nuevaExpr(ClaseExpr::Binaria, op, op.lexema);
        binaria->hijos.push_back(std::move(izquierda));
        binaria->hijos.push_back(termino());
        izquierda = std::move(binaria);
    }
    return izquierda;
}

std::unique_ptr<Expr> Parser::termino() {
    std::unique_ptr<Expr> izquierda = factor();
    while (es(TipoToken::Por) || es(TipoToken::Entre)) {
        const Token& op = actual();
        avanzar();
        auto binaria = nuevaExpr(ClaseExpr::Binaria, op, op.lexema);
        binaria->hijos.push_back(std::move(izquierda));
        binaria->hijos.push_back(factor());
        izquierda = std::move(binaria);
    }
    return izquierda;
}

std::unique_ptr<Expr> Parser::factor() {
    const Token& t = actual();

    switch (t.tipo) {
        case TipoToken::ParIzq: {
            avanzar();
            std::unique_ptr<Expr> interna = expresion();
            esperar(TipoToken::ParDer, "')'");
            return interna;
        }
        case TipoToken::Id:
            avanzar();
            return nuevaExpr(ClaseExpr::Var, t, t.lexema);
        case TipoToken::Num:
            avanzar();
            return nuevaExpr(ClaseExpr::Num, t, t.lexema);
        case TipoToken::Str:
            avanzar();
            return nuevaExpr(ClaseExpr::Str, t, t.lexema);
        case TipoToken::True:
        case TipoToken::False:
            avanzar();
            return nuevaExpr(ClaseExpr::Bool, t, t.lexema);
        case TipoToken::Run:
        case TipoToken::Output: {
            avanzar();
            const Token& nombre = esperar(TipoToken::Id, DESC_ID);
            auto llamada = nuevaExpr(t.tipo == TipoToken::Run ? ClaseExpr::Run : ClaseExpr::Output,
                                     t, nombre.lexema);
            esperar(TipoToken::ParIzq, "'('");
            llamada->hijos.push_back(expresion());
            esperar(TipoToken::Coma, "','");
            llamada->hijos.push_back(expresion());
            esperar(TipoToken::ParDer, "')'");
            return llamada;
        }
        case TipoToken::Repeat: {
            avanzar();
            auto llamada = nuevaExpr(ClaseExpr::Repeat, t, "");
            esperar(TipoToken::ParIzq, "'('");
            llamada->hijos.push_back(expresion());
            esperar(TipoToken::Coma, "','");
            llamada->hijos.push_back(expresion());
            esperar(TipoToken::ParDer, "')'");
            return llamada;
        }
        case TipoToken::Input: {
            avanzar();
            auto llamada = nuevaExpr(ClaseExpr::Input, t, "");
            esperar(TipoToken::ParIzq, "'('");
            esperar(TipoToken::ParDer, "')'");
            return llamada;
        }
        case TipoToken::Int:
        case TipoToken::String: {
            avanzar();
            auto conversion = nuevaExpr(
                t.tipo == TipoToken::Int ? ClaseExpr::ConvInt : ClaseExpr::ConvString, t, "");
            esperar(TipoToken::ParIzq, "'('");
            conversion->hijos.push_back(expresion());
            esperar(TipoToken::ParDer, "')'");
            return conversion;
        }
        default:
            errorEsperado("una expresión");
    }
}
