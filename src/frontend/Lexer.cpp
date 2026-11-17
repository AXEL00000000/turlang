#include "frontend/Lexer.h"

#include <unordered_map>

namespace {

const std::unordered_map<std::string, TipoToken> PALABRAS_RESERVADAS = {
    {"machine", TipoToken::Machine}, {"start", TipoToken::Start},
    {"accept", TipoToken::Accept},   {"state", TipoToken::State},
    {"int", TipoToken::Int},         {"string", TipoToken::String},
    {"bool", TipoToken::Bool},       {"true", TipoToken::True},
    {"false", TipoToken::False},     {"if", TipoToken::If},
    {"else", TipoToken::Else},       {"while", TipoToken::While},
    {"run", TipoToken::Run},         {"output", TipoToken::Output},
    {"repeat", TipoToken::Repeat},   {"print", TipoToken::Print},
    {"input", TipoToken::Input},     {"blank", TipoToken::Blank},
};

bool esLetra(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool esDigito(unsigned char c) {
    return c >= '0' && c <= '9';
}

bool esImprimible(unsigned char c) {
    return c >= 32 && c <= 126;
}

}  // namespace

Lexer::Lexer(const std::string& fuente, ErrorHandler& errores)
    : fuente(fuente), errores(errores) {}

bool Lexer::alFinal() const {
    return pos >= fuente.size();
}

unsigned char Lexer::actual() const {
    return alFinal() ? '\0' : static_cast<unsigned char>(fuente[pos]);
}

unsigned char Lexer::siguiente() const {
    return (pos + 1 < fuente.size()) ? static_cast<unsigned char>(fuente[pos + 1]) : '\0';
}

// Avanza un byte y actualiza la posición. Solo '\n' aumenta la línea; '\r' no
// ocupa columna, para que "\r\n" dé las mismas posiciones que "\n". Los bytes de
// continuación de UTF-8 tampoco ocupan columna: cada carácter cuenta como una.
void Lexer::avanzar() {
    unsigned char c = actual();
    ++pos;
    if (c == '\n') {
        ++linea;
        columna = 1;
    } else if (c == '\r' || (c >= 0x80 && c <= 0xBF)) {
        // sin columna
    } else {
        ++columna;
    }
}

void Lexer::saltarComentario() {
    while (!alFinal() && actual() != '\n') {
        avanzar();
    }
}

Token Lexer::leerIdentificador() {
    Token token{TipoToken::Id, "", linea, columna};
    while (!alFinal() && (esLetra(actual()) || esDigito(actual()))) {
        token.lexema += static_cast<char>(actual());
        avanzar();
    }
    auto reservada = PALABRAS_RESERVADAS.find(token.lexema);
    if (reservada != PALABRAS_RESERVADAS.end()) {
        token.tipo = reservada->second;
    }
    return token;
}

Token Lexer::leerNumero() {
    Token token{TipoToken::Num, "", linea, columna};
    while (!alFinal() && esDigito(actual())) {
        token.lexema += static_cast<char>(actual());
        avanzar();
    }
    return token;
}

bool Lexer::leerCadena(Token& token) {
    token = Token{TipoToken::Str, "", linea, columna};
    avanzar();  // comilla que abre

    bool hayInvalido = false;
    int lineaInvalido = 0;
    int columnaInvalido = 0;

    while (true) {
        bool finDeLinea = alFinal() || actual() == '\n' ||
                          (actual() == '\r' && siguiente() == '\n');
        if (finDeLinea) {
            // El salto de línea se deja para el ciclo principal.
            errores.reportar(token.linea, token.columna, Fase::Lexico, "cadena sin cerrar");
            return false;
        }
        unsigned char c = actual();
        if (c == '"') {
            avanzar();
            if (hayInvalido) {
                errores.reportar(lineaInvalido, columnaInvalido, Fase::Lexico,
                                 "carácter no permitido en la cadena");
                return false;
            }
            return true;
        }
        if (!esImprimible(c)) {
            if (!hayInvalido) {
                hayInvalido = true;
                lineaInvalido = linea;
                columnaInvalido = columna;
            }
        } else {
            token.lexema += static_cast<char>(c);
        }
        avanzar();
    }
}

bool Lexer::leerOperador(Token& token) {
    token = Token{TipoToken::Fin, "", linea, columna};
    unsigned char c = actual();
    unsigned char d = siguiente();

    // Operadores de dos caracteres primero.
    TipoToken tipoDoble = TipoToken::Fin;
    if (c == '-' && d == '>') tipoDoble = TipoToken::Flecha;
    else if (c == '<' && d == '=') tipoDoble = TipoToken::MenorIgual;
    else if (c == '>' && d == '=') tipoDoble = TipoToken::MayorIgual;
    else if (c == '=' && d == '=') tipoDoble = TipoToken::Igual;
    else if (c == '!' && d == '=') tipoDoble = TipoToken::Distinto;

    if (tipoDoble != TipoToken::Fin) {
        token.tipo = tipoDoble;
        token.lexema = std::string{static_cast<char>(c), static_cast<char>(d)};
        avanzar();
        avanzar();
        return true;
    }

    switch (c) {
        case '+': token.tipo = TipoToken::Mas; break;
        case '-': token.tipo = TipoToken::Menos; break;
        case '*': token.tipo = TipoToken::Por; break;
        case '/': token.tipo = TipoToken::Entre; break;
        case '<': token.tipo = TipoToken::Menor; break;
        case '>': token.tipo = TipoToken::Mayor; break;
        case '=': token.tipo = TipoToken::Asignar; break;
        case '(': token.tipo = TipoToken::ParIzq; break;
        case ')': token.tipo = TipoToken::ParDer; break;
        case '{': token.tipo = TipoToken::LlaveIzq; break;
        case '}': token.tipo = TipoToken::LlaveDer; break;
        case ';': token.tipo = TipoToken::PuntoComa; break;
        case ',': token.tipo = TipoToken::Coma; break;
        default: return false;
    }
    token.lexema = std::string(1, static_cast<char>(c));
    avanzar();
    return true;
}

void Lexer::reportarNoReconocido() {
    unsigned char c = actual();
    if (c >= 0x80) {
        // Fuera de ASCII: un solo error por la secuencia de bytes seguidos.
        errores.reportar(linea, columna, Fase::Lexico, "carácter no reconocido");
        while (!alFinal() && actual() >= 0x80) {
            avanzar();
        }
        return;
    }
    if (esImprimible(c)) {
        errores.reportar(linea, columna, Fase::Lexico,
                         std::string("carácter no reconocido '") + static_cast<char>(c) + "'");
    } else {
        errores.reportar(linea, columna, Fase::Lexico, "carácter no reconocido");
    }
    avanzar();
}

std::vector<Token> Lexer::analizar() {
    std::vector<Token> tokens;

    while (!alFinal()) {
        unsigned char c = actual();

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            avanzar();
        } else if (c == '/' && siguiente() == '/') {
            saltarComentario();
        } else if (esLetra(c)) {
            tokens.push_back(leerIdentificador());
        } else if (esDigito(c)) {
            tokens.push_back(leerNumero());
        } else if (c == '"') {
            Token token;
            if (leerCadena(token)) {
                tokens.push_back(token);
            }
        } else {
            Token token;
            if (leerOperador(token)) {
                tokens.push_back(token);
            } else {
                reportarNoReconocido();
            }
        }
    }

    tokens.push_back(Token{TipoToken::Fin, "", linea, columna});
    return tokens;
}
