/*
 * starter_main.cpp - Gemeinsames Startprogramm "Nordlandtrilogie" (Etappe 1 + 2)
 *
 * Aufloesung: 640x400 (SDL2 / Win32-Fallback), komplett mit Maus und Tastatur bedienbar.
 * Funktionen:
 * - Spielauswahl & Einstellungen (Patch-Stufe, Grafikmodus, Probenlog, Intro, Vollbild, Spielordner)
 * - Ziehen & Ablegen (SDL_DROPFILE) fuer Spielordner (auch Oberordner) und .zip-Archive (mit Fortschrittsbalken)
 * - Grafikmodi aus Grafik/MODI.md (original, scharf, xbrz, scalefx, crt, paket) + "Grafikpaket erzeugen..."
 * - Hilfe-Dialog (F1), Sprachumschaltung Deutsch/Englisch (F2/L), Fensterposition merken
 * - "Letztes Spiel direkt starten"-Knopf (F5) in der Spielauswahl
 * - Automatische Pruefung & Anzeige von absturz.log nach Spielende (inkl. Meldehinweis)
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <direct.h>
#include <io.h>

#include "SDL2/SDL.h"
#include "font8x8.h"
#include "zip_extract.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#define SCREEN_W 1040
#define SCREEN_H 647

static uint32_t      g_fb_fallback[SCREEN_W * SCREEN_H];
static uint32_t     *g_fb       = g_fb_fallback;
static HDC           g_mem_dc   = NULL;
static HBITMAP       g_dib_bmp  = NULL;
static HFONT         g_fonts[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int           g_clip_x0  = 0;
static int           g_clip_y0  = 0;
static int           g_clip_x1  = SCREEN_W;
static int           g_clip_y1  = SCREEN_H;
static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture  *g_texture  = NULL;

static std::string g_exe_dir;
static std::string g_starter_dir;
static std::string g_data_dir;
static bool        g_disable_gog_autoscan     = false;
static bool        g_force_no_grafikpaket     = false;
static bool        g_no_browser_exec          = false;
static std::string g_screenshot_progress_path;
static bool        g_screenshot_progress_saved = false;

/* Komplett auf Deutsch (NACHTRAG 1 Etappe 6: kein Sprachumschalter) + gemerkte Fensterposition & Mausposition fuer Hover */
static bool g_lang_en = false;
static int  g_win_x   = (int)SDL_WINDOWPOS_CENTERED;
static int  g_win_y   = (int)SDL_WINDOWPOS_CENTERED;
static int  g_win_w   = SCREEN_W;
static int  g_win_h   = SCREEN_H;
static int  g_mouse_x = -1000;
static int  g_mouse_y = -1000;

static inline const char *L(const char *de, const char *en) {
    (void)en;
    return de;
}

static bool save_framebuffer_bmp(const std::string &bmp_path);
static void render_frame(void);

/* =========================================================================
 * 1. String- & Pfad-Hilfsfunktionen (UTF-8 <-> Windows-1252, Dateipruefung)
 * ========================================================================= */

static std::string trim_str(const std::string &s) {
    size_t a = 0;
    while (a < s.size() && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    size_t b = s.size();
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
    return s.substr(a, b - a);
}

static std::string utf8_to_win1252(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    const char *p = in.c_str();
    while (*p) {
        uint32_t cp = starter_next_codepoint(&p);
        if (cp <= 0xFF) {
            out.push_back((char)(unsigned char)cp);
        } else {
            out.push_back('?');
        }
    }
    return out;
}

static std::string normalize_win_sep(const std::string &p) {
    std::string r = p;
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] == '/') r[i] = '\\';
    }
    while (r.size() > 3 && r.back() == '\\') {
        r.pop_back();
    }
    return r;
}

static std::string normalize_slash_lower(const std::string &p) {
    std::string r = p;
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] == '\\') r[i] = '/';
        else r[i] = (char)tolower((unsigned char)r[i]);
    }
    return r;
}

static bool ends_with_ci(const std::string &s, const std::string &suffix) {
    if (suffix.size() > s.size()) return false;
    return _stricmp(s.c_str() + s.size() - suffix.size(), suffix.c_str()) == 0;
}

static bool is_abs_win_path(const std::string &p) {
    if (p.size() >= 2 && isalpha((unsigned char)p[0]) && p[1] == ':') return true;
    if (p.size() >= 2 && p[0] == '\\' && p[1] == '\\') return true;
    return false;
}

static std::string join_path(const std::string &base, const std::string &rel) {
    if (rel.empty() || rel == ".") return normalize_win_sep(base);
    if (is_abs_win_path(rel)) return normalize_win_sep(rel);
    std::string b = normalize_win_sep(base);
    std::string r = normalize_win_sep(rel);
    if (b.empty()) return r;
    if (b.back() == '\\') return b + r;
    return b + "\\" + r;
}

static std::string full_win_path(const std::string &p) {
    std::string w = utf8_to_win1252(normalize_win_sep(p));
    char buf[MAX_PATH * 2] = "";
    if (_fullpath(buf, w.c_str(), sizeof(buf) - 1)) {
        return normalize_win_sep(std::string(buf));
    }
    return w;
}

static bool file_exists_win(const std::string &p) {
    if (p.empty()) return false;
    std::string w = utf8_to_win1252(normalize_win_sep(p));
    DWORD attr = GetFileAttributesA(w.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) return true;
    attr = GetFileAttributesA(p.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

static bool dir_exists_win(const std::string &p) {
    if (p.empty()) return false;
    std::string w = utf8_to_win1252(normalize_win_sep(p));
    DWORD attr = GetFileAttributesA(w.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) return true;
    attr = GetFileAttributesA(p.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

static long file_size_win(const std::string &p) {
    std::string w = utf8_to_win1252(normalize_win_sep(p));
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExA(w.c_str(), GetFileExInfoStandard, &fad)) {
        return (long)fad.nFileSizeLow;
    }
    return -1;
}

static void mkdir_recursive_win(const std::string &dir_path) {
    std::string w = utf8_to_win1252(normalize_win_sep(dir_path));
    if (w.empty()) return;
    for (size_t i = 0; i < w.size(); ++i) {
        if (w[i] == '\\' && i > 0) {
            if (i == 2 && w[1] == ':') continue;
            std::string sub = w.substr(0, i);
            _mkdir(sub.c_str());
        }
    }
    _mkdir(w.c_str());
}

static void ensure_parent_dirs_win(const std::string &file_path) {
    std::string w = normalize_win_sep(file_path);
    size_t pos = w.find_last_of('\\');
    if (pos != std::string::npos && pos > 0) {
        mkdir_recursive_win(w.substr(0, pos));
    }
}

static void init_starter_dirs(void) {
    char mod[MAX_PATH * 2] = "";
    DWORD n = GetModuleFileNameA(NULL, mod, sizeof(mod) - 1);
    if (n > 0) {
        mod[n] = '\0';
        char *sep = strrchr(mod, '\\');
        if (!sep) sep = strrchr(mod, '/');
        if (sep) *sep = '\0';
        g_exe_dir = normalize_win_sep(mod);
    } else {
        g_exe_dir = ".";
    }
    if (file_exists_win(join_path(g_exe_dir, "_starter\\spiele.json"))) {
        g_starter_dir = g_exe_dir;
        g_data_dir    = join_path(g_exe_dir, "_starter");
    } else if (file_exists_win(join_path(g_exe_dir, "spiele.json"))) {
        g_starter_dir = g_exe_dir;
        g_data_dir    = dir_exists_win(join_path(g_exe_dir, "_starter")) ? join_path(g_exe_dir, "_starter") : g_exe_dir;
    } else if (file_exists_win(join_path(g_exe_dir, "..\\_starter\\spiele.json"))) {
        g_starter_dir = full_win_path(join_path(g_exe_dir, ".."));
        g_data_dir    = join_path(g_starter_dir, "_starter");
    } else if (file_exists_win(join_path(g_exe_dir, "..\\spiele.json"))) {
        g_starter_dir = full_win_path(join_path(g_exe_dir, ".."));
        g_data_dir    = g_starter_dir;
    } else {
        g_starter_dir = g_exe_dir;
        g_data_dir    = g_exe_dir;
    }
}

/* =========================================================================
 * 2. Anzeige- & Skalierungs-Optionen (Etappe 3: Immer exakt 4:3, ohne Filter)
 * ========================================================================= */

static void load_grafik_modi_md(void) {
    /* S3: Keine Grafikfilter und kein Grafikpaket im Starter */
}

/* =========================================================================
 * 3. Datenstrukturen fuer Spiele-Katalog (spiele.json) & Einstellungen
 * ========================================================================= */

enum PatchStufe {
    STUFE_ORIG    = 0,
    STUFE_WICHTIG = 1,
    STUFE_ALLES   = 2
};

struct GameCatalogEntry {
    std::string id;
    int         teil;
    std::string name;
    std::string untertitel;
    std::string version;      /* Stand der Fehlerliste, z. B. "Fanpatch v1.5h" (spiele.json) */
    std::string status_cfg;
    std::string status_wenn_exe_fehlt;
    std::string port_ordner;
    std::vector<std::string> port_ordner_alt;
    std::string exe_orig;
    std::string exe_wichtig;
    std::string exe_alles;
    std::string exe_standard;
    std::string kennungsdatei;
    std::vector<std::string> pflichtdateien;
    std::vector<std::string> gog_pfade;
    std::vector<std::string> grafikmodi;
    bool        probenlog_supported;
    std::string probenlog_skript;
    std::string dos_ordner;
    std::string browser_url;
    std::string browser_status;
    std::string erwartete_exe_datei;
    int         erwartete_exe_groesse;
    std::string erwartete_exe_md5;

    /* Laufzeit-Einstellungen (aus einstellungen.ini) */
    PatchStufe  chosen_stufe;
    std::string chosen_grafik;
    std::string chosen_fenster;     /* "2", "3", "4", "5", "vollbild" (Standard: "vollbild") */
    std::string chosen_skalierung;  /* "ganzzahlig", "fuellend" (Standard: "fuellend") */
    bool        chosen_probenlog;
    bool        chosen_intro;
    bool        chosen_drehen;      /* Fluessig drehen (Schicksalsklinge), Standard: true -> DSA_DREHEN=1/0 */
    bool        chosen_vollbild;
    std::string custom_spielpfad;
    std::string custom_port_ordner;

    /* Laufzeit-Erkennung */
    std::string resolved_port_dir;
    bool        has_exe_orig;
    bool        has_exe_wichtig;
    bool        has_exe_alles;
    bool        has_exe_standard;
    bool        has_any_exe;
    bool        verfuegbar;
    std::string resolved_gamedir;
    bool        gamedir_valid;
    bool        gamedir_writable;
    bool        gamedir_wrong_version;
    std::string wrong_version_dir;
    std::string wrong_version_msg_de;
    std::string wrong_version_msg_en;
    std::string resolved_dos_dir;
    std::string resolved_dos_copy_dir;
    bool        has_dos_patcher;
    bool        dos_fix_exists;
    bool        has_grafikpaket;
    std::string resolved_paket_dir;

    /* T1 (Etappe 4): Zur Laufzeit aus dem Spielordner gelesenes Original-Titelbild (nur im RAM) */
    bool                  has_title_image;
    int                   title_w;
    int                   title_h;
    std::vector<uint32_t> title_pixels;
    std::string           title_loaded_dir;
};

static std::vector<GameCatalogEntry> g_games;
static int g_selected_game = 0;

enum ScreenMode {
    SCREEN_SELECT_GAME = 0,
    SCREEN_SETTINGS    = 1
};

static ScreenMode  g_screen                  = SCREEN_SELECT_GAME;
static int         g_settings_row            = 0;
static int         g_open_dd_game            = -1;   /* -1 = geschlossen, 0..2 = Anzeige-Klappliste offen fuer Karte */
static int         g_open_dd_sel             = -1;   /* 0..4 = hervorgehobener Eintrag in der offenen Klappliste */
static int         g_card_focus              = 0;    /* 0=Fixes, 1=Anzeige, 2=Intro, 3=Probenlog, 4=Ordner, 5=Zahnrad, 6=Windows, 7=DOS, 8=Browser */
static bool        g_show_focus_ring         = false;
static bool        g_modal_active            = false;
static bool        g_modal_is_ok             = false;
static bool        g_modal_is_crash          = false;
static bool        g_modal_is_readonly_dos   = false;
static int         g_modal_readonly_game_idx = -1;
static std::string g_modal_title;
static std::string g_modal_line1;
static std::string g_modal_line2;
static std::string g_modal_line3;
static std::string g_modal_line4;
static bool        g_help_active      = false;
static bool        g_fp_active        = false;
static int         g_fp_game_idx      = 0;
static std::string g_status_msg;
static int         g_hover_game       = -1;

/* Laufende Kindprozess-Ueberwachung fuer interaktiven Absturz-Check */
static HANDLE      g_watched_game_proc = NULL;
static std::string g_watched_exe_dir;
static std::string g_watched_save_dir;
static std::string g_watched_game_name;

/* =========================================================================
 * 4. JSON-Parser fuer Starter/spiele.json
 * ========================================================================= */

static void skip_ws(const char *&p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) ++p;
}

static std::string parse_json_string(const char *&p) {
    skip_ws(p);
    if (*p != '"') return "";
    ++p;
    std::string out;
    while (*p && *p != '"') {
        if (*p == '\\' && p[1]) {
            ++p;
            switch (*p) {
            case '"':  out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/':  out.push_back('/'); break;
            case 'n':  out.push_back('\n'); break;
            case 'r':  out.push_back('\r'); break;
            case 't':  out.push_back('\t'); break;
            default:   out.push_back(*p); break;
            }
        } else {
            out.push_back(*p);
        }
        ++p;
    }
    if (*p == '"') ++p;
    return out;
}

static int parse_json_int(const char *&p) {
    skip_ws(p);
    int sign = 1;
    if (*p == '-') { sign = -1; ++p; }
    int v = 0;
    while (*p >= '0' && *p <= '9') {
        v = v * 10 + (*p - '0');
        ++p;
    }
    return v * sign;
}

static bool parse_json_bool(const char *&p) {
    skip_ws(p);
    if (strncmp(p, "true", 4) == 0) { p += 4; return true; }
    if (strncmp(p, "false", 5) == 0) { p += 5; return false; }
    return false;
}

static void skip_json_value(const char *&p);

static std::vector<std::string> parse_json_string_array(const char *&p) {
    std::vector<std::string> arr;
    skip_ws(p);
    if (*p != '[') { skip_json_value(p); return arr; }
    ++p;
    while (*p) {
        skip_ws(p);
        if (*p == ']') { ++p; break; }
        if (*p == '"') {
            arr.push_back(parse_json_string(p));
        } else {
            skip_json_value(p);
        }
        skip_ws(p);
        if (*p == ',') ++p;
    }
    return arr;
}

static void skip_json_value(const char *&p) {
    skip_ws(p);
    if (*p == '"') {
        parse_json_string(p);
    } else if (*p == '{') {
        int depth = 1;
        ++p;
        while (*p && depth > 0) {
            if (*p == '"') { parse_json_string(p); continue; }
            if (*p == '{') ++depth;
            else if (*p == '}') --depth;
            ++p;
        }
    } else if (*p == '[') {
        int depth = 1;
        ++p;
        while (*p && depth > 0) {
            if (*p == '"') { parse_json_string(p); continue; }
            if (*p == '[') ++depth;
            else if (*p == ']') --depth;
            ++p;
        }
    } else {
        while (*p && *p != ',' && *p != '}' && *p != ']' && *p != ' ' && *p != '\r' && *p != '\n') ++p;
    }
}

static GameCatalogEntry make_default_entry(int teil) {
    GameCatalogEntry e;
    e.teil = teil;
    e.chosen_stufe = STUFE_ALLES;
    e.chosen_grafik = "original";
    e.chosen_fenster = "vollbild";
    e.chosen_skalierung = "fuellend";
    e.chosen_probenlog = false;
    e.chosen_intro = true;
    e.chosen_drehen = true;
    e.chosen_vollbild = true;
    e.probenlog_supported = true;
    e.has_exe_orig = false;
    e.has_exe_wichtig = false;
    e.has_exe_alles = false;
    e.has_exe_standard = false;
    e.has_any_exe = false;
    e.verfuegbar = false;
    e.gamedir_valid = false;
    e.gamedir_writable = false;
    e.gamedir_wrong_version = false;
    e.has_dos_patcher = false;
    e.dos_fix_exists = false;
    e.browser_status = "verfuegbar";
    e.erwartete_exe_groesse = 0;
    if (teil == 1) {
        e.dos_ordner = "Schicksalsklinge\\DOS";
        e.browser_url = "https://meowmuhmuhcow.github.io/schicksalsklinge/";
        e.erwartete_exe_datei = "SCHICKM.EXE";
        e.erwartete_exe_groesse = 556208;
        e.erwartete_exe_md5 = "4ce035f95ead01a8b2553b4487544e5d";
    } else if (teil == 2) {
        e.dos_ordner = "Sternenschweif\\DOS";
        e.browser_url = "https://meowmuhmuhcow.github.io/sternenschweif/";
        e.erwartete_exe_datei = "SCHWEIF.EXE";
        e.erwartete_exe_groesse = 1387552;
        e.erwartete_exe_md5 = "da3b59ce86a82e4b8bb14d36ed37d4d0";
    } else if (teil == 3) {
        e.dos_ordner = "Riva\\DOS";
        e.browser_url = "https://meowmuhmuhcow.github.io/riva/";
        e.erwartete_exe_datei = "RIVA.EXE";
        e.erwartete_exe_groesse = 2477899;
        e.erwartete_exe_md5 = "fb8f5fc3ba5454c7344c17aaf362de35";
    }
    e.has_grafikpaket = false;
    e.has_title_image = false;
    e.title_w = 0;
    e.title_h = 0;
    return e;
}

static void parse_game_object(const char *&p, GameCatalogEntry &e) {
    skip_ws(p);
    if (*p != '{') { skip_json_value(p); return; }
    ++p;
    while (*p) {
        skip_ws(p);
        if (*p == '}') { ++p; break; }
        std::string key = parse_json_string(p);
        skip_ws(p);
        if (*p == ':') ++p;
        skip_ws(p);

        if (key == "id") e.id = parse_json_string(p);
        else if (key == "teil") e.teil = parse_json_int(p);
        else if (key == "name") e.name = parse_json_string(p);
        else if (key == "untertitel") e.untertitel = parse_json_string(p);
        else if (key == "version") e.version = parse_json_string(p);
        else if (key == "status") e.status_cfg = parse_json_string(p);
        else if (key == "status_wenn_exe_fehlt") e.status_wenn_exe_fehlt = parse_json_string(p);
        else if (key == "port_ordner") e.port_ordner = parse_json_string(p);
        else if (key == "port_ordner_alternativen") e.port_ordner_alt = parse_json_string_array(p);
        else if (key == "kennungsdatei") e.kennungsdatei = parse_json_string(p);
        else if (key == "pflichtdateien") e.pflichtdateien = parse_json_string_array(p);
        else if (key == "gog_pfade") e.gog_pfade = parse_json_string_array(p);
        else if (key == "grafikmodi") {
            std::vector<std::string> gm = parse_json_string_array(p);
            if (!gm.empty()) e.grafikmodi = gm;
        }
        else if (key == "probenlog") e.probenlog_supported = parse_json_bool(p);
        else if (key == "probenlog_skript") e.probenlog_skript = parse_json_string(p);
        else if (key == "dos_ordner") e.dos_ordner = parse_json_string(p);
        else if (key == "browser_url") e.browser_url = parse_json_string(p);
        else if (key == "browser_status") e.browser_status = parse_json_string(p);
        else if (key == "erwartete_exe_datei") e.erwartete_exe_datei = parse_json_string(p);
        else if (key == "erwartete_exe_groesse") e.erwartete_exe_groesse = parse_json_int(p);
        else if (key == "erwartete_exe_md5") e.erwartete_exe_md5 = parse_json_string(p);
        else if (key == "exe_stufen" && *p == '{') {
            ++p;
            while (*p) {
                skip_ws(p);
                if (*p == '}') { ++p; break; }
                std::string sk = parse_json_string(p);
                skip_ws(p);
                if (*p == ':') ++p;
                skip_ws(p);
                std::string sv = parse_json_string(p);
                if (sk == "original") e.exe_orig = sv;
                else if (sk == "wichtig") e.exe_wichtig = sv;
                else if (sk == "alles") e.exe_alles = sv;
                else if (sk == "standard") e.exe_standard = sv;
                skip_ws(p);
                if (*p == ',') ++p;
            }
        } else {
            skip_json_value(p);
        }
        skip_ws(p);
        if (*p == ',') ++p;
    }
    if (e.pflichtdateien.empty() && !e.kennungsdatei.empty()) {
        e.pflichtdateien.push_back(e.kennungsdatei);
    }
}

static bool load_spiele_json(const std::string &json_path) {
    std::string wpath = utf8_to_win1252(json_path);
    FILE *fp = fopen(wpath.c_str(), "rb");
    if (!fp) fp = fopen(json_path.c_str(), "rb");
    if (!fp) return false;

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz <= 0 || sz > 1024 * 1024) {
        fclose(fp);
        return false;
    }
    std::string content((size_t)sz, '\0');
    fread(&content[0], 1, (size_t)sz, fp);
    fclose(fp);

    g_games.clear();
    const char *p = content.c_str();
    skip_ws(p);
    if (*p != '{') return false;
    ++p;
    while (*p) {
        skip_ws(p);
        if (*p == '}') { ++p; break; }
        std::string key = parse_json_string(p);
        skip_ws(p);
        if (*p == ':') ++p;
        skip_ws(p);
        if (key == "spiele" && *p == '[') {
            ++p;
            int idx = 1;
            while (*p) {
                skip_ws(p);
                if (*p == ']') { ++p; break; }
                if (*p == '{') {
                    GameCatalogEntry e = make_default_entry(idx++);
                    parse_game_object(p, e);
                    g_games.push_back(e);
                } else {
                    skip_json_value(p);
                }
                skip_ws(p);
                if (*p == ',') ++p;
            }
        } else {
            skip_json_value(p);
        }
        skip_ws(p);
        if (*p == ',') ++p;
    }
    return !g_games.empty();
}

/* =========================================================================
 * 5. Einstellungen lesen & speichern (einstellungen.ini) inkl. Sprache & Fenster
 * ========================================================================= */

static const char *stufe_to_str(PatchStufe s) {
    switch (s) {
    case STUFE_ORIG:    return "original";
    case STUFE_WICHTIG: return "wichtig";
    case STUFE_ALLES:
    default:            return "alles";
    }
}

static PatchStufe str_to_stufe(const std::string &s) {
    if (_stricmp(s.c_str(), "original") == 0 || s == "0" || _stricmp(s.c_str(), "orig") == 0) return STUFE_ORIG;
    if (_stricmp(s.c_str(), "wichtig") == 0 || s == "1") return STUFE_WICHTIG;
    return STUFE_ALLES;
}

static void load_einstellungen_ini(void) {
    std::string p0 = join_path(g_data_dir, "einstellungen.ini");
    std::string p1 = join_path(g_exe_dir, "einstellungen.ini");
    std::string p2 = join_path(g_starter_dir, "einstellungen.ini");
    std::string chosen = file_exists_win(p0) ? p0 : (file_exists_win(p1) ? p1 : (file_exists_win(p2) ? p2 : ""));
    if (chosen.empty()) return;

    FILE *fp = fopen(utf8_to_win1252(chosen).c_str(), "r");
    if (!fp) return;

    std::string section;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        std::string s = trim_str(line);
        if (s.empty() || s[0] == ';' || s[0] == '#') continue;
        if (s.front() == '[' && s.back() == ']') {
            section = trim_str(s.substr(1, s.size() - 2));
            continue;
        }
        size_t eq = s.find('=');
        if (eq == std::string::npos) continue;
        std::string k = trim_str(s.substr(0, eq));
        std::string v = trim_str(s.substr(eq + 1));

        if (_stricmp(section.c_str(), "allgemein") == 0) {
            if (_stricmp(k.c_str(), "letztes_spiel") == 0) {
                for (size_t i = 0; i < g_games.size(); ++i) {
                    if (_stricmp(g_games[i].id.c_str(), v.c_str()) == 0) {
                        g_selected_game = (int)i;
                        break;
                    }
                }
            } else if (_stricmp(k.c_str(), "letzter_bildschirm") == 0) {
                if (_stricmp(v.c_str(), "einstellungen") == 0) g_screen = SCREEN_SETTINGS;
                else g_screen = SCREEN_SELECT_GAME;
            } else if (_stricmp(k.c_str(), "sprache") == 0) {
                /* NACHTRAG 1 (Etappe 6): Einstellung "sprache" beim Laden ignorieren */
                g_lang_en = false;
            } else if (_stricmp(k.c_str(), "fenster_x") == 0 && !v.empty()) {
                g_win_x = atoi(v.c_str());
            } else if (_stricmp(k.c_str(), "fenster_y") == 0 && !v.empty()) {
                g_win_y = atoi(v.c_str());
            } else if (_stricmp(k.c_str(), "fenster_b") == 0 && !v.empty()) {
                int w = atoi(v.c_str());
                if (w >= SCREEN_W && w <= 3840 && w != 1060) g_win_w = w;
            } else if (_stricmp(k.c_str(), "fenster_h") == 0 && !v.empty()) {
                int h = atoi(v.c_str());
                if (h >= SCREEN_H && h <= 2160 && h != 760 && h != 680) g_win_h = h;
            }
        } else {
            for (size_t i = 0; i < g_games.size(); ++i) {
                if (_stricmp(g_games[i].id.c_str(), section.c_str()) == 0) {
                    GameCatalogEntry &g = g_games[i];
                    if (_stricmp(k.c_str(), "stufe") == 0) g.chosen_stufe = str_to_stufe(v);
                    else if (_stricmp(k.c_str(), "fenster") == 0 && !v.empty()) {
                        if (_stricmp(v.c_str(), "vollbild") == 0 || v == "2" || v == "3" || v == "4" || v == "5") {
                            g.chosen_fenster = (_stricmp(v.c_str(), "vollbild") == 0) ? "vollbild" : v;
                            g.chosen_vollbild = (g.chosen_fenster == "vollbild");
                        }
                    } else if (_stricmp(k.c_str(), "skalierung") == 0 && !v.empty()) {
                        if (_stricmp(v.c_str(), "ganzzahlig") == 0) g.chosen_skalierung = "ganzzahlig";
                        else if (_stricmp(v.c_str(), "fuellend") == 0) g.chosen_skalierung = "fuellend";
                    } else if (_stricmp(k.c_str(), "grafik") == 0) {
                        /* S3: Alte grafik=-Eintraege beim Laden stillschweigend ignorieren */
                    } else if (_stricmp(k.c_str(), "probenlog") == 0) g.chosen_probenlog = (v == "1" || _stricmp(v.c_str(), "true") == 0 || _stricmp(v.c_str(), "ja") == 0);
                    else if (_stricmp(k.c_str(), "intro") == 0) g.chosen_intro = (v == "1" || _stricmp(v.c_str(), "true") == 0 || _stricmp(v.c_str(), "ja") == 0);
                    else if (_stricmp(k.c_str(), "drehen") == 0 || _stricmp(k.c_str(), "fluessig_drehen") == 0) g.chosen_drehen = (v == "1" || _stricmp(v.c_str(), "true") == 0 || _stricmp(v.c_str(), "ja") == 0);
                    else if (_stricmp(k.c_str(), "vollbild") == 0) {
                        g.chosen_vollbild = (v == "1" || _stricmp(v.c_str(), "true") == 0 || _stricmp(v.c_str(), "ja") == 0);
                        if (g.chosen_vollbild) g.chosen_fenster = "vollbild";
                        else if (g.chosen_fenster == "vollbild") g.chosen_fenster = "4";
                    } else if (_stricmp(k.c_str(), "spielpfad") == 0) {
                        if (v != "-" && _stricmp(v.c_str(), "keiner") != 0 && _stricmp(v.c_str(), "none") != 0) {
                            g.custom_spielpfad = v;
                        }
                    }
                    else if (_stricmp(k.c_str(), "port_ordner") == 0) g.custom_port_ordner = v;
                }
            }
        }
    }
    fclose(fp);
}

static void sync_window_geometry_to_globals(void) {
    if (!g_window) return;
    const char *hl = getenv("STARTER_HEADLESS");
    if (hl && hl[0] == '1') return;
    int wx = 0, wy = 0, ww = 0, wh = 0;
    SDL_GetWindowPosition(g_window, &wx, &wy);
    SDL_GetWindowSize(g_window, &ww, &wh);
    if (wx >= -100 && wx <= 4000 && wy >= -100 && wy <= 4000) {
        g_win_x = wx;
        g_win_y = wy;
    }
    if (ww >= 640 && wh >= 400) {
        g_win_w = ww;
        g_win_h = wh;
    }
}

static void write_one_ini(const std::string &ini_path) {
    FILE *fp = fopen(utf8_to_win1252(ini_path).c_str(), "w");
    if (!fp) return;
    fprintf(fp, "; Einstellungen fuer den gemeinsamen Starter der Nordlandtrilogie\n");
    fprintf(fp, "[allgemein]\n");
    if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        fprintf(fp, "letztes_spiel=%s\n", g_games[g_selected_game].id.c_str());
    }
    fprintf(fp, "letzter_bildschirm=%s\n", (g_screen == SCREEN_SETTINGS) ? "einstellungen" : "auswahl");
    if (g_win_x != (int)SDL_WINDOWPOS_CENTERED && g_win_x != (int)SDL_WINDOWPOS_UNDEFINED) {
        fprintf(fp, "fenster_x=%d\n", g_win_x);
        fprintf(fp, "fenster_y=%d\n", g_win_y);
    }
    fprintf(fp, "fenster_b=%d\n", g_win_w);
    fprintf(fp, "fenster_h=%d\n\n", g_win_h);

    for (size_t i = 0; i < g_games.size(); ++i) {
        const GameCatalogEntry &g = g_games[i];
        fprintf(fp, "[%s]\n", g.id.c_str());
        fprintf(fp, "stufe=%s\n", stufe_to_str(g.chosen_stufe));
        fprintf(fp, "fenster=%s\n", g.chosen_fenster.c_str());
        fprintf(fp, "skalierung=%s\n", g.chosen_skalierung.c_str());
        fprintf(fp, "probenlog=%d\n", g.chosen_probenlog ? 1 : 0);
        fprintf(fp, "intro=%d\n", g.chosen_intro ? 1 : 0);
        if (g.teil == 1) {
            fprintf(fp, "drehen=%d\n", g.chosen_drehen ? 1 : 0);
        }
        if (!g.custom_spielpfad.empty() && g.custom_spielpfad != "-" &&
            _stricmp(g.custom_spielpfad.c_str(), "keiner") != 0 &&
            _stricmp(g.custom_spielpfad.c_str(), "none") != 0) {
            fprintf(fp, "spielpfad=%s\n", g.custom_spielpfad.c_str());
        }
        if (!g.custom_port_ordner.empty()) {
            fprintf(fp, "port_ordner=%s\n", g.custom_port_ordner.c_str());
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

static void save_einstellungen_ini(void) {
    sync_window_geometry_to_globals();
    std::string p0 = join_path(g_data_dir, "einstellungen.ini");
    mkdir_recursive_win(g_data_dir);
    write_one_ini(p0);
    if (_stricmp(g_data_dir.c_str(), g_exe_dir.c_str()) == 0) {
        std::string p2 = join_path(g_starter_dir, "einstellungen.ini");
        if (_stricmp(p0.c_str(), p2.c_str()) != 0) {
            write_one_ini(p2);
        }
    }
}

/* =========================================================================
 * 6. Erkennung von Port-EXEs, Spielordner, Grafikpaket & Fassungspruefung
 * ========================================================================= */

static bool is_program_files_dir(const std::string &dir) {
    std::string low = normalize_slash_lower(dir);
    if (low.find("/program files/") != std::string::npos) return true;
    if (low.find("/program files (x86)/") != std::string::npos) return true;
    return false;
}

static bool is_dir_writable_real_win(const std::string &dir) {
    if (dir.empty()) return false;
    std::string wdir = utf8_to_win1252(normalize_win_sep(dir));
    char test_file[MAX_PATH * 2];
    snprintf(test_file, sizeof(test_file), "%s\\.starter_wtest_%lu.tmp", wdir.c_str(), (unsigned long)GetCurrentProcessId());
    HANDLE h = CreateFileA(test_file, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (h == INVALID_HANDLE_VALUE) return false;
    CloseHandle(h);
    DeleteFileA(test_file);
    return true;
}

/* Nachtrag 3: Nur der echte Schreibtest entscheidet (Testdatei anlegen und wieder loeschen),
 * KEINE pauschale Program-Files-Sperre. DSA_FORCE_READONLY_GAMEDIR bleibt fuer Tests erhalten. */
static bool is_dir_writable_win(const std::string &dir) {
    const char *force_ro = getenv("DSA_FORCE_READONLY_GAMEDIR");
    if (force_ro && force_ro[0] == '1') return false;
    return is_dir_writable_real_win(dir);
}

/* RFC 1321 MD5 fuer die Vorab-Pruefung der Original-Spiel-EXEs (T2) */
static void compute_buffer_md5_hex(const uint8_t *msg, size_t len, char out_hex[33]) {
    static const uint32_t K[64] = {
        0xd76aa478u,0xe8c7b756u,0x242070dbu,0xc1bdceeeu,0xf57c0fafu,0x4787c62au,0xa8304613u,0xfd469501u,
        0x698098d8u,0x8b44f7afu,0xffff5bb1u,0x895cd7beu,0x6b901122u,0xfd987193u,0xa679438eu,0x49b40821u,
        0xf61e2562u,0xc040b340u,0x265e5a51u,0xe9b6c7aau,0xd62f105du,0x02441453u,0xd8a1e681u,0xe7d3fbc8u,
        0x21e1cde6u,0xc33707d6u,0xf4d50d87u,0x455a14edu,0xa9e3e905u,0xfcefa3f8u,0x676f02d9u,0x8d2a4c8au,
        0xfffa3942u,0x8771f681u,0x6d9d6122u,0xfde5380cu,0xa4beea44u,0x4bdecfa9u,0xf6bb4b60u,0xbebfbc70u,
        0x289b7ec6u,0xeaa127fau,0xd4ef3085u,0x04881d05u,0xd9d4d039u,0xe6db99e5u,0x1fa27cf8u,0xc4ac5665u,
        0xf4292244u,0x432aff97u,0xab9423a7u,0xfc93a039u,0x655b59c3u,0x8f0ccc92u,0xffeff47du,0x85845dd1u,
        0x6fa87e4fu,0xfe2ce6e0u,0xa3014314u,0x4e0811a1u,0xf7537e82u,0xbd3af235u,0x2ad7d2bbu,0xeb86d391u
    };
    static const uint32_t S[64] = {
        7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
        5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
        4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
        6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
    };
    uint32_t a0 = 0x67452301u, b0 = 0xefcdab89u, c0 = 0x98badcfeu, d0 = 0x10325476u;
    size_t new_len = ((len + 8) / 64 + 1) * 64;
    std::vector<uint8_t> buf(new_len, 0);
    if (len > 0) memcpy(buf.data(), msg, len);
    buf[len] = 0x80;
    uint64_t bits_len = (uint64_t)len * 8ull;
    memcpy(buf.data() + new_len - 8, &bits_len, 8);

    for (size_t offset = 0; offset < new_len; offset += 64) {
        const uint32_t *M = (const uint32_t *)(buf.data() + offset);
        uint32_t A = a0, B = b0, C = c0, D = d0;
        for (uint32_t i = 0; i < 64; ++i) {
            uint32_t F, g;
            if (i < 16)      { F = (B & C) | ((~B) & D); g = i; }
            else if (i < 32) { F = (D & B) | ((~D) & C); g = (5 * i + 1) % 16; }
            else if (i < 48) { F = B ^ C ^ D;            g = (3 * i + 5) % 16; }
            else             { F = C ^ (B | (~D));       g = (7 * i) % 16; }
            F = F + A + K[i] + M[g];
            A = D; D = C; C = B;
            B = B + ((F << S[i]) | (F >> (32 - S[i])));
        }
        a0 += A; b0 += B; c0 += C; d0 += D;
    }
    uint8_t digest[16];
    memcpy(digest + 0,  &a0, 4);
    memcpy(digest + 4,  &b0, 4);
    memcpy(digest + 8,  &c0, 4);
    memcpy(digest + 12, &d0, 4);
    static const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < 16; ++i) {
        out_hex[i * 2]     = hex_chars[(digest[i] >> 4) & 0xF];
        out_hex[i * 2 + 1] = hex_chars[digest[i] & 0xF];
    }
    out_hex[32] = '\0';
}

struct Md5FileCacheEntry {
    std::string path;
    long        size;
    DWORD       mtime_low;
    DWORD       mtime_high;
    std::string md5;
};
static std::vector<Md5FileCacheEntry> g_md5_cache;

static bool get_file_size_and_md5(const std::string &path, long &out_size, std::string &out_md5) {
    out_size = -1;
    out_md5.clear();
    std::string w = utf8_to_win1252(normalize_win_sep(path));
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(w.c_str(), GetFileExInfoStandard, &fad)) {
        return false;
    }
    out_size = (long)fad.nFileSizeLow;
    for (size_t i = 0; i < g_md5_cache.size(); ++i) {
        if (_stricmp(g_md5_cache[i].path.c_str(), w.c_str()) == 0 &&
            g_md5_cache[i].size == out_size &&
            g_md5_cache[i].mtime_low == fad.ftLastWriteTime.dwLowDateTime &&
            g_md5_cache[i].mtime_high == fad.ftLastWriteTime.dwHighDateTime) {
            out_md5 = g_md5_cache[i].md5;
            return true;
        }
    }
    if (out_size < 0 || out_size > 32 * 1024 * 1024) return false;
    FILE *fp = fopen(w.c_str(), "rb");
    if (!fp) return false;
    std::vector<uint8_t> buf((size_t)out_size);
    if (out_size > 0 && fread(buf.data(), 1, (size_t)out_size, fp) != (size_t)out_size) {
        fclose(fp);
        return false;
    }
    fclose(fp);
    char hex[33];
    compute_buffer_md5_hex(buf.data(), (size_t)out_size, hex);
    out_md5 = hex;

    Md5FileCacheEntry ce;
    ce.path       = w;
    ce.size       = out_size;
    ce.mtime_low  = fad.ftLastWriteTime.dwLowDateTime;
    ce.mtime_high = fad.ftLastWriteTime.dwHighDateTime;
    ce.md5        = out_md5;
    g_md5_cache.push_back(ce);
    return true;
}

static void get_wrong_version_messages(const GameCatalogEntry &g, std::string &out_de, std::string &out_en) {
    if (g.teil == 3) {
        out_de = "Diese Riva-Fassung wird nicht unterstützt - benötigt wird die deutsche GOG-Fassung";
        out_en = "This Riva version is not supported - the German GOG version is required";
    } else if (g.teil == 2) {
        out_de = "Diese Sternenschweif-Fassung wird nicht unterstützt - benötigt wird die deutsche GOG-Fassung";
        out_en = "This Star Trail version is not supported - the German GOG version is required";
    } else {
        out_de = "Diese Schicksalsklinge-Fassung wird nicht unterstützt - benötigt wird die deutsche GOG-Fassung";
        out_en = "This Blade of Destiny version is not supported - the German GOG version is required";
    }
}

/* Prueft, ob die Spieldaten im Ordner zur erwarteten deutschen GOG-Fassung passen (T2) */
static bool validate_game_version(const GameCatalogEntry &g, const std::string &dir) {
    std::string exe_rel = !g.erwartete_exe_datei.empty()
        ? g.erwartete_exe_datei
        : ((g.teil == 1) ? "SCHICKM.EXE" : ((g.teil == 2) ? "SCHWEIF.EXE" : "RIVA.EXE"));
    int exp_size = (g.erwartete_exe_groesse > 0)
        ? g.erwartete_exe_groesse
        : ((g.teil == 1) ? 556208 : ((g.teil == 2) ? 1387552 : 2477899));
    std::string exp_md5 = !g.erwartete_exe_md5.empty()
        ? g.erwartete_exe_md5
        : ((g.teil == 1) ? "4ce035f95ead01a8b2553b4487544e5d"
                         : ((g.teil == 2) ? "da3b59ce86a82e4b8bb14d36ed37d4d0"
                                          : "fb8f5fc3ba5454c7344c17aaf362de35"));

    std::string exe_path = join_path(dir, exe_rel);
    if (!file_exists_win(exe_path) && file_exists_win(join_path(dir, std::string("DATA\\") + exe_rel))) {
        exe_path = join_path(dir, std::string("DATA\\") + exe_rel);
    }
    long actual_size = -1;
    std::string actual_md5;
    if (!get_file_size_and_md5(exe_path, actual_size, actual_md5)) {
        return false;
    }
    if (actual_size != (long)exp_size || _stricmp(actual_md5.c_str(), exp_md5.c_str()) != 0) {
        return false;
    }
    if (g.teil == 1) {
        long dsagen_sz = file_size_win(join_path(dir, "DSAGEN.DAT"));
        if (dsagen_sz != 663221L) {
            return false;
        }
    }
    return true;
}

static bool file_exists_in_gamedir(const std::string &dir, const std::string &rel) {
    if (file_exists_win(join_path(dir, rel))) return true;
    if (_strnicmp(rel.c_str(), "DATA\\", 5) == 0 || _strnicmp(rel.c_str(), "DATA/", 5) == 0) {
        if (file_exists_win(join_path(dir, rel.substr(5)))) return true;
    } else {
        if (file_exists_win(join_path(dir, std::string("DATA\\") + rel))) return true;
    }
    return false;
}

static bool validate_gamedir_files(const GameCatalogEntry &g, const std::string &dir, std::string &out_missing) {
    out_missing.clear();
    if (!file_exists_in_gamedir(dir, g.kennungsdatei)) {
        out_missing = g.kennungsdatei;
        return false;
    }
    for (size_t i = 0; i < g.pflichtdateien.size(); ++i) {
        if (!file_exists_in_gamedir(dir, g.pflichtdateien[i])) {
            out_missing = g.pflichtdateien[i];
            return false;
        }
    }
    return true;
}

static bool check_gamedir_candidate_ex(const GameCatalogEntry &g, const std::string &cand, std::string &out_dir, bool &out_wrong_version) {
    out_wrong_version = false;
    std::string c = trim_str(cand);
    if (c.empty()) return false;
    if (!is_abs_win_path(c)) {
        c = join_path(g_starter_dir, c);
    }
    std::string missing;
    if (validate_gamedir_files(g, c, missing)) {
        if (!validate_game_version(g, c)) {
            out_dir = full_win_path(c);
            out_wrong_version = true;
            return false;
        }
        out_dir = full_win_path(c);
        return true;
    }
    std::string parent = join_path(c, "..");
    if (validate_gamedir_files(g, parent, missing)) {
        if (!validate_game_version(g, parent)) {
            out_dir = full_win_path(parent);
            out_wrong_version = true;
            return false;
        }
        out_dir = full_win_path(parent);
        return true;
    }
    return false;
}

static bool check_gamedir_candidate(const GameCatalogEntry &g, const std::string &cand, std::string &out_dir) {
    bool wrong_ver = false;
    return check_gamedir_candidate_ex(g, cand, out_dir, wrong_ver);
}

static bool read_spielpfad_txt(const GameCatalogEntry &g, const std::string &txt_path, std::string &out_dir) {
    if (!file_exists_win(txt_path)) return false;
    FILE *fp = fopen(utf8_to_win1252(txt_path).c_str(), "r");
    if (!fp) return false;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        std::string s = trim_str(line);
        if (s.empty() || s[0] == '#' || s[0] == ';') continue;
        if (strncmp(s.c_str(), "gamedir=", 8) == 0) s = trim_str(s.substr(8));
        if (check_gamedir_candidate(g, s, out_dir)) {
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

static bool dir_has_grafikpaket(const std::string &dir) {
    if (!dir_exists_win(dir)) return false;
    if (file_exists_win(join_path(dir, "manifest.json"))) return true;
    if (file_exists_win(join_path(dir, "paket.bin"))) return true;
    if (file_exists_win(join_path(dir, "status.json"))) return true;
    if (file_exists_win(join_path(dir, "portraets\\manifest.json"))) return true;
    return false;
}

/* =========================================================================
 * 6b. Original-Titelbilder zur Laufzeit aus dem Spielordner lesen (T1)
 *     RECHTLICH ZWINGEND: Keine Bildbytes mitgeliefert oder zwischengespeichert!
 * ========================================================================= */

static inline uint16_t rd_u16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline int16_t rd_i16_le(const uint8_t *p) {
    return (int16_t)rd_u16_le(p);
}

static inline uint32_t rd_u32_le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline uint32_t vga6_to_argb32(uint8_t r6, uint8_t g6, uint8_t b6) {
    uint8_t r = (uint8_t)(((r6 & 0x3F) << 2) | ((r6 & 0x3F) >> 4));
    uint8_t g = (uint8_t)(((g6 & 0x3F) << 2) | ((g6 & 0x3F) >> 4));
    uint8_t b = (uint8_t)(((b6 & 0x3F) << 2) | ((b6 & 0x3F) >> 4));
    return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static inline uint8_t rev_byte8(uint8_t v) {
    v = (uint8_t)(((v & 0xF0u) >> 4) | ((v & 0x0Fu) << 4));
    v = (uint8_t)(((v & 0xCCu) >> 2) | ((v & 0x33u) << 2));
    v = (uint8_t)(((v & 0xAAu) >> 1) | ((v & 0x55u) << 1));
    return v;
}

static inline uint16_t rev_word16(uint16_t v) {
    return (uint16_t)(((uint16_t)rev_byte8((uint8_t)(v & 0xFFu)) << 8) |
                      (uint16_t)rev_byte8((uint8_t)(v >> 8)));
}

static bool decomp_pp20_buf(const uint8_t *src, size_t plen, std::vector<uint8_t> &dest) {
    if (!src || plen < 12) return false;
    uint32_t unplen = ((uint32_t)src[plen - 4] << 16) | ((uint32_t)src[plen - 3] << 8) | (uint32_t)src[plen - 2];
    if (unplen == 0 || unplen > 1024 * 1024) return false;
    uint8_t skip = src[plen - 1];
    uint8_t off_tuple[4] = { src[4], src[5], src[6], src[7] };
    dest.assign(unplen, 0);

    size_t buf_pos = plen - 4;
    size_t out_pos = unplen;
    uint32_t bb = 0;
    int bl = 0;

    while (bl < (int)skip && buf_pos > 8) {
        --buf_pos;
        bb |= (uint32_t)src[buf_pos] << bl;
        bl += 8;
    }
    bb >>= skip;
    bl -= (int)skip;

    while (out_pos > 0) {
        if (bl < 16 && buf_pos > 8) {
            if (buf_pos >= 10) {
                buf_pos -= 2;
                bb |= (uint32_t)(src[buf_pos + 1] | ((uint32_t)src[buf_pos] << 8)) << bl;
                bl += 16;
            } else {
                --buf_pos;
                bb |= (uint32_t)src[buf_pos] << bl;
                bl += 8;
            }
        }
        uint32_t b1 = bb & 1u;
        bb >>= 1;
        bl -= 1;
        if (b1 == 0) {
            uint32_t todo = 1;
            for (;;) {
                if (bl < 2 && buf_pos > 8) {
                    --buf_pos;
                    bb |= (uint32_t)src[buf_pos] << bl;
                    bl += 8;
                }
                uint32_t x = (uint32_t)(rev_word16((uint16_t)(bb & 3u)) >> 14);
                bb >>= 2;
                bl -= 2;
                todo += x;
                if (x != 3) break;
            }
            for (uint32_t k = 0; k < todo; ++k) {
                if (bl < 8 && buf_pos > 8) {
                    --buf_pos;
                    bb |= (uint32_t)src[buf_pos] << bl;
                    bl += 8;
                }
                if (out_pos == 0) break;
                --out_pos;
                dest[out_pos] = rev_byte8((uint8_t)(bb & 0xFFu));
                bb >>= 8;
                bl -= 8;
            }
            if (out_pos == 0) break;
        }

        if (bl < 2 && buf_pos > 8) {
            --buf_pos;
            bb |= (uint32_t)src[buf_pos] << bl;
            bl += 8;
        }
        uint32_t x = (uint32_t)(rev_word16((uint16_t)(bb & 3u)) >> 14);
        bb >>= 2;
        bl -= 2;
        int offbits = (int)off_tuple[x];
        if (offbits <= 0 || offbits > 16) return false;
        uint32_t todo = x + 2;
        uint32_t offset = 0;
        if (x == 3) {
            if (bl < 1 && buf_pos > 8) {
                --buf_pos;
                bb |= (uint32_t)src[buf_pos] << bl;
                bl += 8;
            }
            if ((bb & 1u) == 0) offbits = 7;
            bb >>= 1;
            bl -= 1;
            while (bl < offbits && buf_pos > 8) {
                --buf_pos;
                bb |= (uint32_t)src[buf_pos] << bl;
                bl += 8;
            }
            offset = (uint32_t)(rev_word16((uint16_t)(bb & ((1u << offbits) - 1u))) >> (16 - offbits));
            bb >>= offbits;
            bl -= offbits;
            for (;;) {
                if (bl < 3 && buf_pos > 8) {
                    --buf_pos;
                    bb |= (uint32_t)src[buf_pos] << bl;
                    bl += 8;
                }
                uint32_t x2 = (uint32_t)(rev_word16((uint16_t)(bb & 7u)) >> 13);
                bb >>= 3;
                bl -= 3;
                todo += x2;
                if (x2 != 7) break;
            }
        } else {
            while (bl < offbits && buf_pos > 8) {
                --buf_pos;
                bb |= (uint32_t)src[buf_pos] << bl;
                bl += 8;
            }
            offset = (uint32_t)(rev_word16((uint16_t)(bb & ((1u << offbits) - 1u))) >> (16 - offbits));
            bb >>= offbits;
            bl -= offbits;
        }

        for (uint32_t k = 0; k < todo; ++k) {
            if (out_pos == 0) break;
            size_t src_idx = out_pos + offset;
            uint8_t v = (src_idx < unplen) ? dest[src_idx] : 0;
            --out_pos;
            dest[out_pos] = v;
        }
    }
    return true;
}

static void decomp_rle7f_buf(const uint8_t *src, size_t n, size_t total, std::vector<uint8_t> &out) {
    out.clear();
    out.reserve(total);
    size_t pos = 0;
    while (out.size() < total && pos < n) {
        uint8_t b = src[pos++];
        if (b == 0x7Fu && pos + 1 < n) {
            uint8_t cnt = src[pos++];
            uint8_t val = src[pos++];
            size_t add = cnt;
            if (out.size() + add > total) add = total - out.size();
            out.insert(out.end(), add, val);
        } else {
            out.push_back(b);
        }
    }
    if (out.size() < total) {
        out.resize(total, 0);
    }
}

static void decomp_packbits_buf(const uint8_t *src, size_t n, size_t total, std::vector<uint8_t> &out) {
    out.clear();
    out.reserve(total);
    size_t pos = 0;
    while (pos < n && out.size() < total) {
        uint8_t c = src[pos++];
        if (c & 0x80u) {
            size_t rep = (257u - (size_t)c) & 0xFFu;
            if (pos >= n) break;
            uint8_t val = src[pos++];
            if (out.size() + rep > total) rep = total - out.size();
            out.insert(out.end(), rep, val);
        } else {
            size_t lit = (size_t)c + 1u;
            if (pos + lit > n) lit = n - pos;
            if (out.size() + lit > total) lit = total - out.size();
            out.insert(out.end(), src + pos, src + pos + lit);
            pos += lit;
        }
    }
    if (out.size() < total) {
        out.resize(total, 0);
    }
}

/* Entpackt eine .ULI-Datei (INTRODAT.VOL aus Schicksalsklinge) in 8-Bit-Indizes + 256er ARGB-Palette */
static bool decode_uli_indexed(
    const uint8_t *data, size_t len,
    int &out_w, int &out_h,
    std::vector<uint8_t> &out_pix,
    uint32_t out_pal[256]
) {
    if (!data || len < 38) return false;
    if (memcmp(data, "(C) 1991 by Ulrich Walther", 26) != 0) return false;
    int w = (int)rd_u16_le(data + 29) + 1;
    int h = (int)rd_u16_le(data + 31) + 1;
    if (w != 320 || h != 200) return false;

    int b33 = (int)data[33];
    int b34 = (int)data[34];
    int first_col = b34 / 3;
    int pal_colors = b33 - first_col;
    if (first_col < 0 || first_col > 255 || pal_colors < 0 || first_col + pal_colors > 256) return false;

    std::vector<uint8_t> unp;
    unp.reserve(70000);
    size_t pos = 36;
    while (pos < len) {
        uint8_t b = data[pos++];
        if (b == 0) break;
        if (b < 0x80u) {
            if (pos >= len) break;
            unp.insert(unp.end(), (size_t)b, data[pos++]);
        } else {
            size_t cnt = (size_t)(b & 0x7Fu);
            if (pos + cnt > len) cnt = len - pos;
            unp.insert(unp.end(), data + pos, data + pos + cnt);
            pos += cnt;
        }
    }

    size_t pal_bytes = (size_t)pal_colors * 3u;
    if (unp.size() < pal_bytes) return false;

    for (int i = 0; i < 256; ++i) out_pal[i] = 0xFF000000u;
    out_pal[255] = 0xFFFFFFFFu;
    for (int i = 0; i < pal_colors; ++i) {
        out_pal[first_col + i] = vga6_to_argb32(
            unp[(size_t)i * 3u + 0],
            unp[(size_t)i * 3u + 1],
            unp[(size_t)i * 3u + 2]
        );
    }

    const uint8_t *pix_raw = unp.data() + pal_bytes;
    size_t pix_len = unp.size() - pal_bytes;
    out_pix.assign(64000, 0);

    if (pix_len == 8000) {
        size_t idx = 0;
        for (size_t i = 0; i < 8000; ++i) {
            uint8_t bv = pix_raw[i];
            for (int s = 7; s >= 0; --s) {
                out_pix[idx++] = ((bv >> s) & 1) ? (uint8_t)first_col : 0;
            }
        }
    } else if (pix_len == 16000) {
        size_t idx = 0;
        for (size_t i = 0; i < 16000; ++i) {
            uint8_t bv = pix_raw[i];
            for (int s = 6; s >= 0; s -= 2) {
                int v = (bv >> s) & 3;
                out_pix[idx++] = v ? (uint8_t)(first_col - 1 + v) : 0;
            }
        }
    } else if (pix_len == 32000) {
        size_t idx = 0;
        for (size_t i = 0; i < 32000; ++i) {
            uint8_t bv = pix_raw[i];
            int v1 = (bv >> 4) & 0x0F;
            int v2 = bv & 0x0F;
            out_pix[idx++] = v1 ? (uint8_t)(first_col - 1 + v1) : 0;
            out_pix[idx++] = v2 ? (uint8_t)(first_col - 1 + v2) : 0;
        }
    } else if (pix_len >= 64000) {
        memcpy(out_pix.data(), pix_raw, 64000);
    } else {
        return false;
    }

    out_w = 320;
    out_h = 200;
    return true;
}

/* Entpackt eine .AIF-Datei (DSA 2 STAR.DAT / DSA 3 RIVA.ALF & game.gog) */
static bool decode_aif_indexed(
    const uint8_t *data, size_t len,
    int &out_w, int &out_h,
    std::vector<uint8_t> &out_pix,
    uint32_t out_pal[256]
) {
    if (!data || len < 30) return false;
    if (data[0] != 'A' || data[1] != 'I' || data[2] != 'F') return false;
    uint8_t pack   = data[4];
    uint8_t no_pal = data[5];
    int w          = (int)rd_i16_le(data + 6);
    int h          = (int)rd_i16_le(data + 8);
    int ncolors    = (int)rd_i16_le(data + 10);
    int first_col  = (int)rd_i16_le(data + 12);
    if (w <= 0 || w > 640 || h <= 0 || h > 480) return false;
    if (first_col < 0 || first_col >= 256) first_col = 0;
    if (ncolors < 0 || first_col + ncolors > 256) ncolors = 256 - first_col;

    size_t pal_len = no_pal ? 0u : (size_t)ncolors * 3u;
    if (30u + pal_len > len) return false;
    const uint8_t *comp_data = data + 30;
    size_t comp_len = len - 30u - pal_len;
    const uint8_t *pal_raw = data + len - pal_len;

    size_t total = (size_t)w * (size_t)h;
    if (pack == 0) {
        if (comp_len < total) return false;
        out_pix.assign(comp_data, comp_data + total);
    } else if (pack == 1) {
        decomp_rle7f_buf(comp_data, comp_len, total, out_pix);
    } else if (pack == 2) {
        decomp_packbits_buf(comp_data, comp_len, total, out_pix);
    } else if (pack == 3) {
        if (!decomp_pp20_buf(comp_data, comp_len, out_pix) || out_pix.size() < total) return false;
        out_pix.resize(total);
    } else {
        return false;
    }

    for (int i = 0; i < 256; ++i) out_pal[i] = 0xFF000000u;
    out_pal[255] = 0xFFFFFFFFu;
    if (!no_pal) {
        for (int i = 0; i < ncolors; ++i) {
            out_pal[first_col + i] = vga6_to_argb32(
                pal_raw[(size_t)i * 3u + 0],
                pal_raw[(size_t)i * 3u + 1],
                pal_raw[(size_t)i * 3u + 2]
            );
        }
    }
    out_w = w;
    out_h = h;
    return true;
}

/* Entpackt ein einzelnes Frame aus einer .NVF-Datei (fuer GPHILT.NVF in Riva) */
static bool decode_nvf_frame_indexed(
    const uint8_t *data, size_t len, int target_frame,
    int &out_w, int &out_h,
    std::vector<uint8_t> &out_pix
) {
    if (!data || len < 7) return false;
    uint8_t nvf_type = data[0] & 0x7Fu;
    int pics = (int)rd_i16_le(data + 1);
    if (pics <= 0 || pics > 500 || target_frame < 0 || target_frame >= pics || nvf_type > 5) return false;

    if (nvf_type == 0) {
        int w = (int)rd_i16_le(data + 3);
        int h = (int)rd_i16_le(data + 5);
        if (w <= 0 || w > 640 || h <= 0 || h > 480) return false;
        size_t psize = (size_t)w * (size_t)h;
        size_t offs = 7u + (size_t)target_frame * psize;
        if (offs + psize > len) return false;
        out_w = w; out_h = h;
        out_pix.assign(data + offs, data + offs + psize);
        return true;
    } else if (nvf_type == 1) {
        size_t hdr_end = 3u + (size_t)pics * 4u;
        if (hdr_end > len) return false;
        size_t offs = hdr_end;
        for (int i = 0; i <= target_frame; ++i) {
            int w = (int)rd_i16_le(data + 3 + i * 4);
            int h = (int)rd_i16_le(data + 5 + i * 4);
            if (w <= 0 || w > 640 || h <= 0 || h > 480) return false;
            size_t psize = (size_t)w * (size_t)h;
            if (offs + psize > len) return false;
            if (i == target_frame) {
                out_w = w; out_h = h;
                out_pix.assign(data + offs, data + offs + psize);
                return true;
            }
            offs += psize;
        }
    } else if (nvf_type == 2 || nvf_type == 4) {
        int w = (int)rd_i16_le(data + 3);
        int h = (int)rd_i16_le(data + 5);
        if (w <= 0 || w > 640 || h <= 0 || h > 480) return false;
        size_t hdr_end = 7u + (size_t)pics * 4u;
        if (hdr_end > len) return false;
        size_t offs = hdr_end;
        for (int i = 0; i <= target_frame; ++i) {
            uint32_t sz = rd_u32_le(data + 7 + i * 4);
            if (offs + sz > len) return false;
            if (i == target_frame) {
                out_w = w; out_h = h;
                if (nvf_type == 2) return decomp_pp20_buf(data + offs, sz, out_pix) && out_pix.size() >= (size_t)(w * h);
                decomp_rle7f_buf(data + offs, sz, (size_t)(w * h), out_pix);
                return true;
            }
            offs += sz;
        }
    } else if (nvf_type == 3 || nvf_type == 5) {
        size_t hdr_end = 3u + (size_t)pics * 8u;
        if (hdr_end > len) return false;
        size_t offs = hdr_end;
        for (int i = 0; i <= target_frame; ++i) {
            int w = (int)rd_i16_le(data + 3 + i * 8);
            int h = (int)rd_i16_le(data + 5 + i * 8);
            uint32_t sz = rd_u32_le(data + 7 + i * 8);
            if (w <= 0 || w > 640 || h <= 0 || h > 480 || offs + sz > len) return false;
            if (i == target_frame) {
                out_w = w; out_h = h;
                if (nvf_type == 3) return decomp_pp20_buf(data + offs, sz, out_pix) && out_pix.size() >= (size_t)(w * h);
                decomp_rle7f_buf(data + offs, sz, (size_t)(w * h), out_pix);
                return true;
            }
            offs += sz;
        }
    }
    return false;
}

/* Liest aus einem ISO-9660-CD-Abbild (gamea.gog / game.gog, 2352 oder 2048 Bytes/Sektor) einen Dateibereich */
static bool read_iso_sectors(FILE *fp, bool raw2352, uint32_t lba, uint32_t offset, uint32_t length, std::vector<uint8_t> &out) {
    out.resize(length);
    uint32_t cur = offset;
    uint32_t rem = length;
    size_t written = 0;
    while (rem > 0) {
        uint32_t sec = lba + (cur >> 11);
        uint32_t soff = cur & 0x7FFu;
        uint32_t take = (2048u - soff < rem) ? (2048u - soff) : rem;
        long file_off = raw2352 ? ((long)sec * 2352L + 24L + (long)soff) : ((long)sec * 2048L + (long)soff);
        if (fseek(fp, file_off, SEEK_SET) != 0) return false;
        if (fread(out.data() + written, 1, take, fp) != take) return false;
        cur += take;
        rem -= take;
        written += take;
    }
    return true;
}

static bool find_file_in_gog_iso(
    FILE *fp, const char *dir_name, const char *file_name,
    bool &out_raw2352, uint32_t &out_lba, uint32_t &out_len
) {
    uint8_t pvd[2048];
    bool raw2352 = true;
    if (fseek(fp, 16L * 2352L + 24L, SEEK_SET) == 0 && fread(pvd, 1, 2048, fp) == 2048 && memcmp(pvd + 1, "CD001", 5) == 0) {
        raw2352 = true;
    } else if (fseek(fp, 16L * 2048L, SEEK_SET) == 0 && fread(pvd, 1, 2048, fp) == 2048 && memcmp(pvd + 1, "CD001", 5) == 0) {
        raw2352 = false;
    } else {
        return false;
    }
    out_raw2352 = raw2352;
    uint32_t root_lba = rd_u32_le(pvd + 158);
    uint32_t root_len = rd_u32_le(pvd + 166);
    if (root_len == 0 || root_len > 256 * 1024) return false;

    std::vector<uint8_t> rdir;
    if (!read_iso_sectors(fp, raw2352, root_lba, 0, root_len, rdir)) return false;

    uint32_t target_dir_lba = root_lba;
    uint32_t target_dir_len = root_len;
    if (dir_name && dir_name[0]) {
        bool found_dir = false;
        size_t pos = 0;
        while (pos < rdir.size()) {
            uint8_t reclen = rdir[pos];
            if (reclen == 0) { pos = ((pos >> 11) + 1) << 11; continue; }
            if (pos + reclen > rdir.size() || reclen < 34) break;
            uint32_t flba = rd_u32_le(&rdir[pos + 2]);
            uint32_t flen = rd_u32_le(&rdir[pos + 10]);
            uint8_t nlen = rdir[pos + 32];
            if (pos + 33 + nlen <= pos + reclen) {
                std::string nm((const char *)&rdir[pos + 33], nlen);
                size_t sc = nm.find(';');
                if (sc != std::string::npos) nm.resize(sc);
                if (_stricmp(nm.c_str(), dir_name) == 0) {
                    target_dir_lba = flba;
                    target_dir_len = flen;
                    found_dir = true;
                    break;
                }
            }
            pos += reclen;
        }
        if (!found_dir || target_dir_len == 0 || target_dir_len > 256 * 1024) return false;
    }

    std::vector<uint8_t> ddir;
    if (!read_iso_sectors(fp, raw2352, target_dir_lba, 0, target_dir_len, ddir)) return false;
    size_t pos = 0;
    while (pos < ddir.size()) {
        uint8_t reclen = ddir[pos];
        if (reclen == 0) { pos = ((pos >> 11) + 1) << 11; continue; }
        if (pos + reclen > ddir.size() || reclen < 34) break;
        uint32_t flba = rd_u32_le(&ddir[pos + 2]);
        uint32_t flen = rd_u32_le(&ddir[pos + 10]);
        uint8_t nlen = ddir[pos + 32];
        if (pos + 33 + nlen <= pos + reclen) {
            std::string nm((const char *)&ddir[pos + 33], nlen);
            size_t sc = nm.find(';');
            if (sc != std::string::npos) nm.resize(sc);
            if (_stricmp(nm.c_str(), file_name) == 0) {
                out_lba = flba;
                out_len = flen;
                return true;
            }
        }
        pos += reclen;
    }
    return false;
}

/* 1. Titelbild Teil 1 (Die Schicksalsklinge): DSAART.ULI aus INTRODAT.DIR / INTRODAT.VOL
 *    Fallback: KARTE.DAT (Eintrag 5) aus SCHICK.DAT */
static bool load_dsa1_title_image(const std::string &gamedir, std::vector<uint32_t> &out_argb) {
    std::string dir_path = join_path(gamedir, "INTRODAT.DIR");
    std::string vol_path = join_path(gamedir, "INTRODAT.VOL");
    if (file_exists_win(dir_path) && file_exists_win(vol_path)) {
        FILE *fdir = fopen(utf8_to_win1252(dir_path).c_str(), "rb");
        FILE *fvol = fopen(utf8_to_win1252(vol_path).c_str(), "rb");
        if (fdir && fvol) {
            fseek(fdir, 0, SEEK_END);
            long dsz = ftell(fdir);
            fseek(fdir, 0, SEEK_SET);
            if (dsz > 0 && dsz <= 65536) {
                std::vector<uint8_t> dbuf((size_t)dsz);
                fread(dbuf.data(), 1, (size_t)dsz, fdir);
                uint32_t art_off = 0, art_sz = 0;
                for (size_t i = 0; i + 32 <= dbuf.size(); i += 32) {
                    uint8_t nlen = dbuf[i];
                    if (nlen >= 1 && nlen <= 12) {
                        std::string nm((const char *)&dbuf[i + 1], nlen);
                        uint32_t off = rd_u32_le(&dbuf[i + 26]);
                        uint16_t sz  = rd_u16_le(&dbuf[i + 30]);
                        if (_stricmp(nm.c_str(), "DSAART.ULI") == 0) { art_off = off; art_sz = sz; break; }
                    }
                }
                if (art_sz > 0 && art_sz <= 131072) {
                    std::vector<uint8_t> abuf(art_sz);
                    if (fseek(fvol, (long)art_off, SEEK_SET) == 0 && fread(abuf.data(), 1, art_sz, fvol) == art_sz) {
                        int w1 = 0, h1 = 0;
                        std::vector<uint8_t> pix1;
                        uint32_t pal1[256];
                        if (decode_uli_indexed(abuf.data(), abuf.size(), w1, h1, pix1, pal1)) {
                            out_argb.resize(64000);
                            for (size_t k = 0; k < 64000; ++k) {
                                out_argb[k] = pal1[pix1[k]];
                            }
                            fclose(fdir);
                            fclose(fvol);
                            return true;
                        }
                    }
                }
            }
        }
        if (fdir) fclose(fdir);
        if (fvol) fclose(fvol);
    }

    /* Fallback: SCHICK.DAT Eintrag #5 (KARTE.DAT, 320x200 + 32-Farben-Palette bei +64002) */
    std::string schick_path = join_path(gamedir, "SCHICK.DAT");
    if (file_exists_win(schick_path)) {
        FILE *fp = fopen(utf8_to_win1252(schick_path).c_str(), "rb");
        if (fp) {
            uint8_t offs_buf[32];
            if (fread(offs_buf, 1, 32, fp) == 32) {
                uint32_t off5 = rd_u32_le(offs_buf + 5 * 4);
                uint32_t off6 = rd_u32_le(offs_buf + 6 * 4);
                if (off6 > off5 && (off6 - off5) >= 64098) {
                    std::vector<uint8_t> kbuf(64098);
                    if (fseek(fp, (long)off5, SEEK_SET) == 0 && fread(kbuf.data(), 1, 64098, fp) == 64098) {
                        uint32_t pal[256];
                        for (int i = 0; i < 256; ++i) pal[i] = 0xFF000000u;
                        for (int i = 0; i < 32; ++i) {
                            pal[i] = vga6_to_argb32(kbuf[64002 + i * 3], kbuf[64002 + i * 3 + 1], kbuf[64002 + i * 3 + 2]);
                        }
                        out_argb.resize(64000);
                        for (size_t k = 0; k < 64000; ++k) {
                            out_argb[k] = pal[kbuf[k] & 0x1Fu];
                        }
                        fclose(fp);
                        return true;
                    }
                }
            }
            fclose(fp);
        }
    }
    return false;
}

/* 2. Titelbild Teil 2 (Sternenschweif): ARTWORK.AIF (PIC_TITLE = 0x436) aus DATA\STAR.DAT bzw. gamea.gog
 *    Fallback bei lokaler STAR.DAT ohne gamea.gog: LOWANGEN.AIF oder CREDITS.AIF */
static bool read_stardat_aif_entry(const std::string &gamedir, const char *entry_name, std::vector<uint8_t> &out_raw) {
    /* Zuerst in gamea.gog / game.gog schauen, falls das Eintrag-Flag in der lokalen STAR.DAT 0 ist */
    std::string star_path = join_path(gamedir, "DATA\\STAR.DAT");
    if (file_exists_win(star_path)) {
        FILE *fp = fopen(utf8_to_win1252(star_path).c_str(), "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long fsz = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            uint8_t cnt_buf[4];
            if (fsz > 24842 && fread(cnt_buf, 1, 4, fp) == 4) {
                uint32_t cnt = rd_u32_le(cnt_buf);
                if (cnt > 1 && cnt <= 2000 && (long)(4 + (cnt - 1) * 20) <= fsz) {
                    std::vector<uint8_t> hdr((cnt - 1) * 20);
                    if (fread(hdr.data(), 1, hdr.size(), fp) == hdr.size()) {
                        for (uint32_t i = 0; i + 1 < cnt - 1; ++i) {
                            const uint8_t *rec = &hdr[i * 20];
                            char nm[15];
                            memcpy(nm, rec, 14);
                            nm[14] = '\0';
                            uint32_t off = rd_u32_le(rec + 14);
                            uint16_t flg = rd_u16_le(rec + 18);
                            if (_stricmp(nm, entry_name) == 0 && flg == 1) {
                                /* Naechsten Eintrag mit flg==1 fuer das Ende finden */
                                uint32_t next_off = (uint32_t)(fsz - 24842);
                                for (uint32_t j = i + 1; j < cnt - 1; ++j) {
                                    uint32_t o2 = rd_u32_le(&hdr[j * 20 + 14]);
                                    uint16_t f2 = rd_u16_le(&hdr[j * 20 + 18]);
                                    if (f2 == 1 && o2 > off) { next_off = o2; break; }
                                }
                                uint32_t sz = (next_off > off) ? (next_off - off) : 0;
                                if (sz >= 30 && sz <= 512 * 1024 && (long)(24842u + off + sz) <= fsz) {
                                    out_raw.resize(sz);
                                    if (fseek(fp, (long)(24842u + off), SEEK_SET) == 0 &&
                                        fread(out_raw.data(), 1, sz, fp) == sz) {
                                        fclose(fp);
                                        return true;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            fclose(fp);
        }
    }

    /* Falls in lokaler STAR.DAT flg==0 war: aus CD-Image gamea.gog / game.gog lesen */
    const char *iso_names[2] = { "gamea.gog", "game.gog" };
    for (int k = 0; k < 2; ++k) {
        std::string iso_path = join_path(gamedir, iso_names[k]);
        if (!file_exists_win(iso_path)) continue;
        FILE *fp = fopen(utf8_to_win1252(iso_path).c_str(), "rb");
        if (!fp) continue;
        bool raw2352 = true;
        uint32_t star_lba = 0, star_len = 0;
        if (find_file_in_gog_iso(fp, "DATA", "STAR.DAT", raw2352, star_lba, star_len) && star_len > 24844) {
            std::vector<uint8_t> cd_hdr;
            if (read_iso_sectors(fp, raw2352, star_lba, 0, 24844, cd_hdr)) {
                uint32_t cnt = rd_u32_le(cd_hdr.data());
                if (cnt > 1 && cnt <= 1243) {
                    for (uint32_t i = 0; i < cnt - 1; ++i) {
                        const uint8_t *rec = &cd_hdr[4 + i * 20];
                        char nm[15];
                        memcpy(nm, rec, 14);
                        nm[14] = '\0';
                        if (_stricmp(nm, entry_name) == 0) {
                            uint32_t off = rd_u32_le(rec + 14);
                            uint32_t next_off = (i + 1 < cnt - 1) ? rd_u32_le(&cd_hdr[4 + (i + 1) * 20 + 14]) : (star_len - 24842u);
                            uint32_t sz = (next_off > off) ? (next_off - off) : 0;
                            if (sz >= 30 && sz <= 512 * 1024 && 24842u + off + sz <= star_len) {
                                if (read_iso_sectors(fp, raw2352, star_lba, 24842u + off, sz, out_raw)) {
                                    fclose(fp);
                                    return true;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        fclose(fp);
    }
    return false;
}

static bool load_dsa2_title_image(const std::string &gamedir, std::vector<uint32_t> &out_argb) {
    const char *candidates[4] = { "ARTWORK.AIF", "FLAME1.AIF", "LOWANGEN.AIF", "CREDITS.AIF" };
    for (int i = 0; i < 4; ++i) {
        std::vector<uint8_t> raw;
        if (read_stardat_aif_entry(gamedir, candidates[i], raw)) {
            int w = 0, h = 0;
            std::vector<uint8_t> pix;
            uint32_t pal[256];
            if (decode_aif_indexed(raw.data(), raw.size(), w, h, pix, pal) && w == 320 && h == 200) {
                out_argb.resize(64000);
                for (int y = 0; y < 200; ++y) {
                    for (int x = 0; x < 320; ++x) {
                        size_t k = (size_t)y * 320u + (size_t)x;
                        uint32_t c = pal[pix[k]];
                        /* Bei ARTWORK.AIF das Lade-Overlay ("Bitte warten!" + CD unten rechts) weich in den Kartenhintergrund (#1b2232) ueberblenden */
                        if (i == 0 && x >= 184 && y >= 120) {
                            int fx = (x >= 210) ? 0 : (210 - x) * 256 / 26;
                            int fy = (y >= 142) ? 0 : (142 - y) * 256 / 22;
                            int f = (fx > fy) ? fx : fy;
                            int r = ((((int)((c >> 16) & 0xFF)) * f) + 0x1B * (256 - f)) >> 8;
                            int g = ((((int)((c >> 8)  & 0xFF)) * f) + 0x22 * (256 - f)) >> 8;
                            int b = ((((int)(c         & 0xFF)) * f) + 0x32 * (256 - f)) >> 8;
                            c = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
                        }
                        out_argb[k] = c;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

/* 3. Titelbild Teil 3 (Schatten ueber Riva): GPMASK00.AIF aus DATA\RIVA.ALF
 *    Fallback: DATA\SCHATTEN.AIF (aus game.gog oder lose) */
static bool load_dsa3_title_image(const std::string &gamedir, std::vector<uint32_t> &out_argb, int &out_w, int &out_h) {
    std::string alf_path = join_path(gamedir, "DATA\\RIVA.ALF");
    if (file_exists_win(alf_path)) {
        FILE *fp = fopen(utf8_to_win1252(alf_path).c_str(), "rb");
        if (fp) {
            uint8_t hdr[48];
            if (fread(hdr, 1, 48, fp) == 48) {
                uint16_t cnt       = rd_u16_le(hdr + 8);
                uint32_t dir_off   = rd_u32_le(hdr + 10);
                uint32_t data_base = rd_u32_le(hdr + 16);
                if (cnt > 0 && cnt <= 3000) {
                    std::vector<uint8_t> dir_buf((size_t)cnt * 28u);
                    if (fseek(fp, (long)dir_off, SEEK_SET) == 0 &&
                        fread(dir_buf.data(), 1, dir_buf.size(), fp) == dir_buf.size()) {
                        uint32_t gp_off = 0, gp_sz = 0;
                        for (uint16_t i = 0; i < cnt; ++i) {
                            const uint8_t *rec = &dir_buf[(size_t)i * 28u];
                            char nm[15];
                            memcpy(nm, rec, 14);
                            nm[14] = '\0';
                            uint32_t sz  = rd_u32_le(rec + 14);
                            uint32_t off = rd_u32_le(rec + 24);
                            if (_stricmp(nm, "GPMASK00.AIF") == 0) { gp_off = data_base + off; gp_sz = sz; break; }
                        }
                        if (gp_sz >= 30 && gp_sz <= 256 * 1024) {
                            std::vector<uint8_t> gp_raw(gp_sz);
                            if (fseek(fp, (long)gp_off, SEEK_SET) == 0 && fread(gp_raw.data(), 1, gp_sz, fp) == gp_sz) {
                                int w = 0, h = 0;
                                std::vector<uint8_t> pix;
                                uint32_t pal[256];
                                if (decode_aif_indexed(gp_raw.data(), gp_raw.size(), w, h, pix, pal) && w == 320 && h >= 140 && h <= 200) {
                                    /* Volle Motivbreite (Stadttor + alle 3 Helden) und echte Motivhoehe h (149) ohne schwarze Unterkante */
                                    int crop_w = 304;
                                    out_argb.resize((size_t)crop_w * (size_t)h);
                                    for (int y = 0; y < h; ++y) {
                                        for (int x = 0; x < crop_w; ++x) {
                                            out_argb[(size_t)y * (size_t)crop_w + (size_t)x] = pal[pix[(size_t)y * 320u + (size_t)x]];
                                        }
                                    }
                                    out_w = crop_w;
                                    out_h = h;
                                    fclose(fp);
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
            fclose(fp);
        }
    }

    /* Fallback: DATA/SCHATTEN.AIF aus game.gog */
    std::string iso_path = join_path(gamedir, "game.gog");
    if (file_exists_win(iso_path)) {
        FILE *fp = fopen(utf8_to_win1252(iso_path).c_str(), "rb");
        if (fp) {
            bool raw2352 = true;
            uint32_t lba = 0, len = 0;
            if (find_file_in_gog_iso(fp, "DATA", "SCHATTEN.AIF", raw2352, lba, len) && len >= 30 && len <= 256 * 1024) {
                std::vector<uint8_t> raw;
                if (read_iso_sectors(fp, raw2352, lba, 0, len, raw)) {
                    int w = 0, h = 0;
                    std::vector<uint8_t> pix;
                    uint32_t pal[256];
                    if (decode_aif_indexed(raw.data(), raw.size(), w, h, pix, pal) && w == 320 && h == 200) {
                        out_argb.resize(64000);
                        for (size_t k = 0; k < 64000; ++k) out_argb[k] = pal[pix[k]];
                        out_w = 320;
                        out_h = 200;
                        fclose(fp);
                        return true;
                    }
                }
            }
            fclose(fp);
        }
    }
    return false;
}

/* Entfernt schwarze Pillarbox-/Letterbox-Raender (z. B. links/rechts bei DSAART.ULI und ARTWORK.AIF),
 * damit alle drei Titelbilder exakt buendig am selben linken Rand starten und die volle Hoehe/Breite fuellen. */
static void crop_title_image_black_margins(std::vector<uint32_t> &pixels, int &w, int &h) {
    if (w <= 16 || h <= 16 || pixels.size() < (size_t)(w * h)) return;

    auto is_visible_px = [](uint32_t c) -> bool {
        int r = (int)((c >> 16) & 0xFF);
        int g = (int)((c >> 8)  & 0xFF);
        int b = (int)(c         & 0xFF);
        return (r + g + b) > 18;
    };

    int band_y0 = h / 10;
    int band_y1 = (h * 6) / 10;
    int x0 = 0;
    while (x0 < w / 3) {
        int cnt = 0;
        for (int y = band_y0; y < band_y1; ++y) {
            if (is_visible_px(pixels[(size_t)y * (size_t)w + (size_t)x0])) ++cnt;
        }
        if (cnt > 4) break;
        ++x0;
    }

    int x1 = w - 1;
    while (x1 > (w * 2) / 3) {
        int cnt = 0;
        for (int y = band_y0; y < band_y1; ++y) {
            if (is_visible_px(pixels[(size_t)y * (size_t)w + (size_t)x1])) ++cnt;
        }
        if (cnt > 4) break;
        --x1;
    }

    int y0 = 0;
    while (y0 < h / 4) {
        int cnt = 0;
        for (int x = x0; x <= x1; ++x) {
            if (is_visible_px(pixels[(size_t)y0 * (size_t)w + (size_t)x])) ++cnt;
        }
        if (cnt > 4) break;
        ++y0;
    }

    int y1 = h - 1;
    while (y1 > (h * 3) / 4) {
        int cnt = 0;
        for (int x = x0; x <= x1; ++x) {
            if (is_visible_px(pixels[(size_t)y1 * (size_t)w + (size_t)x])) ++cnt;
        }
        if (cnt > 4) break;
        --y1;
    }

    int nw = x1 - x0 + 1;
    int nh = y1 - y0 + 1;
    if (nw <= 32 || nh <= 32 || (nw == w && nh == h)) return;

    std::vector<uint32_t> cropped((size_t)nw * (size_t)nh);
    for (int y = 0; y < nh; ++y) {
        for (int x = 0; x < nw; ++x) {
            cropped[(size_t)y * (size_t)nw + (size_t)x] = pixels[(size_t)(y0 + y) * (size_t)w + (size_t)(x0 + x)];
        }
    }
    pixels.swap(cropped);
    w = nw;
    h = nh;
}

static void load_game_title_image(GameCatalogEntry &g) {
    if (!g.gamedir_valid || g.resolved_gamedir.empty()) {
        g.has_title_image = false;
        g.title_w = 0;
        g.title_h = 0;
        g.title_pixels.clear();
        g.title_loaded_dir.clear();
        return;
    }
    if (g.has_title_image && _stricmp(g.title_loaded_dir.c_str(), g.resolved_gamedir.c_str()) == 0) {
        return;
    }
    g.has_title_image = false;
    g.title_w = 0;
    g.title_h = 0;
    g.title_pixels.clear();
    g.title_loaded_dir.clear();

    bool ok = false;
    int lw = 320, lh = 200;
    if (g.teil == 1) {
        ok = load_dsa1_title_image(g.resolved_gamedir, g.title_pixels);
    } else if (g.teil == 2) {
        ok = load_dsa2_title_image(g.resolved_gamedir, g.title_pixels);
    } else if (g.teil == 3) {
        ok = load_dsa3_title_image(g.resolved_gamedir, g.title_pixels, lw, lh);
    }
    if (ok && !g.title_pixels.empty()) {
        g.title_w = lw;
        g.title_h = lh;
        if (g.title_w == 320 && g.title_h == 200) {
            crop_title_image_black_margins(g.title_pixels, g.title_w, g.title_h);
        }
        g.has_title_image = (g.title_w > 0 && g.title_h > 0);
        g.title_loaded_dir = g.resolved_gamedir;
    } else {
        g.title_pixels.clear();
    }
}

static void refresh_game_status(GameCatalogEntry &g) {
    /* 1. Port-Ordner & EXEs pruefen (inkl. g_exe_dir, Unterordner Nativ/bin sowie gewaehltem Spielpfad fuer Einzelpakete) */
    std::vector<std::string> dirs_to_check;
    if (!g.custom_port_ordner.empty()) {
        dirs_to_check.push_back(g.custom_port_ordner);
        dirs_to_check.push_back(join_path(g.custom_port_ordner, "bin"));
        dirs_to_check.push_back(join_path(g.custom_port_ordner, "Nativ"));
    }
    if (!g.port_ordner.empty() && g.port_ordner != ".") {
        dirs_to_check.push_back(g.port_ordner);
        dirs_to_check.push_back(join_path(g.port_ordner, "bin"));
    }
    dirs_to_check.push_back(g_exe_dir);
    if (!g.port_ordner.empty()) dirs_to_check.push_back(g.port_ordner);
    for (size_t i = 0; i < g.port_ordner_alt.size(); ++i) {
        dirs_to_check.push_back(g.port_ordner_alt[i]);
        dirs_to_check.push_back(join_path(g.port_ordner_alt[i], "bin"));
    }
    const char *sub_folder = (g.teil == 1) ? "Schicksalsklinge" : ((g.teil == 2) ? "Sternenschweif" : "Riva");
    dirs_to_check.push_back(std::string(sub_folder) + "\\Nativ");
    dirs_to_check.push_back(std::string("trilogie\\Nordlandtrilogie\\") + sub_folder + "\\Nativ");
    dirs_to_check.push_back(std::string("..\\trilogie\\Nordlandtrilogie\\") + sub_folder + "\\Nativ");
    dirs_to_check.push_back("release");
    dirs_to_check.push_back("..\\release");
    if (!g.custom_spielpfad.empty() && g.custom_spielpfad != "-") {
        dirs_to_check.push_back(g.custom_spielpfad);
        dirs_to_check.push_back(join_path(g.custom_spielpfad, "bin"));
        dirs_to_check.push_back(join_path(g.custom_spielpfad, "Nativ"));
    }

    g.has_exe_orig = false;
    g.has_exe_wichtig = false;
    g.has_exe_alles = false;
    g.has_exe_standard = false;
    g.has_any_exe = false;
    g.verfuegbar = false;
    g.resolved_port_dir = !g.port_ordner.empty() ? join_path(g_starter_dir, g.port_ordner) : g_starter_dir;

    for (size_t i = 0; i < dirs_to_check.size(); ++i) {
        std::string d = join_path(g_starter_dir, dirs_to_check[i]);
        bool o = !g.exe_orig.empty() && file_exists_win(join_path(d, g.exe_orig));
        bool w = !g.exe_wichtig.empty() && file_exists_win(join_path(d, g.exe_wichtig));
        bool a = !g.exe_alles.empty() && file_exists_win(join_path(d, g.exe_alles));
        bool s = !g.exe_standard.empty() && file_exists_win(join_path(d, g.exe_standard));
        if (o || w || a || s) {
            g.resolved_port_dir = d;
            g.has_exe_orig = o;
            g.has_exe_wichtig = w;
            g.has_exe_alles = a;
            g.has_exe_standard = s;
            g.has_any_exe = true;
            break;
        }
    }

    /* 2. Spielordner (SPIELPFAD) automatisch suchen + Fassung pruefen (T2) */
    g.resolved_gamedir.clear();
    g.gamedir_valid = false;
    g.gamedir_writable = false;
    g.gamedir_wrong_version = false;
    g.wrong_version_dir.clear();
    get_wrong_version_messages(g, g.wrong_version_msg_de, g.wrong_version_msg_en);

    bool explicitly_disabled = (g.custom_spielpfad == "-" ||
                                _stricmp(g.custom_spielpfad.c_str(), "keiner") == 0 ||
                                _stricmp(g.custom_spielpfad.c_str(), "none") == 0);

    if (!explicitly_disabled && !g.custom_spielpfad.empty()) {
        std::string cand_dir;
        bool wrong_ver = false;
        if (check_gamedir_candidate_ex(g, g.custom_spielpfad, cand_dir, wrong_ver)) {
            g.resolved_gamedir = cand_dir;
            g.gamedir_valid = true;
        } else if (wrong_ver) {
            g.gamedir_wrong_version = true;
            g.wrong_version_dir = cand_dir;
        }
    }
    if (!explicitly_disabled && !g.gamedir_valid && !g.gamedir_wrong_version) {
        std::string data_spieldaten = join_path(g_data_dir, "Spieldaten\\" + g.id);
        if (check_gamedir_candidate(g, data_spieldaten, g.resolved_gamedir)) {
            g.gamedir_valid = true;
        }
    }
    if (!explicitly_disabled && !g.gamedir_valid && !g.gamedir_wrong_version) {
        std::string local_spieldaten = join_path(g_starter_dir, "Spieldaten\\" + g.id);
        if (check_gamedir_candidate(g, local_spieldaten, g.resolved_gamedir)) {
            g.gamedir_valid = true;
        }
    }
    if (!explicitly_disabled && !g.gamedir_valid && !g.gamedir_wrong_version && !g_disable_gog_autoscan) {
        const char *env_sp = getenv("SPIELPFAD");
        if (!env_sp || !env_sp[0]) env_sp = getenv("DSA_GAMEDIR");
        if (!env_sp || !env_sp[0]) env_sp = getenv("DSA_GAME_DIR");
        if (env_sp && env_sp[0] && check_gamedir_candidate(g, env_sp, g.resolved_gamedir)) {
            g.gamedir_valid = true;
        }
    }
    if (!explicitly_disabled && !g.gamedir_valid && !g.gamedir_wrong_version && !g_disable_gog_autoscan) {
        std::string sp_candidates[] = {
            join_path(g_data_dir, "SPIELPFAD.txt"),
            join_path(g_exe_dir, "SPIELPFAD.txt"),
            join_path(g_starter_dir, "SPIELPFAD.txt"),
            join_path(g.resolved_port_dir, "SPIELPFAD.txt"),
            join_path(g.resolved_port_dir, "..\\SPIELPFAD.txt")
        };
        for (size_t i = 0; i < 5; ++i) {
            if (read_spielpfad_txt(g, sp_candidates[i], g.resolved_gamedir)) {
                g.gamedir_valid = true;
                break;
            }
        }
    }
    if (!explicitly_disabled && !g.gamedir_valid && !g.gamedir_wrong_version && !g_disable_gog_autoscan) {
        for (size_t i = 0; i < g.gog_pfade.size(); ++i) {
            if (check_gamedir_candidate(g, g.gog_pfade[i], g.resolved_gamedir)) {
                g.gamedir_valid = true;
                break;
            }
        }
    }
    if (g.gamedir_valid) {
        g.gamedir_writable = is_dir_writable_win(g.resolved_gamedir);
    }

    /* T1 (Etappe 4): Original-Titelbild zur Laufzeit aus dem Spielordner laden */
    load_game_title_image(g);

    /* 3. DOS-Patch-Ordner & erzeugte Fix-EXEs pruefen (Nachtrag 1 & Nachtrag 3) */
    g.resolved_dos_dir.clear();
    g.resolved_dos_copy_dir.clear();
    g.has_dos_patcher = false;
    g.dos_fix_exists = false;
    {
        std::vector<std::string> dos_cands;
        if (!g.dos_ordner.empty()) {
            dos_cands.push_back(join_path(g_starter_dir, g.dos_ordner));
            dos_cands.push_back(join_path(g_exe_dir, g.dos_ordner));
        }
        if (!g.resolved_port_dir.empty()) {
            dos_cands.push_back(join_path(g.resolved_port_dir, "..\\DOS"));
            dos_cands.push_back(g.resolved_port_dir);
        }
        dos_cands.push_back(join_path(g_starter_dir, std::string(sub_folder) + "\\DOS"));
        dos_cands.push_back(join_path(g_starter_dir, std::string("trilogie\\Nordlandtrilogie\\") + sub_folder + "\\DOS"));
        dos_cands.push_back(join_path(g_exe_dir, std::string("..\\trilogie\\Nordlandtrilogie\\") + sub_folder + "\\DOS"));
        if (g.teil == 1) {
            dos_cands.push_back(join_path(g_starter_dir, "..\\GitHub\\schicksalsklinge"));
        }
        for (size_t di = 0; di < dos_cands.size(); ++di) {
            if (file_exists_win(join_path(dos_cands[di], "Patch\\patchen.ps1"))) {
                g.resolved_dos_dir = full_win_path(dos_cands[di]);
                g.has_dos_patcher = true;
                break;
            }
        }
        if (g.has_dos_patcher && g.gamedir_valid) {
            const char *prefix = (g.teil == 1) ? "BLADEM" : ((g.teil == 2) ? "SCHWEIF" : "RIVA");
            std::string fw_b = join_path(g.resolved_dos_dir, std::string("Build\\") + prefix + "_FIX_WICHTIG.EXE");
            std::string fa_b = join_path(g.resolved_dos_dir, std::string("Build\\") + prefix + "_FIX_ALLES.EXE");
            std::string marker = join_path(g.resolved_dos_dir, "Build\\gepatchter_spielpfad.txt");
            std::string src_marker = join_path(g.resolved_dos_dir, "Build\\quell_spielpfad.txt");
            if (file_exists_win(fw_b) && file_exists_win(fa_b) && file_exists_win(marker)) {
                std::string patched_dir;
                std::string source_dir;
                FILE *mfp = fopen(utf8_to_win1252(marker).c_str(), "r");
                if (mfp) {
                    char mline[1024] = "";
                    if (fgets(mline, sizeof(mline), mfp)) {
                        patched_dir = trim_str(mline);
                    }
                    fclose(mfp);
                }
                FILE *sfp = fopen(utf8_to_win1252(src_marker).c_str(), "r");
                if (sfp) {
                    char sline[1024] = "";
                    if (fgets(sline, sizeof(sline), sfp)) {
                        source_dir = trim_str(sline);
                    }
                    fclose(sfp);
                }
                if (!patched_dir.empty()) {
                    std::string fw_p = join_path(patched_dir, std::string(prefix) + "_FIX_WICHTIG.EXE");
                    std::string fa_p = join_path(patched_dir, std::string(prefix) + "_FIX_ALLES.EXE");
                    if (file_exists_win(fw_p) && file_exists_win(fa_p)) {
                        if (_stricmp(patched_dir.c_str(), g.resolved_gamedir.c_str()) == 0) {
                            g.dos_fix_exists = true;
                        } else if (!source_dir.empty() && _stricmp(source_dir.c_str(), g.resolved_gamedir.c_str()) == 0) {
                            g.dos_fix_exists = true;
                            g.resolved_dos_copy_dir = patched_dir;
                        }
                    }
                }
            }
        }
    }

    g.has_grafikpaket = false;
    g.resolved_paket_dir.clear();
    g.verfuegbar = (g.has_any_exe && g.gamedir_valid);
}

static void refresh_all_games(void) {
    for (size_t i = 0; i < g_games.size(); ++i) {
        refresh_game_status(g_games[i]);
    }
    if (g_selected_game < 0 || g_selected_game >= (int)g_games.size() || !g_games[g_selected_game].verfuegbar) {
        for (size_t i = 0; i < g_games.size(); ++i) {
            if (g_games[i].verfuegbar) {
                g_selected_game = (int)i;
                break;
            }
        }
    }
}

/* =========================================================================
 * 7. 640x400 Software-Renderer (Farben, Rahmen, Text, Embleme, Dialoge)
 * ========================================================================= */

static inline void put_px(int x, int y, uint32_t col) {
    if (x >= g_clip_x0 && x < g_clip_x1 && y >= g_clip_y0 && y < g_clip_y1) {
        g_fb[y * SCREEN_W + x] = col;
    }
}

static void fill_rect(int x, int y, int w, int h, uint32_t col) {
    int x0 = (x < g_clip_x0) ? g_clip_x0 : x;
    int y0 = (y < g_clip_y0) ? g_clip_y0 : y;
    int x1 = (x + w > g_clip_x1) ? g_clip_x1 : (x + w);
    int y1 = (y + h > g_clip_y1) ? g_clip_y1 : (y + h);
    for (int py = y0; py < y1; ++py) {
        uint32_t *row = &g_fb[py * SCREEN_W];
        for (int px = x0; px < x1; ++px) {
            row[px] = col;
        }
    }
}

static void set_clip_rect(int x, int y, int w, int h) {
    g_clip_x0 = (x < 0) ? 0 : x;
    g_clip_y0 = (y < 0) ? 0 : y;
    g_clip_x1 = (x + w > SCREEN_W) ? SCREEN_W : (x + w);
    g_clip_y1 = (y + h > SCREEN_H) ? SCREEN_H : (y + h);
    if (g_mem_dc) {
        HRGN rgn = CreateRectRgn(g_clip_x0, g_clip_y0, g_clip_x1, g_clip_y1);
        SelectClipRgn(g_mem_dc, rgn);
        DeleteObject(rgn);
    }
}

static void reset_clip_rect(void) {
    g_clip_x0 = 0;
    g_clip_y0 = 0;
    g_clip_x1 = SCREEN_W;
    g_clip_y1 = SCREEN_H;
    if (g_mem_dc) {
        SelectClipRgn(g_mem_dc, NULL);
    }
}

static void fill_gradient_v(int x, int y, int w, int h, uint32_t col_top, uint32_t col_bot) {
    if (h <= 0 || w <= 0) return;
    int r0 = (col_top >> 16) & 0xFF, g0 = (col_top >> 8) & 0xFF, b0 = col_top & 0xFF;
    int r1 = (col_bot >> 16) & 0xFF, g1 = (col_bot >> 8) & 0xFF, b1 = col_bot & 0xFF;
    for (int i = 0; i < h; ++i) {
        int r = r0 + ((r1 - r0) * i) / (h > 1 ? h - 1 : 1);
        int g = g0 + ((g1 - g0) * i) / (h > 1 ? h - 1 : 1);
        int b = b0 + ((b1 - b0) * i) / (h > 1 ? h - 1 : 1);
        fill_rect(x, y + i, w, 1, 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b);
    }
}

static void draw_border_1px(int x, int y, int w, int h, uint32_t col) {
    if (w <= 0 || h <= 0) return;
    fill_rect(x, y, w, 1, col);
    fill_rect(x, y + h - 1, w, 1, col);
    fill_rect(x, y, 1, h, col);
    fill_rect(x + w - 1, y, 1, h, col);
}

static void draw_rect_border(int x, int y, int w, int h, uint32_t col_outer, uint32_t col_inner) {
    fill_rect(x, y, w, 1, col_outer);
    fill_rect(x, y + h - 1, w, 1, col_outer);
    fill_rect(x, y, 1, h, col_outer);
    fill_rect(x + w - 1, y, 1, h, col_outer);
    if (w > 4 && h > 4 && col_inner != col_outer) {
        fill_rect(x + 1, y + 1, w - 2, 1, col_inner);
        fill_rect(x + 1, y + h - 2, w - 2, 1, col_inner);
        fill_rect(x + 1, y + 1, 1, h - 2, col_inner);
        fill_rect(x + w - 2, y + 1, 1, h - 2, col_inner);
    }
}

static void draw_corner_ornaments(int x, int y, int w, int h, uint32_t col) {
    int sz = 5;
    fill_rect(x, y, sz, 2, col);
    fill_rect(x, y, 2, sz, col);
    fill_rect(x + w - sz, y, sz, 2, col);
    fill_rect(x + w - 2, y, 2, sz, col);
    fill_rect(x, y + h - 2, sz, 2, col);
    fill_rect(x, y + h - sz, 2, sz, col);
    fill_rect(x + w - sz, y + h - 2, sz, 2, col);
    fill_rect(x + w - 2, y + h - sz, 2, sz, col);
}

enum StarterFontIndex {
    FONT_12      = 0,
    FONT_13      = 1,
    FONT_14      = 2,
    FONT_15      = 3,
    FONT_15_SEMI = 4,
    FONT_23_BOLD = 5,
    FONT_27_BOLD = 6,
    FONT_12_BOLD = 7,
    FONT_13_BOLD = 8
};

static void apply_font_variant(int variant);
static int  g_font_variant        = 4;
static int  g_loaded_font_variant = 0;

static void init_gdi_font_canvas(void) {
    if (!g_mem_dc) {
        HDC screen_dc = GetDC(NULL);
        g_mem_dc = CreateCompatibleDC(screen_dc);
        if (screen_dc) ReleaseDC(NULL, screen_dc);

        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = SCREEN_W;
        bmi.bmiHeader.biHeight      = -SCREEN_H; /* Top-Down DIB */
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void *bits = NULL;
        g_dib_bmp = CreateDIBSection(g_mem_dc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
        if (g_dib_bmp && bits) {
            SelectObject(g_mem_dc, g_dib_bmp);
            g_fb = (uint32_t *)bits;
        } else {
            g_fb = g_fb_fallback;
        }
        SetBkMode(g_mem_dc, TRANSPARENT);
    }
    if (g_loaded_font_variant != g_font_variant) {
        apply_font_variant(g_font_variant);
    }
}

static void apply_font_variant(int variant) {
    if (variant < 1 || variant > 6) variant = 4;
    g_font_variant = variant;
    g_loaded_font_variant = variant;

    static const int heights[9] = { -12, -13, -14, -15, -15, -23, -27, -12, -13 };
    static const int weights[9] = { FW_NORMAL, FW_NORMAL, FW_NORMAL, FW_NORMAL, FW_SEMIBOLD, FW_BOLD, FW_BOLD, FW_BOLD, FW_BOLD };

    for (int i = 0; i < 9; ++i) {
        if (g_fonts[i]) {
            DeleteObject(g_fonts[i]);
            g_fonts[i] = NULL;
        }
        g_fonts[i] = CreateFontW(
            heights[i], 0, 0, 0, weights[i],
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe UI"
        );
        if (!g_fonts[i]) {
            g_fonts[i] = CreateFontW(
                heights[i], 0, 0, 0, weights[i],
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS,
                L"Segoe UI"
            );
        }
    }
}

static int utf8_to_wstr(const char *text, wchar_t *wbuf, int cap) {
    if (!text || !text[0] || cap <= 1) {
        if (wbuf && cap > 0) wbuf[0] = 0;
        return 0;
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf, cap);
    if (n <= 0) {
        n = MultiByteToWideChar(1252, 0, text, -1, wbuf, cap);
    }
    if (n > 0) return n - 1;
    wbuf[0] = 0;
    return 0;
}

static HFONT select_font_idx(int font_idx) {
    init_gdi_font_canvas();
    if (font_idx < 0) font_idx = 0;
    if (font_idx > 8) font_idx = 8;
    return g_fonts[font_idx];
}

/* Zeichnet das 14x14 Goldsack-Symbol (💰, U+1F4B0) fuer Exploits direkt in den Framebuffer */
static void draw_moneybag_icon(int x0, int y0) {
    /* 0=transparent, 1=dunkler Rand/Dollar (#5A3A10), 2=Braun/Schnur (#A86420), 3=Gold (#E5A938), 4=Glanz (#FBE078) */
    static const uint8_t bag[14][14] = {
        {0,0,0,0,1,3,4,3,3,1,0,0,0,0},
        {0,0,0,0,0,1,3,4,1,0,0,0,0,0},
        {0,0,0,0,1,2,2,2,2,1,0,0,0,0},
        {0,0,0,1,3,4,3,3,3,3,1,0,0,0},
        {0,0,1,3,4,4,3,1,3,3,3,1,0,0},
        {0,1,3,4,4,3,1,1,1,3,3,3,1,0},
        {0,1,3,4,3,1,3,1,0,3,3,3,1,0},
        {1,3,3,3,3,3,1,1,1,3,3,3,3,1},
        {1,3,3,3,3,3,0,1,3,1,3,3,3,1},
        {1,3,3,3,3,1,1,1,1,3,3,3,3,1},
        {1,3,3,3,3,3,3,1,3,3,3,3,3,1},
        {0,1,3,3,3,3,3,3,3,3,3,3,1,0},
        {0,0,1,3,3,3,3,3,3,3,3,1,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,0,0,0}
    };
    static const uint32_t pal[5] = {
        0x00000000u,
        0xFF4A2E08u,
        0xFFA85E18u,
        0xFFE6A832u,
        0xFFFCE484u
    };
    for (int dy = 0; dy < 14; ++dy) {
        for (int dx = 0; dx < 14; ++dx) {
            uint8_t c = bag[dy][dx];
            if (c > 0) {
                put_px(x0 + dx, y0 + dy, pal[c]);
            }
        }
    }
}

static int text_width_px_no_icon(const char *text, int font_idx) {
    if (!text || !text[0]) return 0;
    HFONT hf = select_font_idx(font_idx);
    if (g_mem_dc && hf) {
        SelectObject(g_mem_dc, hf);
        wchar_t wbuf[1024];
        int wlen = utf8_to_wstr(text, wbuf, 1024);
        SIZE sz = {0, 0};
        if (wlen > 0 && GetTextExtentPoint32W(g_mem_dc, wbuf, wlen, &sz)) {
            return (int)sz.cx;
        }
    }
    return (int)strlen(text) * 8;
}

static int text_width_px(const char *text, int font_idx = FONT_14) {
    if (!text || !text[0]) return 0;
    const char *bag_utf8 = "\xF0\x9F\x92\xB0";
    const char *found = strstr(text, bag_utf8);
    if (!found) {
        return text_width_px_no_icon(text, font_idx);
    }
    int total = 0;
    const char *cur = text;
    while ((found = strstr(cur, bag_utf8)) != NULL) {
        if (found > cur) {
            std::string seg(cur, (size_t)(found - cur));
            total += text_width_px_no_icon(seg.c_str(), font_idx);
        }
        total += 15;
        cur = found + 4;
    }
    if (*cur) {
        total += text_width_px_no_icon(cur, font_idx);
    }
    return total;
}

static int text_height_px(int font_idx = FONT_14) {
    HFONT hf = select_font_idx(font_idx);
    if (g_mem_dc && hf) {
        SelectObject(g_mem_dc, hf);
        SIZE sz = {0, 0};
        if (GetTextExtentPoint32W(g_mem_dc, L"Äg", 2, &sz)) {
            return (int)sz.cy;
        }
    }
    return 18;
}

static void draw_text_no_icon(int x, int y, const char *text, uint32_t col, int font_idx, bool shadow) {
    if (!text || !text[0]) return;
    HFONT hf = select_font_idx(font_idx);
    if (g_mem_dc && hf) {
        SelectObject(g_mem_dc, hf);
        wchar_t wbuf[1024];
        int wlen = utf8_to_wstr(text, wbuf, 1024);
        if (wlen <= 0) return;

        SIZE sz = {0, 0};
        GetTextExtentPoint32W(g_mem_dc, wbuf, wlen, &sz);

        SetBkMode(g_mem_dc, TRANSPARENT);
        if (shadow) {
            SetTextColor(g_mem_dc, RGB(6, 8, 14));
            TextOutW(g_mem_dc, x + 1, y + 1, wbuf, wlen);
        }
        int r = (int)((col >> 16) & 0xFF);
        int g = (int)((col >> 8)  & 0xFF);
        int b = (int)(col         & 0xFF);
        SetTextColor(g_mem_dc, RGB(r, g, b));
        TextOutW(g_mem_dc, x, y, wbuf, wlen);

        /* Alpha-Byte auf 0xFF setzen fuer SDL_PIXELFORMAT_ARGB8888 */
        int x0 = (x < g_clip_x0) ? g_clip_x0 : x;
        int y0 = (y < g_clip_y0) ? g_clip_y0 : y;
        int x1 = x + (int)sz.cx + 3;
        int y1 = y + (int)sz.cy + 3;
        if (x1 > g_clip_x1) x1 = g_clip_x1;
        if (y1 > g_clip_y1) y1 = g_clip_y1;
        for (int py = y0; py < y1; ++py) {
            uint32_t *row = &g_fb[py * SCREEN_W];
            for (int px = x0; px < x1; ++px) {
                row[px] |= 0xFF000000u;
            }
        }
    }
}

static void draw_text(int x, int y, const char *text, uint32_t col, int font_idx = FONT_14, bool shadow = false) {
    if (!text || !text[0]) return;
    const char *bag_utf8 = "\xF0\x9F\x92\xB0";
    const char *found = strstr(text, bag_utf8);
    if (!found) {
        draw_text_no_icon(x, y, text, col, font_idx, shadow);
        return;
    }
    int cur_x = x;
    int th = text_height_px(font_idx);
    const char *cur = text;
    while ((found = strstr(cur, bag_utf8)) != NULL) {
        if (found > cur) {
            std::string seg(cur, (size_t)(found - cur));
            draw_text_no_icon(cur_x, y, seg.c_str(), col, font_idx, shadow);
            cur_x += text_width_px_no_icon(seg.c_str(), font_idx);
        }
        draw_moneybag_icon(cur_x, y + (th - 14) / 2);
        cur_x += 15;
        cur = found + 4;
    }
    if (*cur) {
        draw_text_no_icon(cur_x, y, cur, col, font_idx, shadow);
    }
}

static void draw_text_centered(int x, int y, int w, const char *text, uint32_t col, int font_idx = FONT_14, bool shadow = false) {
    int tw = text_width_px(text, font_idx);
    int tx = x + (w - tw) / 2;
    draw_text(tx, y, text, col, font_idx, shadow);
}

static uint32_t lighten_color(uint32_t c, int delta) {
    int r = (int)((c >> 16) & 0xFF) + delta;
    int g = (int)((c >> 8)  & 0xFF) + delta;
    int b = (int)(c         & 0xFF) + delta;
    if (r > 255) r = 255; else if (r < 0) r = 0;
    if (g > 255) g = 255; else if (g < 0) g = 0;
    if (b > 255) b = 255; else if (b < 0) b = 0;
    return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

/* Antialias-Masken fuer die 4 UI-Symbole aus Starter_Entwurf_Etappe6.png (Zahnrad, Haken, Pfeil, Dreieck) */
static const uint8_t g_mask_gear_14x14[14][14] = {
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0, 34, 15,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0, 20, 62,  0,235,119,  4, 73,  0,  0,  0,  0},
    {  0,  0,  0,140,247,196,255,236,191,252, 29,  0,  0,  0},
    {  0,  0, 14, 88,236, 97, 22, 48,164,223,  8, 12,  0,  0},
    {  0, 14,248,246, 66,  0,  0,  0,  0,183,246,148,  0,  0},
    {  0,  9,125,224,  0, 37,233,158,  0, 87,221, 52,  0,  0},
    {  0,  3,116,223,  0, 38,244,167,  0, 86,220, 38,  0,  0},
    {  0, 21,251,255, 73,  0, 16,  2,  0,190,255,158,  0,  0},
    {  0,  0, 24,101,236, 98, 23, 49,164,227, 24, 14,  0,  0},
    {  0,  0,  0,136,247,196,255,236,207,250, 28,  0,  0,  0},
    {  0,  0,  0, 20, 62,  0,236,120,  7, 75,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0, 61, 28,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

static const uint8_t g_mask_check_13x13[13][13] = {
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0, 10,105,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0, 12,192,252, 55,  0,  0},
    {  0,  0,  0, 68,  0,  0, 12,193,252, 93,  0,  0,  0},
    {  0,  0, 80,255,131, 14,194,253, 93,  0,  0,  0,  0},
    {  0,  0,  5,175,255,223,252, 96,  0,  0,  0,  0,  0},
    {  0,  0,  0,  5,173,252, 96,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  5, 67,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

static const uint8_t g_mask_dd_arrow_10x10[10][10] = {
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    { 13, 40, 40, 40, 40, 40, 40, 24,  0,  0},
    { 24,244,255,255,255,255,255, 86,  0,  0},
    {  0,141,255,255,255,255,211,  2,  0,  0},
    {  0, 24,244,255,255,255, 86,  0,  0,  0},
    {  0,  0,141,255,255,211,  2,  0,  0,  0},
    {  0,  0, 24,244,255, 86,  0,  0,  0,  0},
    {  0,  0,  0,141,211,  2,  0,  0,  0,  0},
    {  0,  0,  0, 24, 80,  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

static const uint8_t g_mask_play_14x14[14][14] = {
    {  0,  0,  0,235,107,  4,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,255,255,212, 70,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,255,255,255,255,179, 40,  0,  0,  0,  0,  0},
    {  0,  0,  0,255,255,255,255,255,249,143, 17,  0,  0,  0},
    {  0,  0,  0,255,255,255,255,255,255,255,235,107,  4,  0},
    {  0,  0,  0,255,255,255,255,255,255,255,255,255,212, 70},
    {  0,  0,  0,255,255,255,255,255,255,255,255,255,255,255},
    {  0,  0,  0,255,255,255,255,255,255,255,255,255,218, 74},
    {  0,  0,  0,255,255,255,255,255,255,255,241,116,  7,  0},
    {  0,  0,  0,255,255,255,255,255,253,157, 23,  0,  0,  0},
    {  0,  0,  0,255,255,255,255,198, 53,  0,  0,  0,  0,  0},
    {  0,  0,  0,255,255,230, 92,  1,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0,247,134, 14,  0,  0,  0,  0,  0,  0,  0,  0},
    {  0,  0,  0, 36,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

static void draw_alpha_mask(int x0, int y0, int w, int h, const uint8_t *mask, uint32_t fg_col) {
    int fr = (int)((fg_col >> 16) & 0xFF);
    int fg = (int)((fg_col >> 8)  & 0xFF);
    int fb = (int)(fg_col         & 0xFF);
    for (int dy = 0; dy < h; ++dy) {
        int py = y0 + dy;
        if ((unsigned)py >= SCREEN_H) continue;
        uint32_t *row = &g_fb[py * SCREEN_W];
        for (int dx = 0; dx < w; ++dx) {
            int px = x0 + dx;
            if ((unsigned)px >= SCREEN_W) continue;
            int a = (int)mask[dy * w + dx];
            if (a <= 0) continue;
            if (a >= 255) {
                row[px] = 0xFF000000u | ((uint32_t)fr << 16) | ((uint32_t)fg << 8) | (uint32_t)fb;
            } else {
                uint32_t bg = row[px];
                int br = (int)((bg >> 16) & 0xFF);
                int bg_g = (int)((bg >> 8) & 0xFF);
                int bb = (int)(bg & 0xFF);
                int rr = (fr * a + br * (255 - a)) / 255;
                int rg = (fg * a + bg_g * (255 - a)) / 255;
                int rb = (fb * a + bb * (255 - a)) / 255;
                row[px] = 0xFF000000u | ((uint32_t)rr << 16) | ((uint32_t)rg << 8) | (uint32_t)rb;
            }
        }
    }
}

/* Zeichnet das 15x15 Status-Abzeichen (.ok b aus Starter_Entwurf_Etappe6.html):
 * ok == true  -> #1f5a36 mit 1px Rahmen #4caf72 und Haken #ccffee
 * ok == false -> #4e2428 mit 1px Rahmen #8e4a50 und Kreuz #ffcccc
 */
static void draw_status_badge(int x, int y, bool ok) {
    if (ok) {
        fill_rect(x, y, 15, 15, 0xFF1F5A36u);
        draw_border_1px(x, y, 15, 15, 0xFF4CAF72u);
        draw_alpha_mask(x + 1, y + 1, 13, 13, &g_mask_check_13x13[0][0], 0xFFCCFFEEu);
    } else {
        fill_rect(x, y, 15, 15, 0xFF4E2428u);
        draw_border_1px(x, y, 15, 15, 0xFF8E4A50u);
        for (int i = 0; i < 7; ++i) {
            put_px(x + 4 + i, y + 4 + i, 0xFFFFCCCCu);
            put_px(x + 10 - i, y + 4 + i, 0xFFFFCCCCu);
        }
    }
}

static void draw_checkbox_16(int x, int y, bool checked, bool hovered, bool dimmed = false) {
    if (dimmed) {
        if (checked) {
            fill_rect(x, y, 16, 16, 0xFF173326u);
            draw_border_1px(x, y, 16, 16, 0xFF2D5940u);
            draw_alpha_mask(x + 1, y + 2, 13, 13, &g_mask_check_13x13[0][0], 0xFF6E9E82u);
        } else {
            fill_rect(x, y, 16, 16, 0xFF121722u);
            draw_border_1px(x, y, 16, 16, 0xFF2E3646u);
        }
        return;
    }
    if (checked) {
        uint32_t bg = hovered ? lighten_color(0xFF1F5A36u, 16) : 0xFF1F5A36u;
        fill_rect(x, y, 16, 16, bg);
        draw_border_1px(x, y, 16, 16, 0xFF4CAF72u);
        draw_alpha_mask(x + 1, y + 2, 13, 13, &g_mask_check_13x13[0][0], 0xFFCCFFEEu);
    } else {
        uint32_t bg = hovered ? lighten_color(0xFF10151Fu, 14) : 0xFF10151Fu;
        fill_rect(x, y, 16, 16, bg);
        draw_border_1px(x, y, 16, 16, 0xFF4A5568u);
    }
}

static std::string clip_text_left(const std::string &s, size_t max_chars) {
    if (s.size() <= max_chars) return s;
    if (max_chars <= 4) return s.substr(0, max_chars);
    return "..." + s.substr(s.size() - (max_chars - 3));
}

static std::string clip_text_px_left(const std::string &s, int max_px, int font_idx = FONT_13) {
    if (text_width_px(s.c_str(), font_idx) <= max_px) return s;
    for (size_t cut = 1; cut < s.size(); ++cut) {
        std::string cand = "..." + s.substr(cut);
        if (text_width_px(cand.c_str(), font_idx) <= max_px) {
            return cand;
        }
    }
    return "...";
}

static void draw_game_emblem(int x, int y, int teil, bool active) {
    /* 72x72 Emblem als Fallback in der linken 250px-Bildspalte, falls noch keine Spieldaten vorliegen */
    uint32_t bg_top = (teil == 1) ? (active ? 0xFF381A1Au : 0xFF28181Au)
                    : ((teil == 2) ? (active ? 0xFF16283Eu : 0xFF16202Eu)
                                   : (active ? 0xFF2A2418u : 0xFF221E18u));
    uint32_t bg_bot = (teil == 1) ? (active ? 0xFF1C0D0Du : 0xFF160E10u)
                    : ((teil == 2) ? (active ? 0xFF0C1422u : 0xFF0E131Cu)
                                   : (active ? 0xFF16120Cu : 0xFF12100Cu));
    uint32_t border = (teil == 1) ? (active ? 0xFFC85A4Cu : 0xFF7A4440u)
                    : ((teil == 2) ? (active ? 0xFF5A9CD6u : 0xFF466888u)
                                   : (active ? 0xFFC89A48u : 0xFF786238u));
    fill_gradient_v(x, y, 72, 72, bg_top, bg_bot);
    draw_border_1px(x, y, 72, 72, border);

    if (teil == 1) {
        uint32_t blade_hi = active ? 0xFFE8EEF5u : 0xFFA8B0BAu;
        uint32_t blade_lo = active ? 0xFF9EB0C4u : 0xFF78808Au;
        uint32_t hilt     = active ? 0xFFDCA84Cu : 0xFF9A7A44u;
        uint32_t gem      = active ? 0xFFE04038u : 0xFFA83832u;
        for (int py = 12; py < 60; ++py) {
            int half = (py < 36) ? 20 : (20 - (py - 36) * 18 / 24);
            if (half > 0) {
                uint32_t sc = active ? 0xFF581E1Eu : 0xFF3E1A1Cu;
                fill_rect(x + 36 - half, y + py, half * 2, 1, sc);
            }
        }
        fill_rect(x + 33, y + 8, 3, 41, blade_hi);
        fill_rect(x + 36, y + 8, 3, 41, blade_lo);
        fill_rect(x + 34, y + 6, 4, 2, blade_hi);
        fill_rect(x + 21, y + 49, 30, 4, hilt);
        fill_rect(x + 33, y + 53, 6, 9, active ? 0xFF7A4420u : 0xFF583824u);
        fill_rect(x + 32, y + 62, 8, 5, hilt);
        fill_rect(x + 34, y + 49, 4, 4, gem);
    } else if (teil == 2) {
        uint32_t star_col = active ? 0xFFD8F0FFu : 0xFF9AC4E2u;
        uint32_t tail_col = active ? 0xFF68B8F8u : 0xFF528AB8u;
        uint32_t mtn_col  = active ? 0xFF243852u : 0xFF1E2C40u;
        for (int px = 5; px < 67; ++px) {
            int peak1 = 54 - abs(px - 24) * 3 / 4;
            int peak2 = 48 - abs(px - 48) * 3 / 4;
            int top = (peak1 < peak2) ? peak1 : peak2;
            if (top < 67) fill_rect(x + px, y + top, 1, 67 - top, mtn_col);
        }
        for (int i = 0; i < 30; ++i) {
            fill_rect(x + 13 + i, y + 34 - i / 2, 4, 3, tail_col);
        }
        fill_rect(x + 45, y + 14, 9, 9, star_col);
        fill_rect(x + 49, y + 9, 2, 19, star_col);
        fill_rect(x + 40, y + 18, 19, 2, star_col);
    } else {
        uint32_t moon_col  = active ? 0xFFEAD080u : 0xFFC4A45Cu;
        uint32_t tower_col = active ? 0xFF3E4452u : 0xFF343844u;
        uint32_t win_col   = active ? 0xFFE8A038u : 0xFFB88234u;
        uint32_t water_col = active ? 0xFF1A3248u : 0xFF182838u;
        fill_rect(x + 48, y + 10, 12, 12, moon_col);
        fill_rect(x + 22, y + 17, 24, 36, tower_col);
        for (int z = 0; z < 3; ++z) {
            fill_rect(x + 22 + z * 9, y + 12, 6, 5, tower_col);
        }
        fill_rect(x + 29, y + 25, 4, 7, win_col);
        fill_rect(x + 37, y + 25, 4, 7, win_col);
        fill_rect(x + 32, y + 38, 6, 8, win_col);
        fill_rect(x + 5, y + 53, 62, 14, water_col);
        for (int wy = 56; wy <= 64; wy += 4) {
            fill_rect(x + 28, y + wy, 14, 2, win_col);
        }
    }
}

static void draw_button(
    int x, int y, int w, int h,
    const char *label,
    bool selected,
    bool enabled,
    bool primary = false,
    bool allow_hover = true,
    int font_idx = FONT_15,
    bool with_play_icon = false
) {
    uint32_t bg, border, tcol;
    if (!enabled) {
        bg     = 0xFF232B3Cu;
        border = 0xFF323A4Au;
        tcol   = 0xFF5D6576u;
    } else if (primary) {
        bg     = 0xFF1F5A36u;
        border = 0xFF4CAF72u;
        tcol   = 0xFFE6E9EFu;
    } else if (selected) {
        bg     = 0xFF3A4A66u;
        border = 0xFF4A5568u;
        tcol   = 0xFFFFFFFFu;
    } else {
        bg     = 0xFF232B3Cu;
        border = 0xFF4A5568u;
        tcol   = 0xFFE6E9EFu;
    }

    if (enabled && allow_hover &&
        g_mouse_x >= x && g_mouse_x < x + w &&
        g_mouse_y >= y && g_mouse_y < y + h) {
        bg   = lighten_color(bg, 16);
        tcol = 0xFFFFFFFFu;
    }

    fill_rect(x, y, w, h, bg);
    draw_border_1px(x, y, w, h, border);

    int use_font = primary ? FONT_15_SEMI : font_idx;
    int th = text_height_px(use_font);
    int ty = y + (h - th) / 2;
    if (with_play_icon && enabled) {
        int tw = text_width_px(label, use_font);
        int total_w = 18 + tw;
        int sx = x + (w - total_w) / 2;
        draw_alpha_mask(sx, y + (h - 14) / 2, 14, 14, &g_mask_play_14x14[0][0], tcol);
        draw_text(sx + 18, ty, label, tcol, use_font, false);
    } else {
        draw_text_centered(x, ty, w, label, tcol, use_font, false);
    }
}

/* =========================================================================
 * 8. Zeichnen der Bildschirme (Etappe 6: Starter_Entwurf_Etappe6.html/.png)
 * ========================================================================= */

/* Exakte Schriftzugfarben aus Starter_Entwurf_Etappe6.html:
 * .c1 .t { color: #e2553f }
 * .c2 .t { color: #5aa9e6 }
 * .c3 .t { color: #e3a63a }
 */
static uint32_t game_logo_color(int teil, bool verfuegbar) {
    (void)verfuegbar;
    if (teil == 1) {
        return 0xFFE2553Fu;
    } else if (teil == 2) {
        return 0xFF5AA9E6u;
    } else {
        return 0xFFE3A63Au;
    }
}

static inline bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

/* Kopfzeile (NACHTRAG 1 Etappe 6: Sprachumschalter entfernt, nur "Hilfe (F1)" rechts oben) */
/* Versionsnummer des Starters (Aenderungen: AENDERUNGEN.md im Paket) */
#define STARTER_VERSION "1.2"

static const int HDR_HELP_X = 905;
static const int HDR_HELP_Y = 29;
static const int HDR_HELP_W = 104;
static const int HDR_HELP_H = 37;

static void render_background_and_header(const char *subtitle, std::string *out_tooltip = NULL) {
    (void)out_tooltip;
    init_gdi_font_canvas();
    bool bg_hover = (!g_help_active && !g_modal_active && !g_fp_active && g_open_dd_game < 0);

    /* Hintergrund --bg: #141a26 */
    fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xFF141A26u);

    /* Kopfzeile .head: x=14, y=10, w=1012, h=74, bg=#181f2d, border=#3a4558 */
    fill_rect(14, 10, 1012, 74, 0xFF181F2Du);
    draw_border_1px(14, 10, 1012, 74, 0xFF3A4558u);

    draw_text(30, 16, "Das Schwarze Auge: Die Nordland-Trilogie", 0xFFE6E9EFu, FONT_27_BOLD, false);
    {
        int hw = text_width_px("Das Schwarze Auge: Die Nordland-Trilogie", FONT_27_BOLD);
        draw_text(30 + hw + 12, 28, "Starter " STARTER_VERSION, 0xFF9AA3B2u, FONT_13, false);
    }
    draw_text(30, 53, subtitle, 0xFF9AA3B2u, FONT_14, false);

    /* Hilfe-Knopf "Hilfe (F1)" */
    draw_button(HDR_HELP_X, HDR_HELP_Y, HDR_HELP_W, HDR_HELP_H, "Hilfe (F1)", g_help_active, true, false, bg_hover, FONT_15, false);
}

static const char *fenster_label_str(const std::string &f) {
    if (f == "2") return "2x (640x480)";
    if (f == "3") return "3x (960x720)";
    if (f == "4") return "4x (1280x960)";
    if (f == "5") return "5x (1600x1200)";
    return L("Vollbild (4:3)", "Fullscreen (4:3)");
}

static const char *fenster_dd_short_label(const std::string &f) {
    if (f == "2") return L("Fenster 2\xC3\x97", "Window 2\xC3\x97");
    if (f == "3") return L("Fenster 3\xC3\x97", "Window 3\xC3\x97");
    if (f == "4") return L("Fenster 4\xC3\x97", "Window 4\xC3\x97");
    if (f == "5") return L("Fenster 5\xC3\x97", "Window 5\xC3\x97");
    return L("Vollbild", "Fullscreen");
}

static const char *skalierung_label_str(const std::string &s) {
    if (_stricmp(s.c_str(), "ganzzahlig") == 0) return L("Ganzzahlig", "Integer");
    return L("Füllend (scharf)", "Fill (sharp)");
}

static const char *stufe_short_label(PatchStufe s) {
    if (s == STUFE_ORIG) return L("Original", "Original");
    if (s == STUFE_WICHTIG) return L("Wichtige Fixes", "Essential Fixes");
    return L("Alle Fixes", "All Fixes");
}

static std::string get_active_exe_name(const GameCatalogEntry &g) {
    if (g.chosen_stufe == STUFE_ORIG) return g.exe_orig;
    if (g.chosen_stufe == STUFE_WICHTIG) return g.exe_wichtig;
    if (!g.exe_alles.empty()) return g.exe_alles;
    return g.exe_standard;
}

static std::string get_active_dos_exe_name(const GameCatalogEntry &g) {
    if (g.chosen_stufe == STUFE_ORIG) {
        return (g.teil == 1) ? "SCHICKM.EXE" : ((g.teil == 2) ? "SCHWEIF.EXE" : "RIVA.EXE");
    }
    const char *prefix = (g.teil == 1) ? "BLADEM" : ((g.teil == 2) ? "SCHWEIF" : "RIVA");
    return std::string(prefix) + ((g.chosen_stufe == STUFE_WICHTIG) ? "_FIX_WICHTIG.EXE" : "_FIX_ALLES.EXE");
}

static bool is_browser_available(const GameCatalogEntry &g) {
    return !g.browser_url.empty() && _stricmp(g.browser_status.c_str(), "verfuegbar") == 0;
}

static std::string build_browser_launch_url(const GameCatalogEntry &g) {
    if (g.browser_url.empty()) return "";
    std::string url = g.browser_url;
    char sep = (url.find('?') == std::string::npos) ? '?' : '&';
    url += sep;
    url += "stufe=";
    url += stufe_to_str(g.chosen_stufe);
    if (!g.chosen_intro) {
        if (g.teil == 1) {
            url += "&intro=0";
        } else {
            url += "&kein_intro=1";
        }
    }
    if (g.chosen_probenlog && g.chosen_stufe != STUFE_ORIG) {
        url += "&probenlog=1";
    }
    return url;
}

static void draw_tooltip_box(int mx, int my, const std::string &txt) {
    if (txt.empty()) return;
    int tw = text_width_px(txt.c_str(), FONT_13) + 20;
    int th = 28;
    int bx = mx + 12;
    int by = my - th - 6;
    if (bx + tw > SCREEN_W - 14) bx = SCREEN_W - 14 - tw;
    if (bx < 14) bx = 14;
    if (by < 10) by = my + 18;
    fill_rect(bx, by, tw, th, 0xFF181F2Du);
    draw_border_1px(bx, by, tw, th, 0xFF4CAF72u);
    draw_text(bx + 10, by + 5, txt.c_str(), 0xFFE6E9EFu, FONT_13, false);
}

/* Titelbild in eigener linker Spalte (250 px breit, volle Karteninnenhoehe 148 px),
 * rechts weicher Verlauf (ab 55 % = dx 137) in den Kartenhintergrund (#1b2232).
 * NICHTS ueberdeckt das Bild mehr. */
static void draw_card_title_image(const GameCatalogEntry &g, int cx, int cy) {
    const int ix = cx + 1;
    const int iy = cy + 1;
    const int dst_w = 250;
    const int dst_h = 148;
    const int fade_start = 137; /* 55 % von 250 px */
    const int src_w = g.title_w;
    const int src_h = g.title_h;
    if (!g.has_title_image || src_w <= 0 || src_h <= 0 || g.title_pixels.size() < (size_t)(src_w * src_h)) {
        return;
    }

    /* 4:3-gerechter Bildausschnitt (oben verankert wie im freigegebenen Entwurf Starter_Entwurf_Etappe6.png) */
    int vis_h = (src_h * 124) / 176;
    if (vis_h <= 0 || vis_h > src_h) vis_h = src_h;

    for (int dy = 0; dy < dst_h; ++dy) {
        int py = iy + dy;
        if ((unsigned)py >= SCREEN_H) continue;
        int sy = (dy * vis_h) / dst_h;
        if (sy >= src_h) sy = src_h - 1;
        const uint32_t *src_row = &g.title_pixels[(size_t)sy * (size_t)src_w];
        uint32_t *dst_row = &g_fb[py * SCREEN_W];

        for (int dx = 0; dx < dst_w; ++dx) {
            int px = ix + dx;
            if ((unsigned)px >= SCREEN_W) continue;
            int sx = (dx * src_w) / dst_w;
            uint32_t scol = src_row[sx];

            int sr = (int)((scol >> 16) & 0xFF);
            int sg = (int)((scol >> 8)  & 0xFF);
            int sb = (int)(scol         & 0xFF);

            int alpha = 226;
            if (dx >= fade_start) {
                int span = dst_w - 1 - fade_start;
                int rem  = dst_w - 1 - dx;
                if (rem <= 0 || span <= 0) {
                    alpha = 0;
                } else {
                    int t = (rem * 256) / span;
                    int smooth = (t * t * (768 - 2 * t)) >> 16;
                    alpha = (226 * smooth) >> 8;
                }
            }

            uint32_t bcol = dst_row[px];
            int br = (int)((bcol >> 16) & 0xFF);
            int bg = (int)((bcol >> 8)  & 0xFF);
            int bb = (int)(bcol         & 0xFF);

            int rr = (sr * alpha + br * (256 - alpha)) >> 8;
            int rg = (sg * alpha + bg * (256 - alpha)) >> 8;
            int rb = (sb * alpha + bb * (256 - alpha)) >> 8;
            dst_row[px] = 0xFF000000u | ((uint32_t)rr << 16) | ((uint32_t)rg << 8) | (uint32_t)rb;
        }
    }
}

/* Geometrie-Struktur fuer Zeile 4 einer Spielkarte (identisch fuer Rendern & Mausklicks) */
struct CardRow4Layout {
    int y;
    int h;
    int fixes_lbl_x;
    int seg_x;
    int seg_w[3];
    int seg_total_w;
    int anz_lbl_x;
    int dd_x;
    int dd_w;
    int chk_intro_x;
    int chk_intro_w;
    int chk_plog_x;
    int chk_plog_w;
    int btn_folder_x;
    int btn_folder_w;
    int btn_gear_x;
    int btn_gear_w;
};

static void compute_card_row4_layout(const GameCatalogEntry &g, int cx, int cy, int cw, CardRow4Layout &L4) {
    const int tx0 = cx + 1 + 250 + 18; /* 283 */
    const int rx  = cx + cw - 1 - 16;  /* 1009 */
    L4.y = cy + 111;
    L4.h = 29;
    L4.fixes_lbl_x = tx0;

    if (!g_lang_en) {
        L4.seg_x    = 319;
        L4.seg_w[0] = 72; /* Original: 319..391 */
        L4.seg_w[1] = 79; /* Wichtige: 391..470 */
        L4.seg_w[2] = 46; /* Alle:     470..516 */
        L4.seg_total_w = L4.seg_w[0] + L4.seg_w[1] + L4.seg_w[2]; /* 197 */
        L4.anz_lbl_x = L4.seg_x + L4.seg_total_w + 10; /* 526 */
        L4.dd_x      = 579;
        L4.dd_w      = (g.chosen_fenster == "vollbild") ? 81 : 104;
    } else {
        int fw = text_width_px("Fixes", FONT_12);
        L4.seg_x    = tx0 + fw + 6;
        L4.seg_w[0] = 72;
        L4.seg_w[1] = 76;
        L4.seg_w[2] = 44;
        L4.seg_total_w = L4.seg_w[0] + L4.seg_w[1] + L4.seg_w[2];
        L4.anz_lbl_x = L4.seg_x + L4.seg_total_w + 10;
        int dw = text_width_px("Display", FONT_12);
        L4.dd_x      = L4.anz_lbl_x + dw + 6;
        L4.dd_w      = (g.chosen_fenster == "vollbild") ? 96 : 108;
    }

    L4.chk_intro_x = L4.dd_x + L4.dd_w + 10;
    L4.chk_intro_w = !g_lang_en ? 52 : (22 + text_width_px("Intro", FONT_14));
    L4.chk_plog_x  = L4.chk_intro_x + L4.chk_intro_w + 10;
    L4.chk_plog_w  = 22 + text_width_px(L("Probenlog", "Roll Log"), FONT_14);

    L4.btn_gear_w   = 35;
    L4.btn_gear_x   = rx - L4.btn_gear_w; /* 974 */
    L4.btn_folder_w = 84;
    L4.btn_folder_x = L4.btn_gear_x - 10 - L4.btn_folder_w; /* 880 */
}

static const char *g_dd_fenster_vals[5] = { "vollbild", "2", "3", "4", "5" };

static int get_dd_fenster_index(const std::string &f) {
    for (int k = 0; k < 5; ++k) {
        if (_stricmp(f.c_str(), g_dd_fenster_vals[k]) == 0) return k;
    }
    return 0;
}

static void get_open_dd_menu_rect(int game_idx, int &out_x, int &out_y, int &out_w, int &out_h) {
    int cx = 14;
    int cy = 147 + game_idx * 162;
    int cw = 1012;
    CardRow4Layout L4;
    compute_card_row4_layout(g_games[game_idx], cx, cy, cw, L4);
    out_x = L4.dd_x;
    out_w = (L4.dd_w >= 116) ? L4.dd_w : 116;
    out_h = 5 * 26 + 2;
    if (game_idx >= 2) {
        /* Auf Karte 3 nach oben aufklappen, damit alle 5 Eintraege komplett im Fenster liegen */
        out_y = L4.y - out_h + 1;
    } else {
        out_y = L4.y + L4.h - 1;
    }
}

static void get_card_fanpatch_rect(const GameCatalogEntry &g, int cx, int cy, int prog_bx, int &out_dot_x, int &out_ver_x, int &out_ver_y, int &out_ver_w, int &out_ver_h) {
    out_dot_x = 0; out_ver_x = 0; out_ver_y = cy + 16; out_ver_w = 0; out_ver_h = 0;
    if (g.version.empty()) return;
    int tx0 = cx + 1 + 250 + 18;
    char title_buf[160];
    snprintf(title_buf, sizeof(title_buf), "%d. %s", g.teil, g.name.c_str());
    int title_w = text_width_px(title_buf, FONT_23_BOLD);
    const char *jahr = (g.teil == 1) ? "1992" : ((g.teil == 2) ? "1994" : "1996");
    int dot_x = tx0 + title_w + 8 + text_width_px(jahr, FONT_13) + 8;
    int dot_w = text_width_px("\xC2\xB7 ", FONT_13);
    int ver_x = dot_x + dot_w;
    int ver_w = text_width_px(g.version.c_str(), FONT_13);
    if (ver_x + ver_w >= prog_bx - 8) return;
    out_dot_x = dot_x;
    out_ver_x = ver_x;
    out_ver_y = cy + 16;
    out_ver_w = ver_w;
    out_ver_h = text_height_px(FONT_13);
}

static void render_screen_select_game(void) {
    std::string active_tooltip;
    std::string tip_override;
    uint32_t    tip_border_col = 0xFF3A4558u;
    uint32_t    tip_text_col   = 0xFFE6E9EFu;
    bool bg_hover = (!g_help_active && !g_modal_active && !g_fp_active && g_open_dd_game < 0);
    bool any_hand_cursor = false;

    render_background_and_header(
        L("Gemeinsamer Starter – jedes Spiel auf seiner eigenen Karte einstellen und starten",
          "Unified Launcher – configure and start each game on its own card"),
        &active_tooltip
    );

    /* ---------------------------------------------------------------------
     * Die 3 Spielkarten (x=14, y=147 + i*162, w=1012, h=150)
     * --------------------------------------------------------------------- */
    for (size_t i = 0; i < g_games.size() && i < 3; ++i) {
        const GameCatalogEntry &g = g_games[i];
        int cx = 14;
        int cy = 147 + (int)i * 162;
        int cw = 1012;
        int ch = 150;
        int tx0 = cx + 1 + 250 + 18; /* 283 */
        int rx  = cx + cw - 1 - 16;  /* 1009 */

        bool is_sel = ((int)i == g_selected_game);

        /* Kartenhintergrund --card: #1b2232 */
        fill_rect(cx, cy, cw, ch, 0xFF1B2232u);

        /* Linke Spalte (250 px breit, volle Kartenhoehe): Titelbild mit Verlauf nach rechts */
        if (g.has_title_image) {
            draw_card_title_image(g, cx, cy);
        } else {
            draw_game_emblem(cx + 89, cy + 39, g.teil, g.verfuegbar || g.gamedir_valid);
        }

        draw_border_1px(cx, cy, cw, ch, 0xFF3A4558u);

        /* Zeile 1: Spieletitel + "DSA Teil X – Jahr", rechts die zwei Haekchen */
        char title_buf[160];
        snprintf(title_buf, sizeof(title_buf), "%d. %s", g.teil, g.name.c_str());
        uint32_t title_col = game_logo_color(g.teil, g.verfuegbar || g.gamedir_valid);
        draw_text(tx0, cy + 7, title_buf, title_col, FONT_23_BOLD, false);

        int title_w = text_width_px(title_buf, FONT_23_BOLD);
        const char *short_sub = (g.teil == 1)
            ? L("DSA Teil 1 – 1992", "DSA Part 1 – 1992")
            : ((g.teil == 2) ? L("DSA Teil 2 – 1994", "DSA Part 2 – 1994")
                             : L("DSA Teil 3 – 1996", "DSA Part 3 – 1996"));
        /* Mit Versionsangabe nur das Jahr (die Teilnummer steht schon vor dem Titel), sonst wie bisher */
        const char *jahr = (g.teil == 1) ? "1992" : ((g.teil == 2) ? "1994" : "1996");
        if (!g.version.empty()) short_sub = jahr;
        draw_text(tx0 + title_w + 8, cy + 16, short_sub, 0xFF9AA3B2u, FONT_13, false);

        /* Rechtsbuendig die zwei Status-Haekchen (Programm gefunden / Spieldaten gefunden) */
        const char *dat_lbl = g.gamedir_valid
            ? L("Spieldaten gefunden", "Game data found")
            : (g.gamedir_wrong_version
                ? L("Falsche Fassung", "Unsupported version")
                : L("Spieldaten fehlen", "Game data missing"));
        const char *prog_lbl = g.has_any_exe
            ? L("Programm gefunden", "Program found")
            : L("Programm fehlt", "Program missing");

        int dat_tw  = text_width_px(dat_lbl, FONT_14);
        int dat_tx  = rx - dat_tw;
        int dat_bx  = dat_tx - 6 - 15;
        int prog_tw = text_width_px(prog_lbl, FONT_14);
        int prog_tx = dat_bx - 16 - prog_tw;
        int prog_bx = prog_tx - 6 - 15;

        draw_status_badge(prog_bx, cy + 16, g.has_any_exe);
        draw_text(prog_tx, cy + 13, prog_lbl, 0xFFE6E9EFu, FONT_14, false);

        draw_status_badge(dat_bx, cy + 16, g.gamedir_valid);
        draw_text(dat_tx, cy + 13, dat_lbl, g.gamedir_wrong_version ? 0xFFFF8870u : 0xFFE6E9EFu, FONT_14, false);

        /* Versionsanzeige hinter "DSA Teil X - Jahr" (nur wenn bis zu den Haekchen Platz ist) */
        bool hov_fanpatch = false;
        if (!g.version.empty()) {
            int dot_x = 0, ver_x = 0, ver_y = 0, ver_w = 0, ver_h = 0;
            get_card_fanpatch_rect(g, cx, cy, prog_bx, dot_x, ver_x, ver_y, ver_w, ver_h);
            if (ver_w > 0) {
                hov_fanpatch = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, ver_x - 3, cy + 12, ver_w + 6, 24);
                draw_text(dot_x, ver_y, "\xC2\xB7", 0xFF9AA3B2u, FONT_13, false);
                uint32_t vcol = hov_fanpatch ? 0xFFFFFFFFu : 0xFF9AA3B2u;
                draw_text(ver_x, ver_y, g.version.c_str(), vcol, FONT_13, false);
                if (hov_fanpatch) {
                    fill_rect(ver_x, ver_y + ver_h - 2, ver_w, 1, 0xFFFFFFFFu);
                    any_hand_cursor = true;
                }
            }
        }

        /* Zeile 2: Spielordner (klein, grau) + auf Karte 1 rechtsbuendig "Fluessig drehen (Schicksalsklinge)" */
        const char *drehen_lbl = "Fl\xC3\xBCssig drehen (Schicksalsklinge)";
        int chk_drehen_w = (g.teil == 1) ? (22 + text_width_px(drehen_lbl, FONT_13)) : 0;
        int chk_drehen_x = rx - chk_drehen_w;
        int chk_drehen_y = cy + 38;
        int chk_drehen_h = 22;

        std::string path_info;
        uint32_t path_col = 0xFF9AA3B2u;
        int path_avail_w = (rx - tx0) - ((g.teil == 1) ? (chk_drehen_w + 14) : 0);
        if (g.gamedir_wrong_version) {
            path_info = clip_text_px_left(L(g.wrong_version_msg_de.c_str(), g.wrong_version_msg_en.c_str()), path_avail_w, FONT_13);
            path_col = 0xFFFF8870u;
        } else if (g.gamedir_valid) {
            std::string p_prefix = L("Spielordner: ", "Game folder: ");
            int max_path_px = path_avail_w - text_width_px(p_prefix.c_str(), FONT_13);
            if (max_path_px < 160) max_path_px = 160;
            path_info = p_prefix + clip_text_px_left(g.resolved_gamedir, max_path_px, FONT_13);
        } else {
            path_info = clip_text_px_left(
                L("Spielordner: Noch nicht gewählt – bitte Ordner ins Fenster ziehen oder [Ordner …] klicken.",
                  "Game folder: Not set yet – please drag & drop folder into window or click [Folder …]."),
                path_avail_w, FONT_13);
        }
        draw_text(tx0, cy + 41, path_info.c_str(), path_col, FONT_13, false);

        /* Zeile 3: Die drei grossen Knoepfe (235, 236, 235 px mit 10 px Abstand) */
        int btn_y = cy + 66;
        int btn_h = 37;
        bool hov_win_btn = bg_hover && g.has_any_exe && point_in_rect(g_mouse_x, g_mouse_y, tx0, btn_y, 235, btn_h);
        bool hov_dos_btn = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, tx0 + 245, btn_y, 236, btn_h);
        bool hov_web_btn = bg_hover && is_browser_available(g) && point_in_rect(g_mouse_x, g_mouse_y, tx0 + 491, btn_y, 235, btn_h);

        /* Etappe 7 (E3): Wenn der Mauszeiger (oder Tab-Fokus) auf "Im Browser spielen" liegt,
         * werden die fuer den Browser nicht vom Starter steuerbaren Optionen (Anzeige & lokales Probenlog)
         * sanft abgedimmt, ohne das Layout zu verschieben. Bei Fixes="Original" ist Probenlog ebenfalls
         * abgedimmt, da die unveraenderte Original-Engine (1992-1996) kein Probenlog enthaelt. */
        bool web_way_active = hov_web_btn || (is_sel && g_show_focus_ring && !g_help_active && !g_modal_active && g_card_focus == 8);
        bool dim_anzeige    = web_way_active;
        bool dim_probenlog  = (g.chosen_stufe == STUFE_ORIG) || web_way_active;
        bool hov_drehen     = false;
        if (g.teil == 1) {
            bool dim_drehen = (g.chosen_stufe == STUFE_ORIG) || web_way_active;
            hov_drehen = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, chk_drehen_x, chk_drehen_y, chk_drehen_w, chk_drehen_h);
            draw_checkbox_16(chk_drehen_x, cy + 40, g.chosen_drehen, hov_drehen, dim_drehen);
            draw_text(chk_drehen_x + 22, cy + 41, drehen_lbl,
                      dim_drehen ? 0xFF636C7Eu : 0xFFE6E9EFu, FONT_13, false);
        }

        if (!g.has_any_exe) {
            draw_button(tx0, btn_y, 235, btn_h, L("Kommt bald", "Coming soon"), false, false, false, false, FONT_15, false);
        } else {
            draw_button(tx0, btn_y, 235, btn_h, L("Spielen (Windows)", "Play (Windows)"),
                        false, true, true, bg_hover, FONT_15_SEMI, true);
        }

        const char *dos_btn_lbl = g.dos_fix_exists
            ? L("In DOSBox spielen", "Play in DOSBox")
            : L("DOS-Fassung patchen", "Patch DOS Version");
        draw_button(tx0 + 245, btn_y, 236, btn_h, dos_btn_lbl,
                    false, true, false, bg_hover, FONT_15, false);

        if (is_browser_available(g)) {
            draw_button(tx0 + 491, btn_y, 235, btn_h, L("Im Browser spielen", "Play in Browser"),
                        false, true, false, bg_hover, FONT_15, false);
        } else {
            draw_button(tx0 + 491, btn_y, 235, btn_h, L("Im Browser spielen", "Play in Browser"),
                        false, false, false, false, FONT_15, false);
        }

        /* Zeile 4: Optionen in exakt vorgegebener Reihenfolge:
         * 1. "Fixes" als Segmentschalter [Original | Wichtige | Alle]
         * 2. "Anzeige" als Klappliste (Vollbild / Fenster 2x..5x)
         * 3. Haekchen "Intro"
         * 4. Haekchen "Probenlog"
         * 5. Rechtsbuendig "Ordner …" und Zahnrad (Einstellungen) */
        CardRow4Layout L4;
        compute_card_row4_layout(g, cx, cy, cw, L4);

        /* 4.1 "Fixes" + Segmentschalter [Original | Wichtige | Alle] */
        draw_text(L4.fixes_lbl_x, L4.y + 6, "Fixes", 0xFF9AA3B2u, FONT_12, false);

        const char *seg_labels[3] = {
            L("Original", "Original"),
            L("Wichtige", "Essential"),
            L("Alle", "All")
        };
        PatchStufe seg_stufen[3] = { STUFE_ORIG, STUFE_WICHTIG, STUFE_ALLES };
        int cur_sx = L4.seg_x;
        int seg_th = text_height_px(FONT_14);
        int seg_ty = L4.y + (L4.h - seg_th) / 2;

        for (int sk = 0; sk < 3; ++sk) {
            int sw = L4.seg_w[sk];
            bool active_seg = (g.chosen_stufe == seg_stufen[sk]);
            bool hov_seg    = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, cur_sx, L4.y, sw, L4.h);
            uint32_t sbg = active_seg ? 0xFF3A4A66u : (hov_seg ? lighten_color(0xFF232B3Cu, 16) : 0xFF232B3Cu);
            uint32_t sfg = active_seg ? 0xFFFFFFFFu : (hov_seg ? 0xFFE6E9EFu : 0xFF9AA3B2u);
            fill_rect(cur_sx, L4.y, sw, L4.h, sbg);
            if (sk > 0) {
                fill_rect(cur_sx, L4.y, 1, L4.h, 0xFF4A5568u);
            }
            draw_text_centered(cur_sx, seg_ty, sw, seg_labels[sk], sfg, FONT_14, false);
            cur_sx += sw;
        }
        draw_border_1px(L4.seg_x, L4.y, L4.seg_total_w, L4.h, 0xFF4A5568u);

        /* 4.2 "Anzeige" + Klappliste (abgedimmt, wenn Maus ueber "Im Browser spielen" schwebt) */
        draw_text(L4.anz_lbl_x, L4.y + 6, L("Anzeige", "Display"),
                  dim_anzeige ? 0xFF555E6Eu : 0xFF9AA3B2u, FONT_12, false);

        bool dd_is_open = (g_open_dd_game == (int)i);
        bool hov_dd     = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, L4.dd_x, L4.y, L4.dd_w, L4.h);
        uint32_t dd_bg  = dim_anzeige
            ? 0xFF19202Eu
            : (dd_is_open ? 0xFF3A4A66u : (hov_dd ? lighten_color(0xFF232B3Cu, 16) : 0xFF232B3Cu));
        uint32_t dd_brd = dim_anzeige ? 0xFF2E3646u : (dd_is_open ? 0xFF5AA9E6u : 0xFF4A5568u);
        uint32_t dd_fg  = dim_anzeige ? 0xFF636C7Eu : 0xFFE6E9EFu;
        fill_rect(L4.dd_x, L4.y, L4.dd_w, L4.h, dd_bg);
        draw_border_1px(L4.dd_x, L4.y, L4.dd_w, L4.h, dd_brd);
        draw_text(L4.dd_x + 10, seg_ty, fenster_dd_short_label(g.chosen_fenster), dd_fg, FONT_14, false);
        draw_alpha_mask(L4.dd_x + L4.dd_w - 18, L4.y + 10, 10, 10, &g_mask_dd_arrow_10x10[0][0], dd_fg);

        /* 4.3 Haekchen "Intro" */
        bool hov_intro = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, L4.chk_intro_x, L4.y, L4.chk_intro_w, L4.h);
        draw_checkbox_16(L4.chk_intro_x, L4.y + 7, g.chosen_intro, hov_intro, false);
        draw_text(L4.chk_intro_x + 22, seg_ty, "Intro", 0xFFE6E9EFu, FONT_14, false);

        /* 4.4 Haekchen "Probenlog" (abgedimmt bei Fixes="Original" sowie bei Maus ueber "Im Browser spielen") */
        bool hov_plog = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, L4.chk_plog_x, L4.y, L4.chk_plog_w, L4.h);
        draw_checkbox_16(L4.chk_plog_x, L4.y + 7, g.chosen_probenlog, hov_plog, dim_probenlog);
        draw_text(L4.chk_plog_x + 22, seg_ty, L("Probenlog", "Roll Log"),
                  dim_probenlog ? 0xFF636C7Eu : 0xFFE6E9EFu, FONT_14, false);

        /* 4.5 Rechtsbuendig "Ordner …" und Zahnrad (Einstellungen) */
        draw_button(L4.btn_folder_x, L4.y, L4.btn_folder_w, L4.h,
                    L("Ordner \xE2\x80\xA6", "Folder \xE2\x80\xA6"),
                    false, true, false, bg_hover, FONT_14, false);

        bool hov_gear = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, L4.btn_gear_x, L4.y, L4.btn_gear_w, L4.h);
        uint32_t gear_bg = hov_gear ? lighten_color(0xFF232B3Cu, 16) : 0xFF232B3Cu;
        fill_rect(L4.btn_gear_x, L4.y, L4.btn_gear_w, L4.h, gear_bg);
        draw_border_1px(L4.btn_gear_x, L4.y, L4.btn_gear_w, L4.h, 0xFF4A5568u);
        draw_alpha_mask(L4.btn_gear_x + 11, L4.y + 7, 14, 14, &g_mask_gear_14x14[0][0], hov_gear ? 0xFFFFFFFFu : 0xFFE6E9EFu);

        /* Tastatur-Fokusrahmen (wenn per Tab navigiert wird) */
        if (is_sel && g_show_focus_ring && !g_help_active && !g_modal_active) {
            int fx = 0, fy = L4.y, fw = 0, fh = L4.h;
            if (g_card_focus == 0)      { fx = L4.seg_x; fw = L4.seg_total_w; }
            else if (g_card_focus == 1) { fx = L4.dd_x; fw = L4.dd_w; }
            else if (g_card_focus == 2) { fx = L4.chk_intro_x - 2; fy = L4.y + 4; fw = L4.chk_intro_w + 4; fh = 22; }
            else if (g_card_focus == 3) { fx = L4.chk_plog_x - 2;  fy = L4.y + 4; fw = L4.chk_plog_w + 4;  fh = 22; }
            else if (g_card_focus == 4) { fx = L4.btn_folder_x; fw = L4.btn_folder_w; }
            else if (g_card_focus == 5) { fx = L4.btn_gear_x; fw = L4.btn_gear_w; }
            else if (g_card_focus == 6) { fx = tx0; fy = btn_y; fw = 235; fh = btn_h; }
            else if (g_card_focus == 7) { fx = tx0 + 245; fy = btn_y; fw = 236; fh = btn_h; }
            else if (g_card_focus == 8) { fx = tx0 + 491; fy = btn_y; fw = 235; fh = btn_h; }
            if (fw > 0) {
                draw_border_1px(fx - 1, fy - 1, fw + 2, fh + 2, 0xFF5AA9E6u);
            }
        }

        /* Tooltips & ruhiger Hinweis im oberen Info-Balken (Etappe 7 E3) */
        if (bg_hover) {
            bool hov_fixes = point_in_rect(g_mouse_x, g_mouse_y, L4.seg_x, L4.y, L4.seg_total_w, L4.h);
            bool hov_prog  = point_in_rect(g_mouse_x, g_mouse_y, prog_bx, cy + 10, prog_tw + 24, 24);
            if (hov_web_btn) {
                active_tooltip = "Browser: Fixes & Intro werden übergeben – Anzeige (Vollbild-Knopf / F11) & Probenlog direkt auf der Webseite";
                tip_override   = "Im Browser spielen: Übergibt Fixes & Intro an die Webseite – Anzeige (Vollbild / F11) & Probenlog direkt im Browser.";
                tip_border_col = 0xFF5AA9E6u;
            } else if (hov_dos_btn) {
                if (g.dos_fix_exists) {
                    active_tooltip = std::string("DOSBox (") + get_active_dos_exe_name(g) + "): Startet mit Fixes, Anzeige (4:3), Intro" +
                                     (g.chosen_stufe == STUFE_ORIG ? " (Probenlog nur bei Wichtige/Alle)" : " & Probenlog");
                    tip_override   = "In DOSBox spielen: Nutzt die gewählte Fix-Stufe (Original / Wichtige / Alle), Anzeige (4:3), Intro und Probenlog.";
                } else {
                    active_tooltip = "Erzeugt fehlerbereinigte DOS-Programme – danach Start in DOSBox mit Fixes, Anzeige, Intro & Probenlog";
                    tip_override   = "DOS-Fassung patchen: Erzeugt die Fix-Programme (Original bleibt unverändert) – danach gelten alle Optionen für DOSBox.";
                }
                tip_border_col = 0xFF5AA9E6u;
            } else if (hov_win_btn) {
                active_tooltip = std::string("Windows-Port (") + get_active_exe_name(g) + "): Startet mit Fixes, Anzeige (4:3), Intro" +
                                 (g.chosen_stufe == STUFE_ORIG ? " (Probenlog nur bei Wichtige/Alle)" : " & Probenlog");
                tip_override   = "Spielen (Windows): Alle Optionen der Karte (Fixes, Anzeige, Intro, Probenlog & ⚙ Skalierung) gelten direkt.";
                tip_border_col = 0xFF4CAF72u;
            } else if (hov_prog) {
                active_tooltip = std::string("Programmdatei: ") + get_active_exe_name(g);
            } else if (hov_fixes) {
                active_tooltip = std::string("Fixes (") + stufe_short_label(g.chosen_stufe) + "): Gilt für alle drei Wege – Windows (" +
                                 get_active_exe_name(g) + "), DOSBox (" + get_active_dos_exe_name(g) + ") & Browser";
            } else if (hov_dd) {
                active_tooltip = "Anzeige (Vollbild / Fenster 2×–5× in 4:3): Gilt für Windows & DOSBox (im Browser Vollbild-Knopf / F11 der Seite)";
            } else if (hov_intro) {
                active_tooltip = "Intro (Vorspann abspielen oder überspringen): Gilt für alle drei Wege (Windows, DOSBox & Browser)";
            } else if (hov_plog) {
                if (g.chosen_stufe == STUFE_ORIG) {
                    active_tooltip = "Probenlog: Nur mit Fixes »Wichtige« oder »Alle« verfügbar (Original-Fassung ist unverändert)";
                    tip_override   = "Hinweis: Das Probenlog ist in »Original« nicht enthalten – bitte unter Fixes »Wichtige« oder »Alle« wählen.";
                    tip_border_col = 0xFFE3A63Au;
                } else {
                    active_tooltip = "Probenlog (Live-Fenster & PROBEN.CSV): Gilt für Windows & DOSBox (im Browser eigener Knopf oben rechts)";
                }
            } else if (hov_gear) {
                active_tooltip = std::string("Einstellungen öffnen (Skalierung: ") +
                                 skalierung_label_str(g.chosen_skalierung) + " – gilt für Windows)";
            } else if (hov_fanpatch) {
                active_tooltip = g.name + " – " + g.version + ": Klicken für Versionsverlauf, alle Fehlerkorrekturen und Zusatzfunktionen";
                tip_override   = g.name + " (" + g.version + "): Klick öffnet den kompletten Fanpatch-Verlauf mit allen Versionen und Korrekturen.";
                tip_border_col = 0xFFE3A63Au;
            } else if (hov_drehen) {
                if (g.chosen_stufe == STUFE_ORIG) {
                    active_tooltip = "Flüssig drehen (Schicksalsklinge): Nur mit Fixes »Wichtige« oder »Alle« aktiv (Original ohne Schwenk)";
                    tip_override   = "Hinweis: Flüssig drehen ist in »Original« nicht aktiv – bitte unter Fixes »Wichtige« oder »Alle« wählen.";
                    tip_border_col = 0xFFE3A63Au;
                } else {
                    active_tooltip = "Flüssig drehen (Schicksalsklinge): Weicher 90°-Schwenk in Stadt & Dungeon (gilt für Windows & DOSBox)";
                }
            }
        }
    }

    /* Hinweis-Balken .tip: x=14, y=94, w=1012, h=43 (zeigt kontextbezogene Hinweise zu den 3 Wegen & Optionen) */
    fill_rect(14, 94, 1012, 43, 0xFF181F2Du);
    draw_border_1px(14, 94, 1012, 43, tip_border_col);
    const char *default_tip =
        "Tipp: Spielordner oder Archiv einfach mit der Maus ins Fenster ziehen – oder direkt beim jeweiligen Spiel auswählen.";
    draw_text(30, 104, tip_override.empty() ? default_tip : tip_override.c_str(), tip_text_col, FONT_15, false);

    /* Geoeffnete Anzeige-Klappliste als schwebendes Menue ueber den Karten zeichnen */
    if (g_open_dd_game >= 0 && g_open_dd_game < (int)g_games.size()) {
        const GameCatalogEntry &og = g_games[g_open_dd_game];
        int mx = 0, my = 0, mw = 0, mh = 0;
        get_open_dd_menu_rect(g_open_dd_game, mx, my, mw, mh);

        /* Dezenter Schatten + Menue-Hintergrund */
        fill_rect(mx + 3, my + 3, mw, mh, 0xFF0A0E16u);
        fill_rect(mx, my, mw, mh, 0xFF181F2Du);
        draw_border_1px(mx, my, mw, mh, 0xFF5AA9E6u);

        int cur_idx = get_dd_fenster_index(og.chosen_fenster);
        int item_th = text_height_px(FONT_14);

        for (int k = 0; k < 5; ++k) {
            int iy = my + 1 + k * 26;
            bool is_cur = (k == cur_idx);
            bool is_hov = point_in_rect(g_mouse_x, g_mouse_y, mx + 1, iy, mw - 2, 26) || (g_open_dd_sel == k);
            uint32_t ibg = is_cur ? 0xFF3A4A66u : (is_hov ? 0xFF2B364Cu : 0xFF181F2Du);
            uint32_t ifg = (is_cur || is_hov) ? 0xFFFFFFFFu : 0xFFE6E9EFu;
            fill_rect(mx + 1, iy, mw - 2, 26, ibg);
            if (k > 0) {
                fill_rect(mx + 1, iy, mw - 2, 1, 0xFF2A3346u);
            }
            if (is_cur) {
                draw_alpha_mask(mx + 7, iy + 6, 13, 13, &g_mask_check_13x13[0][0], 0xFF4CAF72u);
            }
            const char *ilbl = fenster_dd_short_label(g_dd_fenster_vals[k]);
            draw_text(mx + 24, iy + (26 - item_th) / 2, ilbl, ifg, FONT_14, false);
        }
    }

    if (!g_fp_active) {
        SDL_SetHandCursor(any_hand_cursor ? 1 : 0);
    }

    if (!active_tooltip.empty()) {
        draw_tooltip_box(g_mouse_x, g_mouse_y, active_tooltip);
    }
}

static void render_screen_settings(void) {
    if (g_selected_game < 0 || g_selected_game >= (int)g_games.size()) return;
    const GameCatalogEntry &g = g_games[g_selected_game];
    bool bg_hover = (!g_help_active && !g_modal_active);
    std::string active_tooltip;

    char sub_buf[200];
    snprintf(sub_buf, sizeof(sub_buf),
             L("Teil %d: %s – Einstellungen (inkl. Skalierung) anpassen und starten",
               "Part %d: %s – Customize settings (incl. scaling) and start"),
             g.teil, g.name.c_str());
    render_background_and_header(sub_buf, &active_tooltip);

    fill_rect(14, 92, 1012, 488, 0xFF1B2232u);
    draw_border_1px(14, 92, 1012, 488, 0xFF3A4558u);

    int row_y[6] = { 98, 174, 250, 326, 384, 442 };
    int row_h[6] = { 72,  72,  72,  54,  54, 132 };
    if (g_settings_row >= 0 && g_settings_row < 6) {
        fill_rect(22, row_y[g_settings_row], 996, row_h[g_settings_row], 0xFF232C40u);
        draw_border_1px(22, row_y[g_settings_row], 996, row_h[g_settings_row], 0xFF4A5568u);
    }

    /* 1. Fixes */
    draw_text(36, 110, L("1. Fixes [S]:", "1. Fixes [S]:"), 0xFFE3A63Au, FONT_15_SEMI, false);
    draw_button(268, 104, 224, 34, L("Original", "Original"),                 g.chosen_stufe == STUFE_ORIG,    true, false, bg_hover, FONT_14);
    draw_button(504, 104, 244, 34, L("Wichtige Fixes", "Essential Fixes"),    g.chosen_stufe == STUFE_WICHTIG, true, false, bg_hover, FONT_14);
    draw_button(760, 104, 250, 34, L("Alle Fixes", "All Fixes"),              g.chosen_stufe == STUFE_ALLES,   true, false, bg_hover, FONT_14);

    bool stufe_exe_ok = (g.chosen_stufe == STUFE_ORIG) ? g.has_exe_orig : ((g.chosen_stufe == STUFE_WICHTIG) ? g.has_exe_wichtig : (g.has_exe_alles || g.has_exe_standard));
    const char *stufe_desc = (g.chosen_stufe == STUFE_ORIG)
        ? L("Unverändertes Originalverhalten (gilt für Windows, DOSBox & Browser)", "Unmodified original behavior")
        : ((g.chosen_stufe == STUFE_WICHTIG)
            ? L("Behebt Abstürze und kritische Fehler (gilt für Windows, DOSBox & Browser)", "Fixes crashes and critical savegame issues")
            : L("Alle Fehlerkorrekturen & Komfortfunktionen (Empfohlen – Windows, DOSBox & Browser)", "All bug fixes and quality-of-life improvements"));
    draw_status_badge(268, 146, stufe_exe_ok);
    char stufe_sub[240];
    snprintf(stufe_sub, sizeof(stufe_sub), "%s – %s",
             stufe_exe_ok ? L("Programm gefunden", "Program found") : L("Programm fehlt", "Program missing"),
             stufe_desc);
    draw_text(290, 144, stufe_sub, stufe_exe_ok ? 0xFF9AA3B2u : 0xFFFF8080u, FONT_13, false);

    if (bg_hover && point_in_rect(g_mouse_x, g_mouse_y, 268, 104, 742, 62)) {
        active_tooltip = std::string("Windows: ") + get_active_exe_name(g) + "  |  DOSBox: " + get_active_dos_exe_name(g) +
                         "  |  Browser: ?stufe=" + stufe_to_str(g.chosen_stufe);
    }

    /* 2. Anzeige - Fenstergroesse */
    draw_text(36, 186, L("2. Anzeige [F]:", "2. Display [F]:"), 0xFFE3A63Au, FONT_15_SEMI, false);
    draw_button(268, 180, 140, 34, "2x (640x480)",   g.chosen_fenster == "2", true, false, bg_hover, FONT_14);
    draw_button(416, 180, 140, 34, "3x (960x720)",   g.chosen_fenster == "3", true, false, bg_hover, FONT_14);
    draw_button(564, 180, 150, 34, "4x (1280x960)",  g.chosen_fenster == "4", true, false, bg_hover, FONT_14);
    draw_button(722, 180, 156, 34, "5x (1600x1200)", g.chosen_fenster == "5", true, false, bg_hover, FONT_14);
    draw_button(886, 180, 124, 34, L("Vollbild", "Fullscreen"), g.chosen_fenster == "vollbild", true, false, bg_hover, FONT_14);

    draw_text(268, 220,
        L("Gilt für Windows & DOSBox in 4:3 (Alt+Enter schaltet Vollbild um; im Browser Vollbild-Knopf / F11)",
          "Always exact 4:3 aspect ratio like on a CRT monitor (Alt+Enter toggles fullscreen)"),
        0xFF9AA3B2u, FONT_13, false);

    /* 3. Skalierung (bleibt in Einstellungen) */
    draw_text(36, 262, L("3. Skalierung [K]:", "3. Scaling [K]:"), 0xFFE3A63Au, FONT_15_SEMI, false);
    bool is_ganz = (_stricmp(g.chosen_skalierung.c_str(), "ganzzahlig") == 0);
    draw_button(268, 256, 364, 34, L("Ganzzahlig (exakte Vielfache)", "Integer (exact multiples)"), is_ganz, true, false, bg_hover, FONT_14);
    draw_button(644, 256, 366, 34, L("Füllend (scharf)", "Fill (sharp)"),                          !is_ganz, true, false, bg_hover, FONT_14);

    const char *skal_desc = is_ganz
        ? L("Ganzzahlig (Windows-Port): Nur ganze Vielfache, alle Pixelspalten exakt gleich breit (DOSBox: openglnb 4:3)",
            "Integer: Whole multiples only, all pixel columns have the exact same width")
        : L("Füllend (Windows-Port): Nutzt die gesamte 4:3-Fläche mit scharfen Pixelkanten (DOSBox: openglnb 4:3)",
            "Fill: Uses the full 4:3 area with crisp, sharp pixel edges");
    draw_text(268, 296, skal_desc, 0xFF9AA3B2u, FONT_13, false);

    /* 4. Probenlog */
    draw_text(36, 342, L("4. Probenlog [P]:", "4. Roll Log [P]:"), 0xFFE3A63Au, FONT_15_SEMI, false);
    bool plog_avail = (g.chosen_stufe != STUFE_ORIG);
    draw_button(268, 336, 234, 34, L("Ausgeschaltet", "Disabled"), !g.chosen_probenlog || !plog_avail, true, false, bg_hover, FONT_14);
    draw_button(514, 336, 496, 34,
                plog_avail ? L("Eingeschaltet (Live-Fenster für Windows & DOSBox)", "Enabled (with live window)")
                           : L("Nur mit Fixes »Wichtige« oder »Alle« verfügbar", "Only available with Essential/All Fixes"),
                g.chosen_probenlog && plog_avail, plog_avail, false, bg_hover, FONT_14);

    /* 5. Vorspann / Intro */
    draw_text(36, 400, L("5. Intro [I]:", "5. Intro [I]:"), 0xFFE3A63Au, FONT_15_SEMI, false);
    draw_button(268, 394, 346, 34, L("Überspringen (Windows, DOSBox & Browser)", "Skip"), !g.chosen_intro, true, false, bg_hover, FONT_14);
    draw_button(626, 394, 384, 34, L("Abspielen (Windows, DOSBox & Browser)", "Play (Original Intro)"), g.chosen_intro, true, false, bg_hover, FONT_14);

    /* 6. Spielordner & Status-Checks */
    draw_text(36, 458, L("6. Spieldaten [O]:", "6. Game Data [O]:"), 0xFFE3A63Au, FONT_15_SEMI, false);
    fill_rect(268, 452, 536, 34, 0xFF10151Fu);
    draw_border_1px(268, 452, 536, 34, g.gamedir_valid ? 0xFF4CAF72u : 0xFF8E4A50u);
    std::string disp_gd = g.gamedir_valid
        ? clip_text_px_left(g.resolved_gamedir, 512, FONT_14)
        : L("Noch nicht gewählt – Spielordner oder Archiv ins Fenster ziehen",
            "Not selected yet – Drag & drop game folder or archive into window");
    draw_text(280, 459, disp_gd.c_str(), g.gamedir_valid ? 0xFFE6E9EFu : 0xFFFF9898u, FONT_14, false);
    draw_button(816, 452, 194, 34, L("Ordner \xE2\x80\xA6", "Folder \xE2\x80\xA6"), g_settings_row == 5, true, false, bg_hover, FONT_14);

    draw_status_badge(268, 498, g.gamedir_valid);
    std::string save_info;
    if (g.gamedir_wrong_version) {
        save_info = L(g.wrong_version_msg_de.c_str(), g.wrong_version_msg_en.c_str());
    } else if (!g.gamedir_valid) {
        save_info = L("Spieldaten fehlen – Bitte den Original-Spielordner auswählen",
                      "Game data missing – Please select your original game folder");
    } else if (g.gamedir_writable) {
        save_info = L("Spieldaten gefunden  |  Spielstände werden im Spielordner gespeichert",
                      "Game data found  |  Savegames are stored in the game folder");
    } else {
        save_info = L("Spieldaten gefunden  |  Spielstände werden im eigenen Spielstandsordner gespeichert",
                      "Game data found  |  Savegames are stored in the local savegames folder");
    }
    draw_text(290, 496, save_info.c_str(), g.gamedir_valid ? 0xFFE6E9EFu : 0xFFFF9090u, FONT_13, false);

    draw_status_badge(268, 526, g.verfuegbar);
    const char *ready_info = g.verfuegbar
        ? L("Startbereit – Alle drei Wege (Windows, DOSBox, Browser) stehen bereit",
            "Ready to play – All three ways (Windows, DOSBox, Browser) are available")
        : (!g.has_any_exe
            ? L("Kommt bald – Programm für diesen Teil ist noch in Vorbereitung",
                "Coming soon – Program for this part is still in preparation")
            : L("Noch nicht startbereit – Bitte zuerst die deutsche GOG-Fassung bereitstellen",
                "Not ready yet – Please provide the German GOG version first"));
    draw_text(290, 524, ready_info, g.verfuegbar ? 0xFFE6E9EFu : 0xFFE3A63Au, FONT_13, false);

    if (g.teil == 1) {
        const char *dr_set_lbl = "Fl\xC3\xBCssig drehen (Schicksalsklinge) [R] – weicher 90\xC2\xB0-Schwenk beim Drehen (Windows & DOSBox, ab \xC2\xBBWichtige Fixes\xC2\xAB)";
        bool dim_dr_set = (g.chosen_stufe == STUFE_ORIG);
        bool hov_dr_set = bg_hover && point_in_rect(g_mouse_x, g_mouse_y, 268, 548, 700, 22);
        draw_checkbox_16(268, 550, g.chosen_drehen, hov_dr_set, dim_dr_set);
        draw_text(290, 550, dr_set_lbl, dim_dr_set ? 0xFF636C7Eu : 0xFFE6E9EFu, FONT_13, false);
        if (hov_dr_set) {
            active_tooltip = "Flüssig drehen (Schicksalsklinge): Schaltet den 90°-Schwenk für Windows (DSA_DREHEN) und DOSBox (DREHEN.AUS)";
        }
    }

    /* Untere Aktionsleiste mit allen drei Wegen */
    fill_rect(14, 588, 1012, 48, 0xFF181F2Du);
    draw_border_1px(14, 588, 1012, 48, 0xFF3A4558u);

    draw_button(24, 594, 180, 36, L("< Übersicht (Esc)", "< Overview (Esc)"), false, true, false, bg_hover, FONT_15);
    bool dos_can_patch = (g.gamedir_valid || g.gamedir_wrong_version) && g.has_dos_patcher;
    draw_button(214, 594, 198, 36, L("DOS-Fassung patchen", "Patch DOS Version"), false, true, false, bg_hover, FONT_15);
    draw_button(422, 594, 184, 36, L("In DOSBox spielen", "Play in DOSBox"), g.dos_fix_exists, dos_can_patch && g.dos_fix_exists, false, bg_hover, FONT_15);
    if (is_browser_available(g)) {
        draw_button(616, 594, 186, 36, L("Im Browser spielen", "Play in Browser"), false, true, false, bg_hover, FONT_15);
    } else {
        draw_button(616, 594, 186, 36, L("Im Browser spielen", "Play in Browser"), false, false, false, false, FONT_15);
    }
    if (!g.has_any_exe) {
        draw_button(812, 594, 204, 36, L("Kommt bald", "Coming soon"), false, false, false, false, FONT_15);
    } else {
        draw_button(812, 594, 204, 36, L("Spielen (Windows)", "Play (Windows)"), g_settings_row == 6, true, g.verfuegbar, bg_hover, FONT_15_SEMI, true);
    }

    if (bg_hover) {
        if (point_in_rect(g_mouse_x, g_mouse_y, 616, 594, 186, 36) && is_browser_available(g)) {
            active_tooltip = "Browser: Fixes & Intro werden übergeben – Anzeige (Vollbild/F11) & Probenlog direkt auf der Webseite";
        } else if (point_in_rect(g_mouse_x, g_mouse_y, 422, 594, 184, 36) && g.dos_fix_exists) {
            active_tooltip = std::string("DOSBox (") + get_active_dos_exe_name(g) + "): Startet mit Fixes, Anzeige (4:3), Intro & Probenlog";
        } else if (point_in_rect(g_mouse_x, g_mouse_y, 812, 594, 204, 36) && g.has_any_exe) {
            active_tooltip = std::string("Windows-Port (") + get_active_exe_name(g) + "): Alle Einstellungen gelten direkt";
        }
    }

    if (!g_fp_active) {
        SDL_SetHandCursor(0);
    }

    if (!active_tooltip.empty()) {
        draw_tooltip_box(g_mouse_x, g_mouse_y, active_tooltip);
    }
}

static void render_help_overlay(void) {
    if (!g_help_active) return;
    int mw = 900, mh = 530;
    int mx = (SCREEN_W - mw) / 2;
    int my = (SCREEN_H - mh) / 2;
    fill_rect(mx, my, mw, mh, 0xFF181F2Du);
    draw_border_1px(mx, my, mw, mh, 0xFF5AA9E6u);

    draw_text_centered(mx, my + 16, mw, L("HILFE & BEDIENUNG (F1)", "HELP & CONTROLS (F1)"), 0xFFE3A63Au, FONT_23_BOLD, false);

    const char *lines_de[] = {
        "DIE DREI WEGE ZUM SPIELEN:",
        "   • Spielen (Windows): Startet die neue Windows-Fassung direkt ohne DOSBox",
        "     im originalgetreuen 4:3-Bild mit CD-Musik, Vorspann und Probenlog.",
        "   • DOS-Fassung patchen / In DOSBox spielen: Erzeugt aus deiner deutschen",
        "     GOG-Fassung fehlerbereinigte DOS-Programme, ohne Originaldateien zu",
        "     überschreiben. Danach startet [In DOSBox spielen] das Spiel in DOSBox.",
        "   • Im Browser spielen: Öffnet die Online-Fassung im Standardbrowser",
        "     und übergibt die gewählte Fix-Stufe sowie die Intro-Einstellung.",
        "",
        "SPIELORDNER BEREITSTELLEN:",
        "   Den Hauptordner deiner deutschen GOG-Installation einfach mit der Maus",
        "   in dieses Fenster ziehen oder auf der Spielkarte [Ordner …] klicken.",
        "",
        "WELCHE OPTION GILT FÜR WELCHEN WEG?",
        "   • Fixes [Original | Wichtige | Alle] & Intro: Gelten für ALLE drei Wege",
        "     (Windows, DOSBox und als URL-Parameter für die Browser-Fassung).",
        "   • Anzeige (Vollbild / Fenster 2×..5× in 4:3): Gilt für Windows & DOSBox",
        "     (im Browser über den Vollbild-Knopf der Webseite oder F11; Skalierung via ⚙).",
        "   • Probenlog: Öffnet bei »Wichtige«/»Alle« das Live-Fenster für Windows & DOSBox",
        "     (im Browser eigener Knopf oben rechts; in »Original« nicht enthalten).",
        "",
        "TASTENKÜRZEL: [←/→]=Fixes, [Leertaste]=Häkchen, [Tab]=Element, [F1]=Hilfe, [F5]=Start, [Esc]=Ende"
    };

    for (int i = 0; i < 22; ++i) {
        const char *ln = lines_de[i];
        bool is_hdr = (strncmp(ln, "DIE DREI", 8) == 0 || strncmp(ln, "SPIELORDNER", 11) == 0 ||
                       strncmp(ln, "WELCHE", 6) == 0 || strncmp(ln, "TASTEN", 6) == 0);
        draw_text(mx + 34, my + 54 + i * 19, ln, is_hdr ? 0xFFE3A63Au : 0xFFE6E9EFu, is_hdr ? FONT_15_SEMI : FONT_14, false);
    }

    draw_button(mx + (mw - 320) / 2, my + mh - 50, 320, 36, L("Hilfe schließen (F1 / Enter)", "Close Help (F1 / Enter)"), true, true, true, true, FONT_15);
}

/* =========================================================================
 * 8b. Fanpatch-Verlauf im Starter (Starter 1.2)
 *     - Drei Reiter: "Versionen (N)", "Alle Korrekturen (N)", "Zusatzfunktionen"
 *     - Auf-/Zuklappen mit "+" (zu) und "−" (offen), "Alle aufklappen" / "Alle zuklappen"
 *     - Stufe als Wort: "Wichtig" (gold) bzw. "nur Alle" (blau)
 *     - Suche (Text oder "Nr. 90") und Filter ("Alle" / "nur Wichtige" / "💰 Exploits")
 *     - Scrollen mit Mausrad, Ziehen am Balken und Pfeil-/Bildtasten
 * ========================================================================= */

struct FanpatchVerKorrRef {
    std::string nr;
    std::string art; /* "neu" oder "nachgebessert" */
};

struct FanpatchVersionEntry {
    std::string version;
    std::string datum;
    std::string titel;
    std::string text;
    std::vector<FanpatchVerKorrRef> korrekturen;
    bool expanded;
    std::vector<bool> korr_expanded;
};

struct FanpatchKorrekturEntry {
    std::string nr;
    std::string titel;
    std::string was_war_falsch;
    std::string so_pruefst_du_es;
    std::string stufe;    /* "Wichtig" oder "nur Alle" */
    bool        exploit;
    std::string bereich;  /* z. B. "Dungeons (54)" */
    std::string herkunft;
    bool        expanded;
};

struct FanpatchZusatzEntry {
    std::string titel;
    std::string version;
    std::string stufe;
    std::string text;
    bool        expanded;
};

struct FanpatchAreaGroup {
    std::string raw_bereich;
    std::string display_name;
    std::vector<int> korr_indices;
    bool expanded;
};

struct FanpatchData {
    bool        loaded;
    std::string spiel_id;
    std::string spiel_name;
    std::string aktuelle_version;
    std::string github_url;
    std::vector<FanpatchVersionEntry>   versionen;
    std::vector<FanpatchKorrekturEntry> korrekturen;
    std::vector<FanpatchZusatzEntry>    zusatzfunktionen;
    std::vector<FanpatchAreaGroup>      bereiche;
};

enum FanpatchHitAction {
    FP_ACT_TAB          = 1,
    FP_ACT_EXPAND_ALL   = 2,
    FP_ACT_COLLAPSE_ALL = 3,
    FP_ACT_SEARCH_BOX   = 4,
    FP_ACT_SEARCH_CLEAR = 5,
    FP_ACT_FILTER       = 6,
    FP_ACT_VER_TOGGLE   = 7,
    FP_ACT_VKORR_TOGGLE = 8,
    FP_ACT_AREA_TOGGLE  = 9,
    FP_ACT_KORR_TOGGLE  = 10,
    FP_ACT_ZUS_TOGGLE   = 11,
    FP_ACT_GITHUB       = 12,
    FP_ACT_CLOSE        = 13
};

struct FanpatchHitBox {
    int x, y, w, h;
    int action;
    int idx1;
    int idx2;
};

static FanpatchData                g_fp_data[3];
static bool                        g_fp_attempted[3]    = {false, false, false};
static int                         g_fp_tab             = 0; /* 0=Versionen, 1=Alle Korrekturen, 2=Zusatzfunktionen */
static int                         g_fp_filter          = 0; /* 0=Alle, 1=nur Wichtige, 2=Exploits */
static std::string                 g_fp_search_text;
static bool                        g_fp_search_focused  = false;
static int                         g_fp_scroll_y        = 0;
static int                         g_fp_content_h       = 0;
static int                         g_fp_view_h          = 420;
static bool                        g_fp_dragging_sb     = false;
static int                         g_fp_drag_start_my   = 0;
static int                         g_fp_drag_start_sc   = 0;
static int                         g_fp_sb_track_x      = 0;
static int                         g_fp_sb_track_y      = 0;
static int                         g_fp_sb_track_w      = 10;
static int                         g_fp_sb_track_h      = 420;
static int                         g_fp_sb_thumb_y      = 0;
static int                         g_fp_sb_thumb_h      = 40;
static std::vector<FanpatchHitBox> g_fp_hits;

static std::string default_bugfixes_github_url(const GameCatalogEntry &g) {
    if (g.teil == 2 || _stricmp(g.id.c_str(), "sternenschweif") == 0) {
        return "https://github.com/MeowMuhMuhCow/sternenschweif/blob/main/BUGFIXES.md";
    }
    if (g.teil == 3 || _stricmp(g.id.c_str(), "riva") == 0) {
        return "https://github.com/MeowMuhMuhCow/riva/blob/main/BUGFIXES.md";
    }
    return "https://github.com/MeowMuhMuhCow/schicksalsklinge/blob/main/BUGFIXES.md";
}

static std::string strip_area_count_suffix(const std::string &raw) {
    size_t lp = raw.find_last_of('(');
    if (lp != std::string::npos && lp > 0 && raw.back() == ')') {
        bool all_digits = true;
        for (size_t k = lp + 1; k + 1 < raw.size(); ++k) {
            if (raw[k] < '0' || raw[k] > '9') { all_digits = false; break; }
        }
        if (all_digits && lp + 2 <= raw.size()) {
            return trim_str(raw.substr(0, lp));
        }
    }
    return raw;
}

static void parse_fanpatch_json_content(const std::string &content, FanpatchData &fp) {
    const char *p = content.c_str();
    skip_ws(p);
    if (*p != '{') return;
    ++p;
    while (*p) {
        skip_ws(p);
        if (*p == '}') { ++p; break; }
        std::string key = parse_json_string(p);
        skip_ws(p);
        if (*p == ':') ++p;
        skip_ws(p);

        if (key == "spiel_id") fp.spiel_id = parse_json_string(p);
        else if (key == "spiel_name") fp.spiel_name = parse_json_string(p);
        else if (key == "aktuelle_version") fp.aktuelle_version = parse_json_string(p);
        else if (key == "github_url") fp.github_url = parse_json_string(p);
        else if (key == "versionen" && *p == '[') {
            ++p;
            while (*p) {
                skip_ws(p);
                if (*p == ']') { ++p; break; }
                if (*p == '{') {
                    ++p;
                    FanpatchVersionEntry ve;
                    ve.expanded = false;
                    while (*p) {
                        skip_ws(p);
                        if (*p == '}') { ++p; break; }
                        std::string vk = parse_json_string(p);
                        skip_ws(p);
                        if (*p == ':') ++p;
                        skip_ws(p);
                        if (vk == "version") ve.version = parse_json_string(p);
                        else if (vk == "datum") ve.datum = parse_json_string(p);
                        else if (vk == "titel") ve.titel = parse_json_string(p);
                        else if (vk == "text") ve.text = parse_json_string(p);
                        else if (vk == "korrekturen" && *p == '[') {
                            ++p;
                            while (*p) {
                                skip_ws(p);
                                if (*p == ']') { ++p; break; }
                                if (*p == '{') {
                                    ++p;
                                    FanpatchVerKorrRef kr;
                                    kr.art = "neu";
                                    while (*p) {
                                        skip_ws(p);
                                        if (*p == '}') { ++p; break; }
                                        std::string kk = parse_json_string(p);
                                        skip_ws(p);
                                        if (*p == ':') ++p;
                                        skip_ws(p);
                                        if (kk == "nr") kr.nr = parse_json_string(p);
                                        else if (kk == "art") kr.art = parse_json_string(p);
                                        else skip_json_value(p);
                                        skip_ws(p);
                                        if (*p == ',') ++p;
                                    }
                                    if (!kr.nr.empty()) {
                                        ve.korrekturen.push_back(kr);
                                        ve.korr_expanded.push_back(false);
                                    }
                                } else {
                                    skip_json_value(p);
                                }
                                skip_ws(p);
                                if (*p == ',') ++p;
                            }
                        } else {
                            skip_json_value(p);
                        }
                        skip_ws(p);
                        if (*p == ',') ++p;
                    }
                    fp.versionen.push_back(ve);
                } else {
                    skip_json_value(p);
                }
                skip_ws(p);
                if (*p == ',') ++p;
            }
        } else if (key == "korrekturen" && *p == '[') {
            ++p;
            while (*p) {
                skip_ws(p);
                if (*p == ']') { ++p; break; }
                if (*p == '{') {
                    ++p;
                    FanpatchKorrekturEntry ke;
                    ke.stufe = "Wichtig";
                    ke.exploit = false;
                    ke.expanded = false;
                    while (*p) {
                        skip_ws(p);
                        if (*p == '}') { ++p; break; }
                        std::string kk = parse_json_string(p);
                        skip_ws(p);
                        if (*p == ':') ++p;
                        skip_ws(p);
                        if (kk == "nr") ke.nr = parse_json_string(p);
                        else if (kk == "titel") ke.titel = parse_json_string(p);
                        else if (kk == "was_war_falsch") ke.was_war_falsch = parse_json_string(p);
                        else if (kk == "so_pruefst_du_es") ke.so_pruefst_du_es = parse_json_string(p);
                        else if (kk == "stufe") ke.stufe = parse_json_string(p);
                        else if (kk == "exploit") ke.exploit = parse_json_bool(p);
                        else if (kk == "bereich") ke.bereich = parse_json_string(p);
                        else if (kk == "herkunft") ke.herkunft = parse_json_string(p);
                        else skip_json_value(p);
                        skip_ws(p);
                        if (*p == ',') ++p;
                    }
                    fp.korrekturen.push_back(ke);
                } else {
                    skip_json_value(p);
                }
                skip_ws(p);
                if (*p == ',') ++p;
            }
        } else if (key == "zusatzfunktionen" && *p == '[') {
            ++p;
            while (*p) {
                skip_ws(p);
                if (*p == ']') { ++p; break; }
                if (*p == '{') {
                    ++p;
                    FanpatchZusatzEntry ze;
                    ze.expanded = true;
                    while (*p) {
                        skip_ws(p);
                        if (*p == '}') { ++p; break; }
                        std::string zk = parse_json_string(p);
                        skip_ws(p);
                        if (*p == ':') ++p;
                        skip_ws(p);
                        if (zk == "titel") ze.titel = parse_json_string(p);
                        else if (zk == "version") ze.version = parse_json_string(p);
                        else if (zk == "stufe") ze.stufe = parse_json_string(p);
                        else if (zk == "text" || zk == "beschreibung") ze.text = parse_json_string(p);
                        else skip_json_value(p);
                        skip_ws(p);
                        if (*p == ',') ++p;
                    }
                    fp.zusatzfunktionen.push_back(ze);
                } else {
                    skip_json_value(p);
                }
                skip_ws(p);
                if (*p == ',') ++p;
            }
        } else {
            skip_json_value(p);
        }
        skip_ws(p);
        if (*p == ',') ++p;
    }

    /* Bereiche aus korrekturen gruppieren */
    fp.bereiche.clear();
    for (size_t i = 0; i < fp.korrekturen.size(); ++i) {
        const std::string &b = fp.korrekturen[i].bereich;
        int found_b = -1;
        for (size_t j = 0; j < fp.bereiche.size(); ++j) {
            if (fp.bereiche[j].raw_bereich == b) {
                found_b = (int)j;
                break;
            }
        }
        if (found_b < 0) {
            FanpatchAreaGroup ag;
            ag.raw_bereich  = b;
            ag.display_name = strip_area_count_suffix(b);
            ag.expanded     = fp.bereiche.empty(); /* Erster Bereich standardmaessig offen */
            ag.korr_indices.push_back((int)i);
            fp.bereiche.push_back(ag);
        } else {
            fp.bereiche[found_b].korr_indices.push_back((int)i);
        }
    }
    fp.loaded = (!fp.versionen.empty() || !fp.korrekturen.empty());
}

static void load_fanpatch_data_for_game(int game_idx) {
    if (game_idx < 0 || game_idx >= 3 || game_idx >= (int)g_games.size()) return;
    const GameCatalogEntry &g = g_games[game_idx];
    FanpatchData &fp = g_fp_data[game_idx];
    fp = FanpatchData();
    fp.loaded     = false;
    fp.spiel_id   = g.id;
    fp.spiel_name = g.name;
    fp.github_url = default_bugfixes_github_url(g);
    g_fp_attempted[game_idx] = true;

    const char *force_missing = getenv("STARTER_FORCE_MISSING_FANPATCH_JSON");
    if (force_missing && force_missing[0] == '1') {
        return;
    }

    std::string fname = "fanpatch_" + g.id + ".json";
    std::string cands[5] = {
        join_path(g_data_dir, fname),
        join_path(g_starter_dir, fname),
        join_path(g_exe_dir, "_starter\\" + fname),
        join_path(g_exe_dir, fname),
        join_path(g_starter_dir, "trilogie\\Nordlandtrilogie\\_starter\\" + fname)
    };
    std::string chosen;
    for (int i = 0; i < 5; ++i) {
        if (file_exists_win(cands[i])) {
            chosen = cands[i];
            break;
        }
    }
    if (chosen.empty()) return;

    FILE *fp_file = fopen(utf8_to_win1252(chosen).c_str(), "rb");
    if (!fp_file) fp_file = fopen(chosen.c_str(), "rb");
    if (!fp_file) return;

    fseek(fp_file, 0, SEEK_END);
    long sz = ftell(fp_file);
    fseek(fp_file, 0, SEEK_SET);
    if (sz <= 0 || sz > 4 * 1024 * 1024) {
        fclose(fp_file);
        return;
    }
    std::string content((size_t)sz, '\0');
    fread(&content[0], 1, (size_t)sz, fp_file);
    fclose(fp_file);

    parse_fanpatch_json_content(content, fp);
    if (fp.github_url.empty()) {
        fp.github_url = default_bugfixes_github_url(g);
    }
}

static void open_fanpatch_window(int game_idx) {
    if (game_idx < 0 || game_idx >= (int)g_games.size() || game_idx >= 3) return;
    if (!g_fp_attempted[game_idx]) {
        load_fanpatch_data_for_game(game_idx);
    }
    g_fp_game_idx       = game_idx;
    g_fp_active         = true;
    g_fp_tab            = 0;
    g_fp_filter         = 0;
    g_fp_search_text.clear();
    g_fp_search_focused = false;
    g_fp_scroll_y       = 0;
    g_fp_dragging_sb    = false;
    g_open_dd_game      = -1;
    g_help_active       = false;
    g_modal_active      = false;
}

static int find_korrektur_index_by_nr(const FanpatchData &fp, const std::string &nr) {
    for (size_t i = 0; i < fp.korrekturen.size(); ++i) {
        if (_stricmp(fp.korrekturen[i].nr.c_str(), nr.c_str()) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static std::string utf8_tolower_de(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 'A' && c <= 'Z') {
            out.push_back((char)(c + 32));
        } else if (c == 0xC3 && i + 1 < s.size()) {
            unsigned char c2 = (unsigned char)s[i + 1];
            /* Ä (C3 84)->ä (C3 A4), Ö (C3 96)->ö (C3 B6), Ü (C3 9C)->ü (C3 BC) */
            if (c2 == 0x84 || c2 == 0x96 || c2 == 0x9C) {
                out.push_back((char)0xC3);
                out.push_back((char)(c2 + 0x20));
                ++i;
                continue;
            }
            out.push_back((char)c);
        } else {
            out.push_back((char)c);
        }
    }
    return out;
}

static bool korrektur_matches_filter_and_search(const FanpatchKorrekturEntry &k) {
    if (g_fp_filter == 1 && k.stufe != "Wichtig") return false;
    if (g_fp_filter == 2 && !k.exploit) return false;
    if (g_fp_search_text.empty()) return true;

    std::string q = utf8_tolower_de(trim_str(g_fp_search_text));
    if (q.empty()) return true;

    /* Spezielle Suche nach "nr. 90" oder "nr 90" oder "#90" */
    std::string q_nr = q;
    if (strncmp(q_nr.c_str(), "nr.", 3) == 0) q_nr = trim_str(q_nr.substr(3));
    else if (strncmp(q_nr.c_str(), "nr ", 3) == 0) q_nr = trim_str(q_nr.substr(3));
    else if (!q_nr.empty() && q_nr[0] == '#') q_nr = trim_str(q_nr.substr(1));

    std::string k_nr_low = utf8_tolower_de(k.nr);
    if (q != q_nr && !q_nr.empty()) {
        if (k_nr_low == q_nr || ("r" + q_nr) == k_nr_low) return true;
        return false;
    }

    if (k_nr_low.find(q) != std::string::npos) return true;
    if (utf8_tolower_de(k.titel).find(q) != std::string::npos) return true;
    if (utf8_tolower_de(k.was_war_falsch).find(q) != std::string::npos) return true;
    if (utf8_tolower_de(k.so_pruefst_du_es).find(q) != std::string::npos) return true;
    if (utf8_tolower_de(k.bereich).find(q) != std::string::npos) return true;
    if (utf8_tolower_de(k.herkunft).find(q) != std::string::npos) return true;
    return false;
}

/* Zeichnet "+" (geschlossen) bzw. "−" (offen) statt Pfeilen */
static void draw_plus_minus_sign(int cx, int cy, bool is_open, uint32_t col) {
    /* Horizontale Linie (9 px breit, 2 px hoch) */
    fill_rect(cx - 4, cy, 9, 2, col);
    if (!is_open) {
        /* Vertikale Linie fuer "+" (2 px breit, 9 px hoch) */
        fill_rect(cx, cy - 3, 2, 8, col);
    }
}

/* Zeichnet das Stufen-Feld als Wort: "Wichtig" (gold) bzw. "nur Alle" (blau) */
static void draw_stufe_word_badge(int x, int y, const std::string &stufe) {
    const int bw = 54;
    const int bh = 18;
    bool is_wichtig = (_stricmp(stufe.c_str(), "Wichtig") == 0 || stufe == "W");
    uint32_t bg = is_wichtig ? 0xFFE3A63Au : 0xFF5AA9E6u;
    uint32_t fg = 0xFF141A26u;
    const char *lbl = is_wichtig ? "Wichtig" : "nur Alle";
    fill_rect(x, y, bw, bh, bg);
    int th = text_height_px(FONT_12_BOLD);
    draw_text_centered(x, y + (bh - th) / 2, bw, lbl, fg, FONT_12_BOLD, false);
}

/* Bricht einen langen UTF-8-Text wortweise auf max_width_px um (erste Zeile optional mit Einzug fuer Label) */
static std::vector<std::string> wrap_text_to_lines(const std::string &text, int max_width_px, int font_idx, int first_indent_px = 0) {
    std::vector<std::string> lines;
    if (text.empty()) return lines;

    size_t p_start = 0;
    bool is_very_first_line = true;
    while (p_start <= text.size()) {
        size_t p_end = text.find('\n', p_start);
        std::string para = (p_end == std::string::npos) ? text.substr(p_start) : text.substr(p_start, p_end - p_start);
        p_start = (p_end == std::string::npos) ? (text.size() + 1) : (p_end + 1);

        para = trim_str(para);
        if (para.empty()) continue;

        std::string cur_line;
        int avail_w = is_very_first_line ? (max_width_px - first_indent_px) : max_width_px;
        if (avail_w < 80) avail_w = 80;

        size_t w_pos = 0;
        while (w_pos < para.size()) {
            while (w_pos < para.size() && para[w_pos] == ' ') ++w_pos;
            if (w_pos >= para.size()) break;
            size_t w_next = para.find(' ', w_pos);
            std::string word = (w_next == std::string::npos) ? para.substr(w_pos) : para.substr(w_pos, w_next - w_pos);
            w_pos = (w_next == std::string::npos) ? para.size() : w_next;

            std::string cand = cur_line.empty() ? word : (cur_line + " " + word);
            if (!cur_line.empty() && text_width_px(cand.c_str(), font_idx) > avail_w) {
                lines.push_back(cur_line);
                cur_line = word;
                is_very_first_line = false;
                avail_w = max_width_px;
            } else {
                cur_line = cand;
            }
        }
        if (!cur_line.empty()) {
            lines.push_back(cur_line);
            is_very_first_line = false;
        }
    }
    return lines;
}

/* Zeichnet den aufgeklappten Detailkasten ("Was war falsch:" + gestrichelte Linie + gruen "So prüfst du es:") */
static int draw_korrektur_detail_box(int bx, int by, int bw, const FanpatchKorrekturEntry &k, bool draw_it) {
    const int pad_x = 12;
    const int pad_y = 9;
    const int line_h = 18;
    const int inner_w = bw - 2 * pad_x;

    const char *lbl_was = "Was war falsch: ";
    const char *lbl_prf = "So pr\xC3\xBC""fst du es: ";
    int indent_was = text_width_px(lbl_was, FONT_13_BOLD);
    int indent_prf = text_width_px(lbl_prf, FONT_13_BOLD);

    std::string was_full = k.was_war_falsch;
    if (!k.herkunft.empty() && was_full.find(k.herkunft) == std::string::npos) {
        was_full += " (" + k.herkunft + ")";
    }

    std::vector<std::string> was_lines = wrap_text_to_lines(was_full, inner_w, FONT_13, indent_was);
    if (was_lines.empty()) was_lines.push_back("-");

    std::vector<std::string> prf_lines;
    if (!k.so_pruefst_du_es.empty()) {
        prf_lines = wrap_text_to_lines(k.so_pruefst_du_es, inner_w, FONT_13, indent_prf);
    }

    int bh = pad_y * 2 + (int)was_lines.size() * line_h;
    if (!prf_lines.empty()) {
        bh += 12 + (int)prf_lines.size() * line_h;
    }

    if (!draw_it) return bh;

    fill_rect(bx, by, bw, bh, 0xFF131924u);
    draw_border_1px(bx, by, bw, bh, 0xFF2B3548u);

    int ty = by + pad_y;
    draw_text(bx + pad_x, ty, lbl_was, 0xFFE3A63Au, FONT_13_BOLD, false);
    for (size_t i = 0; i < was_lines.size(); ++i) {
        int lx = bx + pad_x + ((i == 0) ? indent_was : 0);
        draw_text(lx, ty, was_lines[i].c_str(), 0xFFD8DEE9u, FONT_13, false);
        ty += line_h;
    }

    if (!prf_lines.empty()) {
        int sep_y = ty + 5;
        for (int dx = bx + pad_x; dx < bx + bw - pad_x; dx += 5) {
            fill_rect(dx, sep_y, 3, 1, 0xFF2E3A4Eu);
        }
        ty += 12;
        draw_text(bx + pad_x, ty, lbl_prf, 0xFF4CAF72u, FONT_13_BOLD, false);
        for (size_t i = 0; i < prf_lines.size(); ++i) {
            int lx = bx + pad_x + ((i == 0) ? indent_prf : 0);
            draw_text(lx, ty, prf_lines[i].c_str(), 0xFFD8DEE9u, FONT_13, false);
            ty += line_h;
        }
    }

    return bh;
}

/* Zeichnet einen allgemeinen Textkasten (z. B. fuer Zusatzfunktionen oder reine Funktions-Versionen) */
static int draw_plain_detail_box(int bx, int by, int bw, const std::string &text, bool draw_it) {
    const int pad_x = 12;
    const int pad_y = 9;
    const int line_h = 18;
    const int inner_w = bw - 2 * pad_x;

    std::vector<std::string> lines = wrap_text_to_lines(text, inner_w, FONT_13, 0);
    if (lines.empty()) lines.push_back("-");

    int bh = pad_y * 2 + (int)lines.size() * line_h;
    if (!draw_it) return bh;

    fill_rect(bx, by, bw, bh, 0xFF131924u);
    draw_border_1px(bx, by, bw, bh, 0xFF2B3548u);

    int ty = by + pad_y;
    for (size_t i = 0; i < lines.size(); ++i) {
        draw_text(bx + pad_x, ty, lines[i].c_str(), 0xFFD8DEE9u, FONT_13, false);
        ty += line_h;
    }
    return bh;
}

static void darken_framebuffer(int keep_factor_256) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; ++i) {
        uint32_t c = g_fb[i];
        int r = (((int)((c >> 16) & 0xFF)) * keep_factor_256) >> 8;
        int g = (((int)((c >> 8)  & 0xFF)) * keep_factor_256) >> 8;
        int b = (((int)(c         & 0xFF)) * keep_factor_256) >> 8;
        g_fb[i] = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    }
}

static std::string clip_text_px_right(const std::string &s, int max_px, int font_idx = FONT_14) {
    if (text_width_px(s.c_str(), font_idx) <= max_px) return s;
    std::string cur = s;
    while (!cur.empty()) {
        while (!cur.empty() && (((unsigned char)cur.back() & 0xC0) == 0x80)) {
            cur.pop_back();
        }
        if (!cur.empty()) cur.pop_back();
        std::string cand = cur + " \xE2\x80\xA6";
        if (text_width_px(cand.c_str(), font_idx) <= max_px) {
            return cand;
        }
    }
    return "\xE2\x80\xA6";
}

static void clamp_fanpatch_scroll(void) {
    int max_sc = g_fp_content_h - g_fp_view_h;
    if (max_sc < 0) max_sc = 0;
    if (g_fp_scroll_y > max_sc) g_fp_scroll_y = max_sc;
    if (g_fp_scroll_y < 0) g_fp_scroll_y = 0;
}

static void render_fanpatch_overlay(void) {
    if (!g_fp_active) return;
    if (g_fp_game_idx < 0 || g_fp_game_idx >= (int)g_games.size() || g_fp_game_idx >= 3) return;

    const GameCatalogEntry &g = g_games[g_fp_game_idx];
    FanpatchData &fp = g_fp_data[g_fp_game_idx];
    g_fp_hits.clear();

    /* Hintergrund abdunkeln */
    darken_framebuffer(105);

    /* Modales Fenster mittig */
    const int mw = 860;
    const int mh = 610;
    const int mx = (SCREEN_W - mw) / 2; /* 90 */
    const int my = (SCREEN_H - mh) / 2; /* 18 */

    fill_rect(mx, my, mw, mh, 0xFF181F2Du);
    draw_border_1px(mx, my, mw, mh, 0xFF3E4E68u);

    /* Titel: "<Spielname> – Fanpatch vX.Y" in Gold */
    std::string ver_label = !g.version.empty() ? g.version : ("Fanpatch " + fp.aktuelle_version);
    std::string win_title = g.name + " \xE2\x80\x93 " + ver_label;
    draw_text(mx + 22, my + 14, win_title.c_str(), 0xFFE3A63Au, FONT_23_BOLD, false);

    /* Untere Leiste: "Auf GitHub ansehen" (links), Legende (Mitte), "Schließen (Esc)" (rechts) */
    const int bot_line_y = my + mh - 54;
    fill_rect(mx + 1, bot_line_y, mw - 2, 1, 0xFF263042u);

    const int btn_gh_x = mx + 22;
    const int btn_gh_y = my + mh - 43;
    const int btn_gh_w = 158;
    const int btn_gh_h = 32;
    draw_button(btn_gh_x, btn_gh_y, btn_gh_w, btn_gh_h, "Auf GitHub ansehen", false, true, false, true, FONT_14);
    g_fp_hits.push_back({btn_gh_x, btn_gh_y, btn_gh_w, btn_gh_h, FP_ACT_GITHUB, 0, 0});

    const int btn_cl_w = 136;
    const int btn_cl_h = 32;
    const int btn_cl_x = mx + mw - 22 - btn_cl_w;
    const int btn_cl_y = my + mh - 43;
    draw_button(btn_cl_x, btn_cl_y, btn_cl_w, btn_cl_h, "Schlie\xC3\x9F""en (Esc)", false, true, true, true, FONT_14);
    g_fp_hits.push_back({btn_cl_x, btn_cl_y, btn_cl_w, btn_cl_h, FP_ACT_CLOSE, 0, 0});

    /* Legende unten mittig */
    const char *legende =
        "Wichtig = in \xE2\x80\x9E""Wichtige\xE2\x80\x9C und \xE2\x80\x9E""Alle\xE2\x80\x9C enthalten \xC2\xB7 "
        "nur Alle = nur in \xE2\x80\x9E""Alle\xE2\x80\x9C \xC2\xB7 \xF0\x9F\x92\xB0 = Exploit";
    int leg_x = btn_gh_x + btn_gh_w + 6;
    int leg_w = btn_cl_x - leg_x - 6;
    draw_text_centered(leg_x, btn_gh_y + 8, leg_w, legende, 0xFF8A94A6u, FONT_12, false);

    /* Fehlt die json-Datei: Fenster zeigt nur "Verlauf nicht gefunden" + GitHub-Knopf, kein Absturz */
    if (!fp.loaded) {
        fill_rect(mx + 1, my + 52, mw - 2, 1, 0xFF2E384Cu);
        draw_text_centered(mx + 20, my + mh / 2 - 20, mw - 40, "Verlauf nicht gefunden", 0xFFE6E9EFu, FONT_15_SEMI, false);
        SDL_SetHandCursor(0);
        return;
    }

    /* Drei Reiter: "Versionen (N)", "Alle Korrekturen (N)", "Zusatzfunktionen" */
    char tab0_lbl[64], tab1_lbl[64];
    snprintf(tab0_lbl, sizeof(tab0_lbl), "Versionen (%d)", (int)fp.versionen.size());
    snprintf(tab1_lbl, sizeof(tab1_lbl), "Alle Korrekturen (%d)", (int)fp.korrekturen.size());
    const char *tab_labels[3] = { tab0_lbl, tab1_lbl, "Zusatzfunktionen" };

    const int tab_y = my + 50;
    const int tab_h = 31;
    fill_rect(mx + 1, tab_y + tab_h, mw - 2, 1, 0xFF323E52u);

    int cur_tx = mx + 22;
    for (int ti = 0; ti < 3; ++ti) {
        int tw = text_width_px(tab_labels[ti], FONT_14) + 28;
        bool is_act = (g_fp_tab == ti);
        bool is_hov = point_in_rect(g_mouse_x, g_mouse_y, cur_tx, tab_y, tw, tab_h);
        uint32_t tbg  = is_act ? 0xFF232E42u : (is_hov ? 0xFF1E2738u : 0xFF1A2130u);
        uint32_t tbrd = is_act ? 0xFF4A5D7Au : 0xFF2D374Au;
        uint32_t tfg  = is_act ? 0xFFFFFFFFu : (is_hov ? 0xFFE6E9EFu : 0xFF9AA3B2u);
        fill_rect(cur_tx, tab_y, tw, tab_h, tbg);
        draw_border_1px(cur_tx, tab_y, tw, tab_h, tbrd);
        if (is_act) {
            fill_rect(cur_tx + 1, tab_y + tab_h - 2, tw - 2, 2, 0xFF5AA9E6u);
        }
        draw_text_centered(cur_tx, tab_y + 6, tw, tab_labels[ti], tfg, FONT_14, false);
        g_fp_hits.push_back({cur_tx, tab_y, tw, tab_h, FP_ACT_TAB, ti, 0});
        cur_tx += tw + 6;
    }

    /* Werkzeugzeile unter den Reitern */
    const int tb_y = my + 91;
    const int tb_h = 28;
    if (g_fp_tab == 0 || g_fp_tab == 2) {
        const char *hint = (g_fp_tab == 0)
            ? "Klick auf eine Zeile klappt sie auf \xC2\xB7"
            : "Zusatz- und Komfortfunktionen des Fanpatches \xC2\xB7";
        draw_text(mx + 22, tb_y + 5, hint, 0xFF9AA3B2u, FONT_13, false);
        int bx1 = mx + 22 + text_width_px(hint, FONT_13) + 10;
        int bw1 = 114;
        int bx2 = bx1 + bw1 + 8;
        int bw2 = 110;
        draw_button(bx1, tb_y, bw1, tb_h, "Alle aufklappen", false, true, false, true, FONT_13);
        draw_button(bx2, tb_y, bw2, tb_h, "Alle zuklappen",  false, true, false, true, FONT_13);
        g_fp_hits.push_back({bx1, tb_y, bw1, tb_h, FP_ACT_EXPAND_ALL, 0, 0});
        g_fp_hits.push_back({bx2, tb_y, bw2, tb_h, FP_ACT_COLLAPSE_ALL, 0, 0});
    } else {
        /* Reiter "Alle Korrekturen": Suchfeld + Filter "Alle" / "nur Wichtige" / "💰 Exploits" + Auf-/Zuklappen */
        int sx = mx + 22;
        int sw = 294;
        fill_rect(sx, tb_y, sw, tb_h, 0xFF121722u);
        draw_border_1px(sx, tb_y, sw, tb_h, g_fp_search_focused ? 0xFF5AA9E6u : 0xFF3A4558u);
        if (g_fp_search_text.empty()) {
            const char *ph = "Suchen ... z. B. \xE2\x80\x9E""Truhe\xE2\x80\x9C oder \xE2\x80\x9E""Nr. 90\xE2\x80\x9C";
            draw_text(sx + 10, tb_y + 5, ph, 0xFF6E7889u, FONT_13, false);
            if (g_fp_search_focused) {
                fill_rect(sx + 9, tb_y + 6, 1, 16, 0xFF5AA9E6u);
            }
        } else {
            draw_text(sx + 10, tb_y + 5, g_fp_search_text.c_str(), 0xFFE6E9EFu, FONT_13, false);
            if (g_fp_search_focused) {
                int caret_x = sx + 11 + text_width_px(g_fp_search_text.c_str(), FONT_13);
                if (caret_x < sx + sw - 24) {
                    fill_rect(caret_x, tb_y + 6, 1, 16, 0xFF5AA9E6u);
                }
            }
            /* Kleines 'x' zum Loeschen der Suche */
            draw_text_centered(sx + sw - 22, tb_y + 4, 20, "\xC3\x97", 0xFF9AA3B2u, FONT_14, false);
            g_fp_hits.push_back({sx + sw - 22, tb_y, 22, tb_h, FP_ACT_SEARCH_CLEAR, 0, 0});
        }
        g_fp_hits.push_back({sx, tb_y, sw - (g_fp_search_text.empty() ? 0 : 22), tb_h, FP_ACT_SEARCH_BOX, 0, 0});

        const char *flt_lbls[3] = { "Alle", "nur Wichtige", "\xF0\x9F\x92\xB0 Exploits" };
        const int   flt_ws[3]   = { 46, 98, 92 };
        int fx = sx + sw + 8;
        for (int fi = 0; fi < 3; ++fi) {
            int fw = flt_ws[fi];
            draw_button(fx, tb_y, fw, tb_h, flt_lbls[fi], g_fp_filter == fi, true, false, true, FONT_13);
            g_fp_hits.push_back({fx, tb_y, fw, tb_h, FP_ACT_FILTER, fi, 0});
            fx += fw + 6;
        }

        int bw2 = 104;
        int bx2 = mx + mw - 22 - bw2;
        int bw1 = 108;
        int bx1 = bx2 - 6 - bw1;
        draw_button(bx1, tb_y, bw1, tb_h, "Alle aufklappen", false, true, false, true, FONT_13);
        draw_button(bx2, tb_y, bw2, tb_h, "Alle zuklappen",  false, true, false, true, FONT_13);
        g_fp_hits.push_back({bx1, tb_y, bw1, tb_h, FP_ACT_EXPAND_ALL, 0, 0});
        g_fp_hits.push_back({bx2, tb_y, bw2, tb_h, FP_ACT_COLLAPSE_ALL, 0, 0});
    }

    /* Scrollbarer Inhaltsbereich */
    const int list_x = mx + 22;
    const int list_y = my + 128;
    const int list_w = mw - 44 - 14; /* 14 px rechts fuer Scrollbalken */
    const int list_h = bot_line_y - 8 - list_y;
    g_fp_view_h = list_h;

    /* Zwei Durchlaeufe: Pass 0 misst Gesamt-Inhaltshoehe fuer Scroll-Begrenzung, Pass 1 zeichnet mit Clip-Rect */
    for (int pass = 0; pass < 2; ++pass) {
        bool draw_it = (pass == 1);
        if (draw_it) {
            clamp_fanpatch_scroll();
            set_clip_rect(list_x, list_y, list_w, list_h);
        }
        int cur_y = list_y - (draw_it ? g_fp_scroll_y : 0);

        if (g_fp_tab == 0) {
            /* Reiter 0: Versionen (neueste oben) */
            for (size_t vi = 0; vi < fp.versionen.size(); ++vi) {
                const FanpatchVersionEntry &v = fp.versionen[vi];
                bool is_installed = (vi == 0);
                if (!g.version.empty()) {
                    std::string expected_v = "Fanpatch " + v.version;
                    is_installed = (_stricmp(g.version.c_str(), expected_v.c_str()) == 0 ||
                                    _stricmp(g.version.c_str(), v.version.c_str()) == 0);
                }

                int row_h = 31;
                if (draw_it) {
                    bool row_vis = (cur_y + row_h > list_y && cur_y < list_y + list_h);
                    if (row_vis) {
                        bool hov_row = point_in_rect(g_mouse_x, g_mouse_y, list_x, cur_y, list_w, row_h) &&
                                       point_in_rect(g_mouse_x, g_mouse_y, list_x, list_y, list_w, list_h);
                        if (is_installed) {
                            fill_rect(list_x, cur_y, list_w, row_h, hov_row ? 0xFF233246u : 0xFF1D2A3Bu);
                        } else if (hov_row) {
                            fill_rect(list_x, cur_y, list_w, row_h, 0xFF1E2636u);
                        }
                        draw_plus_minus_sign(list_x + 10, cur_y + 14, v.expanded, 0xFFB8C2D4u);
                        draw_text(list_x + 26, cur_y + 5, v.version.c_str(), 0xFFE3A63Au, FONT_15_SEMI, false);
                        if (!v.datum.empty()) {
                            draw_text(list_x + 86, cur_y + 7, v.datum.c_str(), 0xFF9AA3B2u, FONT_13, false);
                        }
                        int tit_x = list_x + 168;
                        int inst_w = is_installed ? (text_width_px("installiert", FONT_13_BOLD) + 16) : 0;
                        int max_tit_w = (list_x + list_w - 8) - tit_x - inst_w;
                        std::string disp_tit = clip_text_px_right(v.titel, max_tit_w, FONT_14);
                        draw_text(tit_x, cur_y + 5, disp_tit.c_str(), 0xFFE6E9EFu, FONT_14, false);
                        if (is_installed) {
                            int ix = tit_x + text_width_px(disp_tit.c_str(), FONT_14) + 10;
                            draw_text(ix, cur_y + 7, "installiert", 0xFF4CAF72u, FONT_13_BOLD, false);
                        }
                        g_fp_hits.push_back({list_x, cur_y, list_w, row_h, FP_ACT_VER_TOGGLE, (int)vi, 0});
                    }
                }
                cur_y += row_h;

                if (v.expanded) {
                    int tree_y0 = cur_y;
                    int indent_x = list_x + 26;
                    if (!v.korrekturen.empty()) {
                        for (size_t ki = 0; ki < v.korrekturen.size(); ++ki) {
                            const FanpatchVerKorrRef &kr = v.korrekturen[ki];
                            int k_idx = find_korrektur_index_by_nr(fp, kr.nr);
                            if (k_idx < 0) continue;
                            const FanpatchKorrekturEntry &k = fp.korrekturen[k_idx];
                            bool k_open = (ki < v.korr_expanded.size()) ? v.korr_expanded[ki] : false;

                            int kr_h = 28;
                            if (draw_it) {
                                bool kr_vis = (cur_y + kr_h > list_y && cur_y < list_y + list_h);
                                if (kr_vis) {
                                    bool hov_kr = point_in_rect(g_mouse_x, g_mouse_y, indent_x, cur_y, list_w - 26, kr_h) &&
                                                  point_in_rect(g_mouse_x, g_mouse_y, list_x, list_y, list_w, list_h);
                                    if (hov_kr) {
                                        fill_rect(indent_x, cur_y, list_w - 26, kr_h, 0xFF1E2636u);
                                    }
                                    draw_plus_minus_sign(indent_x + 8, cur_y + 13, k_open, 0xFF9AA3B2u);
                                    std::string nr_lbl = "Nr. " + k.nr;
                                    draw_text(indent_x + 22, cur_y + 5, nr_lbl.c_str(), 0xFF9AA3B2u, FONT_13, false);
                                    draw_stufe_word_badge(indent_x + 88, cur_y + 5, k.stufe);

                                    std::string art_lbl = "(" + kr.art + ")";
                                    int art_w = text_width_px(art_lbl.c_str(), FONT_13) + 12;
                                    int exp_w = k.exploit ? 20 : 0;
                                    int avail_kt = (list_x + list_w - 6) - (indent_x + 150) - art_w - exp_w;
                                    std::string k_tit = clip_text_px_right(k.titel, avail_kt, FONT_14) + (k.exploit ? " \xF0\x9F\x92\xB0" : "");
                                    draw_text(indent_x + 150, cur_y + 4, k_tit.c_str(), 0xFFE6E9EFu, FONT_14, false);
                                    int art_x = indent_x + 150 + text_width_px(k_tit.c_str(), FONT_14) + 8;
                                    draw_text(art_x, cur_y + 5, art_lbl.c_str(), 0xFF7C8698u, FONT_13, false);
                                    g_fp_hits.push_back({indent_x, cur_y, list_w - 26, kr_h, FP_ACT_VKORR_TOGGLE, (int)vi, (int)ki});
                                }
                            }
                            cur_y += kr_h;

                            if (k_open) {
                                int bx = indent_x + 88;
                                int bw = (list_x + list_w) - bx - 4;
                                int bh = draw_korrektur_detail_box(bx, cur_y + 2, bw, k, draw_it);
                                cur_y += bh + 8;
                            }
                        }
                    } else if (!v.text.empty()) {
                        int bx = indent_x + 18;
                        int bw = (list_x + list_w) - bx - 4;
                        int bh = draw_plain_detail_box(bx, cur_y + 2, bw, v.text, draw_it);
                        cur_y += bh + 8;
                    }
                    if (draw_it && cur_y > tree_y0 + 4) {
                        fill_rect(list_x + 10, tree_y0, 1, cur_y - tree_y0 - 4, 0xFF323E52u);
                    }
                }
                if (draw_it) {
                    fill_rect(list_x, cur_y, list_w, 1, 0xFF222B3Cu);
                }
                cur_y += 2;
            }
        } else if (g_fp_tab == 1) {
            /* Reiter 1: Alle Korrekturen (nach Bereich gruppiert, mit Suche und Filter) */
            int total_matches = 0;
            for (size_t bi = 0; bi < fp.bereiche.size(); ++bi) {
                const FanpatchAreaGroup &ag = fp.bereiche[bi];
                std::vector<int> matching;
                for (size_t m = 0; m < ag.korr_indices.size(); ++m) {
                    int k_idx = ag.korr_indices[m];
                    if (korrektur_matches_filter_and_search(fp.korrekturen[k_idx])) {
                        matching.push_back(k_idx);
                    }
                }
                if (matching.empty()) continue;
                total_matches += (int)matching.size();

                bool area_open = ag.expanded || !g_fp_search_text.empty();
                int ar_h = 31;
                if (draw_it) {
                    bool ar_vis = (cur_y + ar_h > list_y && cur_y < list_y + list_h);
                    if (ar_vis) {
                        bool hov_ar = point_in_rect(g_mouse_x, g_mouse_y, list_x, cur_y, list_w, ar_h) &&
                                      point_in_rect(g_mouse_x, g_mouse_y, list_x, list_y, list_w, list_h);
                        if (hov_ar) {
                            fill_rect(list_x, cur_y, list_w, ar_h, 0xFF1E2636u);
                        }
                        draw_plus_minus_sign(list_x + 10, cur_y + 14, area_open, 0xFFB8C2D4u);
                        draw_text(list_x + 26, cur_y + 5, ag.display_name.c_str(), 0xFFE6E9EFu, FONT_15_SEMI, false);
                        char cnt_buf[32];
                        snprintf(cnt_buf, sizeof(cnt_buf), "%d", (int)matching.size());
                        int cx_cnt = list_x + 26 + text_width_px(ag.display_name.c_str(), FONT_15_SEMI) + 8;
                        draw_text(cx_cnt, cur_y + 6, cnt_buf, 0xFF9AA3B2u, FONT_14, false);
                        g_fp_hits.push_back({list_x, cur_y, list_w, ar_h, FP_ACT_AREA_TOGGLE, (int)bi, 0});
                    }
                }
                cur_y += ar_h;

                if (area_open) {
                    int tree_y0 = cur_y;
                    int indent_x = list_x + 26;
                    for (size_t mi = 0; mi < matching.size(); ++mi) {
                        int k_idx = matching[mi];
                        const FanpatchKorrekturEntry &k = fp.korrekturen[k_idx];
                        int kr_h = 28;
                        if (draw_it) {
                            bool kr_vis = (cur_y + kr_h > list_y && cur_y < list_y + list_h);
                            if (kr_vis) {
                                bool hov_kr = point_in_rect(g_mouse_x, g_mouse_y, indent_x, cur_y, list_w - 26, kr_h) &&
                                              point_in_rect(g_mouse_x, g_mouse_y, list_x, list_y, list_w, list_h);
                                if (hov_kr) {
                                    fill_rect(indent_x, cur_y, list_w - 26, kr_h, 0xFF1E2636u);
                                }
                                draw_plus_minus_sign(indent_x + 8, cur_y + 13, k.expanded, 0xFF9AA3B2u);
                                std::string nr_lbl = "Nr. " + k.nr;
                                draw_text(indent_x + 22, cur_y + 5, nr_lbl.c_str(), 0xFF9AA3B2u, FONT_13, false);
                                draw_stufe_word_badge(indent_x + 88, cur_y + 5, k.stufe);

                                int exp_w = k.exploit ? 20 : 0;
                                int avail_kt = (list_x + list_w - 6) - (indent_x + 150) - exp_w;
                                std::string k_tit = clip_text_px_right(k.titel, avail_kt, FONT_14) + (k.exploit ? " \xF0\x9F\x92\xB0" : "");
                                draw_text(indent_x + 150, cur_y + 4, k_tit.c_str(), 0xFFE6E9EFu, FONT_14, false);
                                g_fp_hits.push_back({indent_x, cur_y, list_w - 26, kr_h, FP_ACT_KORR_TOGGLE, k_idx, 0});
                            }
                        }
                        cur_y += kr_h;

                        if (k.expanded) {
                            int bx = indent_x + 88;
                            int bw = (list_x + list_w) - bx - 4;
                            int bh = draw_korrektur_detail_box(bx, cur_y + 2, bw, k, draw_it);
                            cur_y += bh + 8;
                        }
                    }
                    if (draw_it && cur_y > tree_y0 + 4) {
                        fill_rect(list_x + 10, tree_y0, 1, cur_y - tree_y0 - 4, 0xFF323E52u);
                    }
                }
                if (draw_it) {
                    fill_rect(list_x, cur_y, list_w, 1, 0xFF222B3Cu);
                }
                cur_y += 2;
            }
            if (total_matches == 0 && draw_it) {
                draw_text_centered(list_x, list_y + 60, list_w, "Keine passenden Korrekturen gefunden.", 0xFF9AA3B2u, FONT_14, false);
            }
        } else if (g_fp_tab == 2) {
            /* Reiter 2: Zusatzfunktionen */
            for (size_t zi = 0; zi < fp.zusatzfunktionen.size(); ++zi) {
                const FanpatchZusatzEntry &z = fp.zusatzfunktionen[zi];
                int row_h = 31;
                if (draw_it) {
                    bool row_vis = (cur_y + row_h > list_y && cur_y < list_y + list_h);
                    if (row_vis) {
                        bool hov_row = point_in_rect(g_mouse_x, g_mouse_y, list_x, cur_y, list_w, row_h) &&
                                       point_in_rect(g_mouse_x, g_mouse_y, list_x, list_y, list_w, list_h);
                        if (hov_row) {
                            fill_rect(list_x, cur_y, list_w, row_h, 0xFF1E2636u);
                        }
                        draw_plus_minus_sign(list_x + 10, cur_y + 14, z.expanded, 0xFFB8C2D4u);
                        draw_text(list_x + 26, cur_y + 5, z.titel.c_str(), 0xFFE3A63Au, FONT_15_SEMI, false);
                        std::string meta = z.version;
                        if (!z.stufe.empty()) {
                            if (!meta.empty()) meta += " \xC2\xB7 ";
                            meta += z.stufe;
                        }
                        if (!meta.empty()) {
                            int tit_w = text_width_px(z.titel.c_str(), FONT_15_SEMI);
                            int meta_x = list_x + 26 + tit_w + 12;
                            int avail_m = (list_x + list_w - 8) - meta_x;
                            if (avail_m > 40) {
                                std::string disp_m = clip_text_px_right(meta, avail_m, FONT_12);
                                draw_text(meta_x, cur_y + 8, disp_m.c_str(), 0xFF9AA3B2u, FONT_12, false);
                            }
                        }
                        g_fp_hits.push_back({list_x, cur_y, list_w, row_h, FP_ACT_ZUS_TOGGLE, (int)zi, 0});
                    }
                }
                cur_y += row_h;

                if (z.expanded) {
                    int bx = list_x + 26;
                    int bw = list_w - 30;
                    int bh = draw_plain_detail_box(bx, cur_y + 2, bw, z.text, draw_it);
                    cur_y += bh + 8;
                }
                if (draw_it) {
                    fill_rect(list_x, cur_y, list_w, 1, 0xFF222B3Cu);
                }
                cur_y += 3;
            }
        }

        if (pass == 0) {
            g_fp_content_h = cur_y - list_y;
        } else {
            reset_clip_rect();
        }
    }

    /* Scrollbalken rechts neben der Liste */
    g_fp_sb_track_x = mx + mw - 18;
    g_fp_sb_track_y = list_y;
    g_fp_sb_track_w = 8;
    g_fp_sb_track_h = list_h;
    fill_rect(g_fp_sb_track_x, g_fp_sb_track_y, g_fp_sb_track_w, g_fp_sb_track_h, 0xFF121722u);

    if (g_fp_content_h > g_fp_view_h && g_fp_content_h > 0) {
        int th_h = (g_fp_view_h * g_fp_view_h) / g_fp_content_h;
        if (th_h < 28) th_h = 28;
        if (th_h > g_fp_view_h) th_h = g_fp_view_h;
        int max_sc = g_fp_content_h - g_fp_view_h;
        int avail_track = g_fp_view_h - th_h;
        int th_y = list_y + ((avail_track > 0 && max_sc > 0) ? (g_fp_scroll_y * avail_track) / max_sc : 0);
        g_fp_sb_thumb_y = th_y;
        g_fp_sb_thumb_h = th_h;
        bool hov_sb = g_fp_dragging_sb || point_in_rect(g_mouse_x, g_mouse_y, g_fp_sb_track_x - 2, th_y, g_fp_sb_track_w + 4, th_h);
        fill_rect(g_fp_sb_track_x, th_y, g_fp_sb_track_w, th_h, hov_sb ? 0xFF5AA9E6u : 0xFF3A475Cu);
    } else {
        g_fp_sb_thumb_y = list_y;
        g_fp_sb_thumb_h = list_h;
        fill_rect(g_fp_sb_track_x, list_y, g_fp_sb_track_w, list_h, 0xFF232B3Cu);
    }

    SDL_SetHandCursor(0);
}

static void fanpatch_set_expand_all(FanpatchData &fp, bool expand) {
    if (g_fp_tab == 0) {
        for (size_t vi = 0; vi < fp.versionen.size(); ++vi) {
            fp.versionen[vi].expanded = expand;
            /* Bei "Alle aufklappen" auf Versionen auch deren Korrekturen aufklappen (ausser v1.0 mit >40 Eintraegen, damit es uebersichtlich bleibt; bei erneutem Klick auch diese) */
            bool exp_k = expand && (fp.versionen[vi].korrekturen.size() <= 40);
            for (size_t ki = 0; ki < fp.versionen[vi].korr_expanded.size(); ++ki) {
                fp.versionen[vi].korr_expanded[ki] = exp_k;
            }
        }
    } else if (g_fp_tab == 1) {
        for (size_t bi = 0; bi < fp.bereiche.size(); ++bi) {
            fp.bereiche[bi].expanded = expand;
        }
        for (size_t ki = 0; ki < fp.korrekturen.size(); ++ki) {
            fp.korrekturen[ki].expanded = expand;
        }
    } else if (g_fp_tab == 2) {
        for (size_t zi = 0; zi < fp.zusatzfunktionen.size(); ++zi) {
            fp.zusatzfunktionen[zi].expanded = expand;
        }
    }
    if (!expand) {
        g_fp_scroll_y = 0;
    }
}

static void handle_fanpatch_click(int mx, int my) {
    const int win_w = 860, win_h = 610;
    const int win_x = (SCREEN_W - win_w) / 2;
    const int win_y = (SCREEN_H - win_h) / 2;

    /* Klick ausserhalb des Fensters schliesst es */
    if (!point_in_rect(mx, my, win_x, win_y, win_w, win_h)) {
        g_fp_active = false;
        g_fp_search_focused = false;
        g_fp_dragging_sb = false;
        return;
    }

    FanpatchData &fp = g_fp_data[g_fp_game_idx];

    /* Klick auf den Scrollbalken */
    if (fp.loaded && point_in_rect(mx, my, g_fp_sb_track_x - 4, g_fp_sb_track_y, g_fp_sb_track_w + 8, g_fp_sb_track_h)) {
        if (g_fp_content_h > g_fp_view_h) {
            if (my >= g_fp_sb_thumb_y && my < g_fp_sb_thumb_y + g_fp_sb_thumb_h) {
                g_fp_dragging_sb   = true;
                g_fp_drag_start_my = my;
                g_fp_drag_start_sc = g_fp_scroll_y;
            } else {
                int avail_track = g_fp_view_h - g_fp_sb_thumb_h;
                int max_sc = g_fp_content_h - g_fp_view_h;
                int rel_y = my - g_fp_sb_track_y - g_fp_sb_thumb_h / 2;
                if (avail_track > 0) {
                    g_fp_scroll_y = (rel_y * max_sc) / avail_track;
                    clamp_fanpatch_scroll();
                }
                g_fp_dragging_sb   = true;
                g_fp_drag_start_my = my;
                g_fp_drag_start_sc = g_fp_scroll_y;
            }
        }
        return;
    }

    const int list_y0 = win_y + 128;
    const int list_y1 = win_y + win_h - 62;

    for (size_t i = 0; i < g_fp_hits.size(); ++i) {
        const FanpatchHitBox &hb = g_fp_hits[i];
        /* Scrollbare Zeilen nur innerhalb des sichtbaren Listenbereichs klickbar */
        if (hb.action >= FP_ACT_VER_TOGGLE && hb.action <= FP_ACT_ZUS_TOGGLE) {
            if (my < list_y0 || my >= list_y1) continue;
        }
        if (point_in_rect(mx, my, hb.x, hb.y, hb.w, hb.h)) {
            if (hb.action != FP_ACT_SEARCH_BOX) {
                g_fp_search_focused = false;
            }
            switch (hb.action) {
            case FP_ACT_TAB:
                g_fp_tab = hb.idx1;
                g_fp_scroll_y = 0;
                break;
            case FP_ACT_EXPAND_ALL:
                fanpatch_set_expand_all(fp, true);
                break;
            case FP_ACT_COLLAPSE_ALL:
                fanpatch_set_expand_all(fp, false);
                break;
            case FP_ACT_SEARCH_BOX:
                g_fp_search_focused = true;
                break;
            case FP_ACT_SEARCH_CLEAR:
                g_fp_search_text.clear();
                g_fp_search_focused = true;
                g_fp_scroll_y = 0;
                break;
            case FP_ACT_FILTER:
                g_fp_filter = hb.idx1;
                g_fp_scroll_y = 0;
                break;
            case FP_ACT_VER_TOGGLE:
                if (hb.idx1 >= 0 && hb.idx1 < (int)fp.versionen.size()) {
                    fp.versionen[hb.idx1].expanded = !fp.versionen[hb.idx1].expanded;
                }
                break;
            case FP_ACT_VKORR_TOGGLE:
                if (hb.idx1 >= 0 && hb.idx1 < (int)fp.versionen.size()) {
                    FanpatchVersionEntry &ve = fp.versionen[hb.idx1];
                    if (hb.idx2 >= 0 && hb.idx2 < (int)ve.korr_expanded.size()) {
                        ve.korr_expanded[hb.idx2] = !ve.korr_expanded[hb.idx2];
                    }
                }
                break;
            case FP_ACT_AREA_TOGGLE:
                if (hb.idx1 >= 0 && hb.idx1 < (int)fp.bereiche.size()) {
                    fp.bereiche[hb.idx1].expanded = !fp.bereiche[hb.idx1].expanded;
                }
                break;
            case FP_ACT_KORR_TOGGLE:
                if (hb.idx1 >= 0 && hb.idx1 < (int)fp.korrekturen.size()) {
                    fp.korrekturen[hb.idx1].expanded = !fp.korrekturen[hb.idx1].expanded;
                }
                break;
            case FP_ACT_ZUS_TOGGLE:
                if (hb.idx1 >= 0 && hb.idx1 < (int)fp.zusatzfunktionen.size()) {
                    fp.zusatzfunktionen[hb.idx1].expanded = !fp.zusatzfunktionen[hb.idx1].expanded;
                }
                break;
            case FP_ACT_GITHUB: {
                const char *no_exec = getenv("STARTER_NO_BROWSER_EXEC");
                if (!g_no_browser_exec && (!no_exec || no_exec[0] != '1') && !fp.github_url.empty()) {
                    ShellExecuteA(NULL, "open", fp.github_url.c_str(), NULL, NULL, SW_SHOWNORMAL);
                }
                g_status_msg = "GitHub-Änderungsliste geöffnet: " + fp.github_url;
                break;
            }
            case FP_ACT_CLOSE:
                g_fp_active = false;
                g_fp_search_focused = false;
                g_fp_dragging_sb = false;
                break;
            }
            return;
        }
    }
    g_fp_search_focused = false;
}

static void handle_fanpatch_motion(int mx, int my) {
    (void)mx;
    if (g_fp_dragging_sb && g_fp_content_h > g_fp_view_h) {
        int avail_track = g_fp_view_h - g_fp_sb_thumb_h;
        int max_sc = g_fp_content_h - g_fp_view_h;
        if (avail_track > 0) {
            int dy = my - g_fp_drag_start_my;
            g_fp_scroll_y = g_fp_drag_start_sc + (dy * max_sc) / avail_track;
            clamp_fanpatch_scroll();
        }
    }
}

static void handle_fanpatch_mouseup(void) {
    g_fp_dragging_sb = false;
}

static void handle_fanpatch_wheel(int wheel_y) {
    if (!g_fp_active) return;
    g_fp_scroll_y -= wheel_y * 44;
    clamp_fanpatch_scroll();
}

static void handle_fanpatch_keydown(const SDL_KeyboardEvent &kev) {
    int vk = (int)kev.keysym.sym;
    int ascii = (int)(kev.keysym.unused & 0xFF);

    if (vk == VK_ESCAPE) {
        if (g_fp_search_focused && !g_fp_search_text.empty()) {
            g_fp_search_text.clear();
            g_fp_scroll_y = 0;
            return;
        }
        g_fp_active = false;
        g_fp_search_focused = false;
        g_fp_dragging_sb = false;
        return;
    }

    if (vk == VK_UP) {
        g_fp_scroll_y -= 32;
        clamp_fanpatch_scroll();
        return;
    }
    if (vk == VK_DOWN) {
        g_fp_scroll_y += 32;
        clamp_fanpatch_scroll();
        return;
    }
    if (vk == VK_PRIOR) {
        g_fp_scroll_y -= g_fp_view_h - 40;
        clamp_fanpatch_scroll();
        return;
    }
    if (vk == VK_NEXT) {
        g_fp_scroll_y += g_fp_view_h - 40;
        clamp_fanpatch_scroll();
        return;
    }
    if (vk == VK_HOME && !g_fp_search_focused) {
        g_fp_scroll_y = 0;
        return;
    }
    if (vk == VK_END && !g_fp_search_focused) {
        g_fp_scroll_y = g_fp_content_h;
        clamp_fanpatch_scroll();
        return;
    }

    if (g_fp_tab == 1) {
        if (vk == VK_BACK) {
            g_fp_search_focused = true;
            while (!g_fp_search_text.empty() && (((unsigned char)g_fp_search_text.back() & 0xC0) == 0x80)) {
                g_fp_search_text.pop_back();
            }
            if (!g_fp_search_text.empty()) {
                g_fp_search_text.pop_back();
            }
            g_fp_scroll_y = 0;
            return;
        }
        if (ascii >= 32 && ascii != 127 && g_fp_search_text.size() < 42) {
            g_fp_search_focused = true;
            if (ascii < 0x80) {
                g_fp_search_text.push_back((char)ascii);
            } else {
                /* Windows-1252 / Latin-1 Byte (z. B. ä, ö, ü, ß) nach UTF-8 */
                g_fp_search_text.push_back((char)(0xC0 | ((ascii >> 6) & 0x1F)));
                g_fp_search_text.push_back((char)(0x80 | (ascii & 0x3F)));
            }
            g_fp_scroll_y = 0;
            return;
        }
    }
}

static void render_modal_dialog(void) {
    if (!g_modal_active) return;
    bool show_ro_btns = (g_modal_is_readonly_dos && !g_modal_is_ok && !g_modal_is_crash);
    int mw = 840, mh = g_modal_is_crash ? 290 : (show_ro_btns ? 266 : 256);
    int mx = (SCREEN_W - mw) / 2;
    int my = (SCREEN_H - mh) / 2;
    uint32_t bg  = g_modal_is_ok ? 0xFF162820u : 0xFF26171Cu;
    uint32_t brd = g_modal_is_ok ? 0xFF4CAF72u : 0xFFE2553Fu;
    fill_rect(mx, my, mw, mh, bg);
    draw_border_1px(mx, my, mw, mh, brd);

    draw_text_centered(mx, my + 20, mw, g_modal_title.c_str(), g_modal_is_ok ? 0xFF98F4A8u : 0xFFFFB060u, FONT_23_BOLD, false);
    draw_text_centered(mx + 24, my + 68, mw - 48, g_modal_line1.c_str(), 0xFFE6E9EFu, FONT_15, false);
    draw_text_centered(mx + 24, my + 98, mw - 48, g_modal_line2.c_str(), 0xFFD8E0E8u, FONT_14, false);
    draw_text_centered(mx + 24, my + 128, mw - 48, g_modal_line3.c_str(), 0xFF9AA3B2u, FONT_14, false);
    if (!g_modal_line4.empty()) {
        draw_text_centered(mx + 24, my + 160, mw - 48, g_modal_line4.c_str(), 0xFFE3A63Au, FONT_14, false);
    }

    if (show_ro_btns) {
        draw_button(mx + 30,  my + mh - 52, 316, 36, L("Kopie anlegen und patchen", "Create Copy and Patch"), true, true, true, true, FONT_15);
        draw_button(mx + 360, my + mh - 52, 250, 36, L("Zielordner wählen...", "Choose Target Folder..."), false, true, false, true, FONT_15);
        draw_button(mx + 624, my + mh - 52, 186, 36, L("Schließen (Esc)", "Close (Esc)"), false, true, false, true, FONT_15);
    } else {
        draw_button(mx + (mw - 280) / 2, my + mh - 52, 280, 36, L("OK – Schließen (Enter)", "OK – Close (Enter)"), true, true, g_modal_is_ok, true, FONT_15);
    }
}

static void render_progress_modal(const std::string &title, const std::string &sub1, const std::string &cur_file, int percent) {
    if (g_screen == SCREEN_SELECT_GAME) render_screen_select_game();
    else render_screen_settings();

    int mw = 680, mh = 216;
    int mx = (SCREEN_W - mw) / 2;
    int my = (SCREEN_H - mh) / 2;
    fill_rect(mx, my, mw, mh, 0xFF181F2Du);
    draw_border_1px(mx, my, mw, mh, 0xFF5AA9E6u);

    draw_text_centered(mx, my + 20, mw, title.c_str(), 0xFFE3A63Au, FONT_23_BOLD, false);
    draw_text_centered(mx + 20, my + 62, mw - 40, sub1.c_str(), 0xFFE6E9EFu, FONT_15, false);

    int bx = mx + 44, by = my + 102, bw = mw - 88, bh = 32;
    fill_rect(bx, by, bw, bh, 0xFF10151Fu);
    draw_border_1px(bx, by, bw, bh, 0xFF4A5568u);

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    int fill_w = ((bw - 2) * percent) / 100;
    if (fill_w > 0) {
        fill_rect(bx + 1, by + 1, fill_w, bh - 2, 0xFF1F5A36u);
    }
    char pct_buf[64];
    snprintf(pct_buf, sizeof(pct_buf), "%d %%", percent);
    draw_text_centered(bx, by + 6, bw, pct_buf, 0xFFE6E9EFu, FONT_14, false);

    std::string fmsg = std::string(L("Schritt: ", "Step: ")) + clip_text_px_left(cur_file, mw - 120, FONT_13);
    draw_text_centered(mx + 20, my + 152, mw - 40, fmsg.c_str(), 0xFF9AA3B2u, FONT_13, false);

    if (g_texture && g_renderer) {
        SDL_UpdateTexture(g_texture, NULL, g_fb, SCREEN_W * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }

    if (!g_screenshot_progress_path.empty() && !g_screenshot_progress_saved && percent >= 40) {
        save_framebuffer_bmp(g_screenshot_progress_path);
        g_screenshot_progress_saved = true;
    }
}

static void render_frame(void) {
    if (g_screen == SCREEN_SELECT_GAME) {
        render_screen_select_game();
    } else {
        render_screen_settings();
    }
    if (g_fp_active) {
        render_fanpatch_overlay();
    } else if (g_help_active) {
        render_help_overlay();
    } else if (g_modal_active) {
        render_modal_dialog();
    }
    SDL_UpdateTexture(g_texture, NULL, g_fb, SCREEN_W * (int)sizeof(uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
}

static uint32_t png_crc32_buf(const uint8_t *buf, size_t len) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        c ^= buf[i];
        for (int k = 0; k < 8; ++k) {
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
    }
    return c ^ 0xFFFFFFFFu;
}

static void png_write_u32_be(FILE *fp, uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)((v >> 24) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 8)  & 0xFF),
        (uint8_t)(v         & 0xFF)
    };
    fwrite(b, 1, 4, fp);
}

static void png_write_chunk(FILE *fp, const char tag[4], const uint8_t *data, size_t len) {
    png_write_u32_be(fp, (uint32_t)len);
    std::vector<uint8_t> tmp(4 + len);
    memcpy(&tmp[0], tag, 4);
    if (len > 0 && data) memcpy(&tmp[4], data, len);
    fwrite(&tmp[0], 1, 4 + len, fp);
    png_write_u32_be(fp, png_crc32_buf(&tmp[0], 4 + len));
}

static bool save_framebuffer_png(const std::string &png_path) {
    if (png_path.empty()) return false;
    ensure_parent_dirs_win(png_path);
    FILE *fp = fopen(utf8_to_win1252(png_path).c_str(), "wb");
    if (!fp) fp = fopen(png_path.c_str(), "wb");
    if (!fp) return false;

    static const uint8_t sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    fwrite(sig, 1, 8, fp);

    uint8_t ihdr[13] = {
        (uint8_t)((SCREEN_W >> 24) & 0xFF), (uint8_t)((SCREEN_W >> 16) & 0xFF),
        (uint8_t)((SCREEN_W >> 8)  & 0xFF), (uint8_t)(SCREEN_W & 0xFF),
        (uint8_t)((SCREEN_H >> 24) & 0xFF), (uint8_t)((SCREEN_H >> 16) & 0xFF),
        (uint8_t)((SCREEN_H >> 8)  & 0xFF), (uint8_t)(SCREEN_H & 0xFF),
        8, 2, 0, 0, 0
    };
    png_write_chunk(fp, "IHDR", ihdr, 13);

    const size_t raw_row = 1 + SCREEN_W * 3;
    const size_t raw_len = raw_row * SCREEN_H;
    std::vector<uint8_t> raw(raw_len);
    uint32_t s1 = 1, s2 = 0;
    size_t p = 0;
    for (int y = 0; y < SCREEN_H; ++y) {
        raw[p++] = 0;
        s2 = (s2 + s1) % 65521u;
        for (int x = 0; x < SCREEN_W; ++x) {
            uint32_t c = g_fb[y * SCREEN_W + x];
            uint8_t r = (uint8_t)((c >> 16) & 0xFF);
            uint8_t g = (uint8_t)((c >> 8)  & 0xFF);
            uint8_t b = (uint8_t)(c         & 0xFF);
            raw[p++] = r; s1 = (s1 + r) % 65521u; s2 = (s2 + s1) % 65521u;
            raw[p++] = g; s1 = (s1 + g) % 65521u; s2 = (s2 + s1) % 65521u;
            raw[p++] = b; s1 = (s1 + b) % 65521u; s2 = (s2 + s1) % 65521u;
        }
    }
    uint32_t adler = (s2 << 16) | s1;

    std::vector<uint8_t> zdat;
    zdat.reserve(raw_len + (raw_len / 65535 + 2) * 5 + 6);
    zdat.push_back(0x78);
    zdat.push_back(0x01);
    size_t off = 0;
    while (off < raw_len) {
        size_t blk = raw_len - off;
        if (blk > 65535) blk = 65535;
        uint8_t bfinal = (off + blk >= raw_len) ? 1 : 0;
        uint16_t len16 = (uint16_t)blk;
        uint16_t nlen16 = (uint16_t)(~len16);
        zdat.push_back(bfinal);
        zdat.push_back((uint8_t)(len16 & 0xFF));
        zdat.push_back((uint8_t)((len16 >> 8) & 0xFF));
        zdat.push_back((uint8_t)(nlen16 & 0xFF));
        zdat.push_back((uint8_t)((nlen16 >> 8) & 0xFF));
        zdat.insert(zdat.end(), raw.begin() + (ptrdiff_t)off, raw.begin() + (ptrdiff_t)(off + blk));
        off += blk;
    }
    zdat.push_back((uint8_t)((adler >> 24) & 0xFF));
    zdat.push_back((uint8_t)((adler >> 16) & 0xFF));
    zdat.push_back((uint8_t)((adler >> 8)  & 0xFF));
    zdat.push_back((uint8_t)(adler         & 0xFF));

    png_write_chunk(fp, "IDAT", &zdat[0], zdat.size());
    png_write_chunk(fp, "IEND", NULL, 0);
    fclose(fp);
    return true;
}

static bool save_framebuffer_image(const std::string &img_path) {
    if (ends_with_ci(img_path, ".png")) {
        return save_framebuffer_png(img_path);
    }
    return save_framebuffer_bmp(img_path);
}

static void run_starter_verlauf_test(const std::string &out_dir) {
    mkdir_recursive_win(out_dir);
    for (int gi = 0; gi < (int)g_games.size() && gi < 3; ++gi) {
        g_selected_game = gi;
        open_fanpatch_window(gi);
        FanpatchData &fp = g_fp_data[gi];
        const std::string &gid = g_games[gi].id;

        /* 1. Versionen zu (alle Versionen zugeklappt) */
        g_fp_tab = 0;
        g_fp_scroll_y = 0;
        fanpatch_set_expand_all(fp, false);
        render_frame();
        save_framebuffer_png(join_path(out_dir, "test_verlauf_" + gid + "_1_versionen_zu.png"));

        /* 2. Version + Korrektur aufgeklappt (Schicksalsklinge: v1.5g + Nr. 18; Sternenschweif: v1.1 + Nr. 377; Riva: v1.0 + R001) */
        fanpatch_set_expand_all(fp, false);
        const char *target_ver = (gi == 0) ? "v1.5g" : ((gi == 1) ? "v1.1" : "v1.0");
        const char *target_nr  = (gi == 0) ? "18"    : ((gi == 1) ? "377"   : "R001");
        for (size_t vi = 0; vi < fp.versionen.size(); ++vi) {
            if (_stricmp(fp.versionen[vi].version.c_str(), target_ver) == 0) {
                fp.versionen[vi].expanded = true;
                for (size_t ki = 0; ki < fp.versionen[vi].korrekturen.size(); ++ki) {
                    if (_stricmp(fp.versionen[vi].korrekturen[ki].nr.c_str(), target_nr) == 0) {
                        fp.versionen[vi].korr_expanded[ki] = true;
                    }
                }
            }
        }
        render_frame();
        save_framebuffer_png(join_path(out_dir, "test_verlauf_" + gid + "_2_version_aufgeklappt.png"));

        /* 3. Alle Korrekturen mit Suche "Truhe" */
        g_fp_tab = 1;
        g_fp_filter = 0;
        g_fp_search_text = "Truhe";
        g_fp_scroll_y = 0;
        for (size_t bi = 0; bi < fp.bereiche.size(); ++bi) fp.bereiche[bi].expanded = false;
        bool first_truhe_opened = false;
        for (size_t ki = 0; ki < fp.korrekturen.size(); ++ki) {
            fp.korrekturen[ki].expanded = false;
            if (!first_truhe_opened && korrektur_matches_filter_and_search(fp.korrekturen[ki])) {
                fp.korrekturen[ki].expanded = true;
                first_truhe_opened = true;
            }
        }
        render_frame();
        save_framebuffer_png(join_path(out_dir, "test_verlauf_" + gid + "_3_suche_truhe.png"));

        /* 4. Alle Korrekturen mit Filter "nur Wichtige" (und bei Schicksalsklinge Nr. 90 aufgeklappt wie Vorlage 2) */
        g_fp_tab = 1;
        g_fp_filter = 1;
        g_fp_search_text.clear();
        g_fp_scroll_y = 0;
        for (size_t bi = 0; bi < fp.bereiche.size(); ++bi) fp.bereiche[bi].expanded = (bi == 0);
        for (size_t ki = 0; ki < fp.korrekturen.size(); ++ki) fp.korrekturen[ki].expanded = false;
        if (gi == 0) {
            int idx90 = find_korrektur_index_by_nr(fp, "90");
            if (idx90 >= 0) fp.korrekturen[idx90].expanded = true;
        } else if (!fp.bereiche.empty() && !fp.bereiche[0].korr_indices.empty()) {
            fp.korrekturen[fp.bereiche[0].korr_indices[0]].expanded = true;
        }
        render_frame();
        save_framebuffer_png(join_path(out_dir, "test_verlauf_" + gid + "_4_nur_wichtige.png"));

        /* 5. Zusatzfunktionen */
        g_fp_tab = 2;
        g_fp_scroll_y = 0;
        for (size_t zi = 0; zi < fp.zusatzfunktionen.size(); ++zi) {
            fp.zusatzfunktionen[zi].expanded = true;
        }
        render_frame();
        save_framebuffer_png(join_path(out_dir, "test_verlauf_" + gid + "_5_zusatzfunktionen.png"));
    }
    g_fp_active = false;
}

static bool save_framebuffer_bmp(const std::string &bmp_path) {
    if (bmp_path.empty()) return false;
    FILE *fp = fopen(utf8_to_win1252(bmp_path).c_str(), "wb");
    if (!fp) fp = fopen(bmp_path.c_str(), "wb");
    if (!fp) return false;

    uint32_t row_bytes = SCREEN_W * 3;
    uint32_t img_size  = row_bytes * SCREEN_H;
    uint32_t file_size = 54 + img_size;
    unsigned char hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    *(uint32_t *)(hdr + 2)  = file_size;
    *(uint32_t *)(hdr + 10) = 54;
    *(uint32_t *)(hdr + 14) = 40;
    *(int32_t  *)(hdr + 18) = SCREEN_W;
    *(int32_t  *)(hdr + 22) = -SCREEN_H;
    *(uint16_t *)(hdr + 26) = 1;
    *(uint16_t *)(hdr + 28) = 24;
    *(uint32_t *)(hdr + 34) = img_size;
    fwrite(hdr, 1, 54, fp);

    unsigned char row[SCREEN_W * 3];
    for (int y = 0; y < SCREEN_H; ++y) {
        for (int x = 0; x < SCREEN_W; ++x) {
            uint32_t c = g_fb[y * SCREEN_W + x];
            row[x * 3 + 0] = (unsigned char)(c & 0xFF);
            row[x * 3 + 1] = (unsigned char)((c >> 8) & 0xFF);
            row[x * 3 + 2] = (unsigned char)((c >> 16) & 0xFF);
        }
        fwrite(row, 1, row_bytes, fp);
    }
    fclose(fp);
    return true;
}

/* =========================================================================
 * 9. Absturz-Bericht auswerten (T3)
 * ========================================================================= */

static void generate_grafikpaket_for_game(GameCatalogEntry &g) {
    (void)g;
}

static void show_wrong_version_modal(const GameCatalogEntry &g) {
    g_modal_active   = true;
    g_modal_is_ok    = false;
    g_modal_is_crash = false;
    g_modal_title    = L("FASSUNG WIRD NICHT UNTERSTÜTZT", "UNSUPPORTED GAME VERSION");
    g_modal_line1    = L(g.wrong_version_msg_de.c_str(), g.wrong_version_msg_en.c_str());
    g_modal_line2    = L("Bitte den unveränderten Hauptordner der deutschen GOG-Installation auswählen.",
                         "Please select the unmodified main folder of the German GOG installation.");
    std::string disp_dir = !g.wrong_version_dir.empty() ? g.wrong_version_dir : g.custom_spielpfad;
    g_modal_line3    = std::string(L("Geprüfter Ordner: ", "Checked folder: ")) + clip_text_left(disp_dir, 48);
    g_modal_line4.clear();
    g_status_msg     = g_modal_line1;
}

static bool inspect_absturz_log(const std::string &log_path, const std::string &game_name) {
    if (!file_exists_win(log_path) || file_size_win(log_path) <= 0) return false;
    FILE *fp = fopen(utf8_to_win1252(log_path).c_str(), "r");
    if (!fp) return false;

    std::string exc_code, exc_addr, last_fn, loc_str;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        std::string s = trim_str(line);
        if (strncmp(s.c_str(), "Ausnahme:", 9) == 0) exc_code = trim_str(s.substr(9));
        else if (strncmp(s.c_str(), "Adresse:", 8) == 0) exc_addr = trim_str(s.substr(8));
        else if (strncmp(s.c_str(), "Letzte_Funktion:", 16) == 0) last_fn = trim_str(s.substr(16));
        else if (strncmp(s.c_str(), "Spielort:", 9) == 0) loc_str = trim_str(s.substr(9));
    }
    fclose(fp);

    g_modal_active   = true;
    g_modal_is_ok    = false;
    g_modal_is_crash = true;
    g_modal_title    = L("SPIEL UNERWARTET BEENDET", "GAME CLOSED UNEXPECTEDLY");
    g_modal_line1    = game_name + L(" wurde unerwartet beendet.", " terminated unexpectedly.");
    g_modal_line2    = std::string(L("Ausnahme: ", "Exception: ")) + (exc_code.empty() ? "?" : exc_code)
                     + " @ " + (exc_addr.empty() ? "?" : exc_addr);
    g_modal_line3    = loc_str.empty() ? L("Ein Fehlerprotokoll wurde im Spielordner abgelegt.", "A crash log was saved in the game folder.")
                                       : (std::string(L("Spielort: ", "Location: ")) + clip_text_left(loc_str, 50));
    g_modal_line4    = L("Bitte das Fehlerprotokoll unter GitHub Issues (MeowMuhMuhCow/nordlandtrilogie) melden!",
                         "Please report the crash log on GitHub Issues (MeowMuhMuhCow/nordlandtrilogie)!");
    g_status_msg     = L("Spiel unerwartet beendet.", "Game closed unexpectedly.");
    return true;
}

/* =========================================================================
 * 10. Ziehen & Ablegen (SDL_DROPFILE): Ordner-Erkennung & ZIP-Entpackung
 * ========================================================================= */

static void collect_subdirs(const std::string &dir, int max_depth, std::vector<std::string> &out) {
    out.push_back(dir);
    if (max_depth <= 0) return;
    std::string pat = utf8_to_win1252(join_path(dir, "*"));
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
                collect_subdirs(join_path(dir, fd.cFileName), max_depth - 1, out);
            }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

static bool dir_looks_like_game(const GameCatalogEntry &g, const std::string &dir) {
    if (file_exists_in_gamedir(dir, g.kennungsdatei)) return true;
    for (size_t i = 0; i < g.pflichtdateien.size(); ++i) {
        if (file_exists_in_gamedir(dir, g.pflichtdateien[i])) return true;
    }
    if (g.teil == 1 && (file_exists_in_gamedir(dir, "BLADEM.EXE") || file_exists_in_gamedir(dir, "BLADE.DAT"))) return true;
    if (g.teil == 2 && (file_exists_in_gamedir(dir, "STARTRAIL.EXE") || file_exists_in_gamedir(dir, "ROA2.EXE"))) return true;
    if (g.teil == 3 && (file_exists_in_gamedir(dir, "RIVA.ALF") || file_exists_in_gamedir(dir, "DUNGEON.ALF") ||
                        file_exists_in_gamedir(dir, "game.gog") || file_exists_in_gamedir(dir, "dosbox_realms3.conf"))) return true;
    return false;
}

static bool dir_has_port_exe(const GameCatalogEntry &g, const std::string &dir) {
    if (!g.exe_orig.empty() && file_exists_win(join_path(dir, g.exe_orig))) return true;
    if (!g.exe_wichtig.empty() && file_exists_win(join_path(dir, g.exe_wichtig))) return true;
    if (!g.exe_alles.empty() && file_exists_win(join_path(dir, g.exe_alles))) return true;
    if (!g.exe_standard.empty() && file_exists_win(join_path(dir, g.exe_standard))) return true;
    return false;
}

static void handle_dropped_folder(const std::string &raw_folder) {
    std::string base = full_win_path(raw_folder);
    std::vector<std::string> candidates;
    collect_subdirs(base, 3, candidates);
    candidates.push_back(full_win_path(join_path(base, "..")));

    std::vector<std::string> unlocked_names;
    std::string first_missing_game;
    std::string first_missing_file;
    int first_wrong_version_idx = -1;
    int first_matched_idx = -1;

    /* 1. Pruefe, ob im gezogenen/gewaehlten Ordner Port-EXEs liegen */
    for (size_t gi = 0; gi < g_games.size(); ++gi) {
        GameCatalogEntry &g = g_games[gi];
        for (size_t ci = 0; ci < candidates.size(); ++ci) {
            const std::string &cand = candidates[ci];
            if (dir_has_port_exe(g, cand)) {
                g.custom_port_ordner = cand;
                refresh_game_status(g);
                unlocked_names.push_back(g.name);
                if (first_matched_idx < 0) first_matched_idx = (int)gi;
                break;
            }
        }
    }

    /* 2. Pruefe, ob im gezogenen/gewaehlten Ordner Original-Spieldaten (GOG/CD) liegen */
    for (size_t gi = 0; gi < g_games.size(); ++gi) {
        GameCatalogEntry &g = g_games[gi];
        for (size_t ci = 0; ci < candidates.size(); ++ci) {
            const std::string &cand = candidates[ci];
            if (dir_looks_like_game(g, cand)) {
                std::string missing;
                if (validate_gamedir_files(g, cand, missing)) {
                    g.custom_spielpfad = cand;
                    refresh_game_status(g);
                    if (g.gamedir_wrong_version) {
                        if (first_wrong_version_idx < 0) first_wrong_version_idx = (int)gi;
                    } else {
                        unlocked_names.push_back(g.name);
                        if (first_matched_idx < 0) first_matched_idx = (int)gi;
                    }
                    break;
                } else if (first_missing_file.empty()) {
                    first_missing_game = g.name;
                    first_missing_file = missing;
                }
            }
        }
    }

    save_einstellungen_ini();

    if (first_wrong_version_idx >= 0) {
        g_selected_game = first_wrong_version_idx;
        show_wrong_version_modal(g_games[first_wrong_version_idx]);
    } else if (!unlocked_names.empty()) {
        if (first_matched_idx >= 0) g_selected_game = first_matched_idx;
        std::string names_joined;
        for (size_t i = 0; i < unlocked_names.size(); ++i) {
            if (i > 0) names_joined += ", ";
            names_joined += unlocked_names[i];
        }
        g_status_msg = std::string(L("Ordner erkannt & freigeschaltet: ", "Folder recognized & unlocked: ")) + names_joined;
        g_modal_active = false;
    } else if (!first_missing_file.empty()) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("UNVOLLSTÄNDIGER SPIELORDNER", "INCOMPLETE GAME FOLDER");
        g_modal_line1    = std::string(L("Der gewählte Ordner für \"", "The selected folder for \"")) + first_missing_game + L("\" ist unvollständig.", "\" is incomplete.");
        g_modal_line2    = L("Benötigt wird der vollständige Hauptordner der deutschen GOG-Fassung.", "The complete main folder of the German GOG version is required.");
        g_modal_line3    = std::string(L("Ordner: ", "Folder: ")) + clip_text_left(base, 48);
        g_modal_line4.clear();
        g_status_msg     = L("Gewählter Spielordner ist unvollständig.", "Selected game folder is incomplete.");
    } else {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("KEIN SPIELORDNER ERKANNT", "NO GAME FOLDER RECOGNIZED");
        g_modal_line1    = L("Im gewählten Ordner wurde kein Teil der Nordlandtrilogie gefunden.", "No Realms of Arkania game was found in the selected folder.");
        g_modal_line2    = L("Bitte den Original-Spielordner (deutsche GOG-Fassung) eines der drei Spiele wählen.", "Please select the original game folder (German GOG version) of one of the three games.");
        g_modal_line3    = std::string(L("Geprüfter Pfad: ", "Checked path: ")) + clip_text_left(base, 46);
        g_modal_line4.clear();
        g_status_msg     = L("Kein bekannter Spielordner erkannt.", "No known game folder recognized.");
    }
}

static bool zip_has_rel_file(const std::vector<ZipEntryInfo> &entries, const std::string &prefix_lower, const std::string &rel_file) {
    std::string target = prefix_lower + normalize_slash_lower(rel_file);
    for (size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].is_dir && normalize_slash_lower(entries[i].name) == target) {
            return true;
        }
    }
    return false;
}

static void handle_dropped_zip(const std::string &raw_zip_path) {
    std::string w_zip = utf8_to_win1252(full_win_path(raw_zip_path));
    std::vector<ZipEntryInfo> entries;
    if (!zip_list_entries(w_zip.c_str(), entries)) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("UNGÜLTIGES ARCHIV", "INVALID ARCHIVE");
        g_modal_line1    = L("Das gezogene Archiv konnte nicht gelesen werden:", "The dropped archive could not be read:");
        g_modal_line2    = clip_text_left(raw_zip_path, 52);
        g_modal_line3    = L("Bitte ein gültiges Archiv oder einen Spielordner ziehen.", "Please drop a valid archive or a game folder.");
        g_modal_line4.clear();
        g_status_msg     = L("Fehler beim Lesen des Archivs.", "Error reading archive.");
        return;
    }

    std::vector<std::string> unlocked_names;
    std::string first_missing_game;
    std::string first_missing_file;
    int first_wrong_version_idx = -1;
    int first_matched_idx = -1;

    FILE *fp = fopen(w_zip.c_str(), "rb");
    if (!fp) return;

    for (size_t gi = 0; gi < g_games.size(); ++gi) {
        GameCatalogEntry &g = g_games[gi];
        std::string found_prefix_lower;
        bool game_cand_found = false;

        std::vector<std::string> indicators = g.pflichtdateien;
        if (g.teil == 1) { indicators.push_back("BLADEM.EXE"); indicators.push_back("BLADE.DAT"); }
        if (g.teil == 2) { indicators.push_back("STARTRAIL.EXE"); indicators.push_back("ROA2.EXE"); }

        for (size_t ind_i = 0; ind_i < indicators.size() && !game_cand_found; ++ind_i) {
            std::string ind_low = normalize_slash_lower(indicators[ind_i]);
            for (size_t ei = 0; ei < entries.size(); ++ei) {
                if (entries[ei].is_dir) continue;
                std::string en_low = normalize_slash_lower(entries[ei].name);
                if (en_low == ind_low) {
                    found_prefix_lower = "";
                    game_cand_found = true;
                    break;
                } else if (en_low.size() > ind_low.size() + 1 &&
                           en_low.substr(en_low.size() - ind_low.size()) == ind_low &&
                           en_low[en_low.size() - ind_low.size() - 1] == '/') {
                    found_prefix_lower = en_low.substr(0, en_low.size() - ind_low.size());
                    game_cand_found = true;
                    break;
                }
            }
        }

        if (!game_cand_found) continue;

        std::string missing;
        if (!zip_has_rel_file(entries, found_prefix_lower, g.kennungsdatei)) {
            missing = g.kennungsdatei;
        } else {
            for (size_t pi = 0; pi < g.pflichtdateien.size(); ++pi) {
                if (!zip_has_rel_file(entries, found_prefix_lower, g.pflichtdateien[pi])) {
                    missing = g.pflichtdateien[pi];
                    break;
                }
            }
        }

        if (!missing.empty()) {
            if (first_missing_file.empty()) {
                first_missing_game = g.name;
                first_missing_file = missing;
            }
            continue;
        }

        std::vector<size_t> game_entry_indices;
        uint64_t total_bytes = 0;
        for (size_t ei = 0; ei < entries.size(); ++ei) {
            if (entries[ei].is_dir) continue;
            std::string en_low = normalize_slash_lower(entries[ei].name);
            if (found_prefix_lower.empty() ||
                (en_low.size() > found_prefix_lower.size() && en_low.substr(0, found_prefix_lower.size()) == found_prefix_lower)) {
                game_entry_indices.push_back(ei);
                total_bytes += entries[ei].uncomp_size;
            }
        }

        std::string abs_target_dir = join_path(g_data_dir, "Spieldaten\\" + g.id);
        mkdir_recursive_win(abs_target_dir);

        uint64_t done_bytes = 0;
        std::string sub_msg = g.name;
        render_progress_modal(L("ENTPACKE SPIELDATEN...", "EXTRACTING GAME DATA..."), sub_msg, L("Starte...", "Starting..."), 0);

        bool all_extracted = true;
        for (size_t k = 0; k < game_entry_indices.size(); ++k) {
            const ZipEntryInfo &ze = entries[game_entry_indices[k]];
            std::string rel_in_game = ze.name.substr(found_prefix_lower.size());
            if (rel_in_game.find("..") != std::string::npos) continue;

            int pct = (total_bytes > 0)
                ? (int)((done_bytes * 100ull) / total_bytes)
                : (int)(((k + 1) * 100) / (game_entry_indices.size() ? game_entry_indices.size() : 1));
            render_progress_modal(L("ENTPACKE SPIELDATEN...", "EXTRACTING GAME DATA..."), sub_msg, rel_in_game, pct);

            std::string dst_file = join_path(abs_target_dir, rel_in_game);
            ensure_parent_dirs_win(dst_file);
            if (!zip_extract_single_entry(fp, ze, utf8_to_win1252(dst_file).c_str())) {
                all_extracted = false;
                break;
            }
            done_bytes += ze.uncomp_size;
        }

        render_progress_modal(L("ENTPACKE SPIELDATEN...", "EXTRACTING GAME DATA..."), sub_msg, L("Abgeschlossen", "Completed"), 100);

        if (all_extracted) {
            g.custom_spielpfad = abs_target_dir;
            refresh_game_status(g);
            if (g.gamedir_wrong_version) {
                if (first_wrong_version_idx < 0) first_wrong_version_idx = (int)gi;
            } else {
                unlocked_names.push_back(g.name);
                if (first_matched_idx < 0) first_matched_idx = (int)gi;
            }
        }
    }

    fclose(fp);
    save_einstellungen_ini();

    if (first_wrong_version_idx >= 0) {
        g_selected_game = first_wrong_version_idx;
        show_wrong_version_modal(g_games[first_wrong_version_idx]);
    } else if (!unlocked_names.empty()) {
        if (first_matched_idx >= 0) g_selected_game = first_matched_idx;
        std::string names_joined;
        for (size_t i = 0; i < unlocked_names.size(); ++i) {
            if (i > 0) names_joined += ", ";
            names_joined += unlocked_names[i];
        }
        g_status_msg = std::string(L("Archiv entpackt & Spiel freigeschaltet: ", "Archive unpacked & game unlocked: ")) + names_joined;
        g_modal_active = false;
    } else if (!first_missing_file.empty()) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("UNVOLLSTÄNDIGES ARCHIV", "INCOMPLETE ARCHIVE");
        g_modal_line1    = std::string(L("Das Archiv für \"", "The archive for \"")) + first_missing_game + L("\" ist unvollständig.", "\" is incomplete.");
        g_modal_line2    = L("Benötigt wird die vollständige deutsche GOG-Fassung.", "The complete German GOG version is required.");
        g_modal_line3    = std::string(L("Archiv: ", "Archive: ")) + clip_text_left(raw_zip_path, 48);
        g_modal_line4.clear();
        g_status_msg     = L("Archiv enthält keine vollständigen Spieldaten.", "Archive does not contain complete game data.");
    } else {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("KEIN SPIEL IM ARCHIV ERKANNT", "NO GAME FOUND IN ARCHIVE");
        g_modal_line1    = L("Im gezogenen Archiv wurde kein Teil der Nordlandtrilogie gefunden.", "No Realms of Arkania game was found in the dropped archive.");
        g_modal_line2    = L("Bitte die deutsche GOG-Fassung eines der drei Spiele bereitstellen.", "Please provide the German GOG version of one of the three games.");
        g_modal_line3    = std::string(L("Archiv: ", "Archive: ")) + clip_text_left(raw_zip_path, 48);
        g_modal_line4.clear();
        g_status_msg     = L("Kein bekanntes Spiel im Archiv erkannt.", "No known game found in archive.");
    }
}

static void handle_drop_path(const std::string &dropped_path) {
    std::string p = trim_str(dropped_path);
    if (p.empty()) return;
    if (dir_exists_win(p)) {
        handle_dropped_folder(p);
    } else if (ends_with_ci(p, ".zip") && file_exists_win(p)) {
        handle_dropped_zip(p);
    } else if (file_exists_win(p)) {
        std::string parent = full_win_path(join_path(p, ".."));
        handle_dropped_folder(parent);
    } else {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("PFAD NICHT GEFUNDEN", "PATH NOT FOUND");
        g_modal_line1    = L("Der gezogene Pfad existiert nicht:", "The dropped path does not exist:");
        g_modal_line2    = clip_text_left(p, 52);
        g_modal_line3    = L("Bitte einen Spielordner oder ein Archiv ziehen.", "Please drop a game folder or an archive.");
        g_modal_line4.clear();
    }
}

static std::string browse_for_gamedir_dialog(const std::string &prompt_title) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    char display_name[MAX_PATH] = "";
    std::string wtitle = utf8_to_win1252(prompt_title);

    BROWSEINFOA bi;
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = GetActiveWindow();
    bi.pszDisplayName = display_name;
    bi.lpszTitle = wtitle.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (!pidl) return "";
    char selected_path[MAX_PATH * 2] = "";
    BOOL ok = SHGetPathFromIDListA(pidl, selected_path);
    CoTaskMemFree(pidl);
    if (!ok || !selected_path[0]) return "";
    return std::string(selected_path);
}

/* =========================================================================
 * 11. Spielstart: Schreibschutz-Umleitung, Probenlog-Archiv & Prozessstart
 * ========================================================================= */

static void copy_file_if_missing_win(const std::string &src, const std::string &dst) {
    if (!file_exists_win(src) || file_exists_win(dst)) return;
    if (CopyFileA(utf8_to_win1252(src).c_str(), utf8_to_win1252(dst).c_str(), TRUE)) {
        SetFileAttributesA(utf8_to_win1252(dst).c_str(), FILE_ATTRIBUTE_NORMAL);
    }
}

static void seed_savedir_from_gamedir(const GameCatalogEntry &g, const std::string &gamedir, const std::string &savedir) {
    if (_stricmp(gamedir.c_str(), savedir.c_str()) == 0) return;
    if (g.teil == 1) {
        copy_file_if_missing_win(join_path(gamedir, "GAMES.NAM"), join_path(savedir, "GAMES.NAM"));
        copy_file_if_missing_win(join_path(gamedir, "HIMMEL.AN"), join_path(savedir, "HIMMEL.AN"));
        copy_file_if_missing_win(join_path(gamedir, "APFREI.AN"), join_path(savedir, "APFREI.AN"));
        const char *patterns[2] = {"*.GAM", "*.CHR"};
        for (int p = 0; p < 2; ++p) {
            std::string pat = utf8_to_win1252(join_path(gamedir, patterns[p]));
            WIN32_FIND_DATAA fd;
            HANDLE h = FindFirstFileA(pat.c_str(), &fd);
            if (h != INVALID_HANDLE_VALUE) {
                do {
                    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        copy_file_if_missing_win(join_path(gamedir, fd.cFileName), join_path(savedir, fd.cFileName));
                    }
                } while (FindNextFileA(h, &fd));
                FindClose(h);
            }
        }
    } else if (g.teil == 2) {
        copy_file_if_missing_win(join_path(gamedir, "SCHWEIF.CFG"), join_path(savedir, "SCHWEIF.CFG"));
        copy_file_if_missing_win(join_path(gamedir, "SONDS.CFG"),   join_path(savedir, "SONDS.CFG"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\GAMES.NAM"),  join_path(savedir, "GAMES\\GAMES.NAM"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\CHARS1.DAT"), join_path(savedir, "GAMES\\CHARS1.DAT"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\EMPTY"),      join_path(savedir, "GAMES\\EMPTY"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\LASTGAME"),   join_path(savedir, "GAMES\\LASTGAME"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\START.GAM"),  join_path(savedir, "GAMES\\LASTGAME"));
        const char *patterns[2] = {"GAMES\\*.GAM", "GAMES\\*.CHR"};
        for (int p = 0; p < 2; ++p) {
            std::string pat = utf8_to_win1252(join_path(gamedir, patterns[p]));
            WIN32_FIND_DATAA fd;
            HANDLE h = FindFirstFileA(pat.c_str(), &fd);
            if (h != INVALID_HANDLE_VALUE) {
                do {
                    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        copy_file_if_missing_win(join_path(gamedir, std::string("GAMES\\") + fd.cFileName),
                                                 join_path(savedir, std::string("GAMES\\") + fd.cFileName));
                    }
                } while (FindNextFileA(h, &fd));
                FindClose(h);
            }
        }
    } else if (g.teil == 3) {
        copy_file_if_missing_win(join_path(gamedir, "DATA\\RIVA.CFG"),    join_path(savedir, "RIVA.CFG"));
        copy_file_if_missing_win(join_path(gamedir, "RIVA.CFG"),          join_path(savedir, "RIVA.CFG"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\CHARS1.DAT"), join_path(savedir, "GAMES\\CHARS1.DAT"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\EMPTY"),      join_path(savedir, "GAMES\\EMPTY"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\LASTGAME"),   join_path(savedir, "GAMES\\LASTGAME"));
        copy_file_if_missing_win(join_path(gamedir, "GAMES\\START.GAM"),  join_path(savedir, "GAMES\\LASTGAME"));
        std::string pat = utf8_to_win1252(join_path(gamedir, "GAMES\\*.GAM"));
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(pat.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    copy_file_if_missing_win(join_path(gamedir, std::string("GAMES\\") + fd.cFileName),
                                             join_path(savedir, std::string("GAMES\\") + fd.cFileName));
                }
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
    }
}

static std::string current_timestamp_str(void) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04u-%02u-%02u_%02u-%02u-%02u",
             (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
             (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond);
    return std::string(buf);
}

static HANDLE prepare_probenlog(const GameCatalogEntry &g, const std::string &exe_dir, const std::string &savedir, DWORD *out_plog_pid) {
    if (out_plog_pid) *out_plog_pid = 0;
    std::string proben_csv = join_path(savedir, "PROBEN.CSV");
    std::string w_proben_csv = utf8_to_win1252(proben_csv);

    const char *logtag = (g.chosen_stufe == STUFE_WICHTIG) ? "Fix_Wichtig"
                       : ((g.chosen_stufe == STUFE_ORIG) ? "Original" : "Fix_Alles");

    if (file_exists_win(proben_csv)) {
        long sz = file_size_win(proben_csv);
        if (sz <= 0) {
            DeleteFileA(w_proben_csv.c_str());
        } else {
            std::string ts = current_timestamp_str();
            std::string arch_name = std::string("PROBEN_") + logtag + "_" + ts + ".CSV";

            std::string exe_plogs = join_path(exe_dir, "Probenlogs");
            _mkdir(utf8_to_win1252(exe_plogs).c_str());
            std::string dst_exe = join_path(exe_plogs, arch_name);

            std::string save_plogs = join_path(savedir, "Probenlogs");
            if (_stricmp(save_plogs.c_str(), exe_plogs.c_str()) != 0) {
                _mkdir(utf8_to_win1252(save_plogs).c_str());
                std::string dst_save = join_path(save_plogs, arch_name);
                CopyFileA(w_proben_csv.c_str(), utf8_to_win1252(dst_save).c_str(), FALSE);
            }
            CopyFileA(w_proben_csv.c_str(), utf8_to_win1252(dst_exe).c_str(), FALSE);
            DeleteFileA(w_proben_csv.c_str());
        }
    }

    if (!g.chosen_probenlog || g.chosen_stufe == STUFE_ORIG) {
        return NULL;
    }

    FILE *fp = fopen(w_proben_csv.c_str(), "wb");
    if (fp) fclose(fp);

    const char *no_win = getenv("DSA_NO_PROBENLOG_WINDOW");
    if (no_win && no_win[0] == '1') {
        return NULL;
    }

    std::string ps1_candidates[6] = {
        join_path(exe_dir, "Probenlog\\probenlog_fenster.ps1"),
        join_path(exe_dir, "..\\Probenlog\\probenlog_fenster.ps1"),
        join_path(g_starter_dir, g.probenlog_skript),
        join_path(g_exe_dir, "Probenlog\\probenlog_fenster.ps1"),
        join_path(g_starter_dir, "..\\Port\\release\\Probenlog\\probenlog_fenster.ps1"),
        join_path(g.resolved_port_dir, "..\\release\\Probenlog\\probenlog_fenster.ps1")
    };
    std::string ps1_path;
    for (int i = 0; i < 6; ++i) {
        if (file_exists_win(ps1_candidates[i])) {
            ps1_path = full_win_path(ps1_candidates[i]);
            break;
        }
    }
    if (ps1_path.empty()) return NULL;

    char cmd_line[2048];
    snprintf(cmd_line, sizeof(cmd_line),
             "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%s\" -Datei \"%s\"",
             utf8_to_win1252(ps1_path).c_str(), w_proben_csv.c_str());

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNOACTIVATE;

    if (CreateProcessA(NULL, cmd_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        if (out_plog_pid) *out_plog_pid = pi.dwProcessId;
        return pi.hProcess;
    }
    return NULL;
}

static int launch_selected_game(
    GameCatalogEntry &g,
    bool force_missing_exe_test,
    const std::vector<std::string> &extra_port_args,
    bool wait_for_exit,
    DWORD timeout_ms
) {
    save_einstellungen_ini();

    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }

    std::string exe_name;
    if (g.chosen_stufe == STUFE_ORIG)         exe_name = g.exe_orig;
    else if (g.chosen_stufe == STUFE_WICHTIG) exe_name = g.exe_wichtig;
    else                                      exe_name = g.exe_alles;

    std::string exe_path = join_path(g.resolved_port_dir, exe_name);
    if (g.chosen_stufe == STUFE_ALLES && !file_exists_win(exe_path) && !g.exe_standard.empty()) {
        std::string alt_exe = join_path(g.resolved_port_dir, g.exe_standard);
        if (file_exists_win(alt_exe)) {
            exe_name = g.exe_standard;
            exe_path = alt_exe;
        }
    }

    if (force_missing_exe_test || !file_exists_win(exe_path)) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("PROGRAMM NICHT GEFUNDEN", "PROGRAM NOT FOUND");
        g_modal_line1    = L("Das gewählte Spielprogramm wurde nicht gefunden.", "The selected game program was not found.");
        g_modal_line2    = g.name;
        g_modal_line3    = L("Bitte prüfen, ob das Paket vollständig entpackt wurde.", "Please check that the package was extracted completely.");
        g_modal_line4.clear();
        g_status_msg     = L("Programm nicht gefunden.", "Program not found.");
        return -2;
    }

    if (!g.gamedir_valid) {
        const char *hl = getenv("STARTER_HEADLESS");
        if (!hl || hl[0] != '1') {
            std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
        }
    }
    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.gamedir_valid) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("SPIELORDNER FEHLT", "GAME FOLDER MISSING");
        g_modal_line1    = std::string(L("Der Original-Spielordner für \"", "The game data folder for \"")) + g.name + L("\" fehlt.", "\" is missing.");
        g_modal_line2    = L("Benötigt wird die deutsche GOG-Fassung des Spiels.", "The German GOG version of the game is required.");
        g_modal_line3    = L("Bitte den Spielordner ins Fenster ziehen oder [Ordner wählen...] klicken.", "Please drag & drop the game folder or click [Select Folder...].");
        g_modal_line4.clear();
        g_status_msg     = L("Spielordner noch nicht gewählt.", "Game folder not selected yet.");
        return -3;
    }

    /* Vorab-Fassungspruefung direkt vor dem Start (T2) */
    if (!validate_game_version(g, g.resolved_gamedir)) {
        g.gamedir_wrong_version = true;
        g.wrong_version_dir = g.resolved_gamedir;
        g.gamedir_valid = false;
        g.verfuegbar = false;
        show_wrong_version_modal(g);
        return -5;
    }

    std::string full_exe = full_win_path(exe_path);
    std::string exe_dir  = full_win_path(g.resolved_port_dir);
    std::string gamedir  = full_win_path(g.resolved_gamedir);
    std::string savedir  = gamedir;

    static bool s_initial_env_captured = false;
    static std::string s_initial_env_savedir;
    static std::string s_initial_env_gamedir;
    static std::string s_initial_env_game_dir;
    static std::string s_initial_env_spielpfad;
    static std::string s_initial_env_intro;
    static std::string s_initial_env_riva_skip;
    static std::string s_initial_env_fenster;
    static std::string s_initial_env_skalierung;
    static std::string s_initial_env_fullscreen;
    static std::string s_initial_env_probenlog;
    static std::string s_initial_env_drehen;
    if (!s_initial_env_captured) {
        const char *v;
        if ((v = getenv("DSA_SAVE_DIR")) != NULL && v[0])       s_initial_env_savedir    = v;
        if ((v = getenv("DSA_GAMEDIR")) != NULL && v[0])        s_initial_env_gamedir    = v;
        if ((v = getenv("DSA_GAME_DIR")) != NULL && v[0])       s_initial_env_game_dir   = v;
        if ((v = getenv("SPIELPFAD")) != NULL && v[0])          s_initial_env_spielpfad  = v;
        if ((v = getenv("DSA_INTRO")) != NULL && v[0])          s_initial_env_intro      = v;
        if ((v = getenv("RIVA_SKIP_INTRO")) != NULL && v[0])    s_initial_env_riva_skip  = v;
        if ((v = getenv("DSA_FENSTER")) != NULL && v[0])        s_initial_env_fenster    = v;
        if ((v = getenv("DSA_SKALIERUNG")) != NULL && v[0])     s_initial_env_skalierung = v;
        if ((v = getenv("DSA_FULLSCREEN")) != NULL && v[0])     s_initial_env_fullscreen = v;
        if ((v = getenv("PROBENLOG")) != NULL && v[0])          s_initial_env_probenlog  = v;
        if ((v = getenv("DSA_DREHEN")) != NULL && v[0])         s_initial_env_drehen     = v;
        s_initial_env_captured = true;
    }
    g.gamedir_writable = is_dir_writable_win(g.resolved_gamedir);
    if (!s_initial_env_savedir.empty()) {
        savedir = full_win_path(s_initial_env_savedir);
    } else if (!g.gamedir_writable) {
        /* Spielstaende-Umleitung je Spiel wie in den Einzelpaketen (neben der jeweiligen Port-EXE) */
        savedir = join_path(exe_dir, "Spielstaende");
    }

    _mkdir(utf8_to_win1252(savedir).c_str());
    _mkdir(utf8_to_win1252(join_path(savedir, "TEMP")).c_str());
    if (g.teil >= 2) {
        _mkdir(utf8_to_win1252(join_path(savedir, "GAMES")).c_str());
        _mkdir(utf8_to_win1252(join_path(savedir, "CTEMP")).c_str());
    }
    if (_stricmp(savedir.c_str(), gamedir.c_str()) != 0) {
        seed_savedir_from_gamedir(g, gamedir, savedir);
    }

    std::string crash_log_exe  = join_path(exe_dir, "absturz.log");
    std::string crash_log_save = join_path(savedir, "absturz.log");
    DeleteFileA(utf8_to_win1252(crash_log_exe).c_str());
    DeleteFileA(utf8_to_win1252(crash_log_save).c_str());

    DWORD plog_pid = 0;
    HANDLE h_plog = prepare_probenlog(g, exe_dir, savedir, &plog_pid);

    _putenv(("DSA_GAMEDIR=" + utf8_to_win1252(gamedir)).c_str());
    _putenv(("DSA_GAME_DIR=" + utf8_to_win1252(gamedir)).c_str());
    _putenv(("SPIELPFAD=" + utf8_to_win1252(gamedir)).c_str());
    _putenv(("DSA_SAVE_DIR=" + utf8_to_win1252(savedir)).c_str());
    _putenv(g.chosen_intro ? "DSA_INTRO=1" : "DSA_INTRO=0");
    _putenv(g.chosen_intro ? "RIVA_SKIP_INTRO=0" : "RIVA_SKIP_INTRO=1");
    _putenv(("DSA_FENSTER=" + g.chosen_fenster).c_str());
    _putenv(("DSA_SKALIERUNG=" + g.chosen_skalierung).c_str());
    _putenv((g.chosen_fenster == "vollbild") ? "DSA_FULLSCREEN=1" : "DSA_FULLSCREEN=0");
    _putenv((g.chosen_probenlog && g.chosen_stufe != STUFE_ORIG) ? "PROBENLOG=1" : "PROBENLOG=0");
    if (g.id == "schicksalsklinge" || g.teil == 1) {
        _putenv(g.chosen_drehen ? "DSA_DREHEN=1" : "DSA_DREHEN=0");
    } else {
        _putenv(("DSA_DREHEN=" + s_initial_env_drehen).c_str());
    }
    if (wait_for_exit && timeout_ms > 0) {
        char wd_buf[64];
        snprintf(wd_buf, sizeof(wd_buf), "DSA_WATCHDOG_MS=%lu", (unsigned long)timeout_ms);
        _putenv(wd_buf);
        snprintf(wd_buf, sizeof(wd_buf), "RIVA_WATCHDOG_MS=%lu", (unsigned long)timeout_ms);
        _putenv(wd_buf);
    }

    std::string cmd = "\"" + utf8_to_win1252(full_exe) + "\" --gamedir \"" + utf8_to_win1252(gamedir) + "\"";
    if (g.id == "schicksalsklinge" || g.id == "riva") {
        cmd += " --savedir \"" + utf8_to_win1252(savedir) + "\"";
    }
    if (g.chosen_intro) {
        cmd += " --intro";
    } else {
        cmd += " --kein-intro";
    }
    cmd += " --fenster=" + g.chosen_fenster;
    cmd += " --skalierung=" + g.chosen_skalierung;
    if (g.chosen_fenster == "vollbild") {
        cmd += " --vollbild";
    }
    for (size_t i = 0; i < extra_port_args.size(); ++i) {
        const std::string &ea = extra_port_args[i];
        if (ea.find(' ') != std::string::npos && ea.find('"') == std::string::npos) {
            size_t eq = ea.find('=');
            if (eq != std::string::npos && ea[0] == '-') {
                cmd += " " + ea.substr(0, eq + 1) + "\"" + ea.substr(eq + 1) + "\"";
            } else {
                cmd += " \"" + ea + "\"";
            }
        } else {
            cmd += " " + ea;
        }
    }

    std::string work_dir = (g.id == "riva") ? exe_dir : savedir;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    std::vector<char> mutable_cmd(cmd.begin(), cmd.end());
    mutable_cmd.push_back('\0');

    BOOL started = CreateProcessA(
        NULL,
        mutable_cmd.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        utf8_to_win1252(work_dir).c_str(),
        &si,
        &pi
    );
    _putenv(("DSA_GAMEDIR=" + s_initial_env_gamedir).c_str());
    _putenv(("DSA_GAME_DIR=" + s_initial_env_game_dir).c_str());
    _putenv(("SPIELPFAD=" + s_initial_env_spielpfad).c_str());
    _putenv(("DSA_SAVE_DIR=" + s_initial_env_savedir).c_str());
    _putenv(("DSA_INTRO=" + s_initial_env_intro).c_str());
    _putenv(("RIVA_SKIP_INTRO=" + s_initial_env_riva_skip).c_str());
    _putenv(("DSA_FENSTER=" + s_initial_env_fenster).c_str());
    _putenv(("DSA_SKALIERUNG=" + s_initial_env_skalierung).c_str());
    _putenv(("DSA_FULLSCREEN=" + s_initial_env_fullscreen).c_str());
    _putenv(("PROBENLOG=" + s_initial_env_probenlog).c_str());
    _putenv(("DSA_DREHEN=" + s_initial_env_drehen).c_str());
    _putenv("DSA_WATCHDOG_MS=");
    _putenv("RIVA_WATCHDOG_MS=");

    if (!started) {
        if (h_plog) {
            TerminateProcess(h_plog, 0);
            CloseHandle(h_plog);
        }
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("START FEHLGESCHLAGEN", "LAUNCH FAILED");
        g_modal_line1    = L("Das Spielprogramm konnte nicht gestartet werden:", "The game program could not be started:");
        g_modal_line2    = g.name;
        g_modal_line3    = L("Bitte prüfen, ob alle Dateien vollständig entpackt wurden.", "Please check that all files were extracted completely.");
        g_modal_line4.clear();
        return -4;
    }

    CloseHandle(pi.hThread);
    g_status_msg = std::string(L("Gestartet: ", "Started: ")) + g.name;

    /* Starter-Fenster ausblenden, solange das Spiel laeuft; nach Spielende kehrt der Starter zurueck */
    if (g_window) {
        SDL_HideWindow(g_window);
    }

    DWORD exit_code = 0;
    if (wait_for_exit) {
        DWORD wait_ms = (timeout_ms > 0) ? timeout_ms : 25000;
        DWORD wres = WaitForSingleObject(pi.hProcess, wait_ms);
        if (wres == WAIT_TIMEOUT) {
            TerminateProcess(pi.hProcess, 124);
            WaitForSingleObject(pi.hProcess, 2000);
            exit_code = 124;
        } else {
            GetExitCodeProcess(pi.hProcess, &exit_code);
        }
        if (h_plog) {
            Sleep(250);
            TerminateProcess(h_plog, 0);
            CloseHandle(h_plog);
        }
        CloseHandle(pi.hProcess);

        if (g_window) {
            SDL_ShowWindow(g_window);
            SDL_RaiseWindow(g_window);
        }

        if (!inspect_absturz_log(crash_log_exe, g.name)) {
            inspect_absturz_log(crash_log_save, g.name);
        }
        g_status_msg = g.name + L(" beendet – zurück im Starter.", " closed – welcome back to the launcher.");
    } else {
        if (h_plog) CloseHandle(h_plog);
        if (g_watched_game_proc) CloseHandle(g_watched_game_proc);
        g_watched_game_proc = pi.hProcess;
        g_watched_exe_dir   = exe_dir;
        g_watched_save_dir  = savedir;
        g_watched_game_name = g.name;
    }

    mkdir_recursive_win(g_data_dir);
    std::string log_path = join_path(g_data_dir, "letzter_start.log");
    FILE *lfp = fopen(utf8_to_win1252(log_path).c_str(), "w");
    if (lfp) {
        fprintf(lfp, "zeit=%s\n", current_timestamp_str().c_str());
        fprintf(lfp, "spiel=%s\n", g.id.c_str());
        fprintf(lfp, "stufe=%s\n", stufe_to_str(g.chosen_stufe));
        fprintf(lfp, "exe=%s\n", exe_name.c_str());
        fprintf(lfp, "savedir=%s\n", savedir.c_str());
        fprintf(lfp, "fenster=%s\n", g.chosen_fenster.c_str());
        fprintf(lfp, "skalierung=%s\n", g.chosen_skalierung.c_str());
        fprintf(lfp, "probenlog=%d\n", (g.chosen_probenlog && g.chosen_stufe != STUFE_ORIG) ? 1 : 0);
        fprintf(lfp, "intro=%d\n", g.chosen_intro ? 1 : 0);
        if (g.teil == 1) {
            fprintf(lfp, "drehen=%d\n", g.chosen_drehen ? 1 : 0);
        }
        fprintf(lfp, "pid=%lu\n", (unsigned long)pi.dwProcessId);
        fprintf(lfp, "probenlog_pid=%lu\n", (unsigned long)plog_pid);
        fprintf(lfp, "exit_code=%lu\n", (unsigned long)exit_code);
        fclose(lfp);
    }

    return (int)exit_code;
}

/* =========================================================================
 * 11b. Weg 2: DOS-Fassung patchen & In DOSBox spielen (Nachtrag 1 & Nachtrag 3)
 *      Schreibt NIE ueber Originaldateien, erzeugt nur neue *_FIX_*.EXE!
 *      Ist der Spielordner wirklich schreibgeschuetzt, kopiert
 *      "Kopie anlegen und patchen" die fuer DOS noetigen Spieldateien in einen
 *      beschreibbaren Ordner (Nordlandtrilogie\<Spiel>\DOS\Spiel\ bzw. waehlbar),
 *      patcht dort und spielt von dort ueber die GOG-DOSBox.
 * ========================================================================= */

static bool should_skip_dos_copy_dir(const char *dirname) {
    static const char *skip_dirs[] = {
        ".", "..", "DOSBOX", "MUSIC", "__support", "cloud_saves",
        "Documentation", "Video Codec", "Build", "Probenlogs",
        "Nativ", "DOS", "Browser", "Spiel", "Spielstaende", NULL
    };
    for (int i = 0; skip_dirs[i]; ++i) {
        if (_stricmp(dirname, skip_dirs[i]) == 0) return true;
    }
    return false;
}

static bool should_skip_dos_copy_file(const char *filename) {
    std::string low = filename;
    for (size_t i = 0; i < low.size(); ++i) low[i] = (char)tolower((unsigned char)low[i]);
    if (strncmp(low.c_str(), "unins000.", 9) == 0) return true;
    if (strncmp(low.c_str(), "goggame", 7) == 0) return true;
    if (strncmp(low.c_str(), "dosbox", 6) == 0) return true;
    if (strncmp(low.c_str(), ".starter_", 9) == 0) return true;
    if (low.find("_fix_") != std::string::npos) return true;
    static const char *skip_exts[] = {
        ".gog", ".ins", ".inst", ".pdf", ".jpg", ".zip", ".lnk", ".ico",
        ".dll", ".ini", ".info", ".hashdb", ".script", ".log", ".tmp",
        ".ndp", ".ps1", ".py", NULL
    };
    for (int i = 0; skip_exts[i]; ++i) {
        if (ends_with_ci(low, skip_exts[i])) return true;
    }
    return false;
}

static void collect_dos_copy_files(
    const std::string &src_root,
    const std::string &rel_sub,
    int max_depth,
    std::vector<std::string> &out_rel_files
) {
    if (max_depth < 0) return;
    std::string cur_dir = rel_sub.empty() ? src_root : join_path(src_root, rel_sub);
    std::string pat = utf8_to_win1252(join_path(cur_dir, "*"));
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!should_skip_dos_copy_dir(fd.cFileName)) {
                std::string sub_rel = rel_sub.empty() ? std::string(fd.cFileName) : (rel_sub + "\\" + fd.cFileName);
                collect_dos_copy_files(src_root, sub_rel, max_depth - 1, out_rel_files);
            }
        } else {
            if (!should_skip_dos_copy_file(fd.cFileName)) {
                std::string file_rel = rel_sub.empty() ? std::string(fd.cFileName) : (rel_sub + "\\" + fd.cFileName);
                out_rel_files.push_back(file_rel);
            }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

static bool copy_single_file_writable_win(const std::string &src_path, const std::string &dst_path) {
    ensure_parent_dirs_win(dst_path);
    std::string wsrc = utf8_to_win1252(normalize_win_sep(src_path));
    std::string wdst = utf8_to_win1252(normalize_win_sep(dst_path));
    if (file_exists_win(dst_path)) {
        SetFileAttributesA(wdst.c_str(), FILE_ATTRIBUTE_NORMAL);
    }
    if (CopyFileA(wsrc.c_str(), wdst.c_str(), FALSE)) {
        SetFileAttributesA(wdst.c_str(), FILE_ATTRIBUTE_NORMAL);
        return true;
    }
    FILE *in = fopen(wsrc.c_str(), "rb");
    if (!in) return false;
    FILE *out = fopen(wdst.c_str(), "wb");
    if (!out) {
        fclose(in);
        return false;
    }
    std::vector<uint8_t> buf(65536);
    size_t n = 0;
    bool ok = true;
    while ((n = fread(buf.data(), 1, buf.size(), in)) > 0) {
        if (fwrite(buf.data(), 1, n, out) != n) {
            ok = false;
            break;
        }
    }
    fclose(out);
    fclose(in);
    if (ok) {
        SetFileAttributesA(wdst.c_str(), FILE_ATTRIBUTE_NORMAL);
    }
    return ok;
}

static std::string find_gog_dosbox_root(const GameCatalogEntry &g) {
    if (!g.resolved_gamedir.empty() && file_exists_win(join_path(g.resolved_gamedir, "DOSBOX\\DOSBox.exe"))) {
        return full_win_path(g.resolved_gamedir);
    }
    if (!g.resolved_dos_copy_dir.empty() && file_exists_win(join_path(g.resolved_dos_copy_dir, "DOSBOX\\DOSBox.exe"))) {
        return full_win_path(g.resolved_dos_copy_dir);
    }
    for (size_t i = 0; i < g.gog_pfade.size(); ++i) {
        if (file_exists_win(join_path(g.gog_pfade[i], "DOSBOX\\DOSBox.exe"))) {
            return full_win_path(g.gog_pfade[i]);
        }
    }
    return "";
}

static std::string win_short_or_full_path(const std::string &path) {
    std::string w = utf8_to_win1252(full_win_path(path));
    char sbuf[MAX_PATH * 2] = "";
    DWORD n = GetShortPathNameA(w.c_str(), sbuf, sizeof(sbuf) - 1);
    if (n > 0 && n < sizeof(sbuf)) {
        return std::string(sbuf);
    }
    return w;
}

static std::string format_display_copy_path(const std::string &abs_path) {
    std::string full_p = normalize_win_sep(full_win_path(abs_path));
    std::string full_s = normalize_win_sep(full_win_path(g_starter_dir));
    if (!full_s.empty() && full_p.size() > full_s.size() + 1 &&
        _strnicmp(full_p.c_str(), full_s.c_str(), full_s.size()) == 0 &&
        full_p[full_s.size()] == '\\') {
        return std::string("Nordlandtrilogie\\") + full_p.substr(full_s.size() + 1);
    }
    return clip_text_left(full_p, 54);
}

static int run_dos_patcher_on_dir(
    GameCatalogEntry &g,
    const std::string &target_gamedir,
    const std::string &source_gamedir,
    bool is_copy
) {
    std::string gamedir     = full_win_path(target_gamedir);
    std::string src_gamedir = full_win_path(source_gamedir);
    std::string dos_dir     = full_win_path(g.resolved_dos_dir);
    std::string build_dir   = join_path(dos_dir, "Build");
    std::string patchen_ps1 = join_path(dos_dir, "Patch\\patchen.ps1");
    const char *prefix      = (g.teil == 1) ? "BLADEM" : ((g.teil == 2) ? "SCHWEIF" : "RIVA");
    const char *orig_exe    = (g.teil == 1) ? "SCHICKM.EXE" : ((g.teil == 2) ? "SCHWEIF.EXE" : "RIVA.EXE");

    mkdir_recursive_win(build_dir);

    /* SPIELPFAD.txt im DOS-Ordner auf den Ordner mit DOSBox (bzw. Spielordner) setzen */
    {
        std::string gog_box = find_gog_dosbox_root(g);
        std::string sp_val = !gog_box.empty() ? gog_box : gamedir;
        std::string sp_txt = join_path(dos_dir, "SPIELPFAD.txt");
        FILE *fp = fopen(utf8_to_win1252(sp_txt).c_str(), "w");
        if (fp) {
            fprintf(fp, "%s\n", utf8_to_win1252(sp_val).c_str());
            fclose(fp);
        }
    }

    render_progress_modal(
        L("DOS-FASSUNG WIRD GEPATCHT...", "PATCHING DOS VERSION..."),
        g.name,
        L("Erzeuge fehlerbereinigte DOS-Programme (Original bleibt unverändert)...",
          "Generating bug-fixed DOS executables (original stays untouched)..."),
        60
    );

    std::string fw_build = join_path(build_dir, std::string(prefix) + "_FIX_WICHTIG.EXE");
    std::string fa_build = join_path(build_dir, std::string(prefix) + "_FIX_ALLES.EXE");
    DeleteFileA(utf8_to_win1252(fw_build).c_str());
    DeleteFileA(utf8_to_win1252(fa_build).c_str());

    char pcmd[2048];
    if (g.teil == 1) {
        snprintf(pcmd, sizeof(pcmd),
                 "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%s\" -Spiel \"%s\" -Name %s -Ziel \"%s\" -Muster \"BLADEM_FIX_*.ndp\"",
                 utf8_to_win1252(patchen_ps1).c_str(), utf8_to_win1252(gamedir).c_str(), orig_exe, utf8_to_win1252(build_dir).c_str());
    } else {
        snprintf(pcmd, sizeof(pcmd),
                 "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%s\" -Spiel \"%s\" -Name %s -Ziel \"%s\"",
                 utf8_to_win1252(patchen_ps1).c_str(), utf8_to_win1252(gamedir).c_str(), orig_exe, utf8_to_win1252(build_dir).c_str());
    }

    STARTUPINFOA psi;
    PROCESS_INFORMATION ppi;
    memset(&psi, 0, sizeof(psi));
    memset(&ppi, 0, sizeof(ppi));
    psi.cb = sizeof(psi);
    DWORD ps_exit = 1;
    if (CreateProcessA(NULL, pcmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, utf8_to_win1252(dos_dir).c_str(), &psi, &ppi)) {
        WaitForSingleObject(ppi.hProcess, 30000);
        GetExitCodeProcess(ppi.hProcess, &ps_exit);
        CloseHandle(ppi.hThread);
        CloseHandle(ppi.hProcess);
    }
    if (g.teil == 1 && ps_exit == 0) {
        std::string orig_sdat_bak = join_path(gamedir, "SCHICK.DAT.ORIG");
        if (file_exists_win(orig_sdat_bak)) {
            copy_single_file_writable_win(orig_sdat_bak, join_path(gamedir, "SCHICK.DAT"));
        }
        std::string sdat_src_dir = file_exists_win(join_path(src_gamedir, "SCHICK.DAT")) ? src_gamedir : gamedir;
        snprintf(pcmd, sizeof(pcmd),
                 "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%s\" -Spiel \"%s\" -Name SCHICK.DAT -Ziel \"%s\" -Muster \"SCHICK.DAT.ndp\"",
                 utf8_to_win1252(patchen_ps1).c_str(), utf8_to_win1252(sdat_src_dir).c_str(), utf8_to_win1252(build_dir).c_str());
        memset(&psi, 0, sizeof(psi));
        memset(&ppi, 0, sizeof(ppi));
        psi.cb = sizeof(psi);
        if (CreateProcessA(NULL, pcmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, utf8_to_win1252(dos_dir).c_str(), &psi, &ppi)) {
            WaitForSingleObject(ppi.hProcess, 30000);
            CloseHandle(ppi.hThread);
            CloseHandle(ppi.hProcess);
        }
    }

    bool ok = (ps_exit == 0 && file_exists_win(fw_build) && file_exists_win(fa_build));
    if (!ok) {
        g_modal_active          = true;
        g_modal_is_ok           = false;
        g_modal_is_crash        = false;
        g_modal_is_readonly_dos = false;
        g_modal_title           = L("DOS-PATCH FEHLGESCHLAGEN", "DOS PATCH FAILED");
        g_modal_line1           = L("Die fehlerbereinigten DOS-Programme konnten nicht erzeugt werden.", "The bug-fixed DOS programs could not be generated.");
        g_modal_line2           = L("Bitte prüfen, ob die unveränderte deutsche GOG-Fassung vorliegt.", "Please verify that you selected the unmodified German GOG version.");
        g_modal_line3           = g.name;
        g_modal_line4.clear();
        return -6;
    }

    /* Neue Fix-EXEs neben die Original-EXE in den Zielordner legen,
     * NIEMALS Originaldateien (SCHICKM.EXE, SCHICK.DAT, SCHWEIF.EXE, RIVA.EXE) ueberschreiben! */
    std::string fw_game = join_path(gamedir, std::string(prefix) + "_FIX_WICHTIG.EXE");
    std::string fa_game = join_path(gamedir, std::string(prefix) + "_FIX_ALLES.EXE");
    copy_single_file_writable_win(fw_build, fw_game);
    copy_single_file_writable_win(fa_build, fa_game);

    /* Markierungen fuer welchen Spielordner bzw. welche Kopie der DOS-Patch erzeugt wurde */
    {
        std::string marker = join_path(build_dir, "gepatchter_spielpfad.txt");
        FILE *mfp = fopen(utf8_to_win1252(marker).c_str(), "w");
        if (mfp) {
            fprintf(mfp, "%s\n", utf8_to_win1252(gamedir).c_str());
            fclose(mfp);
        }
        std::string src_marker = join_path(build_dir, "quell_spielpfad.txt");
        FILE *sfp = fopen(utf8_to_win1252(src_marker).c_str(), "w");
        if (sfp) {
            fprintf(sfp, "%s\n", utf8_to_win1252(src_gamedir).c_str());
            fclose(sfp);
        }
    }

    refresh_game_status(g);

    mkdir_recursive_win(g_data_dir);
    std::string plog = join_path(g_data_dir, "letzter_dos_patch.log");
    FILE *pfp = fopen(utf8_to_win1252(plog).c_str(), "w");
    if (pfp) {
        fprintf(pfp, "zeit=%s\n", current_timestamp_str().c_str());
        fprintf(pfp, "spiel=%s\n", g.id.c_str());
        fprintf(pfp, "spielpfad=%s\n", gamedir.c_str());
        fprintf(pfp, "quell_spielpfad=%s\n", src_gamedir.c_str());
        fprintf(pfp, "kopie=%d\n", is_copy ? 1 : 0);
        fprintf(pfp, "dos_dir=%s\n", dos_dir.c_str());
        fprintf(pfp, "fix_wichtig=%s\n", fw_game.c_str());
        fprintf(pfp, "fix_alles=%s\n", fa_game.c_str());
        fprintf(pfp, "erfolg=1\n");
        fclose(pfp);
    }

    g_modal_active          = true;
    g_modal_is_ok           = true;
    g_modal_is_crash        = false;
    g_modal_is_readonly_dos = false;
    if (is_copy) {
        g_modal_title = L("DOS-KOPIE ANGELEGT & GEPATCHT", "DOS COPY CREATED & PATCHED");
        g_modal_line1 = g.name + L(" – Beschreibbare DOS-Kopie angelegt und fehlerbereinigt.", " – Writable DOS copy created and bug-fixed.");
        g_modal_line2 = L("Der schreibgeschützte Originalordner wurde nur gelesen und NICHT verändert.", "The write-protected original folder was only read and NOT modified.");
        g_modal_line3 = std::string(L("DOS-Spielkopie: ", "DOS game copy: ")) + format_display_copy_path(gamedir);
        g_modal_line4 = L("Du kannst jetzt über [In DOSBox spielen] direkt aus der Kopie starten.", "You can now start directly from the copy via [Play in DOSBox].");
        g_status_msg  = g.name + L(" – DOS-Kopie angelegt und gepatcht (Original unverändert).", " – DOS copy created and patched (original untouched).");
    } else {
        g_modal_title = L("DOS-FASSUNG ERFOLGREICH GEPATCHT", "DOS VERSION PATCHED SUCCESSFULLY");
        g_modal_line1 = g.name + L(" – Fehlerbereinigte DOS-Programme wurden erzeugt.", " – Bug-fixed DOS programs have been created.");
        g_modal_line2 = L("Die Originaldateien im Spielordner wurden nur gelesen und NICHT verändert.", "The original files in the game folder were only read and NOT modified.");
        g_modal_line3 = L("Du kannst die gepatchte Fassung jetzt über [In DOSBox spielen] starten.", "You can now start the patched version via [Play in DOSBox].");
        g_modal_line4 = std::string(L("Gewählte DOS-Patch-Stufe: ", "Selected DOS patch level: ")) +
                        (g.chosen_stufe == STUFE_ORIG ? L("Original", "Original") :
                         (g.chosen_stufe == STUFE_WICHTIG ? L("Wichtige Fixes", "Essential Fixes") : L("Alle Fixes", "All Fixes")));
        g_status_msg  = g.name + L(" – DOS-Fassung gepatcht (Original unverändert).", " – DOS version patched (original untouched).");
    }
    return 0;
}

static int copy_and_patch_dos_game(GameCatalogEntry &g, const std::string &custom_target_dir) {
    save_einstellungen_ini();

    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.gamedir_valid || g.resolved_gamedir.empty()) {
        return -3;
    }
    if (!validate_game_version(g, g.resolved_gamedir)) {
        g.gamedir_wrong_version = true;
        g.wrong_version_dir = g.resolved_gamedir;
        g.gamedir_valid = false;
        g.verfuegbar = false;
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.has_dos_patcher || g.resolved_dos_dir.empty()) {
        return -4;
    }

    std::string src_gamedir = full_win_path(g.resolved_gamedir);
    std::string dos_dir     = full_win_path(g.resolved_dos_dir);
    std::string copy_dir    = !custom_target_dir.empty()
        ? full_win_path(custom_target_dir)
        : join_path(dos_dir, "Spiel");

    mkdir_recursive_win(copy_dir);
    mkdir_recursive_win(join_path(copy_dir, "TEMP"));
    if (g.teil >= 2) {
        mkdir_recursive_win(join_path(copy_dir, "GAMES"));
        mkdir_recursive_win(join_path(copy_dir, "CTEMP"));
    }

    if (!is_dir_writable_real_win(copy_dir) || _stricmp(copy_dir.c_str(), src_gamedir.c_str()) == 0) {
        g_modal_active          = true;
        g_modal_is_ok           = false;
        g_modal_is_crash        = false;
        g_modal_is_readonly_dos = false;
        g_modal_title           = L("ZIELORDNER NICHT BESCHREIBBAR", "TARGET FOLDER NOT WRITABLE");
        g_modal_line1           = L("Der gewählte Zielordner für die DOS-Kopie ist nicht beschreibbar:", "The selected target folder for the DOS copy is not writable:");
        g_modal_line2           = clip_text_left(copy_dir, 58);
        g_modal_line3           = L("Bitte einen freien, beschreibbaren Ordner auswählen.", "Please choose a writable target folder.");
        g_modal_line4.clear();
        return -8;
    }

    std::vector<std::string> rel_files;
    collect_dos_copy_files(src_gamedir, "", 3, rel_files);

    render_progress_modal(
        L("KOPIE FÜR DOS-PATCH WIRD ANGELEGT...", "CREATING COPY FOR DOS PATCH..."),
        g.name,
        L("Kopiere benötigte DOS-Spieldateien...", "Copying required DOS game files..."),
        5
    );

    for (size_t i = 0; i < rel_files.size(); ++i) {
        int pct = 5 + (int)(((i + 1) * 50) / (rel_files.size() ? rel_files.size() : 1));
        if ((i % 3) == 0 || i + 1 == rel_files.size()) {
            render_progress_modal(
                L("KOPIE FÜR DOS-PATCH WIRD ANGELEGT...", "CREATING COPY FOR DOS PATCH..."),
                g.name,
                rel_files[i],
                pct
            );
        }
        std::string sfile = join_path(src_gamedir, rel_files[i]);
        std::string dfile = join_path(copy_dir, rel_files[i]);
        copy_single_file_writable_win(sfile, dfile);
    }

    return run_dos_patcher_on_dir(g, copy_dir, src_gamedir, true);
}

static int patch_dos_game(GameCatalogEntry &g) {
    save_einstellungen_ini();

    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.gamedir_valid) {
        const char *hl = getenv("STARTER_HEADLESS");
        if (!hl || hl[0] != '1') {
            std::string picked = browse_for_gamedir_dialog(L("Spielordner (GOG) wählen:", "Select game folder (GOG):"));
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
        }
    }
    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.gamedir_valid) {
        g_modal_active          = true;
        g_modal_is_ok           = false;
        g_modal_is_crash        = false;
        g_modal_is_readonly_dos = false;
        g_modal_title           = L("SPIELORDNER FEHLT", "GAME FOLDER MISSING");
        g_modal_line1           = std::string(L("Der Original-Spielordner für \"", "The original game folder for \"")) + g.name + L("\" fehlt.", "\" is missing.");
        g_modal_line2           = L("Benötigt wird die deutsche GOG-Fassung des Spiels.", "The German GOG version of the game is required.");
        g_modal_line3           = L("Bitte den Spielordner ins Fenster ziehen oder [Ordner wählen...] klicken.", "Please drag & drop the game folder or click [Select Folder...].");
        g_modal_line4.clear();
        return -3;
    }
    if (!validate_game_version(g, g.resolved_gamedir)) {
        g.gamedir_wrong_version = true;
        g.wrong_version_dir = g.resolved_gamedir;
        g.gamedir_valid = false;
        g.verfuegbar = false;
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.has_dos_patcher || g.resolved_dos_dir.empty()) {
        g_modal_active          = true;
        g_modal_is_ok           = false;
        g_modal_is_crash        = false;
        g_modal_is_readonly_dos = false;
        g_modal_title           = L("DOS-PATCH NICHT GEFUNDEN", "DOS PATCH NOT FOUND");
        g_modal_line1           = L("Der DOS-Patch-Ordner für dieses Spiel wurde nicht gefunden.", "The DOS patch folder for this game was not found.");
        g_modal_line2           = g.name;
        g_modal_line3           = L("Bitte prüfen, ob das Paket vollständig entpackt wurde.", "Please check that the package was extracted completely.");
        g_modal_line4.clear();
        return -4;
    }

    /* Nachtrag 3: Nur der echte Schreibtest entscheidet (Testdatei anlegen und wieder loeschen). */
    g.gamedir_writable = is_dir_writable_win(g.resolved_gamedir);
    if (!g.gamedir_writable) {
        int gidx = 0;
        for (size_t i = 0; i < g_games.size(); ++i) {
            if (&g_games[i] == &g) {
                gidx = (int)i;
                break;
            }
        }
        std::string def_copy = join_path(full_win_path(g.resolved_dos_dir), "Spiel");
        g_modal_active            = true;
        g_modal_is_ok             = false;
        g_modal_is_crash          = false;
        g_modal_is_readonly_dos   = true;
        g_modal_readonly_game_idx = gidx;
        g_modal_title             = L("SPIELORDNER IST SCHREIBGESCHÜTZT", "GAME FOLDER IS WRITE-PROTECTED");
        if (is_program_files_dir(g.resolved_gamedir)) {
            g_modal_line1 = L("Der gewählte Spielordner (unter 'Program Files') ist tatsächlich schreibgeschützt.",
                              "The selected game folder (under 'Program Files') is write-protected.");
        } else {
            g_modal_line1 = L("Der gewählte Spielordner ist schreibgeschützt (keine Schreibberechtigung).",
                              "The selected game folder is write-protected (no write permission).");
        }
        g_modal_line2 = L("Deine Originaldateien bleiben unangetastet. Mit [Kopie anlegen und patchen] werden",
                          "Your original files stay untouched. Click [Create Copy and Patch] to copy the");
        g_modal_line3 = L("die benötigten DOS-Spieldateien in einen beschreibbaren Ordner kopiert und gepatcht:",
                          "required DOS game files to a writable folder and patch & play from there:");
        g_modal_line4 = std::string(L("Zielordner: ", "Target folder: ")) + format_display_copy_path(def_copy);
        g_status_msg  = L("Spielordner schreibgeschützt – [Kopie anlegen und patchen] verfügbar.",
                          "Game folder is write-protected – [Create Copy and Patch] available.");
        return -7;
    }

    std::string gamedir = full_win_path(g.resolved_gamedir);
    return run_dos_patcher_on_dir(g, gamedir, gamedir, false);
}

static bool patch_schick_dat_name_in_exe(const std::string &exe_path, const char *new_name_10) {
    if (!new_name_10 || strlen(new_name_10) != 10) return false;
    std::string wpath = utf8_to_win1252(exe_path);
    FILE *fp = fopen(wpath.c_str(), "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    if (sz <= 32) {
        fclose(fp);
        return false;
    }
    fseek(fp, 0, SEEK_SET);
    std::vector<uint8_t> buf((size_t)sz);
    if (fread(buf.data(), 1, (size_t)sz, fp) != (size_t)sz) {
        fclose(fp);
        return false;
    }
    fclose(fp);

    static const uint8_t needle[11] = { 'S','C','H','I','C','K','.','D','A','T', 0 };
    bool found = false;
    for (size_t i = 0; i + 11 <= buf.size(); ++i) {
        if (memcmp(&buf[i], needle, 11) == 0) {
            memcpy(&buf[i], new_name_10, 10);
            buf[i + 10] = 0;
            found = true;
        }
    }
    if (!found) return false;
    FILE *out = fopen(wpath.c_str(), "wb");
    if (!out) return false;
    size_t wr = fwrite(buf.data(), 1, buf.size(), out);
    fclose(out);
    return (wr == buf.size());
}

static int launch_dos_game(GameCatalogEntry &g) {
    save_einstellungen_ini();

    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.gamedir_valid) {
        const char *hl = getenv("STARTER_HEADLESS");
        if (!hl || hl[0] != '1') {
            std::string picked = browse_for_gamedir_dialog(L("Spielordner (GOG) wählen:", "Select game folder (GOG):"));
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
        }
    }
    if (g.gamedir_wrong_version) {
        show_wrong_version_modal(g);
        return -5;
    }
    if (!g.gamedir_valid) {
        g_modal_active          = true;
        g_modal_is_ok           = false;
        g_modal_is_crash        = false;
        g_modal_is_readonly_dos = false;
        g_modal_title           = L("SPIELORDNER FEHLT", "GAME FOLDER MISSING");
        g_modal_line1           = std::string(L("Der Original-Spielordner für \"", "The original game folder for \"")) + g.name + L("\" fehlt.", "\" is missing.");
        g_modal_line2           = L("Benötigt wird die deutsche GOG-Fassung des Spiels.", "The German GOG version of the game is required.");
        g_modal_line3           = L("Bitte den Spielordner ins Fenster ziehen oder [Ordner wählen...] klicken.", "Please drag & drop the game folder or click [Select Folder...].");
        g_modal_line4.clear();
        return -3;
    }
    if (!validate_game_version(g, g.resolved_gamedir)) {
        g.gamedir_wrong_version = true;
        g.wrong_version_dir = g.resolved_gamedir;
        g.gamedir_valid = false;
        g.verfuegbar = false;
        show_wrong_version_modal(g);
        return -5;
    }

    bool is_orig    = (g.chosen_stufe == STUFE_ORIG);
    bool is_wichtig = (g.chosen_stufe == STUFE_WICHTIG);

    if (!is_orig && !g.dos_fix_exists) {
        int prc = patch_dos_game(g);
        if (prc != 0) return prc;
    }

    std::string gamedir   = full_win_path(g.resolved_gamedir);
    std::string dos_dir   = full_win_path(g.resolved_dos_dir);
    std::string build_dir = join_path(dos_dir, "Build");
    std::string gog_root  = find_gog_dosbox_root(g);
    const char *prefix    = (g.teil == 1) ? "BLADEM" : ((g.teil == 2) ? "SCHWEIF" : "RIVA");
    const char *orig_exe  = (g.teil == 1) ? "SCHICKM.EXE" : ((g.teil == 2) ? "SCHWEIF.EXE" : "RIVA.EXE");
    const char *dos_exe   = (g.teil == 1) ? "BLADEM.EXE"  : ((g.teil == 2) ? "SCHWEIF.EXE" : "RIVA.EXE");
    const char *stufe_dir = is_orig ? "Original" : (is_wichtig ? "Fix_Wichtig" : "Fix_Alles");
    const char *no_dbox   = getenv("STARTER_NO_DOSBOX_EXEC");
    bool skip_exec        = (no_dbox && no_dbox[0] == '1');

    std::string play_dir;
    const char *modus_str = "direkt";
    if (!g.resolved_dos_copy_dir.empty()) {
        play_dir  = full_win_path(g.resolved_dos_copy_dir);
        modus_str = "kopie";
    } else if (is_program_files_dir(gamedir) || !is_dir_writable_win(gamedir)) {
        /* Schreibgeschuetzter bzw. Program-Files-Ordner: NIEMALS direkt als C: in DOSBox mounten,
         * sondern in eine isolierte Testspielstaende-Arbeitskopie unter Build\ umleiten. */
        play_dir  = join_path(build_dir, std::string("Testspielstaende_") + stufe_dir);
        modus_str = "direkt";
        mkdir_recursive_win(play_dir);
        mkdir_recursive_win(join_path(play_dir, "TEMP"));
        if (g.teil >= 2) {
            mkdir_recursive_win(join_path(play_dir, "GAMES"));
            mkdir_recursive_win(join_path(play_dir, "CTEMP"));
        }
        if (!skip_exec && !file_exists_win(join_path(play_dir, orig_exe))) {
            std::vector<std::string> rel_files;
            collect_dos_copy_files(gamedir, "", 3, rel_files);
            for (size_t i = 0; i < rel_files.size(); ++i) {
                copy_single_file_writable_win(join_path(gamedir, rel_files[i]), join_path(play_dir, rel_files[i]));
            }
        }
        seed_savedir_from_gamedir(g, gamedir, play_dir);
    } else {
        play_dir  = gamedir;
        modus_str = "direkt";
        mkdir_recursive_win(join_path(play_dir, "TEMP"));
        if (g.teil >= 2) {
            mkdir_recursive_win(join_path(play_dir, "GAMES"));
            mkdir_recursive_win(join_path(play_dir, "CTEMP"));
        }
    }

    if (!gog_root.empty() || skip_exec) {
        std::string run_dir = join_path(build_dir, stufe_dir);
        mkdir_recursive_win(run_dir);

        std::string fix_src;
        std::string run_exe_name;
        if (is_orig) {
            run_exe_name = orig_exe;
            fix_src = join_path(gamedir, orig_exe);
            if (!file_exists_win(fix_src) && file_exists_win(join_path(gamedir, std::string("DATA\\") + orig_exe))) {
                fix_src = join_path(gamedir, std::string("DATA\\") + orig_exe);
            }
            if (!file_exists_win(fix_src)) {
                fix_src = join_path(play_dir, orig_exe);
            }
            copy_single_file_writable_win(fix_src, join_path(run_dir, orig_exe));
            if (_stricmp(orig_exe, dos_exe) != 0) {
                copy_single_file_writable_win(fix_src, join_path(run_dir, dos_exe));
            }
        } else {
            run_exe_name = dos_exe;
            std::string fix_name = std::string(prefix) + (is_wichtig ? "_FIX_WICHTIG.EXE" : "_FIX_ALLES.EXE");
            fix_src = join_path(play_dir, fix_name);
            if (!file_exists_win(fix_src)) {
                fix_src = join_path(build_dir, fix_name);
            }
            if (!file_exists_win(fix_src)) {
                fix_src = join_path(gamedir, fix_name);
            }
            copy_single_file_writable_win(fix_src, join_path(run_dir, dos_exe));
        }

        /* DSA 1 (Die Schicksalsklinge): Original-SCHICK.DAT bei Fixes=Original,
         * gepatchte Build\SCHICK.DAT (Magischer Brotbeutel) bei Fixes=Wichtige/Alle.
         * Dabei wird gamedir\SCHICK.DAT NIEMALS ueberschrieben! */
        std::string active_schick_dat = join_path(play_dir, "SCHICK.DAT");
        if (g.teil == 1) {
            std::string patched_sdat = join_path(build_dir, "SCHICK.DAT");
            std::string orig_sdat    = join_path(gamedir, "SCHICK.DAT");
            if (_stricmp(play_dir.c_str(), gamedir.c_str()) != 0) {
                std::string play_sdat      = join_path(play_dir, "SCHICK.DAT");
                std::string play_sdat_orig = join_path(play_dir, "SCHICK.DAT.ORIG");
                if (!file_exists_win(play_sdat_orig)) {
                    if (file_exists_win(orig_sdat)) {
                        copy_single_file_writable_win(orig_sdat, play_sdat_orig);
                    } else if (file_exists_win(play_sdat)) {
                        copy_single_file_writable_win(play_sdat, play_sdat_orig);
                    }
                }
                if (is_orig) {
                    if (file_exists_win(play_sdat_orig)) {
                        copy_single_file_writable_win(play_sdat_orig, play_sdat);
                    } else if (file_exists_win(orig_sdat)) {
                        copy_single_file_writable_win(orig_sdat, play_sdat);
                    }
                } else if (file_exists_win(patched_sdat)) {
                    copy_single_file_writable_win(patched_sdat, play_sdat);
                }
                active_schick_dat = play_sdat;
            } else {
                if (!is_orig && file_exists_win(patched_sdat)) {
                    std::string fix_sdat = join_path(gamedir, "SCHICK.FIX");
                    copy_single_file_writable_win(patched_sdat, fix_sdat);
                    patch_schick_dat_name_in_exe(join_path(run_dir, dos_exe), "SCHICK.FIX");
                    active_schick_dat = fix_sdat;
                } else {
                    active_schick_dat = orig_sdat;
                }
            }
        }

        /* Probenlog: prepare_probenlog archiviert/loescht eine evtl. vorhandene PROBEN.CSV immer
         * und legt sie nur neu an (+ oeffnet das Live-Fenster), wenn Probenlog aktiv UND Fixes != Original. */
        DWORD plog_pid = 0;
        HANDLE h_pl = prepare_probenlog(g, dos_dir, play_dir, &plog_pid);
        if (h_pl) CloseHandle(h_pl);

        /* Fluessig drehen (Schicksalsklinge DOS, FIX_DREHEN): Vorgabe AN (keine DREHEN.AUS),
         * abgeschaltet ueber DREHEN.AUS im DOS-Arbeitsverzeichnis (play_dir) sowie DREHEN_AUS.txt im DOS-Ordner. */
        if (g.teil == 1) {
            std::string drehen_aus_play = join_path(play_dir, "DREHEN.AUS");
            std::string drehen_aus_txt  = join_path(dos_dir,  "DREHEN_AUS.txt");
            if (g.chosen_drehen) {
                DeleteFileA(utf8_to_win1252(drehen_aus_play).c_str());
                DeleteFileA(utf8_to_win1252(drehen_aus_txt).c_str());
            } else {
                FILE *df1 = fopen(utf8_to_win1252(drehen_aus_play).c_str(), "wb");
                if (df1) fclose(df1);
                FILE *df2 = fopen(utf8_to_win1252(drehen_aus_txt).c_str(), "w");
                if (df2) {
                    fprintf(df2, "Hartes Drehen gewuenscht, wieder an mit DREHEN_AN.bat\n");
                    fclose(df2);
                }
            }
        }

        /* Intro-Steuerung fuer alle 3 Spiele in DOSBox:
         * - DSA 1: 'if exist INTRO.EXE intro' vor dem Hauptprogramm (nur wenn chosen_intro == true)
         * - DSA 2: 'E:\SCHWEIF.EXE -x' spielt Intro; 'E:\SCHWEIF.EXE' (mit GAMES\LASTGAME) ueberspringt Intro
         * - DSA 3: 'E:\RIVA.EXE' spielt Intro; 'E:\RIVA.EXE LASTGAME' (mit GAMES\LASTGAME.GAM) ueberspringt Intro */
        std::string exe_cmd = std::string("E:\\") + run_exe_name;
        if (g.teil == 2) {
            if (g.chosen_intro) {
                exe_cmd += " -x";
            } else {
                std::string lg_dst = join_path(play_dir, "GAMES\\LASTGAME");
                if (!file_exists_win(lg_dst)) {
                    std::string lg_src = join_path(gamedir, "GAMES\\LASTGAME");
                    if (file_exists_win(lg_src)) {
                        copy_single_file_writable_win(lg_src, lg_dst);
                    } else {
                        FILE *lfp = fopen(utf8_to_win1252(lg_dst).c_str(), "wb");
                        if (lfp) {
                            fwrite("ROAMING.GAM\r\n", 1, 13, lfp);
                            fclose(lfp);
                        }
                    }
                }
            }
        } else if (g.teil == 3) {
            if (!g.chosen_intro) {
                std::string lg_gam = join_path(play_dir, "GAMES\\LASTGAME.GAM");
                std::string lg_cur = join_path(play_dir, "GAMES\\LASTGAME");
                std::string lg_src = join_path(gamedir,  "GAMES\\LASTGAME");
                std::string st_src = join_path(gamedir,  "GAMES\\START.GAM");
                if (file_exists_win(lg_cur)) {
                    copy_single_file_writable_win(lg_cur, lg_gam);
                } else if (file_exists_win(lg_src)) {
                    copy_single_file_writable_win(lg_src, lg_gam);
                } else if (file_exists_win(st_src)) {
                    copy_single_file_writable_win(st_src, lg_gam);
                }
                exe_cmd += " LASTGAME";
            }
        }

        /* Anzeige-Einstellung fuer DOSBox:
         * - Vollbild: fullscreen=true, fullresolution=desktop, output=openglnb, [render] aspect=true
         * - Fenster 2x..5x: fullscreen=false, windowresolution=640x480|960x720|1280x960|1600x1200 (4:3),
         *   output=openglnb, [render] aspect=true, scaler=normal2x (bzw. normal3x ab 3x) */
        bool is_fs = (g.chosen_fenster == "vollbild");
        const char *win_res = "1280x960";
        const char *scaler  = "normal3x";
        if (g.chosen_fenster == "2") {
            win_res = "640x480";
            scaler  = "normal2x";
        } else if (g.chosen_fenster == "3") {
            win_res = "960x720";
            scaler  = "normal3x";
        } else if (g.chosen_fenster == "4") {
            win_res = "1280x960";
            scaler  = "normal3x";
        } else if (g.chosen_fenster == "5") {
            win_res = "1600x1200";
            scaler  = "normal3x";
        }

        std::string conf_path  = join_path(build_dir, "dosbox_starter_kopie.conf");
        std::string play_short = win_short_or_full_path(play_dir);
        std::string run_short  = win_short_or_full_path(run_dir);
        std::string eff_gog    = !gog_root.empty() ? gog_root : gamedir;

        {
            std::string sp_txt = join_path(dos_dir, "SPIELPFAD.txt");
            FILE *sfp = fopen(utf8_to_win1252(sp_txt).c_str(), "w");
            if (sfp) {
                fprintf(sfp, "%s\n", utf8_to_win1252(eff_gog).c_str());
                fclose(sfp);
            }
        }

        /* Mausempfindlichkeit fuer DOSBox: DOSBox bewegt den DOS-Zeiger pro Bildschirmpunkt der echten Maus um einen
         * Schritt (DOS-Breite 640 Schritte), unabhaengig von der Bildgroesse. Auf grossen Bildschirmen rast der Zeiger
         * daher. Wir skalieren so, dass eine Mausbewegung ueber die Bildbreite den Zeiger einmal ueber das Bild fuehrt. */
        int maus_bildbreite = 1280;
        if (is_fs) {
            int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
            if (sw > 0 && sh > 0) maus_bildbreite = (sw * 3 > sh * 4) ? (sh * 4 / 3) : sw;
        } else {
            int ww = atoi(win_res);
            if (ww > 0) maus_bildbreite = ww;
        }
        int maus_sens = (64000 + maus_bildbreite / 2) / maus_bildbreite;
        if (maus_sens < 10) maus_sens = 10;
        if (maus_sens > 100) maus_sens = 100;
        /* Sternenschweif liest die Maus schneller aus als die anderen beiden Teile: zwei Drittel der Empfindlichkeit (halb war zu langsam) */
        if (g.teil == 2) {
            maus_sens = (maus_sens * 2 + 1) / 3;
            if (maus_sens < 5) maus_sens = 5;
        }

        FILE *cfp = fopen(utf8_to_win1252(conf_path).c_str(), "w");
        if (cfp) {
            fprintf(cfp, "[sdl]\n");
            fprintf(cfp, "sensitivity=%d\n", maus_sens);
            if (is_fs) {
                fprintf(cfp, "fullscreen=true\n");
                fprintf(cfp, "fullresolution=desktop\n");
                fprintf(cfp, "output=openglnb\n");
                fprintf(cfp, "[render]\n");
                fprintf(cfp, "aspect=true\n");
            } else {
                fprintf(cfp, "fullscreen=false\n");
                fprintf(cfp, "windowresolution=%s\n", win_res);
                fprintf(cfp, "output=openglnb\n");
                fprintf(cfp, "[render]\n");
                fprintf(cfp, "aspect=true\n");
                fprintf(cfp, "scaler=%s\n", scaler);
            }
            fprintf(cfp, "[autoexec]\n@echo off\n");
            fprintf(cfp, "mount C \"%s\"\n", play_short.c_str());
            if (g.teil == 1 && file_exists_win(join_path(eff_gog, "game.inst"))) {
                fprintf(cfp, "imgmount D \"%s\" -t iso\n", utf8_to_win1252(join_path(eff_gog, "game.inst")).c_str());
            } else if (g.teil == 2 && file_exists_win(join_path(eff_gog, "gamea.ins"))) {
                fprintf(cfp, "imgmount D \"%s\" \"%s\" -t iso\n",
                        utf8_to_win1252(join_path(eff_gog, "gamea.ins")).c_str(),
                        utf8_to_win1252(join_path(eff_gog, "gameb.ins")).c_str());
            } else if (g.teil == 3 && file_exists_win(join_path(eff_gog, "game.ins"))) {
                std::string gog_short = win_short_or_full_path(eff_gog);
                fprintf(cfp, "imgmount D \"%s\" -t iso -fs iso\n", utf8_to_win1252(join_path(gog_short, "game.ins")).c_str());
            }
            fprintf(cfp, "mount E \"%s\"\n", run_short.c_str());
            fprintf(cfp, "C:\ncls\n");
            if (g.teil == 1 && g.chosen_intro) {
                fprintf(cfp, "if exist INTRO.EXE intro\n");
            }
            fprintf(cfp, "%s\nexit\n", exe_cmd.c_str());
            fclose(cfp);
        }

        std::string dosbox_exe = join_path(eff_gog, "DOSBOX\\DOSBox.exe");
        char realm_conf_name[64];
        snprintf(realm_conf_name, sizeof(realm_conf_name), "dosbox_realms%d.conf", g.teil);
        std::string base_conf = join_path(eff_gog, realm_conf_name);
        std::string args;
        if (file_exists_win(base_conf)) {
            args = "-conf \"..\\" + std::string(realm_conf_name) + "\" -conf \"" + utf8_to_win1252(conf_path) + "\" -noconsole";
        } else {
            args = "-conf \"" + utf8_to_win1252(conf_path) + "\" -noconsole";
        }

        mkdir_recursive_win(g_data_dir);
        std::string dlog = join_path(g_data_dir, "letzter_dosbox_start.log");
        FILE *dfp = fopen(utf8_to_win1252(dlog).c_str(), "w");
        if (dfp) {
            fprintf(dfp, "zeit=%s\n", current_timestamp_str().c_str());
            fprintf(dfp, "spiel=%s\n", g.id.c_str());
            fprintf(dfp, "modus=%s\n", modus_str);
            fprintf(dfp, "stufe=%s\n", stufe_to_str(g.chosen_stufe));
            fprintf(dfp, "fenster=%s\n", g.chosen_fenster.c_str());
            fprintf(dfp, "intro=%d\n", g.chosen_intro ? 1 : 0);
            fprintf(dfp, "probenlog=%d\n", (g.chosen_probenlog && !is_orig) ? 1 : 0);
            fprintf(dfp, "spielpfad=%s\n", play_dir.c_str());
            fprintf(dfp, "gog_dosbox=%s\n", dosbox_exe.c_str());
            fprintf(dfp, "fix_exe=%s\n", fix_src.c_str());
            if (g.teil == 1) {
                fprintf(dfp, "drehen=%d\n", (g.chosen_drehen && !is_orig) ? 1 : 0);
                fprintf(dfp, "schick_dat=%s\n", active_schick_dat.c_str());
            }
            fprintf(dfp, "exe_cmd=%s\n", exe_cmd.c_str());
                fprintf(dfp, "maus_empfindlichkeit=%d\n", maus_sens);
            fprintf(dfp, "conf=%s\n", conf_path.c_str());
            fclose(dfp);
        }

        std::string ok_msg = is_orig
            ? (g.name + L(" – Original-DOS-Fassung in DOSBox gestartet.", " – Original DOS version started in DOSBox."))
            : (g.name + L(" – Gepatchte DOS-Fassung in DOSBox gestartet.", " – Patched DOS version started in DOSBox."));

        if (skip_exec) {
            g_status_msg = ok_msg;
            return 0;
        }
        if (file_exists_win(dosbox_exe)) {
            HINSTANCE hinst = ShellExecuteA(
                NULL,
                "open",
                utf8_to_win1252(dosbox_exe).c_str(),
                args.c_str(),
                utf8_to_win1252(join_path(eff_gog, "DOSBOX")).c_str(),
                SW_SHOWNORMAL
            );
            if ((INT_PTR)hinst > 32) {
                g_status_msg = ok_msg;
                return 0;
            }
        }
    }

    g_modal_active          = true;
    g_modal_is_ok           = false;
    g_modal_is_crash        = false;
    g_modal_is_readonly_dos = false;
    g_modal_title           = L("DOSBOX-START NICHT MÖGLICH", "DOSBOX LAUNCH NOT POSSIBLE");
    g_modal_line1           = L("Die DOSBox-Startumgebung konnte nicht geöffnet werden.", "The DOSBox launch environment could not be opened.");
    g_modal_line2           = g.name;
    g_modal_line3           = L("Für die DOS-Fassung wird DOSBox aus der deutschen GOG-Installation benötigt.", "DOSBox from the German GOG installation is required for the DOS version.");
    g_modal_line4.clear();
    return -4;
}

/* =========================================================================
 * 11c. Weg 3: Im Browser spielen (Nachtrag 1 & Etappe 7: URL-Parameter)
 * ========================================================================= */

static int open_browser_game(GameCatalogEntry &g, bool show_modal) {
    save_einstellungen_ini();
    if (!is_browser_available(g)) {
        return -1;
    }

    std::string target_url = build_browser_launch_url(g);

    mkdir_recursive_win(g_data_dir);
    std::string blog = join_path(g_data_dir, "letzter_browser_start.log");
    FILE *bfp = fopen(utf8_to_win1252(blog).c_str(), "w");
    if (bfp) {
        fprintf(bfp, "zeit=%s\n", current_timestamp_str().c_str());
        fprintf(bfp, "spiel=%s\n", g.id.c_str());
        fprintf(bfp, "stufe=%s\n", stufe_to_str(g.chosen_stufe));
        fprintf(bfp, "intro=%d\n", g.chosen_intro ? 1 : 0);
        fprintf(bfp, "probenlog=%d\n", (g.chosen_probenlog && g.chosen_stufe != STUFE_ORIG) ? 1 : 0);
        fprintf(bfp, "url=%s\n", target_url.c_str());
        fclose(bfp);
    }

    const char *no_exec = getenv("STARTER_NO_BROWSER_EXEC");
    if (!g_no_browser_exec && (!no_exec || no_exec[0] != '1')) {
        ShellExecuteA(NULL, "open", target_url.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    g_status_msg = g.name + L(" – Im Standardbrowser geöffnet: ", " – Opened in default browser: ") + target_url;
    if (show_modal) {
        g_modal_active   = true;
        g_modal_is_ok    = true;
        g_modal_is_crash = false;
        g_modal_title    = L("IM BROWSER SPIELEN", "PLAY IN BROWSER");
        g_modal_line1    = g.name + L(" wurde im Standardbrowser geöffnet:", " has been opened in your default web browser:");
        g_modal_line2    = target_url;
        g_modal_line3    = L("In der Online-Fassung einfach den eigenen GOG-Spielordner ins Browserfenster ziehen.",
                             "In the online edition, simply drag & drop your GOG game folder into the browser window.");
        g_modal_line4.clear();
    }
    return 0;
}

/* =========================================================================
 * 12. Maus- und Tastatur-Ereignisbehandlung (komplett bedienbar)
 * ========================================================================= */

static void cycle_fenster_mode(GameCatalogEntry &g, int dir) {
    static const char *opts[5] = {"2", "3", "4", "5", "vollbild"};
    int cur = 4;
    for (int i = 0; i < 5; ++i) {
        if (_stricmp(g.chosen_fenster.c_str(), opts[i]) == 0) {
            cur = i;
            break;
        }
    }
    int next = (cur + dir + 5) % 5;
    g.chosen_fenster = opts[next];
    g.chosen_vollbild = (g.chosen_fenster == "vollbild");
    g_status_msg = std::string(L("Fenstergröße: ", "Window Size: ")) + fenster_label_str(g.chosen_fenster);
    save_einstellungen_ini();
}

static void toggle_skalierung_mode(GameCatalogEntry &g) {
    if (_stricmp(g.chosen_skalierung.c_str(), "ganzzahlig") == 0) {
        g.chosen_skalierung = "fuellend";
    } else {
        g.chosen_skalierung = "ganzzahlig";
    }
    g_status_msg = std::string(L("Skalierung: ", "Scaling: ")) + skalierung_label_str(g.chosen_skalierung);
    save_einstellungen_ini();
}

static void handle_mouse_click(int mx, int my, bool &running) {
    (void)running;
    g_show_focus_ring = false;

    if (g_fp_active) {
        handle_fanpatch_click(mx, my);
        return;
    }

    /* Header-Knopf "Hilfe (F1)" */
    if (point_in_rect(mx, my, HDR_HELP_X, HDR_HELP_Y, HDR_HELP_W, HDR_HELP_H)) {
        g_open_dd_game = -1;
        g_help_active = !g_help_active;
        return;
    }

    if (g_help_active) {
        g_help_active = false;
        return;
    }
    if (g_modal_active) {
        if (g_modal_is_readonly_dos && !g_modal_is_ok && !g_modal_is_crash) {
            int mw = 840, mh = 266;
            int mdx = (SCREEN_W - mw) / 2;
            int mdy = (SCREEN_H - mh) / 2;
            int gidx = (g_modal_readonly_game_idx >= 0 && g_modal_readonly_game_idx < (int)g_games.size())
                ? g_modal_readonly_game_idx : g_selected_game;
            /* Knopf 1: [Kopie anlegen und patchen] -> Nordlandtrilogie\<Spiel>\DOS\Spiel\ */
            if (point_in_rect(mx, my, mdx + 30, mdy + mh - 52, 316, 36)) {
                g_modal_active = false;
                g_modal_is_readonly_dos = false;
                if (gidx >= 0 && gidx < (int)g_games.size()) {
                    copy_and_patch_dos_game(g_games[gidx], "");
                }
                return;
            }
            /* Knopf 2: [Zielordner wählen...] -> frei waehlbarer Zielordner */
            if (point_in_rect(mx, my, mdx + 360, mdy + mh - 52, 250, 36)) {
                std::string picked = browse_for_gamedir_dialog(
                    L("Zielordner für beschreibbare DOS-Kopie wählen:", "Select target folder for writable DOS copy:"));
                if (!picked.empty()) {
                    g_modal_active = false;
                    g_modal_is_readonly_dos = false;
                    if (gidx >= 0 && gidx < (int)g_games.size()) {
                        copy_and_patch_dos_game(g_games[gidx], picked);
                    }
                }
                return;
            }
            /* Knopf 3: [Schließen (Esc)] */
            if (point_in_rect(mx, my, mdx + 624, mdy + mh - 52, 186, 36)) {
                g_modal_active = false;
                g_modal_is_readonly_dos = false;
                return;
            }
            return;
        }
        g_modal_active = false;
        g_modal_is_readonly_dos = false;
        return;
    }

    if (g_screen == SCREEN_SELECT_GAME) {
        /* Falls eine Anzeige-Klappliste offen ist, zuerst Klicks im Klapplisten-Menue auswerten */
        if (g_open_dd_game >= 0 && g_open_dd_game < (int)g_games.size()) {
            int ddx = 0, ddy = 0, ddw = 0, ddh = 0;
            get_open_dd_menu_rect(g_open_dd_game, ddx, ddy, ddw, ddh);
            if (point_in_rect(mx, my, ddx, ddy, ddw, ddh)) {
                int idx = (my - (ddy + 1)) / 26;
                if (idx >= 0 && idx < 5) {
                    GameCatalogEntry &og = g_games[g_open_dd_game];
                    og.chosen_fenster  = g_dd_fenster_vals[idx];
                    og.chosen_vollbild = (og.chosen_fenster == "vollbild");
                    g_status_msg = og.name + " – " + L("Anzeige: ", "Display: ") + fenster_dd_short_label(og.chosen_fenster);
                    g_open_dd_game = -1;
                    g_open_dd_sel  = -1;
                    save_einstellungen_ini();
                }
                return;
            }
            /* Klick auf den Klapplisten-Knopf derselben Karte schliesst die Liste */
            int ocx = 14, ocy = 147 + g_open_dd_game * 162, ocw = 1012;
            CardRow4Layout oL4;
            compute_card_row4_layout(g_games[g_open_dd_game], ocx, ocy, ocw, oL4);
            if (point_in_rect(mx, my, oL4.dd_x, oL4.y, oL4.dd_w, oL4.h)) {
                g_open_dd_game = -1;
                g_open_dd_sel  = -1;
                return;
            }
            g_open_dd_game = -1;
            g_open_dd_sel  = -1;
        }

        /* Die 3 Spielkarten (x=14, y=147 + i*162, w=1012, h=150) */
        for (size_t i = 0; i < g_games.size() && i < 3; ++i) {
            int cx = 14;
            int cy = 147 + (int)i * 162;
            int cw = 1012;
            int ch = 150;
            int tx0 = cx + 1 + 250 + 18; /* 283 */
            int btn_y = cy + 66;
            int btn_h = 37;

            if (point_in_rect(mx, my, cx, cy, cw, ch)) {
                g_selected_game = (int)i;

                /* Zeile 1: Klick auf "Fanpatch vX.Y" oeffnet das Fanpatch-Verlaufsfenster */
                if (!g_games[i].version.empty()) {
                    const char *dat_lbl = g_games[i].gamedir_valid ? "Spieldaten gefunden"
                        : (g_games[i].gamedir_wrong_version ? "Falsche Fassung" : "Spieldaten fehlen");
                    const char *prog_lbl = g_games[i].has_any_exe ? "Programm gefunden" : "Programm fehlt";
                    int rx = cx + cw - 1 - 16;
                    int dat_bx  = (rx - text_width_px(dat_lbl, FONT_14)) - 21;
                    int prog_bx = (dat_bx - 16 - text_width_px(prog_lbl, FONT_14)) - 21;
                    int dot_x = 0, ver_x = 0, ver_y = 0, ver_w = 0, ver_h = 0;
                    get_card_fanpatch_rect(g_games[i], cx, cy, prog_bx, dot_x, ver_x, ver_y, ver_w, ver_h);
                    if (ver_w > 0 && point_in_rect(mx, my, ver_x - 3, cy + 12, ver_w + 6, 24)) {
                        open_fanpatch_window((int)i);
                        return;
                    }
                }

                /* Zeile 2 rechts (nur Karte 1): Haekchen "Fluessig drehen (Schicksalsklinge)" */
                if (g_games[i].teil == 1) {
                    const char *drehen_lbl = "Fl\xC3\xBCssig drehen (Schicksalsklinge)";
                    int chk_drehen_w = 22 + text_width_px(drehen_lbl, FONT_13);
                    int chk_drehen_x = (cx + cw - 1 - 16) - chk_drehen_w;
                    if (point_in_rect(mx, my, chk_drehen_x, cy + 38, chk_drehen_w, 22)) {
                        g_games[i].chosen_drehen = !g_games[i].chosen_drehen;
                        g_status_msg = g_games[i].name + " – Flüssig drehen: " +
                                       (g_games[i].chosen_drehen ? "Eingeschaltet (Windows & DOSBox)" : "Ausgeschaltet (Windows & DOSBox)");
                        save_einstellungen_ini();
                        return;
                    }
                }

                /* Zeile 3 - WEG 1: [▶ Spielen (Windows)] */
                if (point_in_rect(mx, my, tx0, btn_y, 235, btn_h)) {
                    g_card_focus = 6;
                    if (!g_games[i].has_any_exe) return;
                    if (g_games[i].gamedir_wrong_version) {
                        show_wrong_version_modal(g_games[i]);
                        return;
                    }
                    save_einstellungen_ini();
                    std::vector<std::string> no_extra;
                    launch_selected_game(g_games[i], false, no_extra, false, 0);
                    return;
                }
                /* Zeile 3 - WEG 2: [DOS-Fassung patchen] bzw. [In DOSBox spielen] */
                if (point_in_rect(mx, my, tx0 + 245, btn_y, 236, btn_h)) {
                    g_card_focus = 7;
                    if (g_games[i].gamedir_wrong_version) {
                        show_wrong_version_modal(g_games[i]);
                        return;
                    }
                    if (g_games[i].dos_fix_exists) {
                        launch_dos_game(g_games[i]);
                    } else {
                        patch_dos_game(g_games[i]);
                    }
                    return;
                }
                /* Zeile 3 - WEG 3: [Im Browser spielen] */
                if (point_in_rect(mx, my, tx0 + 491, btn_y, 235, btn_h)) {
                    g_card_focus = 8;
                    if (is_browser_available(g_games[i])) {
                        open_browser_game(g_games[i], false);
                    }
                    return;
                }

                /* Zeile 4: Optionen */
                CardRow4Layout L4;
                compute_card_row4_layout(g_games[i], cx, cy, cw, L4);

                /* 4.1 Segmentschalter "Fixes" [Original | Wichtige | Alle] */
                if (point_in_rect(mx, my, L4.seg_x, L4.y, L4.seg_total_w, L4.h)) {
                    g_card_focus = 0;
                    int rel_x = mx - L4.seg_x;
                    if (rel_x < L4.seg_w[0]) {
                        g_games[i].chosen_stufe = STUFE_ORIG;
                    } else if (rel_x < L4.seg_w[0] + L4.seg_w[1]) {
                        g_games[i].chosen_stufe = STUFE_WICHTIG;
                    } else {
                        g_games[i].chosen_stufe = STUFE_ALLES;
                    }
                    g_status_msg = g_games[i].name + " – Fixes: " + stufe_short_label(g_games[i].chosen_stufe);
                    save_einstellungen_ini();
                    return;
                }

                /* 4.2 Klappliste "Anzeige" */
                if (point_in_rect(mx, my, L4.dd_x, L4.y, L4.dd_w, L4.h)) {
                    g_card_focus = 1;
                    g_open_dd_game = (int)i;
                    g_open_dd_sel  = get_dd_fenster_index(g_games[i].chosen_fenster);
                    return;
                }

                /* 4.3 Haekchen "Intro" */
                if (point_in_rect(mx, my, L4.chk_intro_x, L4.y, L4.chk_intro_w, L4.h)) {
                    g_card_focus = 2;
                    g_games[i].chosen_intro = !g_games[i].chosen_intro;
                    g_status_msg = g_games[i].name + " – Intro: " +
                                   (g_games[i].chosen_intro ? L("Abspielen", "Play") : L("Überspringen", "Skip"));
                    save_einstellungen_ini();
                    return;
                }

                /* 4.4 Haekchen "Probenlog" */
                if (point_in_rect(mx, my, L4.chk_plog_x, L4.y, L4.chk_plog_w, L4.h)) {
                    g_card_focus = 3;
                    g_games[i].chosen_probenlog = !g_games[i].chosen_probenlog;
                    g_status_msg = g_games[i].name + L(" – Probenlog: ", " – Roll Log: ") +
                                   (g_games[i].chosen_probenlog
                                       ? L("Eingeschaltet (mit Live-Fenster)", "Enabled (with live window)")
                                       : L("Ausgeschaltet", "Disabled"));
                    save_einstellungen_ini();
                    return;
                }

                /* 4.5 Rechtsbuendig "Ordner …" */
                if (point_in_rect(mx, my, L4.btn_folder_x, L4.y, L4.btn_folder_w, L4.h)) {
                    g_card_focus = 4;
                    std::string picked = browse_for_gamedir_dialog(
                        (g_games[i].name + L(" – Spielordner wählen:", " – Select game folder:")).c_str());
                    if (!picked.empty()) {
                        handle_dropped_folder(picked);
                    }
                    return;
                }

                /* 4.6 Rechtsbuendig Zahnrad (Einstellungen) */
                if (point_in_rect(mx, my, L4.btn_gear_x, L4.y, L4.btn_gear_w, L4.h)) {
                    g_card_focus = 5;
                    g_screen = SCREEN_SETTINGS;
                    g_settings_row = 0;
                    g_status_msg = g_games[i].name + L(" – Einstellungen anpassen.", " – Customize settings.");
                    save_einstellungen_ini();
                    return;
                }

                /* Klick auf die Karte selbst waehlt das Spiel aus */
                g_status_msg = g_games[i].name + L(" ausgewählt.", " selected.");
                save_einstellungen_ini();
                return;
            }
        }
    } else if (g_screen == SCREEN_SETTINGS) {
        if (g_selected_game < 0 || g_selected_game >= (int)g_games.size()) return;
        GameCatalogEntry &g = g_games[g_selected_game];

        /* 1. Fixes */
        if (point_in_rect(mx, my, 268, 104, 224, 34)) {
            g.chosen_stufe = STUFE_ORIG;
            g_settings_row = 0;
            g_status_msg = L("Fixes: Original", "Fixes: Original");
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 504, 104, 244, 34)) {
            g.chosen_stufe = STUFE_WICHTIG;
            g_settings_row = 0;
            g_status_msg = L("Fixes: Wichtige Fixes", "Fixes: Essential Fixes");
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 760, 104, 250, 34)) {
            g.chosen_stufe = STUFE_ALLES;
            g_settings_row = 0;
            g_status_msg = L("Fixes: Alle Fixes", "Fixes: All Fixes");
            save_einstellungen_ini();
            return;
        }

        /* 2. Anzeige (2x, 3x, 4x, 5x, Vollbild) */
        const int fx_x[5] = { 268, 416, 564, 722, 886 };
        const int fx_w[5] = { 140, 140, 150, 156, 124 };
        const char *fx_v[5] = { "2", "3", "4", "5", "vollbild" };
        for (int fi = 0; fi < 5; ++fi) {
            if (point_in_rect(mx, my, fx_x[fi], 180, fx_w[fi], 34)) {
                g.chosen_fenster = fx_v[fi];
                g.chosen_vollbild = (g.chosen_fenster == "vollbild");
                g_settings_row = 1;
                g_status_msg = std::string(L("Anzeige: ", "Display: ")) + fenster_label_str(g.chosen_fenster);
                save_einstellungen_ini();
                return;
            }
        }

        /* 3. Skalierung (Ganzzahlig vs. Fuellend) */
        if (point_in_rect(mx, my, 268, 256, 364, 34)) {
            g.chosen_skalierung = "ganzzahlig";
            g_settings_row = 2;
            g_status_msg = L("Skalierung: Ganzzahlig (exakte Vielfache)", "Scaling: Integer (exact multiples)");
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 644, 256, 366, 34)) {
            g.chosen_skalierung = "fuellend";
            g_settings_row = 2;
            g_status_msg = L("Skalierung: Füllend (scharf)", "Scaling: Fill (sharp)");
            save_einstellungen_ini();
            return;
        }

        /* 4. Probenlog */
        if (point_in_rect(mx, my, 268, 336, 234, 34)) {
            g.chosen_probenlog = false;
            g_settings_row = 3;
            g_status_msg = L("Probenlog ausgeschaltet.", "Roll Log disabled.");
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 514, 336, 496, 34)) {
            g.chosen_probenlog = true;
            g_settings_row = 3;
            g_status_msg = L("Probenlog eingeschaltet (mit Live-Fenster).", "Roll Log enabled (with live window).");
            save_einstellungen_ini();
            return;
        }

        /* 5. Intro */
        if (point_in_rect(mx, my, 268, 394, 346, 34)) {
            g.chosen_intro = false;
            g_settings_row = 4;
            g_status_msg = L("Vorspann überspringen.", "Skip intro.");
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 626, 394, 384, 34)) {
            g.chosen_intro = true;
            g_settings_row = 4;
            g_status_msg = L("Vorspann abspielen.", "Play intro.");
            save_einstellungen_ini();
            return;
        }

        /* 6. Spielordner */
        if (point_in_rect(mx, my, 816, 452, 194, 34)) {
            g_settings_row = 5;
            std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
            return;
        }
        /* Zusatz Schicksalsklinge: Haekchen "Fluessig drehen (Schicksalsklinge)" */
        if (g.teil == 1 && point_in_rect(mx, my, 268, 548, 700, 22)) {
            g.chosen_drehen = !g.chosen_drehen;
            g_status_msg = std::string("Flüssig drehen (Schicksalsklinge): ") +
                           (g.chosen_drehen ? "Eingeschaltet (Windows & DOSBox)" : "Ausgeschaltet (Windows & DOSBox)");
            save_einstellungen_ini();
            return;
        }

        /* Fusszeile: "< Übersicht (Esc)" */
        if (point_in_rect(mx, my, 24, 594, 180, 36)) {
            g_screen = SCREEN_SELECT_GAME;
            g_status_msg = L("Übersicht", "Overview");
            save_einstellungen_ini();
            return;
        }
        /* Fusszeile: "DOS-Fassung patchen" */
        if (point_in_rect(mx, my, 214, 594, 198, 36)) {
            patch_dos_game(g);
            return;
        }
        /* Fusszeile: "In DOSBox spielen" */
        if (point_in_rect(mx, my, 422, 594, 184, 36)) {
            if (g.has_dos_patcher && g.gamedir_valid && g.dos_fix_exists) {
                launch_dos_game(g);
            }
            return;
        }
        /* Fusszeile: "Im Browser spielen" */
        if (point_in_rect(mx, my, 616, 594, 186, 36)) {
            if (is_browser_available(g)) {
                open_browser_game(g, false);
            }
            return;
        }
        /* Fusszeile: "▶ Spielen (Windows)" */
        if (point_in_rect(mx, my, 812, 594, 204, 36)) {
            if (!g.has_any_exe) return;
            g_settings_row = 6;
            std::vector<std::string> no_extra;
            launch_selected_game(g, false, no_extra, false, 0);
            return;
        }
    }
}

static void handle_keydown(const SDL_KeyboardEvent &kev, bool &running) {
    int vk = (int)kev.keysym.sym;
    int ascii = (int)(kev.keysym.unused & 0xFF);
    char ch = (char)toupper(ascii ? ascii : vk);

    if (g_fp_active) {
        handle_fanpatch_keydown(kev);
        return;
    }

    if (vk == VK_F1) {
        g_open_dd_game = -1;
        g_help_active = !g_help_active;
        return;
    }
    /* NACHTRAG 1 (Etappe 6): F2 ohne Funktion (Starter nur noch Deutsch) */
    if (vk == VK_F2) {
        return;
    }

    if (g_help_active) {
        if (vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) {
            g_help_active = false;
        }
        return;
    }

    if (g_modal_active) {
        if (g_modal_is_readonly_dos && !g_modal_is_ok && !g_modal_is_crash) {
            int gidx = (g_modal_readonly_game_idx >= 0 && g_modal_readonly_game_idx < (int)g_games.size())
                ? g_modal_readonly_game_idx : g_selected_game;
            if (vk == VK_RETURN || ch == 'K') {
                g_modal_active = false;
                g_modal_is_readonly_dos = false;
                if (gidx >= 0 && gidx < (int)g_games.size()) {
                    copy_and_patch_dos_game(g_games[gidx], "");
                }
                return;
            }
            if (ch == 'O') {
                std::string picked = browse_for_gamedir_dialog(
                    L("Zielordner für beschreibbare DOS-Kopie wählen:", "Select target folder for writable DOS copy:"));
                if (!picked.empty()) {
                    g_modal_active = false;
                    g_modal_is_readonly_dos = false;
                    if (gidx >= 0 && gidx < (int)g_games.size()) {
                        copy_and_patch_dos_game(g_games[gidx], picked);
                    }
                }
                return;
            }
            if (vk == VK_ESCAPE) {
                g_modal_active = false;
                g_modal_is_readonly_dos = false;
                return;
            }
            return;
        }
        if (vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) {
            g_modal_active = false;
            g_modal_is_readonly_dos = false;
        }
        return;
    }

    if (g_screen == SCREEN_SELECT_GAME) {
        int sel_idx = (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) ? g_selected_game : 0;

        /* Tastaturbedienung bei geoeffneter Anzeige-Klappliste */
        if (g_open_dd_game >= 0 && g_open_dd_game < (int)g_games.size()) {
            GameCatalogEntry &og = g_games[g_open_dd_game];
            if (g_open_dd_sel < 0 || g_open_dd_sel >= 5) {
                g_open_dd_sel = get_dd_fenster_index(og.chosen_fenster);
            }
            if (vk == VK_ESCAPE) {
                g_open_dd_game = -1;
                g_open_dd_sel  = -1;
                return;
            }
            if (vk == VK_UP || vk == VK_LEFT) {
                g_open_dd_sel = (g_open_dd_sel + 4) % 5;
                return;
            }
            if (vk == VK_DOWN || vk == VK_RIGHT || vk == VK_TAB) {
                g_open_dd_sel = (g_open_dd_sel + 1) % 5;
                return;
            }
            if (vk == VK_RETURN || vk == VK_SPACE) {
                og.chosen_fenster  = g_dd_fenster_vals[g_open_dd_sel];
                og.chosen_vollbild = (og.chosen_fenster == "vollbild");
                g_open_dd_game = -1;
                g_open_dd_sel  = -1;
                save_einstellungen_ini();
                return;
            }
            return;
        }

        if (vk == VK_ESCAPE) {
            save_einstellungen_ini();
            running = false;
            return;
        }
        if (vk == VK_F5) {
            if (g_games[sel_idx].verfuegbar || g_games[sel_idx].gamedir_wrong_version) {
                std::vector<std::string> no_extra;
                launch_selected_game(g_games[sel_idx], false, no_extra, false, 0);
            }
            return;
        }

        /* Tab schaltet durch die Bedienelemente der aktiven Spielkarte */
        if (vk == VK_TAB) {
            g_show_focus_ring = true;
            g_card_focus = (g_card_focus + 1) % 9;
            return;
        }

        /* Pfeiltasten Links/Rechts: Im Segmentschalter "Fixes" direkt Stufe umschalten,
         * in "Anzeige" Fenstermodus wechseln, sonst Fokus zwischen Elementen verschieben */
        if (vk == VK_LEFT || vk == VK_RIGHT) {
            int dir = (vk == VK_RIGHT) ? 1 : -1;
            if (g_card_focus == 0) {
                g_games[sel_idx].chosen_stufe = (PatchStufe)(((int)g_games[sel_idx].chosen_stufe + dir + 3) % 3);
                g_status_msg = g_games[sel_idx].name + " – Fixes: " + stufe_short_label(g_games[sel_idx].chosen_stufe);
                save_einstellungen_ini();
            } else if (g_card_focus == 1) {
                cycle_fenster_mode(g_games[sel_idx], dir);
            } else {
                g_show_focus_ring = true;
                g_card_focus = (g_card_focus + dir + 9) % 9;
            }
            return;
        }

        /* Leertaste: Haekchen "Intro" / "Probenlog" umschalten, Klappliste oeffnen oder Knopf ausloesen */
        if (vk == VK_SPACE) {
            if (g_card_focus == 2) {
                g_games[sel_idx].chosen_intro = !g_games[sel_idx].chosen_intro;
                save_einstellungen_ini();
            } else if (g_card_focus == 3) {
                g_games[sel_idx].chosen_probenlog = !g_games[sel_idx].chosen_probenlog;
                save_einstellungen_ini();
            } else if (g_card_focus == 1) {
                g_open_dd_game = sel_idx;
                g_open_dd_sel  = get_dd_fenster_index(g_games[sel_idx].chosen_fenster);
            } else if (g_card_focus == 0) {
                g_games[sel_idx].chosen_stufe = (PatchStufe)(((int)g_games[sel_idx].chosen_stufe + 1) % 3);
                save_einstellungen_ini();
            } else if (g_card_focus == 4) {
                std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
                if (!picked.empty()) handle_dropped_folder(picked);
            } else if (g_card_focus == 5) {
                g_screen = SCREEN_SETTINGS;
                g_settings_row = 0;
                save_einstellungen_ini();
            } else if (g_card_focus == 6) {
                std::vector<std::string> no_extra;
                launch_selected_game(g_games[sel_idx], false, no_extra, false, 0);
            } else if (g_card_focus == 7) {
                if (g_games[sel_idx].dos_fix_exists) launch_dos_game(g_games[sel_idx]);
                else patch_dos_game(g_games[sel_idx]);
            } else if (g_card_focus == 8) {
                open_browser_game(g_games[sel_idx], false);
            }
            return;
        }

        if (ch == 'O') {
            std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
            return;
        }
        if (ch == 'S') {
            g_games[sel_idx].chosen_stufe = (PatchStufe)(((int)g_games[sel_idx].chosen_stufe + 1) % 3);
            save_einstellungen_ini();
            return;
        }
        if (ch == 'F') {
            cycle_fenster_mode(g_games[sel_idx], 1);
            return;
        }
        if (ch == 'K') {
            toggle_skalierung_mode(g_games[sel_idx]);
            return;
        }
        if (ch == 'I') {
            g_games[sel_idx].chosen_intro = !g_games[sel_idx].chosen_intro;
            save_einstellungen_ini();
            return;
        }
        if (ch == 'R') {
            for (size_t k = 0; k < g_games.size(); ++k) {
                if (g_games[k].teil == 1) {
                    g_games[k].chosen_drehen = !g_games[k].chosen_drehen;
                    g_status_msg = std::string("Flüssig drehen (Schicksalsklinge): ") +
                                   (g_games[k].chosen_drehen ? "Eingeschaltet (Windows & DOSBox)" : "Ausgeschaltet (Windows & DOSBox)");
                }
            }
            save_einstellungen_ini();
            return;
        }
        if (ch == 'P') {
            g_games[sel_idx].chosen_probenlog = !g_games[sel_idx].chosen_probenlog;
            g_status_msg = std::string(L("Probenlog: ", "Roll Log: ")) +
                           (g_games[sel_idx].chosen_probenlog
                               ? L("Eingeschaltet (mit Live-Fenster)", "Enabled (with live window)")
                               : L("Ausgeschaltet", "Disabled"));
            save_einstellungen_ini();
            return;
        }
        if (ch == 'E') {
            g_screen = SCREEN_SETTINGS;
            g_settings_row = 0;
            save_einstellungen_ini();
            return;
        }
        if (ch == 'D') {
            if (g_games[sel_idx].dos_fix_exists) launch_dos_game(g_games[sel_idx]);
            else patch_dos_game(g_games[sel_idx]);
            return;
        }
        if (ch == 'B') {
            open_browser_game(g_games[sel_idx], false);
            return;
        }
        if (vk == '1' || vk == '2' || vk == '3') {
            int idx = vk - '1';
            if (idx >= 0 && idx < (int)g_games.size()) {
                g_selected_game = idx;
                save_einstellungen_ini();
            }
            return;
        }
        if (vk == VK_UP) {
            g_selected_game = (g_selected_game - 1 + (int)g_games.size()) % (int)g_games.size();
            save_einstellungen_ini();
            return;
        }
        if (vk == VK_DOWN) {
            g_selected_game = (g_selected_game + 1) % (int)g_games.size();
            save_einstellungen_ini();
            return;
        }
        if (vk == VK_RETURN) {
            if (g_card_focus == 1) {
                g_open_dd_game = sel_idx;
                g_open_dd_sel  = get_dd_fenster_index(g_games[sel_idx].chosen_fenster);
            } else if (g_card_focus == 2) {
                g_games[sel_idx].chosen_intro = !g_games[sel_idx].chosen_intro;
                save_einstellungen_ini();
            } else if (g_card_focus == 3) {
                g_games[sel_idx].chosen_probenlog = !g_games[sel_idx].chosen_probenlog;
                save_einstellungen_ini();
            } else if (g_card_focus == 4) {
                std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
                if (!picked.empty()) handle_dropped_folder(picked);
            } else if (g_card_focus == 5) {
                g_screen = SCREEN_SETTINGS;
                g_settings_row = 0;
                save_einstellungen_ini();
            } else if (g_card_focus == 7) {
                if (g_games[sel_idx].dos_fix_exists) launch_dos_game(g_games[sel_idx]);
                else patch_dos_game(g_games[sel_idx]);
            } else if (g_card_focus == 8) {
                open_browser_game(g_games[sel_idx], false);
            } else if (g_games[sel_idx].has_any_exe) {
                std::vector<std::string> no_extra;
                launch_selected_game(g_games[sel_idx], false, no_extra, false, 0);
            }
            return;
        }
    } else if (g_screen == SCREEN_SETTINGS) {
        if (g_selected_game < 0 || g_selected_game >= (int)g_games.size()) return;
        GameCatalogEntry &g = g_games[g_selected_game];

        if (vk == VK_ESCAPE || vk == VK_BACK) {
            g_screen = SCREEN_SELECT_GAME;
            save_einstellungen_ini();
            return;
        }
        if (vk == VK_UP) {
            g_settings_row = (g_settings_row + 6) % 7;
            return;
        }
        if (vk == VK_DOWN || vk == VK_TAB) {
            g_settings_row = (g_settings_row + 1) % 7;
            return;
        }
        if (vk == VK_F5) {
            if (g.has_any_exe) {
                std::vector<std::string> no_extra;
                launch_selected_game(g, false, no_extra, false, 0);
            }
            return;
        }

        if (ch == 'S') {
            g.chosen_stufe = (PatchStufe)(((int)g.chosen_stufe + 1) % 3);
            g_settings_row = 0;
            save_einstellungen_ini();
            return;
        }
        if (ch == 'F') {
            g_settings_row = 1;
            cycle_fenster_mode(g, 1);
            return;
        }
        if (ch == 'K') {
            g_settings_row = 2;
            toggle_skalierung_mode(g);
            return;
        }
        if (ch == 'P') {
            g.chosen_probenlog = !g.chosen_probenlog;
            g_settings_row = 3;
            save_einstellungen_ini();
            return;
        }
        if (ch == 'I') {
            g.chosen_intro = !g.chosen_intro;
            g_settings_row = 4;
            save_einstellungen_ini();
            return;
        }
        if (ch == 'R' && g.teil == 1) {
            g.chosen_drehen = !g.chosen_drehen;
            g_status_msg = std::string("Flüssig drehen (Schicksalsklinge): ") +
                           (g.chosen_drehen ? "Eingeschaltet (Windows & DOSBox)" : "Ausgeschaltet (Windows & DOSBox)");
            save_einstellungen_ini();
            return;
        }
        if (ch == 'O') {
            g_settings_row = 5;
            std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
            return;
        }
        if (ch == 'D') {
            if (g.dos_fix_exists) launch_dos_game(g);
            else patch_dos_game(g);
            return;
        }
        if (ch == 'B') {
            open_browser_game(g, false);
            return;
        }

        if (vk == VK_LEFT || vk == VK_RIGHT) {
            int dir = (vk == VK_RIGHT) ? 1 : -1;
            if (g_settings_row == 0) {
                g.chosen_stufe = (PatchStufe)(((int)g.chosen_stufe + dir + 3) % 3);
                save_einstellungen_ini();
            } else if (g_settings_row == 1) {
                cycle_fenster_mode(g, dir);
            } else if (g_settings_row == 2) {
                toggle_skalierung_mode(g);
            } else if (g_settings_row == 3) {
                g.chosen_probenlog = !g.chosen_probenlog;
                save_einstellungen_ini();
            } else if (g_settings_row == 4) {
                g.chosen_intro = !g.chosen_intro;
                save_einstellungen_ini();
            }
            return;
        }

        if (vk == VK_RETURN || vk == VK_SPACE) {
            if (g_settings_row == 0 && vk == VK_SPACE) {
                g.chosen_stufe = (PatchStufe)(((int)g.chosen_stufe + 1) % 3);
                save_einstellungen_ini();
            } else if (g_settings_row == 1 && vk == VK_SPACE) {
                cycle_fenster_mode(g, 1);
            } else if (g_settings_row == 2 && vk == VK_SPACE) {
                toggle_skalierung_mode(g);
            } else if (g_settings_row == 3 && vk == VK_SPACE) {
                g.chosen_probenlog = !g.chosen_probenlog;
                save_einstellungen_ini();
            } else if (g_settings_row == 4 && vk == VK_SPACE) {
                g.chosen_intro = !g.chosen_intro;
                save_einstellungen_ini();
            } else if (g_settings_row == 5) {
                std::string picked = browse_for_gamedir_dialog(L("Spielordner wählen:", "Select game folder:"));
                if (!picked.empty()) {
                    handle_dropped_folder(picked);
                }
            } else if (g.has_any_exe) {
                std::vector<std::string> no_extra;
                launch_selected_game(g, false, no_extra, false, 0);
            }
            return;
        }
    }
}

/* =========================================================================
 * 13. Hauptprogramm (main) inkl. SDL_DROPFILE & CLI-Optionen fuer Abnahme
 * ========================================================================= */

int main(int argc, char **argv) {
    init_starter_dirs();
    load_grafik_modi_md();

    std::string spiele_json = join_path(g_data_dir, "spiele.json");
    if (!file_exists_win(spiele_json)) {
        spiele_json = join_path(g_starter_dir, "spiele.json");
    }
    if (!load_spiele_json(spiele_json)) {
        GameCatalogEntry s1 = make_default_entry(1);
        s1.id = "schicksalsklinge";
        s1.name = "Die Schicksalsklinge";
        s1.untertitel = "Das Schwarze Auge - Teil 1 (1992)";
        s1.port_ordner = "Schicksalsklinge\\Nativ";
        s1.exe_orig = "SCHICK_PORT_ORIG.EXE";
        s1.exe_wichtig = "SCHICK_PORT_WICHTIG.EXE";
        s1.exe_alles = "SCHICK_PORT_ALLES.EXE";
        s1.exe_standard = "SCHICK_PORT.EXE";
        s1.kennungsdatei = "SCHICK.DAT";
        s1.pflichtdateien.push_back("SCHICK.DAT");
        s1.pflichtdateien.push_back("DSAGEN.DAT");
        s1.pflichtdateien.push_back("SCHICKM.EXE");
        g_games.push_back(s1);
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-auto-gog") == 0) {
            g_disable_gog_autoscan = true;
        } else if (strcmp(argv[i], "--no-grafikpaket") == 0) {
            g_force_no_grafikpaket = true;
        } else if (strcmp(argv[i], "--no-browser-exec") == 0) {
            g_no_browser_exec = true;
        }
    }

    load_einstellungen_ini();
    refresh_all_games();
    g_status_msg = "Tipp: Spielordner oder Archiv einfach ins Fenster ziehen! (F1=Hilfe, F5=Schnellstart)";

    std::string screenshot_path;
    std::string screenshot_after_path;
    std::string test_absturz_path;
    std::string dos_copy_target_dir;
    std::vector<std::string> simulated_drops;
    bool auto_start          = false;
    bool auto_quick_start    = false;
    bool auto_patch_dos      = false;
    bool auto_patch_dos_copy = false;
    bool auto_launch_dos     = false;
    bool auto_open_browser   = false;
    bool auto_quit           = false;
    bool test_missing_exe    = false;
    bool trigger_build_paket = false;
    DWORD timeout_ms         = 20000;
    std::vector<std::string> extra_port_args;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--no-auto-gog" || a == "--no-grafikpaket" || a == "--no-browser-exec") {
            continue;
        } else if (a == "--clear-spielpfade") {
            for (size_t k = 0; k < g_games.size(); ++k) {
                g_games[k].custom_spielpfad = "-";
            }
            refresh_all_games();
        } else if (strncmp(a.c_str(), "--spielpfad-", 12) == 0) {
            size_t eq = a.find('=', 12);
            if (eq != std::string::npos) {
                std::string gid = a.substr(12, eq - 12);
                std::string gdir = a.substr(eq + 1);
                for (size_t k = 0; k < g_games.size(); ++k) {
                    if (_stricmp(g_games[k].id.c_str(), gid.c_str()) == 0) {
                        g_games[k].custom_spielpfad = gdir.empty() ? "-" : gdir;
                        refresh_game_status(g_games[k]);
                        if (g_games[k].gamedir_wrong_version) {
                            g_selected_game = (int)k;
                            show_wrong_version_modal(g_games[k]);
                        }
                    }
                }
            }
        } else if (strncmp(a.c_str(), "--sprache=", 10) == 0 || strncmp(a.c_str(), "--lang=", 7) == 0) {
            /* NACHTRAG 1 (Etappe 6): Starter nur noch Deutsch, Sprach-Option ignorieren */
            g_lang_en = false;
        } else if (a == "--screenshot" && i + 1 < argc) {
            screenshot_path = argv[++i];
        } else if (strncmp(a.c_str(), "--screenshot=", 13) == 0) {
            screenshot_path = a.substr(13);
        } else if (a == "--screenshot-after" && i + 1 < argc) {
            screenshot_after_path = argv[++i];
        } else if (strncmp(a.c_str(), "--screenshot-after=", 19) == 0) {
            screenshot_after_path = a.substr(19);
        } else if (a == "--screenshot-progress" && i + 1 < argc) {
            g_screenshot_progress_path = argv[++i];
        } else if (strncmp(a.c_str(), "--screenshot-progress=", 22) == 0) {
            g_screenshot_progress_path = a.substr(22);
        } else if (a == "--drop" && i + 1 < argc) {
            simulated_drops.push_back(argv[++i]);
        } else if (strncmp(a.c_str(), "--drop=", 7) == 0) {
            simulated_drops.push_back(a.substr(7));
        } else if (a == "--screen=auswahl") {
            g_screen = SCREEN_SELECT_GAME;
        } else if (a == "--screen=einstellungen") {
            g_screen = SCREEN_SETTINGS;
        } else if (a == "--show-help" || a == "--hilfe") {
            g_help_active = true;
        } else if (strncmp(a.c_str(), "--open-anzeige=", 15) == 0) {
            std::string ov = a.substr(15);
            if (ov == "1" || ov == "2" || ov == "3") {
                int idx = atoi(ov.c_str()) - 1;
                if (idx >= 0 && idx < (int)g_games.size()) {
                    g_open_dd_game = idx;
                    g_open_dd_sel  = get_dd_fenster_index(g_games[idx].chosen_fenster);
                }
            } else {
                for (size_t k = 0; k < g_games.size(); ++k) {
                    if (_stricmp(g_games[k].id.c_str(), ov.c_str()) == 0) {
                        g_open_dd_game = (int)k;
                        g_open_dd_sel  = get_dd_fenster_index(g_games[k].chosen_fenster);
                        break;
                    }
                }
            }
        } else if (strncmp(a.c_str(), "--focus=", 8) == 0) {
            int fc = atoi(a.c_str() + 8);
            if (fc >= 0 && fc <= 8) {
                g_card_focus = fc;
                g_show_focus_ring = true;
            }
        } else if (strncmp(a.c_str(), "--mouse-pos=", 12) == 0) {
            int mx = 0, my = 0;
            if (sscanf(a.c_str() + 12, "%d,%d", &mx, &my) == 2) {
                g_mouse_x = mx;
                g_mouse_y = my;
                for (size_t gi = 0; gi < g_games.size() && gi < 3; ++gi) {
                    int cy = 147 + (int)gi * 162;
                    if (point_in_rect(g_mouse_x, g_mouse_y, 14, cy, 1012, 150)) {
                        g_hover_game = (int)gi;
                        break;
                    }
                }
            }
        } else if (strncmp(a.c_str(), "--win-pos=", 10) == 0) {
            int wx = 0, wy = 0;
            if (sscanf(a.c_str() + 10, "%d,%d", &wx, &wy) == 2) {
                g_win_x = wx;
                g_win_y = wy;
            }
        } else if (strncmp(a.c_str(), "--spiel=", 8) == 0) {
            std::string sp = a.substr(8);
            for (size_t k = 0; k < g_games.size(); ++k) {
                if (_stricmp(g_games[k].id.c_str(), sp.c_str()) == 0) {
                    g_selected_game = (int)k;
                    break;
                }
            }
        } else if (strncmp(a.c_str(), "--stufe=", 8) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_stufe = str_to_stufe(a.substr(8));
            }
        } else if (strncmp(a.c_str(), "--fenster=", 10) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                std::string fm = a.substr(10);
                g_games[g_selected_game].chosen_fenster = fm;
                g_games[g_selected_game].chosen_vollbild = (fm == "vollbild");
            }
        } else if (strncmp(a.c_str(), "--font-variante=", 16) == 0) {
            int fv = atoi(a.c_str() + 16);
            if (fv >= 1 && fv <= 6) {
                g_font_variant = fv;
            }
        } else if (strncmp(a.c_str(), "--skalierung=", 13) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_skalierung = a.substr(13);
            }
        } else if (strncmp(a.c_str(), "--grafik=", 9) == 0) {
            /* Alte --grafik=-Optionen ignorieren */
        } else if (strncmp(a.c_str(), "--probenlog=", 12) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_probenlog = (a.substr(12) == "1");
            }
        } else if (strncmp(a.c_str(), "--intro=", 8) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_intro = (a.substr(8) == "1");
            }
        } else if (strncmp(a.c_str(), "--drehen=", 9) == 0) {
            bool val = (a.substr(9) == "1" || _stricmp(a.substr(9).c_str(), "true") == 0);
            for (size_t k = 0; k < g_games.size(); ++k) {
                if (g_games[k].teil == 1) g_games[k].chosen_drehen = val;
            }
        } else if (strncmp(a.c_str(), "--fluessig-drehen=", 18) == 0) {
            bool val = (a.substr(18) == "1" || _stricmp(a.substr(18).c_str(), "true") == 0);
            for (size_t k = 0; k < g_games.size(); ++k) {
                if (g_games[k].teil == 1) g_games[k].chosen_drehen = val;
            }
        } else if (strncmp(a.c_str(), "--vollbild=", 11) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_vollbild = (a.substr(11) == "1");
            }
        } else if (strncmp(a.c_str(), "--port-ordner-", 14) == 0) {
            size_t eq = a.find('=', 14);
            if (eq != std::string::npos) {
                std::string gid = a.substr(14, eq - 14);
                std::string pdir = a.substr(eq + 1);
                for (size_t k = 0; k < g_games.size(); ++k) {
                    if (_stricmp(g_games[k].id.c_str(), gid.c_str()) == 0) {
                        g_games[k].custom_port_ordner = pdir;
                        refresh_game_status(g_games[k]);
                    }
                }
            }
        } else if (a == "--erzeuge-grafikpaket") {
            trigger_build_paket = true;
            g_screen = SCREEN_SETTINGS;
        } else if (strncmp(a.c_str(), "--check-absturz-log=", 20) == 0) {
            test_absturz_path = a.substr(20);
        } else if (a == "--test-fehlende-exe") {
            test_missing_exe = true;
            g_screen = SCREEN_SETTINGS;
        } else if (a == "--patch-dos") {
            auto_patch_dos = true;
        } else if (a == "--patch-dos-copy") {
            auto_patch_dos_copy = true;
        } else if (strncmp(a.c_str(), "--patch-dos-copy=", 17) == 0) {
            auto_patch_dos_copy = true;
            dos_copy_target_dir = a.substr(17);
        } else if (a == "--launch-dos") {
            auto_launch_dos = true;
        } else if (a == "--open-browser") {
            auto_open_browser = true;
        } else if (a == "--auto-start") {
            auto_start = true;
            g_screen = SCREEN_SETTINGS;
        } else if (a == "--direkt-start") {
            auto_quick_start = true;
            g_screen = SCREEN_SELECT_GAME;
        } else if (a == "--auto-quit") {
            auto_quit = true;
        } else if (strncmp(a.c_str(), "--timeout-ms=", 13) == 0) {
            timeout_ms = (DWORD)strtoul(a.c_str() + 13, NULL, 10);
        } else if (a == "--port-arg" && i + 1 < argc) {
            extra_port_args.push_back(argv[++i]);
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        return 1;
    }

    g_window = SDL_CreateWindow(
        "Das Schwarze Auge: Die Nordland-Trilogie - Starter " STARTER_VERSION,
        g_win_x, g_win_y,
        g_win_w, g_win_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(g_renderer, SCREEN_W, SCREEN_H);
    g_texture = SDL_CreateTexture(
        g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H
    );
    SDL_ShowCursor(1);

    save_einstellungen_ini();

    for (size_t di = 0; di < simulated_drops.size(); ++di) {
        SDL_Event dev;
        memset(&dev, 0, sizeof(dev));
        dev.type = SDL_DROPFILE;
        dev.drop.timestamp = SDL_GetTicks();
        dev.drop.file = (char *)malloc(simulated_drops[di].size() + 1);
        if (dev.drop.file) {
            strcpy(dev.drop.file, simulated_drops[di].c_str());
            SDL_PushEvent(&dev);
        }
    }
    if (!simulated_drops.empty()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_DROPFILE && ev.drop.file) {
                std::string dropped(ev.drop.file);
                SDL_free(ev.drop.file);
                handle_drop_path(dropped);
            }
        }
    }

    if (trigger_build_paket && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        generate_grafikpaket_for_game(g_games[g_selected_game]);
    }

    int rc = 0;
    if (test_missing_exe && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        launch_selected_game(g_games[g_selected_game], true, extra_port_args, false, timeout_ms);
    }

    if (auto_patch_dos && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        rc = patch_dos_game(g_games[g_selected_game]);
    }

    if (auto_patch_dos_copy && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        rc = copy_and_patch_dos_game(g_games[g_selected_game], dos_copy_target_dir);
    }

    if (auto_launch_dos && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        rc = launch_dos_game(g_games[g_selected_game]);
    }

    if (auto_open_browser && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        rc = open_browser_game(g_games[g_selected_game], true);
    }

    if (!test_absturz_path.empty() && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        inspect_absturz_log(test_absturz_path, g_games[g_selected_game].name);
    }

    const char *verlauf_test_dir = getenv("STARTER_VERLAUF_TEST");
    if (verlauf_test_dir && verlauf_test_dir[0]) {
        run_starter_verlauf_test(verlauf_test_dir);
        SDL_DestroyTexture(g_texture);
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }

    render_frame();

    if (!screenshot_path.empty()) {
        save_framebuffer_image(screenshot_path);
    }

    if ((auto_patch_dos || auto_patch_dos_copy || auto_launch_dos || auto_open_browser) && auto_quit) {
        SDL_DestroyTexture(g_texture);
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return (rc < 0) ? 1 : rc;
    }

    if ((auto_start || auto_quick_start) && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        rc = launch_selected_game(g_games[g_selected_game], false, extra_port_args, true, timeout_ms);
        render_frame();
        if (!screenshot_after_path.empty()) {
            save_framebuffer_bmp(screenshot_after_path);
        }
        SDL_DestroyTexture(g_texture);
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return (rc < 0) ? 1 : rc;
    }

    if (auto_quit) {
        SDL_DestroyTexture(g_texture);
        SDL_DestroyRenderer(g_renderer);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 0;
    }

    bool running = true;
    while (running) {
        if (g_watched_game_proc) {
            if (WaitForSingleObject(g_watched_game_proc, 0) == WAIT_OBJECT_0) {
                CloseHandle(g_watched_game_proc);
                g_watched_game_proc = NULL;
                if (g_window) {
                    SDL_ShowWindow(g_window);
                    SDL_RaiseWindow(g_window);
                }
                std::string c1 = join_path(g_watched_exe_dir, "absturz.log");
                std::string c2 = join_path(g_watched_save_dir, "absturz.log");
                if (!inspect_absturz_log(c1, g_watched_game_name)) {
                    inspect_absturz_log(c2, g_watched_game_name);
                }
                g_status_msg = g_watched_game_name + L(" beendet – zurück im Starter.", " closed – welcome back to the launcher.");
            }
        }

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                save_einstellungen_ini();
                running = false;
            } else if (ev.type == SDL_DROPFILE) {
                if (ev.drop.file) {
                    std::string dropped(ev.drop.file);
                    SDL_free(ev.drop.file);
                    handle_drop_path(dropped);
                }
            } else if (ev.type == SDL_MOUSEMOTION) {
                g_mouse_x = ev.motion.x;
                g_mouse_y = ev.motion.y;
                if (g_fp_active) {
                    handle_fanpatch_motion(ev.motion.x, ev.motion.y);
                }
                g_hover_game = -1;
                if (g_screen == SCREEN_SELECT_GAME && !g_help_active && !g_modal_active && !g_fp_active) {
                    for (size_t i = 0; i < g_games.size() && i < 3; ++i) {
                        int cy = 147 + (int)i * 162;
                        if (point_in_rect(g_mouse_x, g_mouse_y, 14, cy, 1012, 150)) {
                            g_hover_game = (int)i;
                            break;
                        }
                    }
                }
            } else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                g_mouse_x = ev.button.x;
                g_mouse_y = ev.button.y;
                handle_mouse_click(ev.button.x, ev.button.y, running);
            } else if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT) {
                handle_fanpatch_mouseup();
            } else if (ev.type == SDL_MOUSEWHEEL) {
                handle_fanpatch_wheel(ev.wheel.y);
            } else if (ev.type == SDL_KEYDOWN) {
                handle_keydown(ev.key, running);
            }
        }
        render_frame();
        SDL_Delay(16);
    }

    if (g_watched_game_proc) {
        CloseHandle(g_watched_game_proc);
        g_watched_game_proc = NULL;
    }

    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    return 0;
}
