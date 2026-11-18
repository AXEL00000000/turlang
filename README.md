# TurLang

Manual de usuario del traductor `turlang`.

## 1. Qué es TurLang

TurLang es un lenguaje para describir Máquinas de Turing deterministas de una cinta y
usarlas desde una sección de control con variables, `if`, `while` y `print`. El
traductor revisa el programa, genera código intermedio, lo optimiza y lo ejecuta en
consola; también produce ensamblador x86 y un compilado `.turc` para el prototipo
(ESP32 con pantalla táctil).

Ejemplo, `examples/sumar_uno.tur`:

```
// SumarUno: suma uno a un numero en unario
machine SumarUno {
    start q0
    accept fin
    state q0 {
        1 -> 1, R, q0
        blank -> 1, S, fin
    }
}

string entrada = "11";
if (run SumarUno(entrada, 100)) {
    print("aceptada");
} else {
    print("rechazada");
}
```

```
$ ./turlang examples/sumar_uno.tur
aceptada
```

## 2. Instalación en Windows

Para usar TurLang sin compilarlo:

1. Descarga `turlang-windows.zip` de la página de releases del repositorio (o genéralo
   con `mingw32-make paquete`, que lo deja en `dist/`).
2. Extrae el zip en cualquier carpeta.
3. Haz doble clic en `instalar.bat`. Copia `turlang.exe` a
   `%LOCALAPPDATA%\Programs\TurLang` y agrega esa carpeta al PATH del usuario, sin
   duplicarla. No necesita permisos de administrador ni toca el PATH del sistema. Si
   TurLang ya estaba instalado, lo reemplaza.
4. Abre una terminal nueva (las que ya estaban abiertas no ven el cambio) y prueba
   desde la carpeta donde extrajiste el zip:

```
turlang ejemplos\factorial.tur
```

```
factorial de 5 = 120
```

Desde ese momento `turlang` funciona en cualquier carpeta. Para desinstalarlo, haz
doble clic en `desinstalar.bat`: borra la carpeta y la quita del PATH del usuario.

| Archivo del zip | Contenido |
|---|---|
| `turlang.exe` | El traductor. |
| `instalar.bat`, `instalar.ps1` | Instalación para el usuario actual. |
| `desinstalar.bat`, `desinstalar.ps1` | Desinstalación. |
| `README.md` | Este manual. |
| `ejemplos/` | `sumar_uno.tur`, `factorial.tur`, `fibonacci.tur`, `binario.tur` y `ceros_unos.tur`. |

## 3. Requisitos

| Para | Herramienta |
|---|---|
| Compilar el traductor | g++ con C++17 y make. En Windows, MinGW (por ejemplo WinLibs) con `mingw32-make`. |
| Ensamblar el `.asm` | SASM (https://dman95.github.io/SASM/), que trae NASM y un gcc de 32 bits. |
| Correr las pruebas | PowerShell (incluido en Windows; `pwsh` en Linux o macOS). |
| Cargar el prototipo | Arduino IDE 2 (firmware del ESP32, pendiente). |

## 4. Compilar el traductor

```
mingw32-make          (Windows)
make                  (Linux / macOS)
```

El resultado es el ejecutable `turlang` (`turlang.exe` en Windows) en la carpeta raíz.
En VS Code, la tarea de compilación del repositorio (`.vscode/tasks.json`, Ctrl+Shift+B)
ejecuta el mismo comando. Los ejemplos de este manual se escriben como `./turlang`,
que funciona en PowerShell y en Git Bash; en `cmd` usa `.\turlang`.

## 5. Uso

```
turlang <archivo.tur> [--tokens | --arbol | --tabla | --cuadruplos | --costos | --asm | --esp32]
turlang <archivo.turc>
```

Sin argumentos imprime la línea de uso y termina con código 2.

| Comando | Resultado |
|---|---|
| `./turlang examples/sumar_uno.tur` | Ejecuta el programa (optimizado) en consola: `aceptada`. |
| `./turlang examples/sumar_uno.tur --tokens` | Tabla de tokens (`Línea  Col  Tipo  Lexema`) y al final `57 tokens`. |
| `./turlang examples/sumar_uno.tur --arbol` | Árbol sintáctico, un nodo por línea: `Programa`, `Maquina SumarUno (inicio: q0, acepta: fin)`, ... |
| `./turlang examples/sumar_uno.tur --tabla` | Tabla de símbolos: Nombre, Tipo, Valor, Scope, Función, # Args, Dirección. |
| `./turlang examples/sumar_uno.tur --cuadruplos` | Cuádruplos y tripletas antes de optimizar, técnicas aplicadas y el código después de optimizar. |
| `./turlang examples/sumar_uno.tur --costos` | Ejecuta el programa y compara el original con el optimizado (tabla Medida / Antes / Después). |
| `./turlang examples/sumar_uno.tur --asm` | `generado: examples/sumar_uno.asm` (ensamblador x86 de 32 bits; no ejecuta). |
| `./turlang examples/sumar_uno.tur --esp32` | `generado: examples/sumar_uno.turc` (compilado para el prototipo; no ejecuta). |
| `./turlang examples/sumar_uno.turc` | Ejecuta el compilado (cuádruplos sin optimizar): `aceptada`. |

Salida real de `--tabla` y de `--costos` con este ejemplo:

```
$ ./turlang examples/sumar_uno.tur --tabla
Nombre    Tipo     Valor  Scope  Función  # Args  Dirección
SumarUno  machine  ---    0                       ---
q0        state    ---    1                       ---
fin       state    ---    1                       ---
entrada   string   "11"   0                       0
run       bool     ---    0      ✓        2       ---
output    string   ---    0      ✓        2       ---
repeat    string   ---    0      ✓        2       ---
input     string   ---    0      ✓        0       ---
print     void     ---    0      ✓        1       ---

$ ./turlang examples/sumar_uno.tur --costos
aceptada

Medida                                          Antes    Después
Cuádruplos de la sección de control             11       11
Cuádruplos ejecutados de la sección de control  9        9
Saltos ejecutados de la sección de control      2        2
Temporales de la sección de control             1        1
Memoria de temporales                           1 bytes  1 bytes
Cuádruplos de la máquina SumarUno               20       19
Cuádruplos ejecutados de la máquina SumarUno    37       35
```

Códigos de salida:

| Código | Significado |
|---|---|
| 0 | Todo correcto. |
| 1 | El programa tiene errores (léxicos, sintácticos o semánticos) o falló al ejecutarse. |
| 2 | Error de uso: argumentos, extensión, archivo que no se puede abrir o escribir, `.turc` dañado. |

## 6. El lenguaje

Un programa tiene cero o más máquinas y después la sección de control (al menos una
sentencia). `//` inicia un comentario hasta el final de la línea.

| Elemento | Reglas | Ejemplo |
|---|---|---|
| Máquina | `machine Nombre { start <estado> accept <estado> <estados> }`. Al menos un estado con reglas. | `machine SumarUno { start q0 accept fin state q0 { ... } }` |
| Estado | `state <nombre> { <reglas> }`, al menos una regla. El estado de aceptación no lleva bloque `state`. | `state q0 { 1 -> 1, R, q0 }` |
| Regla | `<lee> -> <escribe>, <movimiento>, <destino>`. Una regla por símbolo leído en cada estado. | `blank -> 1, S, fin` |
| Símbolo | Una letra, un dígito, `blank` (casilla vacía) o `*`. | `X`, `0`, `blank` |
| Movimiento | `R` (derecha), `L` (izquierda) o `S` (se queda). No son palabras reservadas. | `0 -> X, R, q1` |
| Comodín `*` | Al leer: cualquier símbolo sin regla propia en ese estado, excepto `blank`. Al escribir: deja el mismo símbolo. | `* -> *, L, q2` |
| Tipos | `int` (32 bits), `string`, `bool`. Toda declaración lleva valor inicial. Todas las variables son globales. | `int n = 5;` `bool ok = false;` |
| Cadenas | Entre comillas dobles; caracteres ASCII imprimibles excepto `"`; sin secuencias de escape. | `string s = "n: ";` |
| Asignación | La variable debe estar declarada y el tipo debe coincidir. | `n = n + 1;` |
| Aritméticos | `+ - * /` entre enteros; `/` es división entera. No hay menos unario (`0 - n`). | `int t = n * 2 + 1;` |
| Concatenación | `+` entre cadenas; un `int` o `bool` junto a una cadena se convierte solo. | `print("n = " + n);` |
| Relacionales | `< > <= >= == !=` entre enteros; solo en la condición de `if` o `while`, una vez y sin paréntesis propios. | `while (n <= 3) { ... }` |
| `if` / `else` | Condición bool; llaves obligatorias. | `if (ok) { print(1); } else { print(2); }` |
| `while` | Condición bool; llaves obligatorias. | `while (i < n) { i = i + 1; }` |
| `print(e)` | Imprime un int, string o bool y un salto de línea. | `print(total);` |
| `run M(cadena, límite)` | Ejecuta la máquina `M` con un máximo de pasos; regresa `true` si acepta. | `run SumarUno("11", 100)` |
| `output M(cadena, límite)` | Igual que `run`, pero regresa la cinta final sin los blancos de los extremos. | `output SumarUno("11", 100)` da `"111"` |
| `repeat(s, n)` | La cadena `s` repetida `n` veces. | `repeat("1", 5)` da `"11111"` |
| `input()` | Lee una línea del teclado (string). Asignada directo a un `int`, se convierte sola. | `int n = input();` |
| `int(s)` | Convierte una cadena a entero (admite espacios y un `-`). | `int("12") + 1` da `13` |
| `string(x)` | Convierte un int o un bool a cadena. | `string(5)` da `"5"` |

Límites al ejecutar: las cadenas y la cinta tienen a lo más 2000 caracteres; la cinta
es infinita en los dos sentidos dentro de ese límite.

## 7. Mensajes de error

Los errores del programa tienen el formato

```
archivo.tur:línea:columna: error <fase>: <mensaje>
```

El traductor no se detiene en el primer error: los muestra todos, ordenados por línea
y columna, y termina con `N errores encontrados; no se generó código.` (código 1).
Los errores de ejecución no tienen columna:

```
archivo.tur:línea: error de ejecución: <mensaje>
```

| Fase | Ejemplo real |
|---|---|
| Léxica | `errores_lexicos.tur:1:11: error léxico: carácter no reconocido '@'` |
| Sintáctica | `errores_sintacticos.tur:12:3: error sintáctico: se esperaba '=' y se encontró '+'` |
| Semántica | `regla_duplicada.tur:7:9: error semántico: regla duplicada para el símbolo 1` |
| Ejecución | `lectura.tur:2: error de ejecución: conversión inválida: "hola" no es un número` |

El último ejemplo sale de `examples/lectura.tur` con la entrada `hola`:

```
$ echo hola | ./turlang examples/lectura.tur
n: 
lectura.tur:2: error de ejecución: conversión inválida: "hola" no es un número
```

## 8. Ensamblar y ejecutar el `.asm`

`--asm` genera ensamblador NASM para x86 de 32 bits a partir del código optimizado.
Para obtener un ejecutable en Windows:

```
.\turlang examples\sumar_uno.tur --asm
scripts\ensamblar.bat examples\sumar_uno.asm
examples\sumar_uno.exe
```

```
generado: examples\sumar_uno.asm
generado: c:\...\turlang\examples\sumar_uno.exe
aceptada
```

`ensamblar.bat` busca NASM y el gcc de 32 bits en `C:\Program Files (x86)\SASM`, o en
la carpeta indicada por la variable de entorno `SASM_DIR`, y termina con un mensaje
claro si no los encuentra. El `.exe` da la misma salida y el mismo código de salida
que `turlang` con el mismo programa.

Para verlo en SASM, abre el `.asm` y revisa en la configuración de construcción que
el ensamblador sea NASM y el modo x86 (32 bits); después constrúyelo y ejecútalo
desde el menú de SASM. El archivo usa la macro `C()` para los nombres de la
biblioteca de C, de modo que también se puede ensamblar en Linux con `nasm -f elf32`.

## 9. Llevar un programa al prototipo

1. Genera el compilado: `turlang programa.tur --esp32` (crea `programa.turc` en la
   misma carpeta; lleva el código fuente, las variables y los cuádruplos sin
   optimizar).
2. Puedes probarlo en la computadora antes: `turlang programa.turc`.
3. Copia el `.turc` a la tarjeta SD (formato FAT32).
4. Inserta la SD en el prototipo y elige el programa en el menú. *Pendiente: depende
   del firmware del ESP32.*

## 10. Pruebas

```
mingw32-make test     (Windows)
make test             (Linux / macOS, con pwsh)
```

Compila el traductor y ejecuta `scripts/probar.ps1`, que imprime una línea `OK` o
`FALLA` por prueba, al final `N pruebas, M fallas`, y termina con código 1 si alguna
falla. Junto a cada `.tur` va su salida esperada (`.esperado`) y, si hace falta, su
entrada (`.entrada`), su código de salida (`.codigo`) y su código intermedio
(`.cuadruplos`).

| Carpeta | Qué cubre |
|---|---|
| `tests/lexical` | Errores léxicos: caracteres no reconocidos y cadenas sin cerrar. |
| `tests/syntax` | Errores sintácticos y la recuperación del Parser. |
| `tests/semantic` | Variables no declaradas, tipos, reglas y estados duplicados o inexistentes, máquinas duplicadas. |
| `tests/intermediate` | Optimización (salida de `--cuadruplos`) y ejecución del resultado. |
| `tests/execution` | Ejecución de programas completos, con máquinas y con entrada por teclado. |

En `tests/execution` y `tests/intermediate` cada programa se ejecuta también desde su
`.turc`, para comprobar que el código sin optimizar da la misma salida.

## 11. Estructura del repositorio

```
src/main.cpp        línea de comandos
src/common/         tokens, errores, tabla de símbolos, tablas de texto, entrada y salida de consola
src/frontend/       Lexer, árbol sintáctico, Parser y análisis semántico
src/ir/             código intermedio, tripletas, optimizador, costos e impresión
src/backend/        generador de ensamblador x86
src/runtime/        cuádruplos, intérprete, simulador de máquinas y .turc (también para el ESP32)
scripts/            ensamblar.bat (NASM + gcc de 32 bits), probar.ps1 (pruebas),
                    instalar y desinstalar (.bat y .ps1) y paquete.ps1 (arma el zip)
dist/               turlang-windows.zip, generado con mingw32-make paquete (no se sube)
tests/              pruebas automáticas por fase
examples/           programas .tur de ejemplo
docs/               hojas de instrucciones por etapa
esp32/              firmware del prototipo (pendiente)
```
