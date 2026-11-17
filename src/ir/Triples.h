#pragma once

#include <string>
#include <vector>

#include "runtime/Quad.h"

struct Tripleta {
    std::string op;
    std::string arg1;
    std::string arg2;
};

// Deriva las tripletas de los cuádruplos de un bloque.
std::vector<Tripleta> aTripletas(const std::vector<Quad>& quads);
