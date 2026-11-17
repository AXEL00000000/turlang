#pragma once

#include <string>
#include <vector>

#include "runtime/IO.h"

// Entrada y salida por consola. Los avisos para la pantalla no se implementan.
class ConsoleIO : public IO {
public:
    void imprimir(const std::string& texto) override;
    std::string leer() override;
    // Cada línea que regresó leer(), en orden (para repetir la ejecución en --costos).
    const std::vector<std::string>& lineasLeidas() const;
private:
    std::vector<std::string> leidas;
};
