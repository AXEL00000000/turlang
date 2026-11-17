#pragma once

#include <string>

#include "runtime/IO.h"

// Entrada y salida por consola. Los avisos para la pantalla no se implementan.
class ConsoleIO : public IO {
public:
    void imprimir(const std::string& texto) override;
    std::string leer() override;
};
