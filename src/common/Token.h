#pragma once

#include <string>

enum class TipoToken {
    // Palabras reservadas
    Machine, Start, Accept, State,
    Int, String, Bool, True, False,
    If, Else, While,
    Run, Output, Repeat, Print, Input,
    Blank,
    // Identificadores y literales
    Id, Num, Str,
    // Operadores
    Mas, Menos, Por, Entre,
    Menor, Mayor, MenorIgual, MayorIgual, Igual, Distinto,
    Asignar, Flecha,
    // Agrupación
    ParIzq, ParDer, LlaveIzq, LlaveDer, PuntoComa, Coma,
    // Fin del archivo
    Fin
};

struct Token {
    TipoToken tipo;
    std::string lexema;
    int linea;
    int columna;
};

// Regresa el mismo texto que el nombre del enumerado: "Machine", "Id", "Flecha", ...
inline const char* nombreTipo(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::Machine:    return "Machine";
        case TipoToken::Start:      return "Start";
        case TipoToken::Accept:     return "Accept";
        case TipoToken::State:      return "State";
        case TipoToken::Int:        return "Int";
        case TipoToken::String:     return "String";
        case TipoToken::Bool:       return "Bool";
        case TipoToken::True:       return "True";
        case TipoToken::False:      return "False";
        case TipoToken::If:         return "If";
        case TipoToken::Else:       return "Else";
        case TipoToken::While:      return "While";
        case TipoToken::Run:        return "Run";
        case TipoToken::Output:     return "Output";
        case TipoToken::Repeat:     return "Repeat";
        case TipoToken::Print:      return "Print";
        case TipoToken::Input:      return "Input";
        case TipoToken::Blank:      return "Blank";
        case TipoToken::Id:         return "Id";
        case TipoToken::Num:        return "Num";
        case TipoToken::Str:        return "Str";
        case TipoToken::Mas:        return "Mas";
        case TipoToken::Menos:      return "Menos";
        case TipoToken::Por:        return "Por";
        case TipoToken::Entre:      return "Entre";
        case TipoToken::Menor:      return "Menor";
        case TipoToken::Mayor:      return "Mayor";
        case TipoToken::MenorIgual: return "MenorIgual";
        case TipoToken::MayorIgual: return "MayorIgual";
        case TipoToken::Igual:      return "Igual";
        case TipoToken::Distinto:   return "Distinto";
        case TipoToken::Asignar:    return "Asignar";
        case TipoToken::Flecha:     return "Flecha";
        case TipoToken::ParIzq:     return "ParIzq";
        case TipoToken::ParDer:     return "ParDer";
        case TipoToken::LlaveIzq:   return "LlaveIzq";
        case TipoToken::LlaveDer:   return "LlaveDer";
        case TipoToken::PuntoComa:  return "PuntoComa";
        case TipoToken::Coma:       return "Coma";
        case TipoToken::Fin:        return "Fin";
    }
    return "";
}
