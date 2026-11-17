#include "frontend/AstPrinter.h"

#include <string>

namespace {

void linea(std::ostream& salida, int nivel, const std::string& texto) {
    salida << std::string(static_cast<size_t>(nivel) * 2, ' ') << texto << '\n';
}

std::string textoExpr(const Expr& expr) {
    switch (expr.clase) {
        case ClaseExpr::Num:        return "Num " + expr.texto;
        case ClaseExpr::Str:        return "Str \"" + expr.texto + "\"";
        case ClaseExpr::Bool:       return "Bool " + expr.texto;
        case ClaseExpr::Var:        return "Var " + expr.texto;
        case ClaseExpr::Binaria:    return "Binaria " + expr.texto;
        case ClaseExpr::Run:        return "Run " + expr.texto;
        case ClaseExpr::Output:     return "Output " + expr.texto;
        case ClaseExpr::Repeat:     return "Repeat";
        case ClaseExpr::Input:      return "Input";
        case ClaseExpr::ConvInt:    return "ConvInt";
        case ClaseExpr::ConvString: return "ConvString";
    }
    return "";
}

void imprimirExpr(const Expr& expr, std::ostream& salida, int nivel) {
    linea(salida, nivel, textoExpr(expr));
    for (const auto& hijo : expr.hijos) {
        imprimirExpr(*hijo, salida, nivel + 1);
    }
}

void imprimirSentencias(const std::vector<std::unique_ptr<Stmt>>& sentencias,
                        std::ostream& salida, int nivel);

void imprimirStmt(const Stmt& stmt, std::ostream& salida, int nivel) {
    switch (stmt.clase) {
        case ClaseStmt::Decl:
            linea(salida, nivel, std::string("Decl ") + nombreTipoDato(stmt.tipoDecl) + " " +
                                     stmt.nombre);
            imprimirExpr(*stmt.expr, salida, nivel + 1);
            break;
        case ClaseStmt::Asignacion:
            linea(salida, nivel, "Asignacion " + stmt.nombre);
            imprimirExpr(*stmt.expr, salida, nivel + 1);
            break;
        case ClaseStmt::If:
            linea(salida, nivel, "If");
            linea(salida, nivel + 1, "Cond");
            imprimirExpr(*stmt.expr, salida, nivel + 2);
            linea(salida, nivel + 1, "Entonces");
            imprimirSentencias(stmt.cuerpo, salida, nivel + 2);
            if (stmt.tieneElse) {
                linea(salida, nivel + 1, "Sino");
                imprimirSentencias(stmt.cuerpoElse, salida, nivel + 2);
            }
            break;
        case ClaseStmt::While:
            linea(salida, nivel, "While");
            linea(salida, nivel + 1, "Cond");
            imprimirExpr(*stmt.expr, salida, nivel + 2);
            linea(salida, nivel + 1, "Cuerpo");
            imprimirSentencias(stmt.cuerpo, salida, nivel + 2);
            break;
        case ClaseStmt::Print:
            linea(salida, nivel, "Print");
            imprimirExpr(*stmt.expr, salida, nivel + 1);
            break;
    }
}

void imprimirSentencias(const std::vector<std::unique_ptr<Stmt>>& sentencias,
                        std::ostream& salida, int nivel) {
    for (const auto& stmt : sentencias) {
        imprimirStmt(*stmt, salida, nivel);
    }
}

}  // namespace

void imprimirArbol(const Programa& programa, std::ostream& salida) {
    linea(salida, 0, "Programa");
    for (const Maquina& maq : programa.maquinas) {
        linea(salida, 1, "Maquina " + maq.nombre + " (inicio: " + maq.inicial +
                             ", acepta: " + maq.aceptacion + ")");
        for (const Estado& est : maq.estados) {
            linea(salida, 2, "Estado " + est.nombre);
            for (const Regla& r : est.reglas) {
                linea(salida, 3, "Regla " + r.lee + " -> " + r.escribe + ", " +
                                     std::string(1, r.movimiento) + ", " + r.destino);
            }
        }
    }
    imprimirSentencias(programa.sentencias, salida, 1);
}
