# patchen.ps1 - erzeugt die Fix-EXEs aus der eigenen Original-EXE (GOG) mit den Differenz-Patches (*.ndp)
# in diesem Ordner. Kein Compiler noetig. Die Original-EXE wird nur gelesen, nie veraendert.
# Aufruf ueber PATCHEN.bat. Laeuft mit Windows PowerShell 5.1 und PowerShell 7.
param(
    [Parameter(Mandatory=$true)][string]$Spiel,   # Spielordner (GOG)
    [Parameter(Mandatory=$true)][string]$Name,    # Name der Original-EXE, z. B. SCHWEIF.EXE
    [Parameter(Mandatory=$true)][string]$Ziel     # Ausgabeordner (Build)
)
$ErrorActionPreference = 'Stop'
$md5 = [System.Security.Cryptography.MD5]::Create()
function Hex([byte[]]$b) { ($b | ForEach-Object { $_.ToString('x2') }) -join '' }
function Lies-Patch([string]$pfad) {
    $fs = [System.IO.File]::OpenRead($pfad)
    try {
        $gz = New-Object System.IO.Compression.GZipStream($fs, [System.IO.Compression.CompressionMode]::Decompress)
        $ms = New-Object System.IO.MemoryStream
        $gz.CopyTo($ms); $gz.Dispose()
        return ,$ms.ToArray()
    } finally { $fs.Dispose() }
}
$patches = Get-ChildItem -Path $PSScriptRoot -Filter '*.ndp' | Sort-Object Name
if (-not $patches) { throw "Keine *.ndp-Dateien neben patchen.ps1 gefunden." }

# Quelle: erwartete Groesse und MD5 aus dem ersten Patch
$p0 = Lies-Patch $patches[0].FullName
if ([System.Text.Encoding]::ASCII.GetString($p0, 0, 4) -ne 'NDP1') { throw "$($patches[0].Name) ist kein NDP1-Patch." }
$srcLen = [BitConverter]::ToUInt32($p0, 4)
$srcMd5 = Hex ($p0[8..23])

# Original-EXE suchen: erst im Spielordner, sonst darunter (passende Pruefsumme)
$kand = @()
$direkt = Join-Path $Spiel $Name
if (Test-Path -LiteralPath $direkt) { $kand += Get-Item -LiteralPath $direkt }
$kand += Get-ChildItem -LiteralPath $Spiel -Recurse -Filter $Name -ErrorAction SilentlyContinue | Where-Object { $_.FullName -ne $direkt -and $_.FullName -ne (Resolve-Path -LiteralPath $direkt -ErrorAction SilentlyContinue) }
$src = $null; $gefunden = $null; $gesehen = @()
foreach ($k in $kand) {
    if ($k.Length -ne $srcLen) { $gesehen += "$($k.FullName) (Groesse $($k.Length))"; continue }
    $b = [System.IO.File]::ReadAllBytes($k.FullName)
    $h = Hex ($md5.ComputeHash($b))
    if ($h -eq $srcMd5) { $src = $b; $gefunden = $k.FullName; break }
    $gesehen += "$($k.FullName) (MD5 $h)"
}
if (-not $src) {
    Write-Host ""
    Write-Host "Keine passende $Name gefunden." -ForegroundColor Red
    Write-Host "Der Patch braucht genau die deutsche GOG-Fassung (MD5 $srcMd5, $srcLen Byte)."
    if ($gesehen) { Write-Host "Gefunden, aber passt nicht:"; $gesehen | ForEach-Object { Write-Host "  $_" } }
    exit 1
}
Write-Host "Original: $gefunden"
Write-Host "          MD5 $srcMd5 - passt."
New-Item -ItemType Directory -Force -Path $Ziel | Out-Null

foreach ($pf in $patches) {
    $p = Lies-Patch $pf.FullName
    if ([System.Text.Encoding]::ASCII.GetString($p, 0, 4) -ne 'NDP1') { throw "$($pf.Name) ist kein NDP1-Patch." }
    if ((Hex ($p[8..23])) -ne $srcMd5) { throw "$($pf.Name) gehoert zu einer anderen Original-EXE." }
    $dstLen = [BitConverter]::ToUInt32($p, 24)
    $dstMd5 = Hex ($p[28..43])
    $dst = New-Object byte[] $dstLen
    $o = 44; $d = 0
    while ($true) {
        $t = $p[$o]; $o++
        if ($t -eq 0) { break }
        if ($t -eq 1) {
            $so = [BitConverter]::ToUInt32($p, $o); $l = [BitConverter]::ToUInt32($p, $o + 4); $o += 8
            [Array]::Copy($src, [long]$so, $dst, [long]$d, [long]$l); $d += $l
        } elseif ($t -eq 2) {
            $l = [BitConverter]::ToUInt32($p, $o); $o += 4
            [Array]::Copy($p, [long]$o, $dst, [long]$d, [long]$l); $o += $l; $d += $l
        } else { throw "$($pf.Name): unbekannter Befehl $t" }
    }
    $h = Hex ($md5.ComputeHash($dst))
    $aus = Join-Path $Ziel ([System.IO.Path]::GetFileNameWithoutExtension($pf.Name) + '.EXE')
    if ($d -ne $dstLen -or $h -ne $dstMd5) { throw "$($pf.Name): Ergebnis stimmt nicht (MD5 $h statt $dstMd5)." }
    [System.IO.File]::WriteAllBytes($aus, $dst)
    Write-Host "Erzeugt:  $aus"
    Write-Host "          MD5 $h - geprueft."
}
Write-Host ""
Write-Host "Fertig. Die Original-EXE wurde nicht veraendert." -ForegroundColor Green
