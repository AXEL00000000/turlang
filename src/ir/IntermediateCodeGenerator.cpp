#include "ir/IntermediateCodeGenerator.h"

#include <map>

namespace {

Operando operando(ClaseOperando clase, const std::string& texto) {
    Operando o;
    o.clase = clase;
    o.texto = texto;
    return o;
}

const Operando NADA{};

Operando var(const std::string& nombre)     { return operando(ClaseOperando::Var, nombre); }
Operando entero(const std::string& digitos) { return operando(ClaseOperando::Entero, digitos); }
Operando simbolo(const std::string& s)      { return operando(ClaseOperando::Simbolo, s); }
Operando funcion(const std::string& nombre) { return operando(ClaseOperando::Funcion, nombre); }
Operando cinta()                            { return operando(ClaseOperando::Cinta, "cinta"); }

Op opBinario(const std::string& texto) {
    static const std::map<std::string, Op> ops = {
        {"+", Op::Suma},        {"-", Op::Resta},       {"*", Op::Mult},
        {"/", Op::Div},         {"<", Op::Menor},       {">", Op::Mayor},
        {"<=", Op::MenorIgual}, {">=", Op::MayorIgual}, {"==", Op::Igual},
        {"!=", Op::Distinto},
    };
    return ops.at(texto);
}

}  // namespace

IntermediateCodeGenerator::IntermediateCodeGenerator(const Programa& programa)
    : programa(programa) {}

Program IntermediateCodeGenerator::generar() {
    Program resultado;

    for (const Maquina& maq : programa.maquinas) {
        Bloque b;
        b.nombre = maq.nombre;
        generarMaquina(maq, b);
        resultado.maquinas.push_back(std::move(b));
    }

    iniciarBloque(resultado.principal);
    generarSentencias(programa.sentencias);
    lineaActual = 0;
    emitir(Op::Ret, NADA, NADA, NADA);
    bloque = nullptr;

    return resultado;
}

// ---------------------------------------------------------------- auxiliares

void IntermediateCodeGenerator::iniciarBloque(Bloque& destino) {
    bloque = &destino;
    contadorTemp = 0;
}

// Agrega un cuádruplo y regresa su renglón (desde 1).
int IntermediateCodeGenerator::emitir(Op op, const Operando& arg1, const Operando& arg2,
                                      const Operando& res) {
    Quad q;
    q.op = op;
    q.arg1 = arg1;
    q.arg2 = arg2;
    q.res = res;
    q.linea = lineaActual;
    bloque->quads.push_back(q);
    return static_cast<int>(bloque->quads.size());
}

int IntermediateCodeGenerator::siguienteRenglon() const {
    return static_cast<int>(bloque->quads.size()) + 1;
}

// El destino de un salto va en el resultado del cuádruplo.
void IntermediateCodeGenerator::rellenarSalto(int renglon, int destino) {
    bloque->quads[renglon - 1].res = operando(ClaseOperando::Salto, std::to_string(destino));
}

Operando IntermediateCodeGenerator::nuevoTemp() {
    return operando(ClaseOperando::Temp, "T" + std::to_string(++contadorTemp));
}

// ------------------------------------------------------------------ máquinas

void IntermediateCodeGenerator::generarMaquina(const Maquina& maq, Bloque& destino) {
    iniciarBloque(destino);

    // 1. Inicio.
    lineaActual = maq.linea;
    emitir(Op::Asignar, var("cadena"), NADA, cinta());
    emitir(Op::Asignar, entero("0"), NADA, var("cabezal"));
    emitir(Op::Asignar, entero("0"), NADA, var("pasos"));
    emitir(Op::Asignar, var("maximo"), NADA, var("limite"));

    // 2 y 3. Estados: primero el inicial y después los demás en el orden del fuente.
    std::vector<const Estado*> orden;
    for (const Estado& est : maq.estados) {
        if (est.nombre == maq.inicial) orden.push_back(&est);
    }
    for (const Estado& est : maq.estados) {
        if (est.nombre != maq.inicial) orden.push_back(&est);
    }

    std::map<std::string, int> inicioEstado;
    std::vector<std::pair<int, std::string>> saltosAEstados;   // (renglón, estado destino)
    std::vector<int> saltosARechazo;
    for (const Estado* est : orden) {
        inicioEstado[est->nombre] = siguienteRenglon();
        generarEstado(maq, *est, saltosAEstados, saltosARechazo);
    }

    // 4. Rechazo.
    lineaActual = maq.linea;
    int rechazo = emitir(Op::Ret, operando(ClaseOperando::Bool, "false"), NADA, NADA);

    for (const auto& salto : saltosAEstados) {
        rellenarSalto(salto.first, inicioEstado.at(salto.second));
    }
    for (int renglon : saltosARechazo) {
        rellenarSalto(renglon, rechazo);
    }
    bloque = nullptr;
}

void IntermediateCodeGenerator::generarEstado(
        const Maquina& maq, const Estado& est,
        std::vector<std::pair<int, std::string>>& saltosAEstados,
        std::vector<int>& saltosARechazo) {
    lineaActual = est.linea;
    Operando ta = nuevoTemp();
    emitir(Op::Menor, var("pasos"), var("limite"), ta);
    saltosARechazo.push_back(emitir(Op::IfFalse, ta, NADA, NADA));
    Operando tb = nuevoTemp();
    emitir(Op::Suma, var("pasos"), entero("1"), tb);
    emitir(Op::Asignar, tb, NADA, var("pasos"));
    Operando tc = nuevoTemp();
    emitir(Op::LeerCinta, cinta(), var("cabezal"), tc);

    // Reglas con símbolo propio en el orden del fuente; la del comodín al final.
    std::vector<const Regla*> reglas;
    for (const Regla& r : est.reglas) {
        if (r.lee != "*") reglas.push_back(&r);
    }
    for (const Regla& r : est.reglas) {
        if (r.lee == "*") reglas.push_back(&r);
    }

    int saltoPendiente = 0;   // if-false de la regla anterior, hacia la siguiente
    for (const Regla* r : reglas) {
        lineaActual = r->linea;
        if (saltoPendiente != 0) {
            rellenarSalto(saltoPendiente, siguienteRenglon());
        }

        Operando td = nuevoTemp();
        if (r->lee == "*") {
            emitir(Op::Distinto, tc, simbolo("blank"), td);
        } else {
            emitir(Op::Igual, tc, simbolo(r->lee), td);
        }
        saltoPendiente = emitir(Op::IfFalse, td, NADA, NADA);

        if (r->escribe != "*") {
            emitir(Op::EscribirCinta, simbolo(r->escribe), var("cabezal"), cinta());
        }

        if (r->movimiento == 'R' || r->movimiento == 'L') {
            Operando te = nuevoTemp();
            emitir(r->movimiento == 'R' ? Op::Suma : Op::Resta, var("cabezal"), entero("1"), te);
            emitir(Op::Asignar, te, NADA, var("cabezal"));
        }

        if (r->destino == maq.aceptacion) {
            emitir(Op::Ret, operando(ClaseOperando::Bool, "true"), NADA, NADA);
        } else {
            saltosAEstados.push_back({emitir(Op::Jump, NADA, NADA, NADA), r->destino});
        }
    }
    // La última regla, si no coincide, rechaza.
    if (saltoPendiente != 0) {
        saltosARechazo.push_back(saltoPendiente);
    }
}

// --------------------------------------------------------- sección de control

void IntermediateCodeGenerator::generarSentencias(
        const std::vector<std::unique_ptr<Stmt>>& sentencias) {
    for (const auto& stmt : sentencias) {
        generarSentencia(*stmt);
    }
}

void IntermediateCodeGenerator::generarSentencia(const Stmt& stmt) {
    switch (stmt.clase) {
        case ClaseStmt::Decl:
        case ClaseStmt::Asignacion: {
            lineaActual = stmt.linea;
            Operando valor = generarExpr(*stmt.expr);
            emitir(Op::Asignar, valor, NADA, var(stmt.nombre));
            break;
        }
        case ClaseStmt::Print: {
            lineaActual = stmt.linea;
            Operando valor = generarExpr(*stmt.expr);
            emitir(Op::Param, valor, NADA, NADA);
            emitir(Op::Call, funcion("print"), entero("1"), NADA);
            break;
        }
        case ClaseStmt::If: {
            lineaActual = stmt.linea;
            Operando cond = generarExpr(*stmt.expr);
            int ifFalse = emitir(Op::IfFalse, cond, NADA, NADA);
            generarSentencias(stmt.cuerpo);
            if (stmt.tieneElse) {
                lineaActual = stmt.linea;
                int salto = emitir(Op::Jump, NADA, NADA, NADA);
                rellenarSalto(ifFalse, siguienteRenglon());
                generarSentencias(stmt.cuerpoElse);
                rellenarSalto(salto, siguienteRenglon());
            } else {
                rellenarSalto(ifFalse, siguienteRenglon());
            }
            break;
        }
        case ClaseStmt::While: {
            lineaActual = stmt.linea;
            int inicio = siguienteRenglon();
            Operando cond = generarExpr(*stmt.expr);
            int ifFalse = emitir(Op::IfFalse, cond, NADA, NADA);
            generarSentencias(stmt.cuerpo);
            lineaActual = stmt.linea;
            int salto = emitir(Op::Jump, NADA, NADA, NADA);
            rellenarSalto(salto, inicio);
            rellenarSalto(ifFalse, siguienteRenglon());
            break;
        }
    }
}

// Regresa el operando que contiene el valor de la expresión.
Operando IntermediateCodeGenerator::generarExpr(const Expr& e) {
    switch (e.clase) {
        case ClaseExpr::Num:  return entero(e.texto);
        case ClaseExpr::Str:  return operando(ClaseOperando::Cadena, e.texto);
        case ClaseExpr::Bool: return operando(ClaseOperando::Bool, e.texto);
        case ClaseExpr::Var:  return var(e.texto);
        case ClaseExpr::Binaria: {
            Operando a = generarExpr(*e.hijos[0]);
            Operando b = generarExpr(*e.hijos[1]);
            Operando t = nuevoTemp();
            emitir(opBinario(e.texto), a, b, t);
            return t;
        }
        case ClaseExpr::ConvInt:
        case ClaseExpr::ConvString: {
            Operando a = generarExpr(*e.hijos[0]);
            Operando t = nuevoTemp();
            emitir(e.clase == ClaseExpr::ConvInt ? Op::ConvInt : Op::ConvString, a, NADA, t);
            return t;
        }
        case ClaseExpr::Input:
            return generarLlamada(funcion("input"), {});
        case ClaseExpr::Repeat:
        case ClaseExpr::Run:
        case ClaseExpr::Output: {
            // Primero el código de todos los argumentos; después los param seguidos.
            std::vector<Operando> args;
            for (const auto& hijo : e.hijos) {
                args.push_back(generarExpr(*hijo));
            }
            if (e.clase == ClaseExpr::Repeat) {
                return generarLlamada(funcion("repeat"), args);
            }
            Operando resultado = generarLlamada(funcion(e.texto), args);
            if (e.clase == ClaseExpr::Run) {
                return resultado;
            }
            // output: la cinta final de la última máquina ejecutada.
            Operando t = nuevoTemp();
            emitir(Op::Asignar, cinta(), NADA, t);
            return t;
        }
    }
    return NADA;
}

Operando IntermediateCodeGenerator::generarLlamada(const Operando& fn,
                                                   const std::vector<Operando>& args) {
    for (const Operando& a : args) {
        emitir(Op::Param, a, NADA, NADA);
    }
    Operando t = nuevoTemp();
    emitir(Op::Call, fn, entero(std::to_string(args.size())), t);
    return t;
}
