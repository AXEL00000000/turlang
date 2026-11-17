#pragma once

// Este archivo también se compila en el ESP32: sin iostream ni excepciones.

#include <string>

enum class Op {
    Asignar,                                   // =
    Suma, Resta, Mult, Div,                    // + - * /
    Menor, Mayor, MenorIgual, MayorIgual, Igual, Distinto,   // < > <= >= == !=
    IfFalse, Jump,                             // if-false  jump
    Param, Call, Ret,                          // param  call  ret
    LeerCinta, EscribirCinta,                  // =[]  []=
    ConvInt, ConvString                        // int  string
};

enum class ClaseOperando {
    Nada, Var, Temp, Entero, Cadena, Bool, Simbolo, Salto, Funcion, Cinta
};

struct Operando {
    ClaseOperando clase = ClaseOperando::Nada;
    std::string texto;
};

struct Quad {
    Op op;
    Operando arg1;
    Operando arg2;
    Operando res;
    int linea = 0;        // línea del programa fuente; no se imprime
};

inline const char* nombreOp(Op op) {
    switch (op) {
        case Op::Asignar:       return "=";
        case Op::Suma:          return "+";
        case Op::Resta:         return "-";
        case Op::Mult:          return "*";
        case Op::Div:           return "/";
        case Op::Menor:         return "<";
        case Op::Mayor:         return ">";
        case Op::MenorIgual:    return "<=";
        case Op::MayorIgual:    return ">=";
        case Op::Igual:         return "==";
        case Op::Distinto:      return "!=";
        case Op::IfFalse:       return "if-false";
        case Op::Jump:          return "jump";
        case Op::Param:         return "param";
        case Op::Call:          return "call";
        case Op::Ret:           return "ret";
        case Op::LeerCinta:     return "=[]";
        case Op::EscribirCinta: return "[]=";
        case Op::ConvInt:       return "int";
        case Op::ConvString:    return "string";
    }
    return "";
}

// Como se muestra en una tabla.
inline std::string textoOperando(const Operando& o) {
    switch (o.clase) {
        case ClaseOperando::Nada:   return "---";
        case ClaseOperando::Cadena: return "\"" + o.texto + "\"";
        case ClaseOperando::Salto:  return "(" + o.texto + ")";
        default:                    return o.texto;
    }
}
