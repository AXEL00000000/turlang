#include "ir/Triples.h"

#include <unordered_map>

std::vector<Tripleta> aTripletas(const std::vector<Quad>& quads) {
    // Renglón de tripleta donde empieza cada cuádruplo ([]= ocupa dos).
    std::vector<int> inicio(quads.size());
    int renglon = 1;
    for (size_t i = 0; i < quads.size(); ++i) {
        inicio[i] = renglon;
        renglon += quads[i].op == Op::EscribirCinta ? 2 : 1;
    }

    // Renglón de la tripleta que calculó cada temporal.
    std::unordered_map<std::string, int> renglonTemp;

    auto argumento = [&](const Operando& o) -> std::string {
        if (o.clase == ClaseOperando::Temp) {
            auto it = renglonTemp.find(o.texto);
            if (it != renglonTemp.end()) {
                return "(" + std::to_string(it->second) + ")";
            }
        }
        return textoOperando(o);
    };
    auto salto = [&](const Operando& o) -> std::string {
        int n = std::stoi(o.texto);
        return "(" + std::to_string(inicio[n - 1]) + ")";
    };

    std::vector<Tripleta> tripletas;
    for (size_t i = 0; i < quads.size(); ++i) {
        const Quad& q = quads[i];
        const std::string op = nombreOp(q.op);

        switch (q.op) {
            case Op::Asignar:
                if (q.res.clase == ClaseOperando::Temp) {
                    tripletas.push_back({op, argumento(q.arg1), "---"});
                } else {
                    tripletas.push_back({op, argumento(q.arg1), textoOperando(q.res)});
                }
                break;
            case Op::IfFalse:
                tripletas.push_back({op, argumento(q.arg1), salto(q.res)});
                break;
            case Op::Jump:
                tripletas.push_back({op, "---", salto(q.res)});
                break;
            case Op::EscribirCinta:
                tripletas.push_back({op, textoOperando(q.res), argumento(q.arg2)});
                tripletas.push_back({"=", argumento(q.arg1),
                                     "(" + std::to_string(inicio[i]) + ")"});
                break;
            default:
                // (op, a, b, T) → (op, a, b): operaciones, =[], param, call, ret, int, string.
                tripletas.push_back({op, argumento(q.arg1), argumento(q.arg2)});
                break;
        }

        if (q.res.clase == ClaseOperando::Temp) {
            renglonTemp[q.res.texto] = inicio[i];
        }
    }
    return tripletas;
}
