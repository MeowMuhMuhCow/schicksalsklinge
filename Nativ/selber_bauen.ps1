# selber_bauen.ps1 - Baut alle nativen Windows-EXEs (SCHICK_PORT*.EXE) und den
# Nordlandtrilogie-Starter (NORDLANDTRILOGIE.EXE) direkt aus dem C/C++-Quelltext,
# komplett OHNE Python!
# Falls kein 32-Bit-MinGW-Compiler (gcc.exe/g++.exe) gefunden wird, bietet das
# Skript an, das portable w64devkit (i686) automatisch herunterzuladen.

param(
    [string]$Stufe = "alle_stufen",
    [switch]$OhnePause
)

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$NativDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir  = Split-Path -Parent $NativDir
if (-not (Test-Path (Join-Path $NativDir "src\seg002.c"))) {
    if (Test-Path (Join-Path $NativDir "Nativ\src\seg002.c")) {
        $RootDir  = $NativDir
        $NativDir = Join-Path $RootDir "Nativ"
    } else {
        Write-Host "FEHLER: Quellordner src\seg002.c nicht gefunden." -ForegroundColor Red
        exit 1
    }
}

Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host " Die Schicksalsklinge - Selber-Bauen aus dem C/C++-Quelltext" -ForegroundColor Cyan
Write-Host " (Kein Python erforderlich)" -ForegroundColor Cyan
Write-Host "=================================================================" -ForegroundColor Cyan
Write-Host ""

# 1. Optional: DOS-Bugfix-Paket patchen, falls Patch\patchen.ps1 vorhanden ist
$DosPatchPs1 = Join-Path $RootDir "Patch\patchen.ps1"
if ((Test-Path $DosPatchPs1) -and -not (Test-Path (Join-Path $RootDir "Build\BLADEM_FIX_WICHTIG.EXE"))) {
    Write-Host "[DOS] Erzeuge DOS-Bugfix-EXEs aus deiner GOG-Installation (Patch\patchen.ps1)..." -ForegroundColor Yellow
    try {
        & powershell -NoProfile -ExecutionPolicy Bypass -File $DosPatchPs1
    } catch {
        Write-Host "      Hinweis: DOS-Patch uebersprungen ($($_.Exception.Message))." -ForegroundColor DarkYellow
    }
    Write-Host ""
}

# 2. 32-Bit MinGW-Compiler (gcc.exe / g++.exe) suchen
function Test-Mingw32([string]$BinDir) {
    if (-not $BinDir -or -not (Test-Path $BinDir)) { return $false }
    $gcc = Join-Path $BinDir "gcc.exe"
    $gpp = Join-Path $BinDir "g++.exe"
    if (-not ((Test-Path $gcc) -and (Test-Path $gpp))) { return $false }
    try {
        $out = & $gcc -m32 -dumpmachine 2>&1
        return ($LASTEXITCODE -eq 0)
    } catch {
        return $false
    }
}

$Kandidaten = @(
    $env:SCHICK_MINGW_BIN,
    (Join-Path $RootDir "Compiler\w64devkit\bin"),
    (Join-Path $RootDir "Compiler\mingw32\bin"),
    (Join-Path $NativDir "Compiler\w64devkit\bin"),
    "C:\w64devkit\bin",
    "C:\msys64\mingw32\bin",
    "C:\mingw32\bin",
    "C:\TDM-GCC-32\bin",
    "C:\TDM-GCC-64\bin",
    "C:\Program Files (x86)\Embarcadero\Dev-Cpp\TDM-GCC-64\bin",
    "C:\Program Files\Embarcadero\Dev-Cpp\TDM-GCC-64\bin"
)

# Auch im PATH nach gcc.exe schauen
$PathGcc = Get-Command gcc.exe -ErrorAction SilentlyContinue
if ($PathGcc) {
    $Kandidaten = @((Split-Path -Parent $PathGcc.Source)) + $Kandidaten
}

$MingwBin = $null
foreach ($k in $Kandidaten) {
    if (Test-Mingw32 $k) {
        $MingwBin = $k
        break
    }
}

if (-not $MingwBin) {
    Write-Host "Kein 32-Bit C/C++-Compiler (MinGW-w64 / w64devkit) auf dem PC gefunden." -ForegroundColor Yellow
    Write-Host "Soll der portable Compiler (w64devkit i686, frei & quelloffen, keine Installation"
    Write-Host "im System, wird nur in Compiler\w64devkit entpackt) jetzt automatisch von GitHub"
    $Antwort = Read-Host "heruntergeladen werden? [J/N] (Enter = Ja)"
    if ($Antwort -eq "" -or $Antwort -match "^[jJyY]") {
        $CompDir = Join-Path $RootDir "Compiler"
        New-Item -ItemType Directory -Force -Path $CompDir | Out-Null
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Write-Host "Suche neueste 32-Bit-Version (i686) von skeeto/w64devkit auf GitHub..." -ForegroundColor Cyan
        $ApiUrl = "https://api.github.com/repos/skeeto/w64devkit/releases/latest"
        $Rel = Invoke-RestMethod -Uri $ApiUrl -Headers @{ "User-Agent" = "DSA-SelberBauen" }
        $Asset = $Rel.assets | Where-Object { $_.name -match "w64devkit-(i686|x86)-.*\.(zip|7z\.exe)$" } | Select-Object -First 1
        if (-not $Asset) {
            $Asset = $Rel.assets | Where-Object { $_.name -match "i686.*\.zip$" } | Select-Object -First 1
        }
        if (-not $Asset) {
            Write-Host "FEHLER: Konnte w64devkit-i686 Download-URL nicht automatisch ermitteln." -ForegroundColor Red
            Write-Host "Bitte w64devkit-i686.zip von https://github.com/skeeto/w64devkit/releases herunterladen"
            Write-Host "und nach $CompDir\w64devkit entpacken."
            exit 1
        }
        $DlFile = Join-Path $CompDir $Asset.name
        Write-Host "Lade $($Asset.name) herunter ($([Math]::Round($Asset.size / 1MB, 1)) MB)..." -ForegroundColor Cyan
        Invoke-WebRequest -Uri $Asset.browser_download_url -OutFile $DlFile -UseBasicParsing
        if ($Asset.name -match "\.zip$") {
            Write-Host "Entpacke nach $CompDir ..." -ForegroundColor Cyan
            Expand-Archive -Path $DlFile -DestinationPath $CompDir -Force
        } else {
            Write-Host "Entpacke selbstextrahierendes Archiv nach $CompDir ..." -ForegroundColor Cyan
            & $DlFile -o"$CompDir" -y | Out-Null
        }
        Remove-Item $DlFile -Force -ErrorAction SilentlyContinue
        $CandidateBin = Join-Path $CompDir "w64devkit\bin"
        if (Test-Mingw32 $CandidateBin) {
            $MingwBin = $CandidateBin
        } else {
            throw "Compiler wurde entpackt, aber $CandidateBin\gcc.exe konnte nicht gestartet werden."
        }
    } else {
        Write-Host "Abbruch. Du kannst jederzeit MinGW-w64 / w64devkit (i686) nach Compiler\w64devkit legen."
        exit 1
    }
}

Write-Host "Verwende Compiler: $MingwBin" -ForegroundColor Green
$env:PATH = "$MingwBin;$env:PATH"

$BuildRoot = Join-Path $env:TEMP "schick_selber_bauen"
New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null

function Build-Stufe([int]$FixBugs, [string]$ExeName, [bool]$CopyDefault) {
    $ObjDir = Join-Path $BuildRoot "obj_fix$FixBugs"
    New-Item -ItemType Directory -Force -Path $ObjDir | Out-Null

    Write-Host ""
    Write-Host "[Windows-Port] Kompiliere 112 Spiel-Module fuer FIX_BUGS=$FixBugs ($ExeName)..." -ForegroundColor Cyan
    $CFiles = Get-ChildItem -Path (Join-Path $NativDir "src\*.c") | Sort-Object Name | ForEach-Object { $_.FullName.Replace('\', '/') }
    $CRsp = Join-Path $ObjDir "c_files.rsp"
    [System.IO.File]::WriteAllLines($CRsp, $CFiles)

    $SrcInc  = (Join-Path $NativDir "src").Replace('\', '/')
    $AilInc  = (Join-Path $NativDir "src\AIL").Replace('\', '/')
    $PlatInc = (Join-Path $NativDir "platform").Replace('\', '/')
    $SdlInc  = (Join-Path $NativDir "third_party\SDL2\include").Replace('\', '/')
    $OplInc  = (Join-Path $NativDir "third_party\dbopl").Replace('\', '/')

    Push-Location $ObjDir
    try {
        & gcc.exe -m32 -std=gnu99 -O2 -fno-strict-overflow -fno-aggressive-loop-optimizations -fpack-struct=1 -fsigned-char -w `
            "-DFIX_BUGS=$FixBugs" "-DSKIP_COPY_PROTECTION=1" `
            "-I$SrcInc" "-I$AilInc" "-I$PlatInc" "-I$SdlInc" -c "@c_files.rsp"
        if ($LASTEXITCODE -ne 0) { throw "Kompilieren der Spiel-Module fehlgeschlagen." }

        Write-Host "      Kompiliere Plattform- und Sound-Module (SDL2/Win32, OPL3, Vorbis, Intro)..."
        & gcc.exe -m32 -std=gnu99 -O2 "-I$SrcInc" "-I$PlatInc" "-I$SdlInc" -c (Join-Path $NativDir "platform\vgalib.c") -o "vgalib.o"
        if ($LASTEXITCODE -ne 0) { throw "vgalib.c fehlgeschlagen." }

        & gcc.exe -m32 -std=gnu99 -O2 "-I$SdlInc" -c (Join-Path $NativDir "third_party\SDL2\sdl2_win32_provider.c") -o "sdl2_win32_provider.o"
        if ($LASTEXITCODE -ne 0) { throw "sdl2_win32_provider.c fehlgeschlagen." }

        & gcc.exe -m32 -std=gnu99 -O2 -w -c (Join-Path $NativDir "third_party\stb_vorbis\stb_vorbis.c") -o "stb_vorbis.o"
        if ($LASTEXITCODE -ne 0) { throw "stb_vorbis.c fehlgeschlagen." }

        & g++.exe -m32 -O2 "-I$OplInc" -c (Join-Path $NativDir "third_party\dbopl\dbopl.cpp") -o "dbopl.o"
        if ($LASTEXITCODE -ne 0) { throw "dbopl.cpp fehlgeschlagen." }

        & g++.exe -m32 -O2 "-I$PlatInc" "-I$SdlInc" "-I$OplInc" -c (Join-Path $NativDir "platform\dsa_platform_sdl2.cpp") -o "dsa_platform_sdl2.o"
        if ($LASTEXITCODE -ne 0) { throw "dsa_platform_sdl2.cpp fehlgeschlagen." }

        & g++.exe -m32 -O2 "-I$PlatInc" -c (Join-Path $NativDir "platform\dsa_intro.cpp") -o "dsa_intro.o"
        if ($LASTEXITCODE -ne 0) { throw "dsa_intro.cpp fehlgeschlagen." }

        $ObjList = Get-ChildItem -Path (Join-Path $NativDir "src\*.c") | Sort-Object Name | ForEach-Object { "$($_.BaseName).o" }
        $ObjList += @("vgalib.o", "stb_vorbis.o", "dbopl.o", "dsa_platform_sdl2.o", "dsa_intro.o", "sdl2_win32_provider.o")
        $ObjRsp = Join-Path $ObjDir "objs.rsp"
        [System.IO.File]::WriteAllLines($ObjRsp, $ObjList)

        $OutExe = Join-Path $NativDir $ExeName
        & g++.exe -m32 -mwindows -o $OutExe "@objs.rsp" -lgdi32 -luser32 -lwinmm
        if ($LASTEXITCODE -ne 0) { throw "Linken von $ExeName fehlgeschlagen." }
        Write-Host "      Fertig: $OutExe" -ForegroundColor Green

        if ($RootDir -ne $NativDir) {
            Copy-Item $OutExe (Join-Path $RootDir $ExeName) -Force
        }
        if ($CopyDefault) {
            Copy-Item $OutExe (Join-Path $NativDir "SCHICK_PORT.EXE") -Force
            if ($RootDir -ne $NativDir) {
                Copy-Item $OutExe (Join-Path $RootDir "SCHICK_PORT.EXE") -Force
            }
        }
    } finally {
        Pop-Location
    }
}

if ($Stufe -eq "alle_stufen") {
    Build-Stufe 0 "SCHICK_PORT_ORIG.EXE" $false
    Build-Stufe 1 "SCHICK_PORT_WICHTIG.EXE" $false
    Build-Stufe 2 "SCHICK_PORT_ALLES.EXE" $true
} elseif ($Stufe -eq "wichtig") {
    Build-Stufe 1 "SCHICK_PORT_WICHTIG.EXE" $true
} elseif ($Stufe -eq "original") {
    Build-Stufe 0 "SCHICK_PORT_ORIG.EXE" $true
} else {
    Build-Stufe 2 "SCHICK_PORT_ALLES.EXE" $true
}

# 3. Nordlandtrilogie-Starter (NORDLANDTRILOGIE.EXE) aus Starter\src\starter_main.cpp bauen
$StarterCpp = Join-Path $RootDir "Starter\src\starter_main.cpp"
if (Test-Path $StarterCpp) {
    Write-Host ""
    Write-Host "[Starter] Kompiliere NORDLANDTRILOGIE.EXE aus Starter\src\starter_main.cpp..." -ForegroundColor Cyan
    $StarterExe = Join-Path $RootDir "NORDLANDTRILOGIE.EXE"
    & g++.exe -m32 -std=c++17 -O2 -Wall -Wextra -mwindows -static -static-libgcc -static-libstdc++ `
        $StarterCpp -o $StarterExe -lgdi32 -luser32 -lshell32 -lole32 -luuid -lcomdlg32
    if ($LASTEXITCODE -eq 0) {
        if ($RootDir -ne $NativDir) {
            Copy-Item $StarterExe (Join-Path $NativDir "NORDLANDTRILOGIE.EXE") -Force
        }
        Write-Host "      Fertig: $StarterExe" -ForegroundColor Green
    }
}

# Start-Batchdateien ins Hauptverzeichnis kopieren, damit man auch ohne Starter direkt per .bat starten kann
if ($RootDir -ne $NativDir) {
    foreach ($bat in Get-ChildItem -Path (Join-Path $NativDir "Schicksalsklinge_starten_*.bat") -ErrorAction SilentlyContinue) {
        Copy-Item $bat.FullName (Join-Path $RootDir $bat.Name) -Force
    }
}

Write-Host ""
Write-Host "=================================================================" -ForegroundColor Green
Write-Host " FERTIG! Alle EXEs wurden frisch aus dem Quelltext gebaut." -ForegroundColor Green
Write-Host " Du kannst jetzt starten:" -ForegroundColor Green
Write-Host "   - NORDLANDTRILOGIE.EXE (Auswahl-Starter fuer Windows & DOS)" -ForegroundColor Green
Write-Host "   - oder direkt per .bat (ganz ohne Starter):" -ForegroundColor Green
Write-Host "     Schicksalsklinge_starten_WICHTIGE_FIXES.bat" -ForegroundColor Green
Write-Host "     Schicksalsklinge_starten_ALLE_FIXES.bat" -ForegroundColor Green
Write-Host "     BLADEM_FIX_WICHTIG_STARTEN.bat (DOSBox)" -ForegroundColor Green
Write-Host "=================================================================" -ForegroundColor Green
if (-not $OhnePause) {
    Write-Host ""
    Read-Host "Eingabetaste zum Schliessen"
}
