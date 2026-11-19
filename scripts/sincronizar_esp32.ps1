# sincronizar_esp32.ps1 - prepara el sketch esp32/turlang_esp32
#
# 1. Borra y vuelve a copiar src/runtime/*.h y *.cpp en esp32/turlang_esp32/src/runtime/.
# 2. Compila cada .tur de esp32/programas/ (en orden alfabetico) con turlang --esp32.
#    Si alguno tiene errores, se detiene y los muestra.
# 3. Genera esp32/turlang_esp32/Programas.h con los .turc incrustados como literales
#    de cadena cruda y borra los .turc temporales.
# 4. Imprime cuantos programas incrusto.

$ErrorActionPreference = 'Stop'

$raiz = Split-Path -Parent $PSScriptRoot
$turlang = Join-Path $raiz 'turlang.exe'
if (-not (Test-Path $turlang)) { $turlang = Join-Path $raiz 'turlang' }
if (-not (Test-Path $turlang)) {
    Write-Output "sincronizar_esp32: no se encontro el ejecutable turlang en $raiz (compila con make)"
    exit 1
}

$sketch = Join-Path $raiz 'esp32\turlang_esp32'
$programas = Join-Path $raiz 'esp32\programas'
$utf8 = New-Object System.Text.UTF8Encoding($false)

# 1. Copia del runtime
$destinoRuntime = Join-Path $sketch 'src\runtime'
if (Test-Path $destinoRuntime) { Remove-Item -Recurse -Force $destinoRuntime }
New-Item -ItemType Directory -Force -Path $destinoRuntime | Out-Null
Get-ChildItem (Join-Path $raiz 'src\runtime') -File |
    Where-Object { $_.Extension -eq '.h' -or $_.Extension -eq '.cpp' } |
    ForEach-Object { Copy-Item $_.FullName $destinoRuntime }

# 2. Compilar los programas
$fuentes = @(Get-ChildItem $programas -Filter '*.tur' -File | Sort-Object Name)
$incrustados = @()
foreach ($fuente in $fuentes) {
    $turc = [System.IO.Path]::ChangeExtension($fuente.FullName, '.turc')
    if (Test-Path $turc) { Remove-Item -Force $turc }

    # Con 'Stop', PowerShell 5.1 trata como excepcion lo que turlang escribe en stderr.
    $ErrorActionPreference = 'Continue'
    $salida = & $turlang $fuente.FullName --esp32 2>&1
    $codigo = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    if ($codigo -ne 0 -or -not (Test-Path $turc)) {
        Write-Output "sincronizar_esp32: $($fuente.Name) tiene errores:"
        $salida | ForEach-Object { Write-Output "  $_" }
        if (Test-Path $turc) { Remove-Item -Force $turc }
        exit 1
    }

    $texto = [System.IO.File]::ReadAllText($turc, $utf8) -replace "`r`n", "`n"
    Remove-Item -Force $turc
    if ($texto.Contains(')TURC"')) {
        Write-Output "sincronizar_esp32: $($fuente.Name) contiene el texto )TURC`" y no se puede incrustar"
        exit 1
    }

    # Nombre: el campo NOMBRE del .turc, sin la extension .tur
    $nombre = $null
    foreach ($linea in $texto.Split("`n")) {
        if ($linea.StartsWith("NOMBRE`t")) { $nombre = $linea.Substring(7); break }
    }
    if (-not $nombre) {
        Write-Output "sincronizar_esp32: $($fuente.Name): el .turc no tiene el campo NOMBRE"
        exit 1
    }
    if ($nombre.EndsWith('.tur')) { $nombre = $nombre.Substring(0, $nombre.Length - 4) }
    $incrustados += @{ Nombre = $nombre; Turc = $texto }
}

# 3. Programas.h
$h = New-Object System.Text.StringBuilder
[void]$h.Append("#pragma once`n`n")
[void]$h.Append("// GENERADO por scripts/sincronizar_esp32.ps1 (mingw32-make esp32). No editar a mano:`n")
[void]$h.Append("// los programas salen de esp32/programas/.`n`n")
[void]$h.Append("struct ProgramaIncrustado { const char* nombre; const char* turc; };`n`n")
[void]$h.Append("inline const ProgramaIncrustado PROGRAMAS[] = {`n")
foreach ($p in $incrustados) {
    [void]$h.Append("    {`"$($p.Nombre)`", R`"TURC(")
    [void]$h.Append($p.Turc)
    [void]$h.Append(")TURC`"},`n")
}
if ($incrustados.Count -eq 0) { [void]$h.Append("    {nullptr, nullptr},`n") }
[void]$h.Append("};`n`n")
[void]$h.Append("inline const int NUM_PROGRAMAS = $($incrustados.Count);`n")
[System.IO.File]::WriteAllText((Join-Path $sketch 'Programas.h'), $h.ToString(), $utf8)

# 4. Resumen
Write-Output "sincronizar_esp32: runtime copiado en esp32/turlang_esp32/src/runtime"
Write-Output "sincronizar_esp32: $($incrustados.Count) programas incrustados en esp32/turlang_esp32/Programas.h"
foreach ($p in $incrustados) { Write-Output "  $($p.Nombre)" }
exit 0
