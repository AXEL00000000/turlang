# desinstalar.ps1 - quita TurLang del usuario actual
#
# Borra %LOCALAPPDATA%\Programs\TurLang y quita esa carpeta del PATH del usuario
# (el PATH del sistema no se toca).

$ErrorActionPreference = 'Stop'

$destino = Join-Path $env:LOCALAPPDATA 'Programs\TurLang'

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

# 1. Quitar la carpeta del PATH del usuario (las demas entradas quedan igual).
$entradas = @((Leer-PathUsuario) -split ';')
$quedan = @($entradas | Where-Object { $_ -eq '' -or -not (Es-Destino $_) })
$estabaEnPath = $quedan.Count -ne $entradas.Count
if ($estabaEnPath) {
    Escribir-PathUsuario ($quedan -join ';')
}

# 2. Borrar la carpeta.
$habiaCarpeta = Test-Path $destino
if ($habiaCarpeta) {
    try {
        Remove-Item -Recurse -Force $destino
    } catch {
        Write-Output "desinstalar: no se pudo borrar $destino. Cierra las terminales que esten usando turlang y vuelve a intentar."
        exit 1
    }
}

if (-not $habiaCarpeta -and -not $estabaEnPath) {
    Write-Output "TurLang no estaba instalado."
} else {
    Write-Output "TurLang se desinstalo: se borro $destino y se quito del PATH del usuario."
    Write-Output "Las terminales que ya estaban abiertas lo siguen viendo hasta que se cierren."
}
exit 0
