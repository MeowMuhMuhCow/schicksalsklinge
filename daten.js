/*
 * daten.js - Browser-Spieldatenverwaltung, ZIP-Ein-/Ausgabe (via fflate, MIT)
 *            und WebCrypto-SHA-256-Pruefung fuer DSA 1: Die Schicksalsklinge
 *
 * Lizenz: GNU General Public License Version 3 (GPL v3, siehe COPYING)
 *
 * Quellen & Bibliotheken:
 *   - Konzept der IndexedDB-Spieldatenablage angelehnt an MobileEyes
 *     (https://github.com/Kerygma83/mobile-eyes, Autor: Gero Ohrner, GPL v3)
 *   - fflate v0.8.3 (fflate.min.js, Autor: Arjun Barrett, MIT-Lizenz) fuer ZIP-Entpacken & ZIP-Erstellen
 *   - Web Crypto API (crypto.subtle.digest('SHA-256')) fuer die Versionskennung von SCHICKM.EXE (V3.02de)
 *   - IndexedDB API fuer die lokale Browser-Ablage der Spieldateien
 *   - File and Directory Entries API (webkitGetAsEntry) fuer Ordner-Drag-and-Drop
 */

'use strict';

const SCHICK_IDB_NAME = 'schicksalsklinge-spieldaten';
const SCHICK_IDB_VERSION = 1;
const SCHICK_IDB_TABELLE = 'dateien';

/* SHA-256 der deutschen SCHICKM.EXE V3.02de (GOG-/CD-Fassung, 556.208 Bytes) */
const SCHICKM_V302DE_SHA256 = '05be9c9a3da5798700048e2df6b0cb682f6b1f6e9133ffd1a5038451b03cb627';

/* Pflichtdateien gemaess Starter/spiele.json */
const PFLICHT_KERNDATEIEN = ['SCHICK.DAT', 'DSAGEN.DAT', 'SCHICKM.EXE'];
const PFLICHT_STARTDATEIEN = [
    'SCHICK.DAT', 'DSAGEN.DAT', 'SCHICKM.EXE',
    'FONT.NVF', 'MOUSE.NVF', 'POPUP.NVF'
];

const REGEX_SPIELDATEI = /^(SCHICK\.DAT|SCHICKM\.EXE|DSAGEN\.DAT|FONT\.NVF|MOUSE\.NVF|POPUP\.NVF|SOUND\.(CFG|LTX|NVF)|INTRODAT\.(DIR|SND|VOL)|INTRO_E\.OVL|GAMES\.NAM|HIMMEL\.AN|APFREI\.AN|[A-Z0-9_]+\.(CHR|ADV|GAM))$/i;
const REGEX_MUSIKDATEI = /^TRACK(\d\d)\.OGG$/i;
const REGEX_IGNORIERTES_VERZ = /(^|[\\/])(CTEMP|TEMP|DOSBOX|DOCUMENTATION|PROBENLOGS|CLOUD_SAVES)([\\/]|$)/i;

/* ---------- 1. Hilfsfunktionen fuer Dateinamen & SHA-256 (WebCrypto) ---------- */

function dateiBasisname(pfad) {
    return String(pfad || '').split(/[\\/]/).pop() || '';
}

function ermittleZielPfad(quellPfad, inklusiveMusik = true) {
    if (REGEX_IGNORIERTES_VERZ.test(String(quellPfad || ''))) {
        return null;
    }
    const basis = dateiBasisname(quellPfad).toUpperCase();
    if (!basis) return null;
    if (REGEX_SPIELDATEI.test(basis)) return basis;
    if (inklusiveMusik && REGEX_MUSIKDATEI.test(basis)) return 'MUSIC/' + basis;
    return null;
}

async function berechneSha256Hex(byteArray) {
    const u8 = byteArray instanceof Uint8Array ? byteArray : new Uint8Array(byteArray);
    if (typeof crypto !== 'undefined' && crypto.subtle && typeof crypto.subtle.digest === 'function') {
        const digestBuf = await crypto.subtle.digest('SHA-256', u8);
        const digestBytes = new Uint8Array(digestBuf);
        let hex = '';
        for (let i = 0; i < digestBytes.length; i++) {
            hex += digestBytes[i].toString(16).padStart(2, '0');
        }
        return hex;
    }
    return '';
}

/* ---------- 2. ZIP-Verarbeitung ueber fflate (MIT-Lizenz) ---------- */

function zipErzeugen(eintraegeListe) {
    if (typeof window.fflate === 'undefined' || typeof window.fflate.zipSync !== 'function') {
        throw new Error('ZIP-Bibliothek (fflate) ist nicht geladen.');
    }
    const zipObjekt = {};
    for (const eintrag of eintraegeListe) {
        const bytes = eintrag.daten instanceof Uint8Array
            ? eintrag.daten
            : new Uint8Array(eintrag.daten);
        zipObjekt[eintrag.name] = [bytes, { level: 0 }];
    }
    const archivBytes = window.fflate.zipSync(zipObjekt, { level: 0 });
    return new Blob([archivBytes], { type: 'application/zip' });
}

async function zipLesen(blobOderDatei, pfadFilterFn, fortschrittFn) {
    if (typeof window.fflate === 'undefined' || typeof window.fflate.unzipSync !== 'function') {
        throw new Error('ZIP-Bibliothek (fflate) ist nicht geladen.');
    }
    if (fortschrittFn) {
        fortschrittFn('Lese ZIP-Archiv im Browser …');
    }
    const rohPuffer = new Uint8Array(await blobOderDatei.arrayBuffer());
    let entpackt;
    try {
        entpackt = window.fflate.unzipSync(rohPuffer, {
            filter: (dateiInfo) => {
                if (!pfadFilterFn) return true;
                return Boolean(pfadFilterFn(dateiInfo.name));
            }
        });
    } catch (err) {
        throw new Error('Ungültiges oder beschädigtes ZIP-Archiv: ' + (err?.message || String(err)));
    }

    const namen = Object.keys(entpackt);
    const ergebnis = [];
    for (let i = 0; i < namen.length; i++) {
        const pfad = namen[i];
        const inhalt = entpackt[pfad];
        if (!inhalt || (pfad.endsWith('/') && inhalt.byteLength === 0)) continue;
        if (fortschrittFn) {
            fortschrittFn(`Übernehme ${dateiBasisname(pfad)} (${i + 1}/${namen.length}) …`);
        }
        ergebnis.push({
            pfad: pfad,
            daten: new Blob([inhalt])
        });
    }
    return ergebnis;
}

/* ---------- 3. W9-Vorbereitung: GOG-Offline-Installer (setup_*.exe / Inno Setup) ---------- */

const InnosetupEntpackerWasm = {
    istFreigeschaltet() {
        return Boolean(
            window.SCHICK_W9_INSTALLER_FREIGESCHALTET === true &&
            typeof window.innoextractWasmEntpacken === 'function'
        );
    },
    istGogInstallerName(dateiName) {
        return /^setup_.*\.(exe|bin)$/i.test(dateiBasisname(dateiName));
    },
    async entpackeInstaller(dateiBlob, fortschrittFn) {
        if (!this.istFreigeschaltet()) {
            throw new Error(
                'GOG-Offline-Installer (setup_*.exe) erkannt. ' +
                'Die direkte Inno-Setup-Entpackung im Browser (W9) ist vorbereitet, ' +
                'aber mangels lokalem Test-Installer noch nicht freigeschaltet – ' +
                'bitte den installierten GOG-Spielordner oder eine ZIP-Datei davon wählen.'
            );
        }
        return await window.innoextractWasmEntpacken(dateiBlob, fortschrittFn);
    }
};

/* ---------- 4. Ordner-Drag-and-Drop (webkitGetAsEntry) ---------- */

async function durchlaufeVerzeichnisEintrag(fsEntry, stammPfad = '') {
    if (!fsEntry) return [];
    if (fsEntry.isFile) {
        return new Promise((resolve) => {
            fsEntry.file(
                (fileObj) => resolve([{ pfad: stammPfad + fileObj.name, daten: fileObj }]),
                () => resolve([])
            );
        });
    }
    if (fsEntry.isDirectory) {
        if (/^(DOSBOX|DOCUMENTATION|PROBENLOGS|CTEMP|TEMP|CLOUD_SAVES)$/i.test(fsEntry.name || '')) {
            return [];
        }
        const dirReader = fsEntry.createReader();
        const unterEintraege = [];
        while (true) {
            const teilListe = await new Promise((resolve) => {
                dirReader.readEntries(
                    (entries) => resolve(entries || []),
                    () => resolve([])
                );
            });
            if (!teilListe.length) break;
            unterEintraege.push(...teilListe);
        }
        const gesammelt = [];
        for (const kind of unterEintraege) {
            const teil = await durchlaufeVerzeichnisEintrag(kind, stammPfad + fsEntry.name + '/');
            gesammelt.push(...teil);
        }
        return gesammelt;
    }
    return [];
}

async function extrahiereDropDateien(dataTransfer) {
    if (dataTransfer?.items && dataTransfer.items.length > 0) {
        const wurzelEintraege = [];
        for (let i = 0; i < dataTransfer.items.length; i++) {
            const item = dataTransfer.items[i];
            if (typeof item.webkitGetAsEntry === 'function') {
                const entry = item.webkitGetAsEntry();
                if (entry) wurzelEintraege.push(entry);
            }
        }
        if (wurzelEintraege.some((e) => e.isDirectory)) {
            const alleDateien = [];
            for (const entry of wurzelEintraege) {
                const teil = await durchlaufeVerzeichnisEintrag(entry, '');
                alleDateien.push(...teil);
            }
            return alleDateien;
        }
    }
    return Array.from(dataTransfer?.files || []).map((fileObj) => ({
        pfad: fileObj.webkitRelativePath || fileObj.name,
        daten: fileObj
    }));
}

/* ---------- 5. IndexedDB-Spieldatenspeicher (GPL v3) ---------- */

function spieldatenEinrichten() {
    let offeneDbPromise = null;

    function holeDatenbank() {
        if (!offeneDbPromise) {
            offeneDbPromise = new Promise((resolve, reject) => {
                const anfrage = indexedDB.open(SCHICK_IDB_NAME, SCHICK_IDB_VERSION);
                anfrage.onupgradeneeded = () => {
                    const db = anfrage.result;
                    if (!db.objectStoreNames.contains(SCHICK_IDB_TABELLE)) {
                        db.createObjectStore(SCHICK_IDB_TABELLE);
                    }
                };
                anfrage.onsuccess = () => resolve(anfrage.result);
                anfrage.onerror = () => reject(anfrage.error || new Error('IndexedDB konnte nicht geöffnet werden.'));
            });
        }
        return offeneDbPromise;
    }

    async function fuehreTransaktionAus(modus, aktionFn) {
        const db = await holeDatenbank();
        return new Promise((resolve, reject) => {
            const transaktion = db.transaction(SCHICK_IDB_TABELLE, modus);
            const tabelle = transaktion.objectStore(SCHICK_IDB_TABELLE);
            const anfrageErgebnis = aktionFn(tabelle);
            transaktion.oncomplete = () => resolve(anfrageErgebnis?.result ?? anfrageErgebnis);
            transaktion.onerror = transaktion.onabort = () =>
                reject(transaktion.error || new Error('IndexedDB-Zugriff fehlgeschlagen.'));
        });
    }

    async function listeGespeicherteDateinamen() {
        return await fuehreTransaktionAus('readonly', (tabelle) => tabelle.getAllKeys());
    }

    async function ladeAlleSpieldaten() {
        const schluesselListe = await listeGespeicherteDateinamen();
        const dateienMap = new Map();
        await fuehreTransaktionAus('readonly', (tabelle) => {
            for (const schluessel of schluesselListe) {
                const req = tabelle.get(schluessel);
                req.onsuccess = () => {
                    if (req.result) {
                        dateienMap.set(schluessel, new Uint8Array(req.result));
                    }
                };
            }
        });
        return dateienMap;
    }

    async function loeschen() {
        await fuehreTransaktionAus('readwrite', (tabelle) => tabelle.clear());
    }

    async function istVollstaendig() {
        try {
            const vorhandene = await listeGespeicherteDateinamen();
            return PFLICHT_STARTDATEIEN.every((pflicht) => vorhandene.includes(pflicht));
        } catch (_) {
            return false;
        }
    }

    async function einlesen(eingabeListe, mitMusik = true, fortschrittFn = null) {
        const einzelDateien = [];

        for (const element of eingabeListe) {
            const blobObj = element.daten || element;
            const dateiName = element.pfad || blobObj.name || '';
            if (InnosetupEntpackerWasm.istGogInstallerName(dateiName)) {
                const ausInstaller = await InnosetupEntpackerWasm.entpackeInstaller(blobObj, fortschrittFn);
                einzelDateien.push(...ausInstaller);
            } else if (/\.zip$/i.test(dateiName)) {
                const ausZip = await zipLesen(
                    blobObj,
                    (pfad) => ermittleZielPfad(pfad, mitMusik) !== null,
                    fortschrittFn
                );
                einzelDateien.push(...ausZip);
            } else {
                einzelDateien.push(
                    element.daten
                        ? element
                        : { pfad: element.webkitRelativePath || element.name, daten: element }
                );
            }
        }

        /* Dateien aus dem Hauptverzeichnis gegenueber Unterordnern bevorzugen */
        einzelDateien.sort(
            (a, b) => String(a.pfad || '').split(/[\\/]/).length - String(b.pfad || '').split(/[\\/]/).length
        );

        const enthaeltEnglischeBlade = einzelDateien.some(
            (e) => /^BLADEM?\.(DAT|EXE)$/i.test(dateiBasisname(e.pfad))
        );

        const ausgewaehlteBlobs = new Map();
        for (const eintrag of einzelDateien) {
            const zielPfad = ermittleZielPfad(eintrag.pfad, mitMusik);
            if (zielPfad && !ausgewaehlteBlobs.has(zielPfad)) {
                const blob = eintrag.daten instanceof Blob ? eintrag.daten : new Blob([eintrag.daten]);
                ausgewaehlteBlobs.set(zielPfad, blob);
            }
        }

        const fehlendeKern = PFLICHT_KERNDATEIEN.filter((name) => !ausgewaehlteBlobs.has(name));
        if (fehlendeKern.length > 0) {
            if (enthaeltEnglischeBlade && !ausgewaehlteBlobs.has('SCHICK.DAT')) {
                throw new Error(
                    'Englische Fassung („Blade of Destiny“ / BLADE.DAT) erkannt. ' +
                    'Benötigt wird die deutsche GOG-/CD-Fassung mit SCHICK.DAT.'
                );
            }
            const fehlendeAlle = PFLICHT_STARTDATEIEN.filter((name) => !ausgewaehlteBlobs.has(name));
            throw new Error(
                `Kein vollständiger Schicksalsklinge-Spielordner erkannt. Es fehlen: ${fehlendeAlle.join(', ')}.`
            );
        }

        const fehlendeStart = PFLICHT_STARTDATEIEN.filter((name) => !ausgewaehlteBlobs.has(name));
        if (fehlendeStart.length > 0) {
            throw new Error(`Unvollständiger Spielordner – es fehlen: ${fehlendeStart.join(', ')}.`);
        }

        if (ausgewaehlteBlobs.has('SCHICKM.EXE')) {
            if (fortschrittFn) {
                fortschrittFn('Prüfe deutsche Fassung V3.02de (SCHICKM.EXE per SHA-256) …');
            }
            const exeBytes = new Uint8Array(await ausgewaehlteBlobs.get('SCHICKM.EXE').arrayBuffer());
            if (exeBytes.byteLength !== 556208) {
                throw new Error(
                    `Ungültige Dateigröße von SCHICKM.EXE (${exeBytes.byteLength} Bytes statt 556.208 Bytes) – deutsche GOG-Fassung benötigt (deutsche GOG-Fassung benoetigt, V3.02de, MD5 4ce035f95ead01a8b2553b4487544e5d).`
                );
            }
            const sha256 = await berechneSha256Hex(exeBytes);
            if (sha256 && sha256 !== SCHICKM_V302DE_SHA256) {
                throw new Error(
                    `Ungültige Prüfsumme von SCHICKM.EXE – deutsche GOG-Fassung benötigt (deutsche GOG-Fassung benoetigt, V3.02de, 556.208 Bytes, MD5 4ce035f95ead01a8b2553b4487544e5d).`
                );
            }
        }

        if (fortschrittFn) {
            fortschrittFn(`Speichere ${ausgewaehlteBlobs.size} Spieldateien lokal im Browser (IndexedDB) …`);
        }
        const speicherPaare = [];
        for (const [schluessel, blob] of ausgewaehlteBlobs.entries()) {
            speicherPaare.push([schluessel, await blob.arrayBuffer()]);
        }

        await loeschen();
        await fuehreTransaktionAus('readwrite', (tabelle) => {
            for (const [schluessel, puffer] of speicherPaare) {
                tabelle.put(puffer, schluessel);
            }
        });

        try {
            if (navigator.storage && typeof navigator.storage.persist === 'function') {
                navigator.storage.persist().catch(() => {});
            }
        } catch (_) {}

        return ausgewaehlteBlobs.size;
    }

    async function holen(dialogOeffnenFn) {
        if (await istVollstaendig()) {
            return await ladeAlleSpieldaten();
        }
        if (typeof dialogOeffnenFn === 'function') {
            await dialogOeffnenFn();
        }
        return await ladeAlleSpieldaten();
    }

    return {
        holen,
        einlesen,
        istVollstaendig,
        loeschen
    };
}
