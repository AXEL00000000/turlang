# probar.ps1 - corre todas las pruebas de tests/
#
# Para cada .tur dentro de tests/:
#   1. Ejecuta turlang <archivo> (con su .entrada, si existe) y compara la salida y el
#      codigo con .esperado y .codigo (0 si no existe).
#   2. Si existe .cuadruplos, compara tambien la salida de --cuadruplos.
#   3. En tests/execution y tests/intermediate genera el .turc con --esp32, lo ejecuta
#      y lo compara con el mismo .esperado (programa sin optimizar, desde el compilado).
# Al comparar ignora la diferencia entre \r\n y \n. Termina con codigo 1 si hubo fallas.

$ErrorActionPreference = 'Stop'

$raiz = Split-Path -Parent $PSScriptRoot
$turlang = Join-Path $raiz 'turlang.exe'
if (-not (Test-Path $turlang)) { $turlang = Join-Path $raiz 'turlang' }
if (-not (Test-Path $turlang)) {
    Write-Output "probar: no se encontro el ejecutable turlang en $raiz (compila con make)"
    exit 2
}

$utf8 = New-Object System.Text.UTF8Encoding($false)
$script:pruebas = 0
$script:fallas = 0

# Ejecuta turlang con los argumentos dados; regresa la salida (UTF-8) y el codigo.
function Ejecutar([string[]]$argumentos, [string]$entrada) {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $turlang
    $psi.Arguments = ($argumentos | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $psi.WorkingDirectory = $raiz
    $psi.UseShellExecute = $false
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.StandardOutputEncoding = $utf8
    $psi.StandardErrorEncoding = $utf8

    $proceso = [System.Diagnostics.Process]::Start($psi)
    $lecturaSalida = $proceso.StandardOutput.ReadToEndAsync()
    $lecturaError = $proceso.StandardError.ReadToEndAsync()
    if ($entrada) {
        $bytes = $utf8.GetBytes($entrada)
        $proceso.StandardInput.BaseStream.Write($bytes, 0, $bytes.Length)
    }
    $proceso.StandardInput.Close()

    if (-not $proceso.WaitForExit(30000)) {
        $proceso.Kill()
        return @{ Salida = ''; Codigo = 'sin terminar (30 s)' }
    }
    $proceso.WaitForExit()
    return @{ Salida = $lecturaSalida.Result + $lecturaError.Result; Codigo = $proceso.ExitCode }
}

function Normalizar([string]$texto) {
    return $texto -replace "`r`n", "`n"
}

function LeerTexto([string]$ruta) {
    return Normalizar ([System.IO.File]::ReadAllText($ruta, $utf8))
}

# Registra una prueba: OK si la salida y el codigo son los esperados.
function Comparar([string]$nombre, $resultado, [string]$esperado, $codigoEsperado) {
    $script:pruebas++
    $salida = Normalizar $resultado.Salida
    if ($salida -ceq $esperado -and "$($resultado.Codigo)" -eq "$codigoEsperado") {
        Write-Output "OK     $nombre"
        return
    }
    $script:fallas++
    $motivo = @()
    if ("$($resultado.Codigo)" -ne "$codigoEsperado") {
        $motivo += "codigo $($resultado.Codigo), se esperaba $codigoEsperado"
    }
    if ($salida -cne $esperado) { $motivo += 'la salida es distinta' }
    Write-Output "FALLA  $nombre ($($motivo -join '; '))"
}

$archivos = Get-ChildItem (Join-Path $raiz 'tests') -Recurse -File |
    Where-Object { $_.Extension -eq '.tur' } |
    Sort-Object FullName

foreach ($archivo in $archivos) {
    $base = $archivo.FullName.Substring(0, $archivo.FullName.Length - 4)
    $relativo = $archivo.FullName.Substring($raiz.Length + 1).Replace('\', '/')
    $carpeta = $archivo.Directory.Name

    if (-not (Test-Path "$base.esperado")) {
        $script:pruebas++
        $script:fallas++
        Write-Output "FALLA  $relativo (no existe el .esperado)"
        continue
    }
    $esperado = LeerTexto "$base.esperado"
    $entrada = if (Test-Path "$base.entrada") { [System.IO.File]::ReadAllText("$base.entrada", $utf8) } else { '' }
    $codigo = if (Test-Path "$base.codigo") { (LeerTexto "$base.codigo").Trim() } else { '0' }

    # 1. Ejecucion normal (programa optimizado).
    Comparar $relativo (Ejecutar @($relativo) $entrada) $esperado $codigo

    # 2. Codigo intermedio.
    if (Test-Path "$base.cuadruplos") {
        Comparar "$relativo --cuadruplos" (Ejecutar @($relativo, '--cuadruplos') '') (LeerTexto "$base.cuadruplos") '0'
    }

    # 3. El mismo programa desde el compilado .turc (sin optimizar).
    if ($carpeta -eq 'execution' -or $carpeta -eq 'intermediate') {
        $turc = $relativo + 'c'
        $generado = Ejecutar @($relativo, '--esp32') ''
        if ($generado.Codigo -ne 0) {
            $script:pruebas++
            $script:fallas++
            Write-Output "FALLA  $turc (--esp32 termino con codigo $($generado.Codigo))"
        } else {
            Comparar $turc (Ejecutar @($turc) $entrada) $esperado $codigo
        }
        Remove-Item -ErrorAction SilentlyContinue (Join-Path $raiz $turc)
    }
}

Write-Output ''
Write-Output "$($script:pruebas) pruebas, $($script:fallas) fallas"
if ($script:fallas -gt 0) { exit 1 }
exit 0
