# probenlog_fenster.ps1 - zeigt das Probenlog der Bugfix-Fassung live als Klartext (Konsolenfenster neben dem Spiel).
# Aufruf ueber die *_MIT_PROBENLOG.bat. Das Spiel schreibt nur Zahlen nach PROBEN.CSV, dieses Fenster liest mit.
# Laeuft mit Windows PowerShell 5.1 und PowerShell 7.
param([Parameter(Mandatory=$true)][string]$Datei, [string]$Spiel = 'Schicksalsklinge')
$ErrorActionPreference = 'Stop'
try { $Host.UI.RawUI.WindowTitle = "Probenlog $Spiel" } catch {}
$EIG = @('MU', 'KL', 'CH', 'FF', 'GE', 'IN', 'KK', 'AG', 'HA', 'RA', 'GG', 'TA', 'NG', 'JZ')
$TALENT = @('Waffenlos', 'Hiebwaffen', 'Stichwaffen', 'Schwerter', 'Äxte', 'Speere', 'Zweihänder', 'Schusswaffen', 'Wurfwaffen', 'Akrobatik', 'Klettern', 'Körperbeherrschung', 'Reiten', 'Schleichen', 'Schwimmen', 'Selbstbeherrschung', 'Tanzen', 'Verstecken', 'Zechen', 'Bekehren', 'Betören', 'Feilschen', 'Gassenwissen', 'Lügen', 'Menschenkenntnis', 'Schätzen', 'Fährtensuchen', 'Fesseln', 'Orientierung', 'Pflanzenkunde', 'Tierkunde', 'Wildnisleben', 'Alchimie', 'Alte Sprachen', 'Geographie', 'Geschichte', 'Götter/Kulte', 'Kriegskunst', 'Lesen', 'Magiekunde', 'Sprachen', 'Abrichten', 'Fahrzeuge', 'Falschspiel', 'Heilen Gift', 'Heilen Krankheiten', 'Heilen Wunden', 'Musizieren', 'Schlösser', 'Taschendiebstahl', 'Gefahrensinn', 'Sinnesschärfe')
$ZAUBER = @('-', 'Beherrschung brechen', 'Destructibo', 'Gardianum', 'Illusionen zerstören', 'Verwandlung beenden', 'Band und Fessel', 'Bannbaladin', 'Böser Blick', 'Große Gier', 'Große Verwirrung', 'Herr der Tiere', 'Horriphobus', 'Magischer Raub', 'Respondami', 'Sanftmut', 'Somnigravis', 'Zwingtanz', 'Furor Blut', 'Geister bannen', 'Geister rufen', 'Heptagon', 'Krähenruf', 'Skelettarius', 'Elementare', 'Nihilatio', 'Solidirid', 'Axxeleratus', 'Foramen', 'Motoricus', 'Spurlos', 'Transversalis', 'Über Eis', 'Balsam', 'Hexenspeichel', 'Klarum Purum', 'Ruhe Körper', 'Tiere heilen', 'Adleraug', 'Analüs', 'Eigenschaften', 'Exposami', 'Odem Arcanum', 'Penetrizzel', 'Sensibar', 'Chamaelioni', 'Duplicatus', 'Harmlos', 'Hexenknoten', 'Blitz', 'Ecliptifactus', 'Eisenrost', 'Fulminictus', 'Ignifaxius', 'Plumbumbarum', 'Radau', 'Saft, Kraft, Monstermacht', 'Scharfes Auge', 'Hexenblick', 'Nekropathia', 'Adler, Wolf', 'Arcano', 'Armatrutz', 'CH steigern', 'Feuerbann', 'FF steigern', 'GE steigern', 'IN steigern', 'KK steigern', 'KL steigern', 'MU steigern', 'Mutabili', 'Paralü', 'Salander', 'See und Fluss', 'Visibili', 'Abvenenum', 'Aeolitus', 'Brenne', 'Claudibus', 'Dunkelheit', 'Erstarre', 'Flim Flam', 'Schmelze', 'Silentium', 'Sturmgebrüll')
$GEGNER = @('', 'Oger', 'Ork', 'Goblin', 'Heshthot', 'Säbelzahntiger', 'Skelett', 'Waldschrat', 'Mumie', 'Harpyie', 'Waldspinne', 'Großer Schröter', 'Pirat', 'Steuermann', 'Kapitän', 'Streuner', 'Zombie', 'Heshthot', 'Druide', 'Druide', 'Thorwaler', 'Thorwaler', 'Thorwaler', 'Thorwaler', 'Goblin', 'Goblin', 'Ork', 'Ork', 'Pirat', 'Pirat', 'Streuner', 'Streuner', 'Streuner', 'Skelett', 'Folterknecht', 'Grimwolf', 'Grimwolf', 'Höhlenspinne', 'Höhlenspinne', 'Höhlenspinne', 'Krieger', 'Stadtgardist', 'Stadtgardist', 'Stadtgardist', 'Kriegsveteran', 'Mumie', 'Magier', 'Magier', 'Zauberer', 'Zauberer', 'Zwerg', 'Lindwurm', 'Höhlendrache', 'Kultist', 'Kultist', 'Kultist', 'Kultist', 'Priester', 'Priester', 'Hohepriester', 'Estorik', 'Piratenboss', 'Pirat', 'Magier', 'Zwergenveteran', 'Räuber', 'Räuber', 'Räuber', 'Räuber', 'Räuber', 'Gorah', 'Steppenhund', 'Orkchampion', 'Pirat', 'Stadtgardist', 'Waldlöwe', 'Spinnendämon', 'Heshthot')
$MONAT = @('', 'Praios', 'Rondra', 'Efferd', 'Travia', 'Boron', 'Hesinde', 'Firun', 'Tsa', 'Phex', 'Peraine', 'Ingerimm', 'Rahja', 'Namenlose Tage')
function Nm($liste, [int]$i) { if ($i -ge 0 -and $i -lt $liste.Count) { $liste[$i] } else { "Nr. $i" } }
function Gegner([int]$i) { if ($i -gt 0 -and $i -lt $GEGNER.Count) { $GEGNER[$i] } else { "Gegner Nr. $i" } }
function Zuschlag([int]$z) { if ($z -gt 0) { " erschwert um $z" } elseif ($z -lt 0) { " erleichtert um $(-$z)" } else { '' } }
function Erg3([int]$r) { if ($r -eq -99) { 'PATZER (zweimal 20)' } elseif ($r -gt 0) { "gelungen, $($r - 1) Punkte übrig" } else { "misslungen, $(1 - $r) Punkte zu wenig" } }
$script:vor = $null
function Zeile([string]$roh) {
    $f = $roh.Trim().Split(';')
    if ($f.Count -lt 6) { return $null }
    $n = @(); foreach ($x in $f) { $n += 0 }
    try { for ($i = 0; $i -lt 5; $i++) { $n[$i] = [int]$f[$i] } } catch { return $null }
    $art = $n[0]; $name = $f[5]
    $w = @(); for ($i = 6; $i -lt $f.Count; $i++) { if ($f[$i] -ne '') { $w += [int]$f[$i] } }
    $min = $n[4]
    $zeit = ('{0,2}. {1} {2} Hal, {3:00}:{4:00}' -f $n[1], (Nm $MONAT $n[2]), $n[3], [math]::Floor($min / 60), ($min % 60))
    $t = $null; $farbe = 'Gray'
    if ($art -eq 1 -and $w.Count -ge 5) {
        $e = Nm $EIG $w[0]; $r = $w[4]
        $t = "${name}: $e-Probe$(Zuschlag $w[2]): W20 = $($w[3]) gegen $e $($w[1])"
        if ($r -eq -99) { $t += ' → PATZER (20)'; $farbe = 'Magenta' } elseif ($r -gt 0) { $t += " → gelungen ($($r - 1) übrig)"; $farbe = 'Green' } else { $t += ' → misslungen'; $farbe = 'Red' }
    } elseif ($art -eq 3 -and $w.Count -ge 11) {
        $eig = ((Nm $EIG $w[0]), (Nm $EIG $w[1]), (Nm $EIG $w[2])) -join '/'
        $eig += " ($($w[3])/$($w[4])/$($w[5]))"
        $wue = (@($w[7], $w[8], $w[9]) | Where-Object { $_ -ne 0 }) -join ', '
        $r = $w[10]
        if ($script:vor -and $script:vor[0] -eq $name) {
            $zs = Zuschlag $w[6]; if (-not $zs) { $zs = ' ohne Zuschlag' }
            $t = "$($script:vor[1]) → Probe auf $eig, gesamt${zs}: $wue → $(Erg3 $r)"
        } else { $t = "${name}: Probe auf $eig$(Zuschlag $w[6]): $wue → $(Erg3 $r)" }
        $script:vor = $null
        if ($r -eq -99) { $farbe = 'Magenta' } elseif ($r -gt 0) { $farbe = 'Green' } else { $farbe = 'Red' }
    } elseif ($art -eq 4 -and $w.Count -ge 6) {
        $ta = Nm $TALENT $w[0]
        if ($w[3] -ne 0) {
            $script:vor = $null; $r = $w[5]
            $t = "${name}: $ta$(Zuschlag $w[2]): W20 = $($w[4]) gegen Zielwert $($w[1])"
            if ($r -eq -1) { $t += ' → PATZER (20)'; $farbe = 'Magenta' } elseif ($r -eq 99) { $t += ' → Glückswurf (1)'; $farbe = 'Green' } elseif ($r -gt 0) { $t += " → getroffen ($($r - 1) übrig)"; $farbe = 'Green' } else { $t += ' → daneben'; $farbe = 'Red' }
        } else { $script:vor = @($name, "${name}: $ta (Talentwert $($w[1]))$(Zuschlag $w[2])"); return $null }
    } elseif ($art -eq 5 -and $w.Count -ge 3) {
        $script:vor = @($name, "$name zaubert $(Nm $ZAUBER $w[0]) (Zauberfertigkeit $($w[1]))$(Zuschlag $w[2])"); return $null
    } elseif (($art -eq 6 -or $art -eq 7) -and $w.Count -ge 4) {
        $script:vor = $null
        $wert = $w[0]; $wurf = $w[1]; $mon = $w[2]; $rtg = $w[3]
        $kurz = if ($art -eq 6) { 'AT' } else { 'PA' }
        $ok = ($wurf -le $wert -and $wurf -ne 20)
        if ($art -eq 6) { $erf = if ($ok) { 'trifft' } else { 'verfehlt' } } else { $erf = if ($ok) { 'pariert' } else { 'pariert nicht' } }
        if ($wurf -eq 20) { $erf += ' (20: Patzer möglich)' }
        if ($art -eq 6) {
            if ($rtg -eq 0) { $wer = if ($mon -lt 0) { "$name attackiert einen Helden" } else { "$name attackiert $(Gegner $mon)" }; $farbe = if ($ok) { 'Green' } else { 'Red' } }
            elseif ($rtg -eq 1) { $wer = "$(Gegner $mon) attackiert $name"; $farbe = if ($ok) { 'Red' } else { 'Green' } }
            else { $wer = "$(Gegner $mon) attackiert einen anderen Gegner" }
        } else {
            if ($rtg -eq 0) { $wer = if ($mon -lt 0) { "$name pariert" } else { "$(Gegner $mon) pariert gegen $name" }; $farbe = if ($ok) { 'Red' } else { 'Green' } }
            elseif ($rtg -eq 1) { $wer = "$name pariert gegen $(Gegner $mon)"; $farbe = if ($ok) { 'Green' } else { 'Red' } }
            else { $wer = "Ein Gegner pariert gegen $(Gegner $mon)" }
        }
        $t = "${wer}: W20 = $wurf gegen $kurz $wert → $erf"
    } elseif ($art -eq 8 -and $w.Count -ge 5) {
        $script:vor = $null; $typ = $w[0]; $nr = $w[1]; $alt = $w[2]; $farbe = 'Cyan'
        if ($typ -eq 1 -or $typ -eq 2) {
            if ($typ -eq 1) { $was = "$(Nm $TALENT $nr) (TaW $alt)" } else { $was = "$(Nm $ZAUBER $nr) (ZfW $alt)" }
            $ber = if ($alt -ge 11) { '3–18' } else { '2–12' }
            $t = "${name}: Stufenanstieg, ${was}: Steigerungswurf $($w[3]) ($ber), muss über $alt liegen"
            if ($w[4]) { $t += " → gelungen, jetzt $($alt + 1)"; $farbe = 'Green' } else { $t += ' → misslungen'; $farbe = 'Red' }
        } elseif ($typ -eq 3) { $t = "${name}: Stufenanstieg, $(Nm $EIG $nr) +1 ($alt → $($alt + 1))" }
        elseif ($typ -eq 4 -and $w.Count -ge 7) {
            $e = Nm $EIG $nr
            $t = "${name}: Stufenanstieg, $e senken ($e $alt): W20 = $($w[3]), $($w[4]), $($w[5]), einer muss höchstens $alt sein"
            if ($w[6]) { $t += " → gelungen, jetzt $($alt - 1)"; $farbe = 'Green' } else { $t += ' → misslungen'; $farbe = 'Red' }
        } elseif ($typ -eq 5) {
            if ($w[4] -gt $w[3]) { $t = "${name}: Stufenanstieg, W6 = $($w[3]) + 2 → $($w[4]) Punkte für LE und AE (LE vorher $alt)" }
            else { $t = "${name}: Stufenanstieg, W6 = $($w[3]) → LE $alt → $($alt + $w[4])" }
        } elseif ($typ -eq 6) { $t = "${name}: Stufenanstieg, 10 Zaubersteigerungen gegen AE getauscht: Wurf $($w[3]) (3–8) → AE $alt → $($alt + $w[4])" }
        else { $t = "${name}: Stufenanstieg (Typ $typ)" }
    } else { $t = "${name}: unbekannte Zeile (Art $art)" }
    if ($t) { return @("$zeit | $t", $farbe) }
    return $null
}
Write-Host "Probenlog $Spiel - jede Probe erscheint hier, sobald im Spiel gewuerfelt wird."
Write-Host "Gruen = gelungen fuer die Helden, Rot = misslungen, Violett = Patzer, Tuerkis = Stufenanstieg. Fenster schliessen beendet nur das Log."
Write-Host ''
try { $enc = [System.Text.Encoding]::GetEncoding(437) } catch { $enc = [System.Text.Encoding]::GetEncoding(28591) }
while (-not (Test-Path -LiteralPath $Datei)) { Start-Sleep -Milliseconds 500 }
$fs = [System.IO.File]::Open($Datei, 'Open', 'Read', 'ReadWrite, Delete')
$sr = New-Object System.IO.StreamReader($fs, $enc)
$rest = ''
while ($true) {
    $s = $sr.ReadToEnd()
    if ($s.Length -eq 0) { Start-Sleep -Milliseconds 300; continue }
    $rest += $s
    $teile = $rest -split "`n"
    $rest = $teile[-1]
    for ($i = 0; $i -lt $teile.Count - 1; $i++) {
        $z = Zeile $teile[$i]
        if ($z) { Write-Host $z[0] -ForegroundColor $z[1] }
    }
}
