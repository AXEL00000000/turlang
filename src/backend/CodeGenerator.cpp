#include "backend/CodeGenerator.h"

#include <algorithm>

#include "runtime/Valor.h"

namespace {

// ------------------------------------------------- rutinas de apoyo y datos

struct Recurso {
    const char* nombre;
    std::vector<std::string> externos;     // funciones de la biblioteca de C
    std::vector<std::string> requiere;     // otras rutinas o textos
    const char* codigo;                    // vacío si es un dato
};

// Orden en que se escriben las rutinas y en que se declaran los extern.
const char* const ORDEN_EXTERN[] = {"printf", "scanf", "getchar", "malloc", "strlen",
                                    "strcpy", "strcat", "sprintf", "strtol", "memset", "exit"};

const std::vector<Recurso>& recursos() {
    static const std::vector<Recurso> lista = {
        {"rt_print_int", {"printf"}, {"rt_fmt_int"},
R"(; rt_print_int(n): imprime un entero y un salto de línea
rt_print_int:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push rt_fmt_int
    call C(printf)
    add esp, 8
    pop ebp
    ret
)"},
        {"rt_print_cadena", {"printf"}, {"rt_fmt_cad"},
R"(; rt_print_cadena(s): imprime una cadena y un salto de línea
rt_print_cadena:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push rt_fmt_cad
    call C(printf)
    add esp, 8
    pop ebp
    ret
)"},
        {"rt_print_bool", {"printf"}, {"rt_fmt_cad", "rt_true", "rt_false"},
R"(; rt_print_bool(b): imprime true o false y un salto de línea
rt_print_bool:
    push ebp
    mov ebp, esp
    mov eax, rt_false
    cmp dword [ebp+8], 0
    je .imprimir
    mov eax, rt_true
.imprimir:
    push eax
    push rt_fmt_cad
    call C(printf)
    add esp, 8
    pop ebp
    ret
)"},
        {"rt_input", {"malloc", "scanf", "getchar", "strlen"}, {"rt_fmt_linea"},
R"(; rt_input(): lee una línea (sin el salto) en una cadena nueva
rt_input:
    push ebp
    mov ebp, esp
    push ebx
    push 2001
    call C(malloc)
    add esp, 4
    mov ebx, eax
    mov byte [ebx], 0
    push ebx
    push rt_fmt_linea
    call C(scanf)
    add esp, 8
    cmp eax, -1
    je .listo
    call C(getchar)
    push ebx
    call C(strlen)
    add esp, 4
    test eax, eax
    jz .listo
    cmp byte [ebx+eax-1], 13
    jne .listo
    mov byte [ebx+eax-1], 0
.listo:
    mov eax, ebx
    pop ebx
    pop ebp
    ret
)"},
        {"rt_concat", {"strlen", "malloc", "strcpy", "strcat"},
         {"rt_error", "rt_msg_larga", "rt_linea"},
R"(; rt_concat(a, b): cadena nueva con a seguida de b
rt_concat:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push dword [ebp+12]
    call C(strlen)
    add esp, 4
    mov ebx, eax
    push dword [ebp+8]
    call C(strlen)
    add esp, 4
    add ebx, eax
    cmp ebx, 2000
    jg .larga
    inc ebx
    push ebx
    call C(malloc)
    add esp, 4
    mov esi, eax
    push dword [ebp+12]
    push esi
    call C(strcpy)
    add esp, 8
    push dword [ebp+8]
    push esi
    call C(strcat)
    add esp, 8
    mov eax, esi
    pop esi
    pop ebx
    pop ebp
    ret
.larga:
    push dword [rt_linea]
    push rt_msg_larga
    call rt_error
)"},
        {"rt_repeat", {"strlen", "malloc", "strcat"},
         {"rt_error", "rt_msg_larga", "rt_linea", "rt_vacia"},
R"(; rt_repeat(s, n): s repetida n veces en una cadena nueva
rt_repeat:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov edi, [ebp+8]
    mov eax, rt_vacia
    cmp edi, 0
    jle .listo
    push dword [ebp+12]
    call C(strlen)
    add esp, 4
    mov ebx, eax
    mov eax, rt_vacia
    test ebx, ebx
    jz .listo
    mov eax, ebx
    mul edi
    test edx, edx
    jnz .larga
    cmp eax, 2000
    ja .larga
    inc eax
    push eax
    call C(malloc)
    add esp, 4
    mov esi, eax
    mov byte [esi], 0
.repetir:
    push dword [ebp+12]
    push esi
    call C(strcat)
    add esp, 8
    dec edi
    jnz .repetir
    mov eax, esi
.listo:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
.larga:
    push dword [rt_linea]
    push rt_msg_larga
    call rt_error
)"},
        {"rt_int_a_cadena", {"malloc", "sprintf"}, {"rt_fmt_d"},
R"(; rt_int_a_cadena(n): los dígitos de n en una cadena nueva
rt_int_a_cadena:
    push ebp
    mov ebp, esp
    push ebx
    push 12
    call C(malloc)
    add esp, 4
    mov ebx, eax
    push dword [ebp+8]
    push rt_fmt_d
    push ebx
    call C(sprintf)
    add esp, 12
    mov eax, ebx
    pop ebx
    pop ebp
    ret
)"},
        {"rt_bool_a_cadena", {}, {"rt_true", "rt_false"},
R"(; rt_bool_a_cadena(b): apunta a "true" o a "false"
rt_bool_a_cadena:
    mov eax, rt_false
    cmp dword [esp+4], 0
    je .listo
    mov eax, rt_true
.listo:
    ret
)"},
        {"rt_cadena_a_int", {"malloc", "sprintf"}, {"rt_error", "rt_fmt_conv"},
R"(; rt_cadena_a_int(s, línea): espacios, un '-' opcional y dígitos; si no, error
rt_cadena_a_int:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, [ebp+12]
.espacios:
    cmp byte [esi], ' '
    jne .signo
    inc esi
    jmp .espacios
.signo:
    xor edi, edi
    cmp byte [esi], '-'
    jne .numero
    mov edi, 1
    inc esi
.numero:
    xor eax, eax
    xor ecx, ecx
.digito:
    movzx edx, byte [esi]
    sub edx, '0'
    cmp edx, 9
    ja .finDigitos
    imul eax, eax, 10
    add eax, edx
    inc ecx
    inc esi
    jmp .digito
.finDigitos:
    test ecx, ecx
    jz .error
.espaciosFinales:
    cmp byte [esi], ' '
    jne .final
    inc esi
    jmp .espaciosFinales
.final:
    cmp byte [esi], 0
    jne .error
    test edi, edi
    jz .listo
    neg eax
.listo:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
.error:
    push 2100
    call C(malloc)
    add esp, 4
    mov ebx, eax
    push dword [ebp+12]
    push rt_fmt_conv
    push ebx
    call C(sprintf)
    add esp, 12
    push dword [ebp+8]
    push ebx
    call rt_error
)"},
        {"rt_cinta_cargar", {"memset"}, {},
R"(; rt_cinta_cargar(s): llena la cinta de espacios y copia s desde cinta + 2000
rt_cinta_cargar:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push 4001
    push ' '
    push cinta
    call C(memset)
    add esp, 12
    mov esi, [ebp+8]
    mov edi, cinta + 2000
    mov ecx, 2001
.copiar:
    mov al, [esi]
    test al, al
    jz .listo
    mov [edi], al
    inc esi
    inc edi
    dec ecx
    jnz .copiar
.listo:
    pop edi
    pop esi
    pop ebp
    ret
)"},
        {"rt_cinta_final", {"malloc"}, {"rt_vacia"},
R"(; rt_cinta_final(): copia nueva de la cinta sin los espacios de los extremos
rt_cinta_final:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov esi, cinta
    mov edi, cinta + 4000
.inicio:
    cmp esi, edi
    ja .vacia
    cmp byte [esi], ' '
    jne .final
    inc esi
    jmp .inicio
.final:
    cmp byte [edi], ' '
    jne .copiar
    dec edi
    jmp .final
.copiar:
    mov ebx, edi
    sub ebx, esi
    inc ebx
    lea eax, [ebx+1]
    push eax
    call C(malloc)
    add esp, 4
    xor ecx, ecx
.copiarCasilla:
    mov dl, [esi+ecx]
    mov [eax+ecx], dl
    inc ecx
    cmp ecx, ebx
    jb .copiarCasilla
    mov byte [eax+ebx], 0
    jmp .listo
.vacia:
    mov eax, rt_vacia
.listo:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
)"},
        {"rt_error", {"printf", "exit"}, {"rt_archivo", "rt_fmt_error"},
R"(; rt_error(línea, mensaje): imprime el error de ejecución y termina con exit(1)
rt_error:
    push ebp
    mov ebp, esp
    push dword [ebp+8]
    push dword [ebp+12]
    push rt_archivo
    push rt_fmt_error
    call C(printf)
    add esp, 16
    push 1
    call C(exit)
)"},
        // Textos de apoyo
        {"rt_fmt_int", {}, {}, nullptr},
        {"rt_fmt_cad", {}, {}, nullptr},
        {"rt_fmt_d", {}, {}, nullptr},
        {"rt_fmt_linea", {}, {}, nullptr},
        {"rt_fmt_error", {}, {}, nullptr},
        {"rt_fmt_conv", {}, {}, nullptr},
        {"rt_msg_div", {}, {}, nullptr},
        {"rt_msg_larga", {}, {}, nullptr},
        {"rt_msg_cinta", {}, {}, nullptr},
        {"rt_true", {}, {}, nullptr},
        {"rt_false", {}, {}, nullptr},
        {"rt_linea", {}, {}, nullptr},
        {"rt_archivo", {}, {}, nullptr},
        {"rt_vacia", {}, {}, nullptr},
    };
    return lista;
}

const Recurso* buscarRecurso(const std::string& nombre) {
    for (const Recurso& r : recursos()) {
        if (nombre == r.nombre) return &r;
    }
    return nullptr;
}

bool esRelacional(Op op) {
    return op == Op::Menor || op == Op::Mayor || op == Op::MenorIgual ||
           op == Op::MayorIgual || op == Op::Igual || op == Op::Distinto;
}

const char* instruccionSet(Op op) {
    switch (op) {
        case Op::Menor:      return "setl";
        case Op::Mayor:      return "setg";
        case Op::MenorIgual: return "setle";
        case Op::MayorIgual: return "setge";
        case Op::Igual:      return "sete";
        default:             return "setne";
    }
}

bool esRegistro(const std::string& lugar) {
    return !lugar.empty() && lugar[0] != '[';
}

// Desplazamiento de los locales de una máquina en su registro de activación.
const std::map<std::string, std::string>& localesMaquina() {
    static const std::map<std::string, std::string> locales = {
        {"cadena", "[ebp+12]"}, {"maximo", "[ebp+8]"},
        {"cabezal", "[ebp-4]"}, {"pasos", "[ebp-8]"}, {"limite", "[ebp-12]"},
    };
    return locales;
}

std::string comentario(const Quad& q) {
    return "; (" + std::string(nombreOp(q.op)) + ", " + textoOperando(q.arg1) + ", " +
           textoOperando(q.arg2) + ", " + textoOperando(q.res) + ")";
}

std::string simboloAsm(const std::string& s) {
    return s == "blank" ? "' '" : "'" + s + "'";
}

}  // namespace

// ---------------------------------------------------------------- interfaz

CodeGenerator::CodeGenerator(const Program& programa, const SymbolTable& tabla,
                             const std::string& nombreArchivo)
    : programa(programa), tabla(tabla), nombreArchivo(nombreArchivo) {}

void CodeGenerator::usar(const std::string& rutina) {
    rutinasUsadas.insert(rutina);
}

void CodeGenerator::generar(std::ostream& salida) {
    recolectarCadenas();

    // Primero el código (para saber qué rutinas usa) y después el archivo completo.
    std::ostringstream texto;
    traducirBloque(programa.principal, false, texto);
    for (const Bloque& maquina : programa.maquinas) {
        texto << '\n';
        traducirBloque(maquina, true, texto);
    }

    // Rutinas y textos que dependen de otros.
    usar("rt_vacia");
    usar("rt_archivo");
    bool agregado = true;
    while (agregado) {
        agregado = false;
        for (const std::string& nombre : std::set<std::string>(rutinasUsadas)) {
            const Recurso* r = buscarRecurso(nombre);
            if (r == nullptr) continue;
            for (const std::string& otro : r->requiere) {
                if (rutinasUsadas.insert(otro).second) agregado = true;
            }
        }
    }

    escribirEncabezado(salida);
    escribirDatos(salida);
    salida << "\nsection .bss\n";
    salida << "cinta resb 4001                 ; casilla 0 de la máquina = cinta + 2000\n";
    salida << "\nsection .text\n\n";
    salida << texto.str();
    escribirRutinas(salida);
}

void CodeGenerator::recolectarCadenas() {
    auto agregar = [&](const Operando& o) {
        if (o.clase != ClaseOperando::Cadena || etiquetaCadena.count(o.texto)) return;
        std::string etiqueta = "cad_" + std::to_string(ordenCadenas.size() + 1);
        etiquetaCadena[o.texto] = etiqueta;
        ordenCadenas.push_back(o.texto);
    };
    for (const Quad& q : programa.principal.quads) {
        agregar(q.arg1);
        agregar(q.arg2);
    }
    for (const Bloque& b : programa.maquinas) {
        for (const Quad& q : b.quads) {
            agregar(q.arg1);
            agregar(q.arg2);
        }
    }
}

// --------------------------------------------------------- partes del archivo

void CodeGenerator::escribirEncabezado(std::ostream& salida) {
    std::string base = nombreArchivo;
    if (base.size() > 4 && base.compare(base.size() - 4, 4, ".tur") == 0) {
        base.erase(base.size() - 4);
    }
    salida << "; " << base << ".asm — generado por turlang a partir de " << nombreArchivo << "\n";
    salida << "; Ensamblador NASM, x86 de 32 bits\n\n";
    salida << "%ifidn __OUTPUT_FORMAT__, win32\n";
    salida << "  %define C(nombre) _ %+ nombre\n";
    salida << "%else\n";
    salida << "  %define C(nombre) nombre\n";
    salida << "%endif\n\n";

    std::set<std::string> externos;
    for (const std::string& nombre : rutinasUsadas) {
        const Recurso* r = buscarRecurso(nombre);
        if (r != nullptr) externos.insert(r->externos.begin(), r->externos.end());
    }
    std::string lista;
    for (const char* e : ORDEN_EXTERN) {
        if (!externos.count(e)) continue;
        if (!lista.empty()) lista += ", ";
        lista += "C(" + std::string(e) + ")";
    }
    if (!lista.empty()) {
        salida << "extern " << lista << "\n";
    }
    salida << "global C(main)\n";
}

void CodeGenerator::escribirDatos(std::ostream& salida) {
    salida << "\nsection .data\n";

    salida << "; Variables del programa (en el orden de su dirección)\n";
    for (const Simbolo& s : tabla.variablesPorDireccion()) {
        std::string declaracion;
        switch (s.tipo) {
            case TipoDato::Bool:   declaracion = "db 0"; break;
            case TipoDato::String: declaracion = "dd rt_vacia"; break;
            default:               declaracion = "dd 0"; break;
        }
        std::string linea = "v_" + s.nombre + " " + declaracion;
        if (linea.size() < 32) linea += std::string(32 - linea.size(), ' ');
        salida << linea << "; " << nombreTipoDato(s.tipo) << ", dirección "
               << s.direccion << "\n";
    }

    if (!ordenCadenas.empty()) {
        salida << "; Cadenas literales\n";
        for (const std::string& texto : ordenCadenas) {
            salida << etiquetaCadena[texto] << " db ";
            if (!texto.empty()) salida << '"' << texto << "\", ";
            salida << "0\n";
        }
    }

    salida << "; Textos de apoyo\n";
    const std::map<std::string, std::string> definiciones = {
        {"rt_vacia", "rt_vacia db 0"},
        {"rt_archivo", "rt_archivo db \"" + nombreArchivo + "\", 0"},
        {"rt_true", "rt_true db \"true\", 0"},
        {"rt_false", "rt_false db \"false\", 0"},
        {"rt_fmt_int", "rt_fmt_int db \"%d\", 10, 0"},
        {"rt_fmt_cad", "rt_fmt_cad db \"%s\", 10, 0"},
        {"rt_fmt_d", "rt_fmt_d db \"%d\", 0"},
        {"rt_fmt_linea", "rt_fmt_linea db \"%2000[^\", 10, \"]\", 0"},
        {"rt_fmt_error", "rt_fmt_error db \"%s:%d: error de ejecución: %s\", 10, 0"},
        {"rt_fmt_conv", "rt_fmt_conv db 'conversión inválida: \"%s\" no es un número', 0"},
        {"rt_msg_div", "rt_msg_div db \"división entre cero\", 0"},
        {"rt_msg_larga", "rt_msg_larga db \"cadena demasiado larga\", 0"},
        {"rt_msg_cinta", "rt_msg_cinta db \"cinta demasiado larga\", 0"},
        {"rt_linea", "rt_linea dd 0                   ; línea para los errores de rt_concat y rt_repeat"},
    };
    for (const Recurso& r : recursos()) {
        if (r.codigo != nullptr || !rutinasUsadas.count(r.nombre)) continue;
        salida << definiciones.at(r.nombre) << "\n";
    }
}

void CodeGenerator::escribirRutinas(std::ostream& salida) {
    bool primera = true;
    for (const Recurso& r : recursos()) {
        if (r.codigo == nullptr || !rutinasUsadas.count(r.nombre)) continue;
        if (primera) {
            salida << "\n; ===================================================== rutinas de apoyo\n";
            primera = false;
        }
        salida << '\n' << r.codigo;
    }
}

// ---------------------------------------------------------------- un bloque

void CodeGenerator::traducirBloque(const Bloque& b, bool maquina, std::ostream& salida) {
    bloque = &b;
    enMaquina = maquina;
    tipoTemp.clear();
    lugarActual.clear();
    tiposParam.clear();
    destinos.clear();
    for (const Quad& q : b.quads) {
        if (q.op == Op::Jump || q.op == Op::IfFalse) destinos.insert(renglonSalto(q.res));
    }

    inferirTipos();
    std::string asignacion = asignarRegistros();
    int locales = (maquina ? 12 : 0) + 4 * casillas;

    if (maquina) {
        salida << "; ---------------------------------------- máquina " << b.nombre << "\n";
        salida << "maq_" << b.nombre << ":\n";
        salida << "; Registro de activación:\n";
        salida << ";   [ebp+12] cadena, [ebp+8] maximo, [ebp+4] dirección de regreso, [ebp] ebp anterior\n";
        salida << ";   [ebp-4] cabezal, [ebp-8] pasos, [ebp-12] limite";
        if (casillas > 0) salida << ", [ebp-16] en adelante: temporales";
        salida << "; después, ebx, esi y edi guardados\n";
    } else {
        salida << "; ---------------------------------------- sección de control\n";
        salida << "C(main):\n";
        salida << "; Registro de activación: [ebp+4] dirección de regreso, [ebp] ebp anterior";
        if (casillas > 0) salida << ", [ebp-4] en adelante: temporales";
        salida << "; después, ebx, esi y edi guardados\n";
    }
    salida << asignacion << "\n";
    salida << "    push ebp\n";
    salida << "    mov ebp, esp\n";
    if (locales > 0) salida << "    sub esp, " << locales << "\n";
    salida << "    push ebx\n";
    salida << "    push esi\n";
    salida << "    push edi\n";

    for (size_t i = 0; i < b.quads.size(); ++i) {
        traducir(static_cast<int>(i), b.quads[i], salida);
    }

    if (maquina) {
        salida << ".fin:\n";
        salida << "    pop edi\n";
        salida << "    pop esi\n";
        salida << "    pop ebx\n";
        salida << "    mov esp, ebp\n";
        salida << "    pop ebp\n";
        salida << "    ret\n";
    }
}

// Tipo de cada temporal según la operación que lo produce.
void CodeGenerator::inferirTipos() {
    for (const Quad& q : bloque->quads) {
        if (q.res.clase != ClaseOperando::Temp) continue;
        Tipo t = Tipo::Int;
        switch (q.op) {
            case Op::Suma:
                t = (tipoDe(q.arg1) == Tipo::Cadena || tipoDe(q.arg2) == Tipo::Cadena)
                        ? Tipo::Cadena : Tipo::Int;
                break;
            case Op::ConvString:
                t = Tipo::Cadena;
                break;
            case Op::Call:
                t = (q.arg1.texto == "input" || q.arg1.texto == "repeat") ? Tipo::Cadena
                                                                          : Tipo::Bool;
                break;
            case Op::Asignar:
                t = q.arg1.clase == ClaseOperando::Cinta ? Tipo::Cadena : tipoDe(q.arg1);
                break;
            default:
                t = esRelacional(q.op) ? Tipo::Bool : Tipo::Int;
                break;
        }
        tipoTemp[q.res.texto] = t;
    }
}

// Recorrido lineal: cada definición de un temporal recibe ebx, esi o edi, o una
// casilla en la pila si no hay registro libre; después de su último uso se libera.
std::string CodeGenerator::asignarRegistros() {
    const std::vector<Quad>& quads = bloque->quads;
    int n = static_cast<int>(quads.size());

    // Último uso de cada definición (cada definición por separado).
    std::vector<int> ultimoUso(n, -1);
    std::map<std::string, int> definicion;
    for (int r = 0; r < n; ++r) {
        for (const Operando* o : {&quads[r].arg1, &quads[r].arg2}) {
            if (o->clase != ClaseOperando::Temp) continue;
            auto it = definicion.find(o->texto);
            if (it != definicion.end()) ultimoUso[it->second] = r;
        }
        if (quads[r].res.clase == ClaseOperando::Temp) {
            definicion[quads[r].res.texto] = r;
            ultimoUso[r] = r;
        }
    }

    const char* registros[] = {"ebx", "esi", "edi"};
    bool registroOcupado[3] = {false, false, false};
    std::vector<bool> casillaOcupada;
    std::vector<int> activos;   // renglones de definición con lugar asignado
    lugarDef.assign(n, "");
    casillas = 0;
    std::string lista;

    auto liberar = [&](int d) {
        const std::string& lugar = lugarDef[d];
        for (int k = 0; k < 3; ++k) {
            if (lugar == registros[k]) registroOcupado[k] = false;
        }
        for (size_t k = 0; k < casillaOcupada.size(); ++k) {
            if (casillaOcupada[k] && lugar == "[ebp-" + std::to_string((enMaquina ? 16 : 4) + 4 * k) + "]") {
                casillaOcupada[k] = false;
            }
        }
    };

    for (int r = 0; r < n; ++r) {
        // Se liberan los que ya no se usan (también los que se usan por última vez
        // aquí: sus valores se cargan antes de guardar el resultado).
        for (size_t a = 0; a < activos.size();) {
            if (ultimoUso[activos[a]] <= r) {
                liberar(activos[a]);
                activos.erase(activos.begin() + a);
            } else {
                ++a;
            }
        }
        if (quads[r].res.clase != ClaseOperando::Temp) continue;

        std::string lugar;
        for (int k = 0; k < 3 && lugar.empty(); ++k) {
            if (!registroOcupado[k]) {
                registroOcupado[k] = true;
                lugar = registros[k];
            }
        }
        if (lugar.empty()) {
            size_t k = 0;
            while (k < casillaOcupada.size() && casillaOcupada[k]) ++k;
            if (k == casillaOcupada.size()) casillaOcupada.push_back(false);
            casillaOcupada[k] = true;
            casillas = std::max(casillas, static_cast<int>(k) + 1);
            lugar = "[ebp-" + std::to_string((enMaquina ? 16 : 4) + 4 * static_cast<int>(k)) + "]";
        }
        lugarDef[r] = lugar;
        activos.push_back(r);
        if (!lista.empty()) lista += ", ";
        lista += quads[r].res.texto + " -> " + lugar;
    }

    return "; Asignación de registros: " + (lista.empty() ? std::string("sin temporales") : lista);
}

// ------------------------------------------------------------- operandos

CodeGenerator::Tipo CodeGenerator::tipoDe(const Operando& o) const {
    switch (o.clase) {
        case ClaseOperando::Cadena: return Tipo::Cadena;
        case ClaseOperando::Bool:   return Tipo::Bool;
        case ClaseOperando::Temp: {
            auto it = tipoTemp.find(o.texto);
            return it == tipoTemp.end() ? Tipo::Int : it->second;
        }
        case ClaseOperando::Var: {
            if (enMaquina) return Tipo::Int;
            const Simbolo* s = tabla.buscar(o.texto);
            if (s == nullptr) return Tipo::Int;
            if (s->tipo == TipoDato::String) return Tipo::Cadena;
            if (s->tipo == TipoDato::Bool) return Tipo::Bool;
            return Tipo::Int;
        }
        default:
            return Tipo::Int;
    }
}

std::string CodeGenerator::lugarVariable(const std::string& nombre) const {
    if (enMaquina) {
        auto it = localesMaquina().find(nombre);
        if (it != localesMaquina().end()) return it->second;
    }
    return "[v_" + nombre + "]";
}

// Carga un operando en un registro.
void CodeGenerator::cargar(const Operando& o, const std::string& r, std::ostream& s) {
    switch (o.clase) {
        case ClaseOperando::Entero:
            s << "    mov " << r << ", " << o.texto << "\n";
            break;
        case ClaseOperando::Bool:
            s << "    mov " << r << ", " << (o.texto == "true" ? 1 : 0) << "\n";
            break;
        case ClaseOperando::Cadena:
            s << "    mov " << r << ", " << etiquetaCadena[o.texto] << "\n";
            break;
        case ClaseOperando::Simbolo:
            s << "    mov " << r << ", " << simboloAsm(o.texto) << "\n";
            break;
        case ClaseOperando::Var:
            if (!enMaquina && tipoDe(o) == Tipo::Bool) {
                s << "    movzx " << r << ", byte " << lugarVariable(o.texto) << "\n";
            } else {
                s << "    mov " << r << ", " << lugarVariable(o.texto) << "\n";
            }
            break;
        case ClaseOperando::Temp: {
            const std::string& lugar = lugarActual[o.texto];
            if (lugar != r) s << "    mov " << r << ", " << lugar << "\n";
            break;
        }
        default:
            s << "    mov " << r << ", 0\n";
            break;
    }
}

// Operando que se puede usar directo como fuente de add, sub, imul o cmp; si no se
// puede (bool en memoria), se carga en ecx.
std::string CodeGenerator::fuente(const Operando& o, std::ostream& s) {
    switch (o.clase) {
        case ClaseOperando::Entero:  return o.texto;
        case ClaseOperando::Bool:    return o.texto == "true" ? "1" : "0";
        case ClaseOperando::Cadena:  return etiquetaCadena[o.texto];
        case ClaseOperando::Simbolo: return simboloAsm(o.texto);
        case ClaseOperando::Temp: {
            const std::string& lugar = lugarActual[o.texto];
            return esRegistro(lugar) ? lugar : "dword " + lugar;
        }
        case ClaseOperando::Var:
            if (enMaquina || tipoDe(o) != Tipo::Bool) return "dword " + lugarVariable(o.texto);
            break;
        default:
            break;
    }
    cargar(o, "ecx", s);
    return "ecx";
}

// Guarda eax en el destino del cuádruplo.
void CodeGenerator::guardar(int renglon, const Operando& destino, std::ostream& s) {
    if (destino.clase == ClaseOperando::Temp) {
        const std::string& lugar = lugarDef[renglon];
        s << "    mov " << lugar << ", eax\n";
        lugarActual[destino.texto] = lugar;
    } else if (destino.clase == ClaseOperando::Var) {
        if (!enMaquina && tipoDe(destino) == Tipo::Bool) {
            s << "    mov " << lugarVariable(destino.texto) << ", al\n";
        } else {
            s << "    mov " << lugarVariable(destino.texto) << ", eax\n";
        }
    }
}

// El cabezal (en eax) debe quedar entre -2000 y 2000.
void CodeGenerator::revisarCinta(int renglon, const Quad& q, std::ostream& s) {
    usar("rt_error");
    usar("rt_msg_cinta");
    std::string fuera = ".fueraCinta" + std::to_string(renglon + 1);
    std::string dentro = ".dentroCinta" + std::to_string(renglon + 1);
    s << "    cmp eax, -2000\n";
    s << "    jl " << fuera << "\n";
    s << "    cmp eax, 2000\n";
    s << "    jle " << dentro << "\n";
    s << fuera << ":\n";
    s << "    push " << q.linea << "\n";
    s << "    push rt_msg_cinta\n";
    s << "    call rt_error\n";
    s << dentro << ":\n";
}

// ------------------------------------------------------------- un cuádruplo

void CodeGenerator::traducir(int r, const Quad& q, std::ostream& s) {
    if (destinos.count(r + 1)) s << ".L" << (r + 1) << ":\n";
    s << comentario(q) << "\n";

    switch (q.op) {
        case Op::Asignar:
            if (q.res.clase == ClaseOperando::Cinta) {
                usar("rt_cinta_cargar");
                s << "    push dword [ebp+12]\n";
                s << "    call rt_cinta_cargar\n";
                s << "    add esp, 4\n";
            } else if (q.arg1.clase == ClaseOperando::Cinta) {
                usar("rt_cinta_final");
                s << "    call rt_cinta_final\n";
                guardar(r, q.res, s);
            } else if (q.res.clase == ClaseOperando::Var &&
                       (q.arg1.clase == ClaseOperando::Entero ||
                        q.arg1.clase == ClaseOperando::Bool ||
                        q.arg1.clase == ClaseOperando::Cadena)) {
                std::string destino = lugarVariable(q.res.texto);
                if (!enMaquina && tipoDe(q.res) == Tipo::Bool) {
                    s << "    mov byte " << destino << ", " << fuente(q.arg1, s) << "\n";
                } else {
                    s << "    mov dword " << destino << ", " << fuente(q.arg1, s) << "\n";
                }
            } else {
                cargar(q.arg1, "eax", s);
                guardar(r, q.res, s);
            }
            break;

        case Op::Suma:
            if (tipoDe(q.arg1) == Tipo::Cadena || tipoDe(q.arg2) == Tipo::Cadena) {
                usar("rt_concat");
                s << "    mov dword [rt_linea], " << q.linea << "\n";
                cargar(q.arg1, "eax", s);
                s << "    push eax\n";
                cargar(q.arg2, "eax", s);
                s << "    push eax\n";
                s << "    call rt_concat\n";
                s << "    add esp, 8\n";
                guardar(r, q.res, s);
                break;
            }
            cargar(q.arg1, "eax", s);
            s << "    add eax, " << fuente(q.arg2, s) << "\n";
            guardar(r, q.res, s);
            break;

        case Op::Resta:
            cargar(q.arg1, "eax", s);
            s << "    sub eax, " << fuente(q.arg2, s) << "\n";
            guardar(r, q.res, s);
            break;

        case Op::Mult:
            cargar(q.arg1, "eax", s);
            s << "    imul eax, " << fuente(q.arg2, s) << "\n";
            guardar(r, q.res, s);
            break;

        case Op::Div: {
            usar("rt_error");
            usar("rt_msg_div");
            std::string n = std::to_string(r + 1);
            cargar(q.arg1, "eax", s);
            cargar(q.arg2, "ecx", s);
            s << "    test ecx, ecx\n";
            s << "    jnz .divisor" << n << "\n";
            s << "    push " << q.linea << "\n";
            s << "    push rt_msg_div\n";
            s << "    call rt_error\n";
            s << ".divisor" << n << ":\n";
            s << "    cmp ecx, -1                     ; evita la excepción de -2147483648 / -1\n";
            s << "    jne .dividir" << n << "\n";
            s << "    neg eax\n";
            s << "    jmp .cociente" << n << "\n";
            s << ".dividir" << n << ":\n";
            s << "    cdq\n";
            s << "    idiv ecx\n";
            s << ".cociente" << n << ":\n";
            guardar(r, q.res, s);
            break;
        }

        case Op::Menor: case Op::Mayor: case Op::MenorIgual:
        case Op::MayorIgual: case Op::Igual: case Op::Distinto:
            cargar(q.arg1, "eax", s);
            s << "    cmp eax, " << fuente(q.arg2, s) << "\n";
            s << "    " << instruccionSet(q.op) << " al\n";
            s << "    movzx eax, al\n";
            guardar(r, q.res, s);
            break;

        case Op::IfFalse:
            cargar(q.arg1, "eax", s);
            s << "    test eax, eax\n";
            s << "    jz .L" << renglonSalto(q.res) << "\n";
            break;

        case Op::Jump:
            s << "    jmp .L" << renglonSalto(q.res) << "\n";
            break;

        case Op::Param:
            cargar(q.arg1, "eax", s);
            s << "    push eax\n";
            tiposParam.push_back(tipoDe(q.arg1));
            break;

        case Op::Call: {
            const std::string& f = q.arg1.texto;
            if (f == "print") {
                Tipo t = tiposParam.empty() ? Tipo::Int : tiposParam.back();
                std::string rutina = t == Tipo::Cadena ? "rt_print_cadena"
                                   : t == Tipo::Bool   ? "rt_print_bool" : "rt_print_int";
                usar(rutina);
                s << "    call " << rutina << "\n";
                s << "    add esp, 4\n";
            } else if (f == "input") {
                usar("rt_input");
                s << "    call rt_input\n";
                guardar(r, q.res, s);
            } else if (f == "repeat") {
                usar("rt_repeat");
                s << "    mov dword [rt_linea], " << q.linea << "\n";
                s << "    call rt_repeat\n";
                s << "    add esp, 8\n";
                guardar(r, q.res, s);
            } else {
                s << "    call maq_" << f << "\n";
                s << "    add esp, 8\n";
                guardar(r, q.res, s);
            }
            tiposParam.clear();
            break;
        }

        case Op::ConvInt:
            usar("rt_cadena_a_int");
            cargar(q.arg1, "eax", s);
            s << "    push eax\n";
            s << "    push " << q.linea << "\n";
            s << "    call rt_cadena_a_int\n";
            s << "    add esp, 8\n";
            guardar(r, q.res, s);
            break;

        case Op::ConvString: {
            std::string rutina = tipoDe(q.arg1) == Tipo::Bool ? "rt_bool_a_cadena"
                                                              : "rt_int_a_cadena";
            usar(rutina);
            cargar(q.arg1, "eax", s);
            s << "    push eax\n";
            s << "    call " << rutina << "\n";
            s << "    add esp, 4\n";
            guardar(r, q.res, s);
            break;
        }

        case Op::Ret:
            if (enMaquina) {
                s << "    mov eax, " << (q.arg1.texto == "true" ? 1 : 0) << "\n";
                s << "    jmp .fin\n";
            } else {
                s << "    xor eax, eax\n";
                s << "    pop edi\n";
                s << "    pop esi\n";
                s << "    pop ebx\n";
                s << "    mov esp, ebp\n";
                s << "    pop ebp\n";
                s << "    ret\n";
            }
            break;

        case Op::LeerCinta:
            cargar(q.arg2, "eax", s);
            revisarCinta(r, q, s);
            s << "    movzx eax, byte [cinta + 2000 + eax]\n";
            guardar(r, q.res, s);
            break;

        case Op::EscribirCinta:
            cargar(q.arg2, "eax", s);
            revisarCinta(r, q, s);
            s << "    mov byte [cinta + 2000 + eax], " << simboloAsm(q.arg1.texto) << "\n";
            break;
    }
}
