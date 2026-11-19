#pragma once

// GENERADO por scripts/sincronizar_esp32.ps1 (mingw32-make esp32). No editar a mano:
// los programas salen de esp32/programas/.

struct ProgramaIncrustado { const char* nombre; const char* turc; };

inline const ProgramaIncrustado PROGRAMAS[] = {
    {"binario", R"TURC(TURC	1
NOMBRE	binario.tur
FUENTE	19
machine Incrementar {
    start derecha
    accept fin
    state derecha {
        0 -> 0, R, derecha
        1 -> 1, R, derecha
        blank -> blank, L, suma
    }
    state suma {
        1 -> 0, L, suma
        0 -> 1, S, fin
        blank -> 1, S, fin
    }
}

print(output Incrementar("1011", 100));
print(output Incrementar("111", 100));
print(output Incrementar("0", 100));
print(run Incrementar("12", 100));
VARS	0
MAQUINAS	1
MAQUINA	Incrementar	47
=	1	Vcadena	-	K
=	1	E0	-	Vcabezal
=	1	E0	-	Vpasos
=	1	Vmaximo	-	Vlimite
<	4	Vpasos	Vlimite	T1
if-false	4	T1	-	J47
+	4	Vpasos	E1	T2
=	4	T2	-	Vpasos
=[]	4	K	Vcabezal	T3
==	5	T3	S0	T4
if-false	5	T4	-	J16
[]=	5	S0	Vcabezal	K
+	5	Vcabezal	E1	T5
=	5	T5	-	Vcabezal
jump	5	-	-	J5
==	6	T3	S1	T6
if-false	6	T6	-	J22
[]=	6	S1	Vcabezal	K
+	6	Vcabezal	E1	T7
=	6	T7	-	Vcabezal
jump	6	-	-	J5
==	7	T3	Sblank	T8
if-false	7	T8	-	J47
[]=	7	Sblank	Vcabezal	K
-	7	Vcabezal	E1	T9
=	7	T9	-	Vcabezal
jump	7	-	-	J28
<	9	Vpasos	Vlimite	T10
if-false	9	T10	-	J47
+	9	Vpasos	E1	T11
=	9	T11	-	Vpasos
=[]	9	K	Vcabezal	T12
==	10	T12	S1	T13
if-false	10	T13	-	J39
[]=	10	S0	Vcabezal	K
-	10	Vcabezal	E1	T14
=	10	T14	-	Vcabezal
jump	10	-	-	J28
==	11	T12	S0	T15
if-false	11	T15	-	J43
[]=	11	S1	Vcabezal	K
ret	11	Btrue	-	-
==	12	T12	Sblank	T16
if-false	12	T16	-	J47
[]=	12	S1	Vcabezal	K
ret	12	Btrue	-	-
ret	1	Bfalse	-	-
PRINCIPAL	24
param	16	C1011	-	-
param	16	E100	-	-
call	16	FIncrementar	E2	T1
=	16	K	-	T2
param	16	T2	-	-
call	16	Fprint	E1	-
param	17	C111	-	-
param	17	E100	-	-
call	17	FIncrementar	E2	T3
=	17	K	-	T4
param	17	T4	-	-
call	17	Fprint	E1	-
param	18	C0	-	-
param	18	E100	-	-
call	18	FIncrementar	E2	T5
=	18	K	-	T6
param	18	T6	-	-
call	18	Fprint	E1	-
param	19	C12	-	-
param	19	E100	-	-
call	19	FIncrementar	E2	T7
param	19	T7	-	-
call	19	Fprint	E1	-
ret	0	-	-	-
FIN
)TURC"},
    {"binario_teclado", R"TURC(TURC	1
NOMBRE	binario_teclado.tur
FUENTE	23
// Pide un numero binario y le suma uno con una maquina
machine Incrementar {
    start derecha
    accept fin
    state derecha {
        0 -> 0, R, derecha
        1 -> 1, R, derecha
        blank -> blank, L, suma
    }
    state suma {
        1 -> 0, L, suma
        0 -> 1, S, fin
        blank -> 1, S, fin
    }
}

print("Escribe un numero binario:");
string b = input();
if (run Incrementar(b, 200)) {
    print(b + " + 1 = " + output Incrementar(b, 200));
} else {
    print("No es un numero binario");
}
VARS	1
b	string
MAQUINAS	1
MAQUINA	Incrementar	47
=	2	Vcadena	-	K
=	2	E0	-	Vcabezal
=	2	E0	-	Vpasos
=	2	Vmaximo	-	Vlimite
<	5	Vpasos	Vlimite	T1
if-false	5	T1	-	J47
+	5	Vpasos	E1	T2
=	5	T2	-	Vpasos
=[]	5	K	Vcabezal	T3
==	6	T3	S0	T4
if-false	6	T4	-	J16
[]=	6	S0	Vcabezal	K
+	6	Vcabezal	E1	T5
=	6	T5	-	Vcabezal
jump	6	-	-	J5
==	7	T3	S1	T6
if-false	7	T6	-	J22
[]=	7	S1	Vcabezal	K
+	7	Vcabezal	E1	T7
=	7	T7	-	Vcabezal
jump	7	-	-	J5
==	8	T3	Sblank	T8
if-false	8	T8	-	J47
[]=	8	Sblank	Vcabezal	K
-	8	Vcabezal	E1	T9
=	8	T9	-	Vcabezal
jump	8	-	-	J28
<	10	Vpasos	Vlimite	T10
if-false	10	T10	-	J47
+	10	Vpasos	E1	T11
=	10	T11	-	Vpasos
=[]	10	K	Vcabezal	T12
==	11	T12	S1	T13
if-false	11	T13	-	J39
[]=	11	S0	Vcabezal	K
-	11	Vcabezal	E1	T14
=	11	T14	-	Vcabezal
jump	11	-	-	J28
==	12	T12	S0	T15
if-false	12	T15	-	J43
[]=	12	S1	Vcabezal	K
ret	12	Btrue	-	-
==	13	T12	Sblank	T16
if-false	13	T16	-	J47
[]=	13	S1	Vcabezal	K
ret	13	Btrue	-	-
ret	2	Bfalse	-	-
PRINCIPAL	20
param	17	CEscribe un numero binario:	-	-
call	17	Fprint	E1	-
call	18	Finput	E0	T1
=	18	T1	-	Vb
param	19	Vb	-	-
param	19	E200	-	-
call	19	FIncrementar	E2	T2
if-false	19	T2	-	J18
+	20	Vb	C + 1 = 	T3
param	20	Vb	-	-
param	20	E200	-	-
call	20	FIncrementar	E2	T4
=	20	K	-	T5
+	20	T3	T5	T6
param	20	T6	-	-
call	20	Fprint	E1	-
jump	19	-	-	J20
param	22	CNo es un numero binario	-	-
call	22	Fprint	E1	-
ret	0	-	-	-
FIN
)TURC"},
    {"ceros_unos", R"TURC(TURC	1
NOMBRE	ceros_unos.tur
FUENTE	31
machine CerosUnos {
    start q0
    accept fin
    state q0 {
        0 -> X, R, q1
        Y -> Y, R, q3
    }
    state q1 {
        0 -> 0, R, q1
        Y -> Y, R, q1
        1 -> Y, L, q2
    }
    state q2 {
        X -> X, R, q0
        * -> *, L, q2
    }
    state q3 {
        Y -> Y, R, q3
        blank -> blank, S, fin
    }
}

print(run CerosUnos("01", 100));
print(run CerosUnos("0011", 100));
print(run CerosUnos("000111", 100));
print(run CerosUnos("001", 100));
print(run CerosUnos("011", 100));
print(run CerosUnos("10", 100));
print(run CerosUnos("", 100));
print(run CerosUnos("000111", 20));
print(output CerosUnos("0011", 100));
VARS	0
MAQUINAS	1
MAQUINA	CerosUnos	76
=	1	Vcadena	-	K
=	1	E0	-	Vcabezal
=	1	E0	-	Vpasos
=	1	Vmaximo	-	Vlimite
<	4	Vpasos	Vlimite	T1
if-false	4	T1	-	J76
+	4	Vpasos	E1	T2
=	4	T2	-	Vpasos
=[]	4	K	Vcabezal	T3
==	5	T3	S0	T4
if-false	5	T4	-	J16
[]=	5	SX	Vcabezal	K
+	5	Vcabezal	E1	T5
=	5	T5	-	Vcabezal
jump	5	-	-	J22
==	6	T3	SY	T6
if-false	6	T6	-	J76
[]=	6	SY	Vcabezal	K
+	6	Vcabezal	E1	T7
=	6	T7	-	Vcabezal
jump	6	-	-	J61
<	8	Vpasos	Vlimite	T8
if-false	8	T8	-	J76
+	8	Vpasos	E1	T9
=	8	T9	-	Vpasos
=[]	8	K	Vcabezal	T10
==	9	T10	S0	T11
if-false	9	T11	-	J33
[]=	9	S0	Vcabezal	K
+	9	Vcabezal	E1	T12
=	9	T12	-	Vcabezal
jump	9	-	-	J22
==	10	T10	SY	T13
if-false	10	T13	-	J39
[]=	10	SY	Vcabezal	K
+	10	Vcabezal	E1	T14
=	10	T14	-	Vcabezal
jump	10	-	-	J22
==	11	T10	S1	T15
if-false	11	T15	-	J76
[]=	11	SY	Vcabezal	K
-	11	Vcabezal	E1	T16
=	11	T16	-	Vcabezal
jump	11	-	-	J45
<	13	Vpasos	Vlimite	T17
if-false	13	T17	-	J76
+	13	Vpasos	E1	T18
=	13	T18	-	Vpasos
=[]	13	K	Vcabezal	T19
==	14	T19	SX	T20
if-false	14	T20	-	J56
[]=	14	SX	Vcabezal	K
+	14	Vcabezal	E1	T21
=	14	T21	-	Vcabezal
jump	14	-	-	J5
!=	15	T19	Sblank	T22
if-false	15	T22	-	J76
-	15	Vcabezal	E1	T23
=	15	T23	-	Vcabezal
jump	15	-	-	J45
<	17	Vpasos	Vlimite	T24
if-false	17	T24	-	J76
+	17	Vpasos	E1	T25
=	17	T25	-	Vpasos
=[]	17	K	Vcabezal	T26
==	18	T26	SY	T27
if-false	18	T27	-	J72
[]=	18	SY	Vcabezal	K
+	18	Vcabezal	E1	T28
=	18	T28	-	Vcabezal
jump	18	-	-	J61
==	19	T26	Sblank	T29
if-false	19	T29	-	J76
[]=	19	Sblank	Vcabezal	K
ret	19	Btrue	-	-
ret	1	Bfalse	-	-
PRINCIPAL	47
param	23	C01	-	-
param	23	E100	-	-
call	23	FCerosUnos	E2	T1
param	23	T1	-	-
call	23	Fprint	E1	-
param	24	C0011	-	-
param	24	E100	-	-
call	24	FCerosUnos	E2	T2
param	24	T2	-	-
call	24	Fprint	E1	-
param	25	C000111	-	-
param	25	E100	-	-
call	25	FCerosUnos	E2	T3
param	25	T3	-	-
call	25	Fprint	E1	-
param	26	C001	-	-
param	26	E100	-	-
call	26	FCerosUnos	E2	T4
param	26	T4	-	-
call	26	Fprint	E1	-
param	27	C011	-	-
param	27	E100	-	-
call	27	FCerosUnos	E2	T5
param	27	T5	-	-
call	27	Fprint	E1	-
param	28	C10	-	-
param	28	E100	-	-
call	28	FCerosUnos	E2	T6
param	28	T6	-	-
call	28	Fprint	E1	-
param	29	C	-	-
param	29	E100	-	-
call	29	FCerosUnos	E2	T7
param	29	T7	-	-
call	29	Fprint	E1	-
param	30	C000111	-	-
param	30	E20	-	-
call	30	FCerosUnos	E2	T8
param	30	T8	-	-
call	30	Fprint	E1	-
param	31	C0011	-	-
param	31	E100	-	-
call	31	FCerosUnos	E2	T9
=	31	K	-	T10
param	31	T10	-	-
call	31	Fprint	E1	-
ret	0	-	-	-
FIN
)TURC"},
    {"factorial", R"TURC(TURC	1
NOMBRE	factorial.tur
FUENTE	8
int n = 5;
int f = 1;
int i = 1;
while (i <= n) {
    f = f * i;
    i = i + 1;
}
print("factorial de " + n + " = " + f);
VARS	3
n	int
f	int
i	int
MAQUINAS	0
PRINCIPAL	18
=	1	E5	-	Vn
=	2	E1	-	Vf
=	3	E1	-	Vi
<=	4	Vi	Vn	T1
if-false	4	T1	-	J11
*	5	Vf	Vi	T2
=	5	T2	-	Vf
+	6	Vi	E1	T3
=	6	T3	-	Vi
jump	4	-	-	J4
string	8	Vn	-	T4
+	8	Cfactorial de 	T4	T5
+	8	T5	C = 	T6
string	8	Vf	-	T7
+	8	T6	T7	T8
param	8	T8	-	-
call	8	Fprint	E1	-
ret	0	-	-	-
FIN
)TURC"},
    {"fibonacci", R"TURC(TURC	1
NOMBRE	fibonacci.tur
FUENTE	11
int a = 0;
int b = 1;
int t = 0;
int i = 0;
while (i < 10) {
    print(a);
    t = a + b;
    a = b;
    b = t;
    i = i + 1;
}
VARS	4
a	int
b	int
t	int
i	int
MAQUINAS	0
PRINCIPAL	16
=	1	E0	-	Va
=	2	E1	-	Vb
=	3	E0	-	Vt
=	4	E0	-	Vi
<	5	Vi	E10	T1
if-false	5	T1	-	J16
param	6	Va	-	-
call	6	Fprint	E1	-
+	7	Va	Vb	T2
=	7	T2	-	Vt
=	8	Vb	-	Va
=	9	Vt	-	Vb
+	10	Vi	E1	T3
=	10	T3	-	Vi
jump	5	-	-	J5
ret	0	-	-	-
FIN
)TURC"},
    {"sumar_uno", R"TURC(TURC	1
NOMBRE	sumar_uno.tur
FUENTE	15
machine SumarUno {
    start q0
    accept fin
    state q0 {
        1 -> 1, R, q0
        blank -> 1, S, fin
    }
}

print(run SumarUno("11", 100));
print(output SumarUno("11", 100));
print(output SumarUno("", 100));
print(output SumarUno(repeat("1", 5), 100));
print(run SumarUno("111", 2));
print(run SumarUno("1a1", 100));
VARS	0
MAQUINAS	1
MAQUINA	SumarUno	20
=	1	Vcadena	-	K
=	1	E0	-	Vcabezal
=	1	E0	-	Vpasos
=	1	Vmaximo	-	Vlimite
<	4	Vpasos	Vlimite	T1
if-false	4	T1	-	J20
+	4	Vpasos	E1	T2
=	4	T2	-	Vpasos
=[]	4	K	Vcabezal	T3
==	5	T3	S1	T4
if-false	5	T4	-	J16
[]=	5	S1	Vcabezal	K
+	5	Vcabezal	E1	T5
=	5	T5	-	Vcabezal
jump	5	-	-	J5
==	6	T3	Sblank	T6
if-false	6	T6	-	J20
[]=	6	S1	Vcabezal	K
ret	6	Btrue	-	-
ret	1	Bfalse	-	-
PRINCIPAL	37
param	10	C11	-	-
param	10	E100	-	-
call	10	FSumarUno	E2	T1
param	10	T1	-	-
call	10	Fprint	E1	-
param	11	C11	-	-
param	11	E100	-	-
call	11	FSumarUno	E2	T2
=	11	K	-	T3
param	11	T3	-	-
call	11	Fprint	E1	-
param	12	C	-	-
param	12	E100	-	-
call	12	FSumarUno	E2	T4
=	12	K	-	T5
param	12	T5	-	-
call	12	Fprint	E1	-
param	13	C1	-	-
param	13	E5	-	-
call	13	Frepeat	E2	T6
param	13	T6	-	-
param	13	E100	-	-
call	13	FSumarUno	E2	T7
=	13	K	-	T8
param	13	T8	-	-
call	13	Fprint	E1	-
param	14	C111	-	-
param	14	E2	-	-
call	14	FSumarUno	E2	T9
param	14	T9	-	-
call	14	Fprint	E1	-
param	15	C1a1	-	-
param	15	E100	-	-
call	15	FSumarUno	E2	T10
param	15	T10	-	-
call	15	Fprint	E1	-
ret	0	-	-	-
FIN
)TURC"},
};

inline const int NUM_PROGRAMAS = 6;
