#pragma once

#include <string>
#include <vector>

#include "runtime/IO.h"

// No imprime nada y regresa, en orden, las líneas que se le dan (después, cadena vacía).
class IOSilenciosa : public IO {
public:
    explicit IOSilenciosa(const std::vector<std::string>& lineas);
    void imprimir(const std::string& texto) override;
    std::string leer() override;
private:
    std::vector<std::string> lineas;
    size_t siguiente = 0;
};
