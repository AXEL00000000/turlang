#pragma once

// Menú de programas. La lista sale de listarProgramas(); hoy son los programas
// incrustados en Programas.h. Para agregar la SD solo se cambian listarProgramas()
// y cargarPrograma().

#include <string>
#include <vector>

struct ProgramaDisponible {
    std::string nombre;
    const char* incrustado;   // texto del .turc incrustado (con la SD: nullptr y la ruta)
};

std::vector<ProgramaDisponible> listarProgramas();

// Texto del .turc de un programa. false y un mensaje en error si no se pudo leer.
bool cargarPrograma(const ProgramaDisponible& programa, std::string& texto, std::string& error);

// Dibuja el menú y espera a que se toque un programa; regresa su índice.
int elegirPrograma(const std::vector<ProgramaDisponible>& programas);

// Pantalla con un error en rojo; regresa con un toque.
void mostrarError(const std::string& titulo, const std::string& mensaje);
