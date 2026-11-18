# instalar.ps1 - instala TurLang para el usuario actual
#
# Copia turlang.exe (de la misma carpeta que este script) a
# %LOCALAPPDATA%\Programs\TurLang y agrega esa carpeta al PATH del usuario, sin
# duplicarla y sin tocar el PATH del sistema. Si ya estaba instalado, lo reemplaza.

$ErrorActionPreference = 'Stop'

$origen = Join-Path $PSScriptRoot 'turlang.exe'
$destino = Join-Path $env:LOCALAPPDATA 'Programs\TurLang'

if (-not (Test-Path $origen)) {
    Write-Output "instalar: no se encontro turlang.exe en $PSScriptRoot"
    exit 1
}

# El PATH del usuario se lee y se escribe en el registro sin expandir las
# variables (%USERPROFILE%, ...) y conservando su tipo REG_EXPAND_SZ.
function Leer-PathUsuario {
    $clave = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey('Environment')
    $valor = $clave.GetValue('Path', '', [Microsoft.Win32.RegistryValueOptions]::DoNotExpandEnvironmentNames)
    $clave.Close()
    return [string]$valor
}

function Escribir-PathUsuario([string]$valor) {
    $clave = [Microsoft.Win32.Registry]::CurrentUser.OpenSubKey('Environment', $true)
    $clave.SetValue('Path', $valor, [Microsoft.Win32.RegistryValueKind]::ExpandString)
    $clave.Close()
    # Avisa a Windows del cambio para que las terminales nuevas lo vean.
    if (-not ('TurLang.Entorno' -as [type])) {
        Add-Type -Namespace TurLang -Name Entorno -MemberDefinition @'
[DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint Msg, UIntPtr wParam, string lParam,
    uint fuFlags, uint uTimeout, out UIntPtr lpdwResult);
'@
    }
    $resultado = [UIntPtr]::Zero
    [void][TurLang.Entorno]::SendMessageTimeout([IntPtr]0xffff, 0x1A, [UIntPtr]::Zero, 'Environment', 2, 5000, [ref]$resultado)
}

function Es-Destino([string]$entrada) {
    $expandida = [Environment]::ExpandEnvironmentVariables($entrada).TrimEnd('\')
    return $expandida -ieq $destino.TrimEnd('\')
}

# 1. Copiar el ejecutable (reemplaza una instalacion anterior).
$yaEstaba = Test-Path (Join-Path $destino 'turlang.exe')
New-Item -ItemType Directory -Force -Path $destino | Out-Null
try {
    Copy-Item -Force $origen (Join-Path $destino 'turlang.exe')
} catch {
    Write-Output "instalar: no se pudo copiar turlang.exe a $destino. Cierra las terminales que lo esten usando y vuelve a intentar."
    exit 1
}

# 2. Agregar la carpeta al final del PATH del usuario si no esta (el resto del
#    texto no se toca).
$actual = Leer-PathUsuario
$enPath = @($actual -split ';' | Where-Object { $_ -ne '' -and (Es-Destino $_) }).Count -gt 0
if (-not $enPath) {
    if ($actual -eq '') {
        $nuevo = $destino
    } elseif ($actual.EndsWith(';')) {
        # Se conserva el ';' final para que al desinstalar quede igual que antes.
        $nuevo = $actual + $destino + ';'
    } else {
        $nuevo = $actual + ';' + $destino
    }
    Escribir-PathUsuario $nuevo
}

if ($yaEstaba) {
    Write-Output "TurLang se actualizo en $destino"
} else {
    Write-Output "TurLang se instalo en $destino"
}
if ($enPath) {
    Write-Output "La carpeta ya estaba en el PATH del usuario."
} else {
    Write-Output "Se agrego la carpeta al PATH del usuario."
}
Write-Output ""
Write-Output "Abre una terminal nueva y escribe: turlang programa.tur"
exit 0
