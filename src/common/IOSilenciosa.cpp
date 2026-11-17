#include "common/IOSilenciosa.h"

IOSilenciosa::IOSilenciosa(const std::vector<std::string>& lineas) : lineas(lineas) {}

void IOSilenciosa::imprimir(const std::string& texto) {
    (void)texto;
}

std::string IOSilenciosa::leer() {
    return siguiente < lineas.size() ? lineas[siguiente++] : "";
}
