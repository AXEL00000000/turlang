# paquete.ps1 - arma dist/turlang-windows.zip (lo llama "make paquete")
#
# Contenido: turlang.exe, instalar.bat, desinstalar.bat, instalar.ps1,
# desinstalar.ps1, README.md y ejemplos/ con cinco programas.

$ErrorActionPreference = 'Stop'

$raiz = Split-Path -Parent $PSScriptRoot
$dist = Join-Path $raiz 'dist'
$preparado = Join-Path $dist 'turlang-windows'
$zip = Join-Path $dist 'turlang-windows.zip'

$archivos = @('turlang.exe', 'scripts\instalar.bat', 'scripts\desinstalar.bat',
              'scripts\instalar.ps1', 'scripts\desinstalar.ps1', 'README.md')
$ejemplos = @('examples\sumar_uno.tur', 'tests\execution\factorial.tur', 'tests\execution\fibonacci.tur',
              'tests\execution\binario.tur', 'tests\execution\ceros_unos.tur')

foreach ($a in $archivos + $ejemplos) {
    if (-not (Test-Path (Join-Path $raiz $a))) {
        Write-Output "paquete: falta $a"
        exit 1
    }
}

if (Test-Path $dist) { Remove-Item -Recurse -Force $dist }
New-Item -ItemType Directory -Force -Path (Join-Path $preparado 'ejemplos') | Out-Null

# Los archivos de texto van con finales de linea de Windows (\r\n): los .bat los necesitan.
function Copiar([string]$relativo, [string]$carpeta) {
    $origen = Join-Path $raiz $relativo
    $nombre = Split-Path -Leaf $relativo
    $destino = Join-Path $carpeta $nombre
    if ($nombre -like '*.exe') {
        Copy-Item $origen $destino
    } else {
        $bytes = [System.IO.File]::ReadAllBytes($origen)
        $texto = [System.Text.Encoding]::UTF8.GetString($bytes) -replace "`r`n", "`n" -replace "`n", "`r`n"
        [System.IO.File]::WriteAllText($destino, $texto, (New-Object System.Text.UTF8Encoding($false)))
    }
}

foreach ($a in $archivos) { Copiar $a $preparado }
foreach ($a in $ejemplos) { Copiar $a (Join-Path $preparado 'ejemplos') }

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory($preparado, $zip,
    [System.IO.Compression.CompressionLevel]::Optimal, $false)
Remove-Item -Recurse -Force $preparado

Write-Output "generado: dist/turlang-windows.zip"
