#pragma once

enum class TipoDato { Int, String, Bool, Machine, State, Void, Desconocido };

// "int", "string", "bool", "machine", "state", "void", "?"
inline const char* nombreTipoDato(TipoDato tipo) {
    switch (tipo) {
        case TipoDato::Int:         return "int";
        case TipoDato::String:      return "string";
        case TipoDato::Bool:        return "bool";
        case TipoDato::Machine:     return "machine";
        case TipoDato::State:       return "state";
        case TipoDato::Void:        return "void";
        case TipoDato::Desconocido: return "?";
    }
    return "?";
}
