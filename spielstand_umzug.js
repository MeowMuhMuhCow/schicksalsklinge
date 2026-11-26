/*
 * spielstand_umzug.js - eigener Spielstand-Speicher je Spiel (Nordlandtrilogie im Browser)
 *
 * Lizenz: GNU General Public License Version 3 (GPL v3, siehe COPYING)
 *
 * Hintergrund: Alle drei Browser-Fassungen laufen unter derselben Adresse und haben ihre Spielstaende
 * bis zum 20.11.2026 in derselben IndexedDB-Datenbank "/spielstand" (IDBFS) abgelegt. Dadurch sahen sich
 * die Spiele gegenseitig ihre Dateien (z. B. LASTGAME von Riva wurde in Sternenschweif geladen).
 * Seitdem hat jedes Spiel seinen eigenen Speicher "/spielstand_<spiel>". Dieses Skript uebernimmt beim
 * ersten Start einmalig die Dateien, die zu diesem Spiel gehoeren, aus dem alten gemeinsamen Speicher.
 * Der alte Speicher bleibt unveraendert erhalten (die anderen Spiele holen sich dort ihre Dateien ab).
 *
 * Zuordnung:
 *  - Dateien mit Kopf "DSA VERSION\0": Byte 13 ist die Spielnummer (1 Schicksalsklinge, 2 Sternenschweif, 3 Riva)
 *  - GAMES.NAM, HIMMEL.AN, APFREI.AN, DREHEN.AUS und Heldendateien *.CHR ohne Kopf (1754 Bytes): Schicksalsklinge
 *  - GAMES.TMP: Sternenschweif, RIVA.CFG und CHARS1.DAT: Riva (CHARS1.DAT vor der Kopfpruefung)
 *  - alles andere bleibt nur im alten Speicher
 */
(function () {
    'use strict';

    var IDBFS_VERSION = 21;          /* wie Emscripten-IDBFS */
    var IDBFS_STORE = 'FILE_DATA';
    var ALT = '/spielstand';
    var NUMMER = { schick: 1, schweif: 2, riva: 3 };

    function spielVonDatei(name, daten) {
        var u = String(name).toUpperCase();
        if (u === 'CHARS1.DAT') return 'riva';                      /* Riva-Datei, Kopf traegt trotzdem eine 2 */
        if (daten && daten.length >= 14) {
            var kopf = 'DSA VERSION';
            var istKopf = true;
            for (var i = 0; i < kopf.length; i++) {
                if (daten[i] !== kopf.charCodeAt(i)) { istKopf = false; break; }
            }
            if (istKopf && daten[11] === 0) {
                var n = daten[13];
                if (n === 1) return 'schick';
                if (n === 2) return 'schweif';
                if (n === 3) return 'riva';
                return null;
            }
        }
        if (u === 'GAMES.NAM' || u === 'HIMMEL.AN' || u === 'APFREI.AN' || u === 'DREHEN.AUS') return 'schick';
        if (/\.CHR$/.test(u) && daten && daten.length === 1754) return 'schick';
        if (u === 'GAMES.TMP') return 'schweif';
        if (u === 'RIVA.CFG') return 'riva';
        return null;
    }

    function oeffne(name) {
        return new Promise(function (ok, fehler) {
            var req;
            try { req = indexedDB.open(name, IDBFS_VERSION); } catch (e) { fehler(e); return; }
            req.onupgradeneeded = function (e) {
                var db = e.target.result, tx = e.target.transaction, store;
                if (db.objectStoreNames.contains(IDBFS_STORE)) store = tx.objectStore(IDBFS_STORE);
                else store = db.createObjectStore(IDBFS_STORE);
                if (!store.indexNames.contains('timestamp')) store.createIndex('timestamp', 'timestamp', { unique: false });
            };
            req.onsuccess = function () { ok(req.result); };
            req.onerror = function () { fehler(req.error); };
        });
    }

    function alleEintraege(db) {
        return new Promise(function (ok, fehler) {
            var liste = [];
            var tx = db.transaction([IDBFS_STORE], 'readonly');
            var cur = tx.objectStore(IDBFS_STORE).openCursor();
            cur.onsuccess = function (e) {
                var c = e.target.result;
                if (c) { liste.push({ key: c.key, value: c.value }); c.continue(); } else ok(liste);
            };
            cur.onerror = function () { fehler(cur.error); };
        });
    }

    function zaehle(db) {
        return new Promise(function (ok, fehler) {
            var tx = db.transaction([IDBFS_STORE], 'readonly');
            var r = tx.objectStore(IDBFS_STORE).count();
            r.onsuccess = function () { ok(r.result); };
            r.onerror = function () { fehler(r.error); };
        });
    }

    function schreibe(db, eintraege) {
        return new Promise(function (ok, fehler) {
            var tx = db.transaction([IDBFS_STORE], 'readwrite');
            var store = tx.objectStore(IDBFS_STORE);
            eintraege.forEach(function (e) { store.put(e.value, e.key); });
            tx.oncomplete = function () { ok(); };
            tx.onerror = function () { fehler(tx.error); };
        });
    }

    function gibtEsDatenbank(name) {
        if (!indexedDB.databases) return Promise.resolve(true);   /* unbekannt -> nachsehen */
        return indexedDB.databases().then(function (l) {
            return l.some(function (d) { return d.name === name; });
        }, function () { return true; });
    }

    /* Liefert immer ein erfuelltes Promise (Fehler werden nur protokolliert). */
    window.dsaSpielstandUmzug = function (spiel) {
        var ziel = '/spielstand_' + spiel;
        if (!NUMMER[spiel] || typeof indexedDB === 'undefined') return Promise.resolve(0);
        var marke = 'dsa_spielstand_umzug_' + spiel;
        try { if (localStorage.getItem(marke) === '1') return Promise.resolve(0); } catch (e) { /* ohne localStorage weiter */ }
        var neuDb = null, altDb = null;
        return oeffne(ziel).then(function (db) {
            neuDb = db;
            return zaehle(db);
        }).then(function (anzahl) {
            if (anzahl > 0) return 0;                      /* neuer Speicher schon in Gebrauch */
            return gibtEsDatenbank(ALT).then(function (da) {
                if (!da) return 0;
                return oeffne(ALT).then(function (db) {
                    altDb = db;
                    return alleEintraege(db);
                }).then(function (liste) {
                    var uebernahme = [];
                    liste.forEach(function (e) {
                        var key = String(e.key);
                        if (key.indexOf(ALT + '/') !== 0) return;
                        var rest = key.substring(ALT.length + 1);
                        if (!rest || rest.indexOf('/') !== -1) return;          /* nur Dateien direkt im Speicher */
                        var v = e.value;
                        if (!v || !v.contents) return;
                        var daten = v.contents instanceof Uint8Array ? v.contents : new Uint8Array(v.contents);
                        if (spielVonDatei(rest, daten) !== spiel) return;
                        uebernahme.push({ key: ziel + '/' + rest, value: v });
                    });
                    if (!uebernahme.length) return 0;
                    return schreibe(neuDb, uebernahme).then(function () {
                        console.log('[Spielstaende] ' + uebernahme.length + ' Datei(en) aus dem alten gemeinsamen Speicher uebernommen: ' +
                            uebernahme.map(function (u) { return u.key.substring(ziel.length + 1); }).join(', '));
                        return uebernahme.length;
                    });
                });
            });
        }).then(function (n) {
            try { localStorage.setItem(marke, '1'); } catch (e) { /* egal */ }
            return n;
        }).catch(function (e) {
            console.warn('[Spielstaende] Uebernahme aus dem alten Speicher fehlgeschlagen:', e);
            return 0;
        }).then(function (n) {
            try { if (neuDb) neuDb.close(); } catch (e) { }
            try { if (altDb) altDb.close(); } catch (e) { }
            return n;
        });
    };
})();
