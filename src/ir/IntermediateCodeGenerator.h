#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "frontend/Ast.h"
#include "runtime/Program.h"

class IntermediateCodeGenerator {
public:
    explicit IntermediateCodeGenerator(const Programa& programa);
    Program generar();
private:
    const Programa& programa;
    Bloque* bloque = nullptr;     // bloque que se está generando
    int contadorTemp = 0;
    int lineaActual = 0;

    // Auxiliares
    void iniciarBloque(Bloque& destino);
    int emitir(Op op, const Operando& arg1, const Operando& arg2, const Operando& res);
    int siguienteRenglon() const;
    void rellenarSalto(int renglon, int destino);
    Operando nuevoTemp();

    // Máquinas
    void generarMaquina(const Maquina& maquina, Bloque& destino);
    void generarEstado(const Maquina& maquina, const Estado& estado,
                       std::vector<std::pair<int, std::string>>& saltosAEstados,
                       std::vector<int>& saltosARechazo);

    // Sección de control
    void generarSentencias(const std::vector<std::unique_ptr<Stmt>>& sentencias);
    void generarSentencia(const Stmt& stmt);
    Operando generarExpr(const Expr& expr);
    Operando generarLlamada(const Operando& funcion, const std::vector<Operando>& args);
};
