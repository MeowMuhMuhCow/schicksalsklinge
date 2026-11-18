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

#define SCREEN_W 1060
#define SCREEN_H 760

static uint32_t      g_fb_fallback[SCREEN_W * SCREEN_H];
static uint32_t     *g_fb       = g_fb_fallback;
static HDC           g_mem_dc   = NULL;
static HBITMAP       g_dib_bmp  = NULL;
static HFONT         g_fonts[5] = {NULL, NULL, NULL, NULL, NULL};
static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture  *g_texture  = NULL;

static std::string g_exe_dir;
static std::string g_starter_dir;
static bool        g_disable_gog_autoscan     = false;
static bool        g_force_no_grafikpaket     = false;
static std::string g_screenshot_progress_path;
static bool        g_screenshot_progress_saved = false;

/* Komplett auf Deutsch + gemerkte Fensterposition & Mausposition fuer Hover */
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
    if (file_exists_win(join_path(g_exe_dir, "spiele.json"))) {
        g_starter_dir = g_exe_dir;
    } else if (file_exists_win(join_path(g_exe_dir, "..\\spiele.json"))) {
        g_starter_dir = full_win_path(join_path(g_exe_dir, ".."));
    } else {
        g_starter_dir = g_exe_dir;
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

    /* Laufzeit-Einstellungen (aus einstellungen.ini) */
    PatchStufe  chosen_stufe;
    std::string chosen_grafik;
    std::string chosen_fenster;     /* "2", "3", "4", "5", "vollbild" (Standard: "vollbild") */
    std::string chosen_skalierung;  /* "ganzzahlig", "fuellend" (Standard: "fuellend") */
    bool        chosen_probenlog;
    bool        chosen_intro;
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
    bool        has_grafikpaket;
    std::string resolved_paket_dir;
};

static std::vector<GameCatalogEntry> g_games;
static int g_selected_game = 0;

enum ScreenMode {
    SCREEN_SELECT_GAME = 0,
    SCREEN_SETTINGS    = 1
};

static ScreenMode  g_screen           = SCREEN_SELECT_GAME;
static int         g_settings_row     = 0;
static bool        g_modal_active     = false;
static bool        g_modal_is_ok      = false;
static bool        g_modal_is_crash   = false;
static std::string g_modal_title;
static std::string g_modal_line1;
static std::string g_modal_line2;
static std::string g_modal_line3;
static std::string g_modal_line4;
static bool        g_help_active      = false;
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
    e.has_grafikpaket = false;
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
    std::string p1 = join_path(g_exe_dir, "einstellungen.ini");
    std::string p2 = join_path(g_starter_dir, "einstellungen.ini");
    std::string chosen = file_exists_win(p1) ? p1 : (file_exists_win(p2) ? p2 : "");
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
                g_lang_en = (_stricmp(v.c_str(), "en") == 0 || _stricmp(v.c_str(), "english") == 0);
            } else if (_stricmp(k.c_str(), "fenster_x") == 0 && !v.empty()) {
                g_win_x = atoi(v.c_str());
            } else if (_stricmp(k.c_str(), "fenster_y") == 0 && !v.empty()) {
                g_win_y = atoi(v.c_str());
            } else if (_stricmp(k.c_str(), "fenster_b") == 0 && !v.empty()) {
                int w = atoi(v.c_str());
                if (w >= SCREEN_W && w <= 3840) g_win_w = w;
            } else if (_stricmp(k.c_str(), "fenster_h") == 0 && !v.empty()) {
                int h = atoi(v.c_str());
                if (h >= SCREEN_H && h <= 2160) g_win_h = h;
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
                    else if (_stricmp(k.c_str(), "vollbild") == 0) {
                        g.chosen_vollbild = (v == "1" || _stricmp(v.c_str(), "true") == 0 || _stricmp(v.c_str(), "ja") == 0);
                        if (g.chosen_vollbild) g.chosen_fenster = "vollbild";
                        else if (g.chosen_fenster == "vollbild") g.chosen_fenster = "4";
                    } else if (_stricmp(k.c_str(), "spielpfad") == 0) g.custom_spielpfad = v;
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
    fprintf(fp, "sprache=%s\n", g_lang_en ? "en" : "de");
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
        if (!g.custom_spielpfad.empty()) {
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
    std::string p1 = join_path(g_exe_dir, "einstellungen.ini");
    std::string p2 = join_path(g_starter_dir, "einstellungen.ini");
    write_one_ini(p1);
    if (_stricmp(p1.c_str(), p2.c_str()) != 0) {
        write_one_ini(p2);
    }
}

/* =========================================================================
 * 6. Erkennung von Port-EXEs, Spielordner, Grafikpaket & Fassungspruefung
 * ========================================================================= */

static bool is_dir_writable_win(const std::string &dir) {
    const char *force_ro = getenv("DSA_FORCE_READONLY_GAMEDIR");
    if (force_ro && force_ro[0] == '1') return false;
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

static bool validate_gamedir_files(const GameCatalogEntry &g, const std::string &dir, std::string &out_missing) {
    out_missing.clear();
    if (!file_exists_win(join_path(dir, g.kennungsdatei))) {
        out_missing = g.kennungsdatei;
        return false;
    }
    for (size_t i = 0; i < g.pflichtdateien.size(); ++i) {
        if (!file_exists_win(join_path(dir, g.pflichtdateien[i]))) {
            out_missing = g.pflichtdateien[i];
            return false;
        }
    }
    return true;
}

static bool check_gamedir_candidate(const GameCatalogEntry &g, const std::string &cand, std::string &out_dir) {
    std::string c = trim_str(cand);
    if (c.empty()) return false;
    if (!is_abs_win_path(c)) {
        c = join_path(g_starter_dir, c);
    }
    std::string missing;
    if (validate_gamedir_files(g, c, missing)) {
        out_dir = full_win_path(c);
        return true;
    }
    std::string parent = join_path(c, "..");
    if (validate_gamedir_files(g, parent, missing)) {
        out_dir = full_win_path(parent);
        return true;
    }
    return false;
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

static void refresh_game_status(GameCatalogEntry &g) {
    /* 1. Port-Ordner & EXEs pruefen (inkl. g_exe_dir fuer Starter/release/) */
    std::vector<std::string> dirs_to_check;
    if (!g.custom_port_ordner.empty()) {
        dirs_to_check.push_back(g.custom_port_ordner);
    } else {
        dirs_to_check.push_back(g_exe_dir);
        if (!g.port_ordner.empty()) dirs_to_check.push_back(g.port_ordner);
        for (size_t i = 0; i < g.port_ordner_alt.size(); ++i) {
            dirs_to_check.push_back(g.port_ordner_alt[i]);
        }
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

    /* 2. Spielordner (SPIELPFAD) automatisch suchen */
    g.resolved_gamedir.clear();
    g.gamedir_valid = false;
    g.gamedir_writable = false;

    if (!g.custom_spielpfad.empty() && check_gamedir_candidate(g, g.custom_spielpfad, g.resolved_gamedir)) {
        g.gamedir_valid = true;
    }
    if (!g.gamedir_valid) {
        std::string local_spieldaten = join_path(g_starter_dir, "Spieldaten\\" + g.id);
        if (check_gamedir_candidate(g, local_spieldaten, g.resolved_gamedir)) {
            g.gamedir_valid = true;
        }
    }
    if (!g.gamedir_valid) {
        std::string exe_spieldaten = join_path(g_exe_dir, "Spieldaten\\" + g.id);
        if (check_gamedir_candidate(g, exe_spieldaten, g.resolved_gamedir)) {
            g.gamedir_valid = true;
        }
    }
    if (!g.gamedir_valid && !g_disable_gog_autoscan) {
        const char *env_sp = getenv("SPIELPFAD");
        if (!env_sp || !env_sp[0]) env_sp = getenv("DSA_GAMEDIR");
        if (!env_sp || !env_sp[0]) env_sp = getenv("DSA_GAME_DIR");
        if (env_sp && env_sp[0] && check_gamedir_candidate(g, env_sp, g.resolved_gamedir)) {
            g.gamedir_valid = true;
        }
    }
    if (!g.gamedir_valid && !g_disable_gog_autoscan) {
        std::string sp_candidates[] = {
            join_path(g_exe_dir, "SPIELPFAD.txt"),
            join_path(g_starter_dir, "SPIELPFAD.txt"),
            join_path(g_starter_dir, "..\\SPIELPFAD.txt"),
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
    if (!g.gamedir_valid && !g_disable_gog_autoscan) {
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

    /* 3. Grafikpaket-Verfuegbarkeit pruefen (T1: Modus 'paket' nur waehlbar, wenn Paket erzeugt wurde) */
    g.has_grafikpaket = false;
    g.resolved_paket_dir.clear();
    if (!g_force_no_grafikpaket) {
        std::string pk_candidates[5] = {
            join_path(g_starter_dir, "Grafikpakete\\" + g.id),
            join_path(g_exe_dir, "Grafikpakete\\" + g.id),
            join_path(g_starter_dir, "..\\Grafik\\pakete\\stiltreu"),
            join_path(g_starter_dir, "..\\Grafik\\pakete\\xbrz"),
            join_path(g_exe_dir, "Grafik\\pakete\\stiltreu")
        };
        for (int i = 0; i < 5; ++i) {
            if (dir_has_grafikpaket(pk_candidates[i])) {
                g.has_grafikpaket = true;
                g.resolved_paket_dir = full_win_path(pk_candidates[i]);
                break;
            }
        }
    }
    if (!g.has_grafikpaket && _stricmp(g.chosen_grafik.c_str(), "paket") == 0) {
        g.chosen_grafik = "original";
    }

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
    if ((unsigned)x < SCREEN_W && (unsigned)y < SCREEN_H) {
        g_fb[y * SCREEN_W + x] = col;
    }
}

static void fill_rect(int x, int y, int w, int h, uint32_t col) {
    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > SCREEN_W) ? SCREEN_W : (x + w);
    int y1 = (y + h > SCREEN_H) ? SCREEN_H : (y + h);
    for (int py = y0; py < y1; ++py) {
        uint32_t *row = &g_fb[py * SCREEN_W];
        for (int px = x0; px < x1; ++px) {
            row[px] = col;
        }
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

static void draw_rect_border(int x, int y, int w, int h, uint32_t col_outer, uint32_t col_inner) {
    fill_rect(x, y, w, 1, col_outer);
    fill_rect(x, y + h - 1, w, 1, col_outer);
    fill_rect(x, y, 1, h, col_outer);
    fill_rect(x + w - 1, y, 1, h, col_outer);
    if (w > 4 && h > 4) {
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

struct FontVariantSpec {
    const char    *name;
    const wchar_t *faces[5];   /* [0]=Detail, [1]=UI/Button, [2]=Spieletitel, [3]=Haupttitel, [4]=Schritt-Header */
    int            heights[5];
    int            weights[5];
};

static const FontVariantSpec g_font_specs[6] = {
    /* Variante 1: Klassik-Rollenspiel (Georgia Bold fuer Titel + ruhige Segoe UI fuer UI/Buttons) */
    {
        "Variante 1: Georgia + Segoe UI (Klassik-Rollenspiel)",
        { L"Segoe UI", L"Segoe UI", L"Georgia", L"Georgia", L"Segoe UI" },
        { -14, -15, -23, -28, -16 },
        { FW_NORMAL, FW_MEDIUM, FW_BOLD, FW_BOLD, FW_BOLD }
    },
    /* Variante 2: Aventurischer Chronist (Palatino Linotype + warme Calibri, 1 Nummer groesser) */
    {
        "Variante 2: Palatino Linotype + Calibri (Aventurischer Chronist)",
        { L"Calibri", L"Calibri", L"Palatino Linotype", L"Palatino Linotype", L"Palatino Linotype" },
        { -15, -17, -23, -29, -17 },
        { FW_NORMAL, FW_SEMIBOLD, FW_BOLD, FW_BOLD, FW_BOLD }
    },
    /* Variante 3: Klares PC-Interface (Trebuchet MS + offene Verdana mit weiter Laufweite) */
    {
        "Variante 3: Trebuchet MS + Verdana (Weite Laufweite & Maximale Schaerfe)",
        { L"Verdana", L"Verdana", L"Trebuchet MS", L"Trebuchet MS", L"Trebuchet MS" },
        { -13, -14, -23, -28, -16 },
        { FW_NORMAL, FW_NORMAL, FW_BOLD, FW_BOLD, FW_BOLD }
    },
    /* Variante 4: Moderne Eleganz (Reine Segoe UI mit feiner Gewichts-Abstufung Regular/Semibold/Bold) */
    {
        "Variante 4: Segoe UI Fein-Abstufung (Gross & Luftig, Regular + Bold)",
        { L"Segoe UI", L"Segoe UI", L"Segoe UI", L"Segoe UI", L"Segoe UI" },
        { -14, -16, -24, -30, -17 },
        { FW_NORMAL, FW_NORMAL, FW_BOLD, FW_BOLD, FW_SEMIBOLD }
    },
    /* Variante 5: Epische Buchschrift (Constantia Serif + geschwungene Candara) */
    {
        "Variante 5: Constantia + Candara (Epische Fantasy-Buchschrift)",
        { L"Candara", L"Candara", L"Constantia", L"Constantia", L"Candara" },
        { -15, -17, -23, -29, -18 },
        { FW_NORMAL, FW_MEDIUM, FW_BOLD, FW_BOLD, FW_BOLD }
    },
    /* Variante 6: Kompakt & Gestochen Scharf (Cambria + klassische Tahoma) */
    {
        "Variante 6: Cambria + Tahoma (Klassisch & Gestochen Scharf)",
        { L"Tahoma", L"Tahoma", L"Cambria", L"Cambria", L"Tahoma" },
        { -13, -15, -22, -28, -16 },
        { FW_NORMAL, FW_NORMAL, FW_BOLD, FW_BOLD, FW_BOLD }
    }
};

static void apply_font_variant(int variant) {
    if (variant < 1 || variant > 6) variant = 1;
    g_font_variant = variant;
    g_loaded_font_variant = variant;
    const FontVariantSpec &spec = g_font_specs[variant - 1];

    for (int i = 0; i < 5; ++i) {
        if (g_fonts[i]) {
            DeleteObject(g_fonts[i]);
            g_fonts[i] = NULL;
        }
        g_fonts[i] = CreateFontW(
            spec.heights[i], 0, 0, 0, spec.weights[i],
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
            spec.faces[i]
        );
        if (!g_fonts[i]) {
            g_fonts[i] = CreateFontW(
                spec.heights[i], 0, 0, 0, spec.weights[i],
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
    if (font_idx > 4) font_idx = 4;
    return g_fonts[font_idx];
}

static int text_width_px(const char *text, int font_idx = 1) {
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
    return (int)strlen(text) * 9;
}

static int text_height_px(int font_idx = 1) {
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

static void draw_text(int x, int y, const char *text, uint32_t col, int font_idx = 1, bool shadow = true) {
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
        int x0 = (x < 0) ? 0 : x;
        int y0 = (y < 0) ? 0 : y;
        int x1 = x + (int)sz.cx + 3;
        int y1 = y + (int)sz.cy + 3;
        if (x1 > SCREEN_W) x1 = SCREEN_W;
        if (y1 > SCREEN_H) y1 = SCREEN_H;
        for (int py = y0; py < y1; ++py) {
            uint32_t *row = &g_fb[py * SCREEN_W];
            for (int px = x0; px < x1; ++px) {
                row[px] |= 0xFF000000u;
            }
        }
    }
}

static void draw_text_centered(int x, int y, int w, const char *text, uint32_t col, int font_idx = 1, bool shadow = true) {
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

/* Zeichnet ein ruhiges, dezentes 18x18 Status-Abzeichen:
 * ok == true  -> Gedaempftes Dunkelgruen mit klarem Haken
 * ok == false -> Gedaempftes Dunkelrot/Schiefer mit klarem Kreuz
 */
static void draw_status_badge(int x, int y, bool ok) {
    if (ok) {
        fill_gradient_v(x, y, 18, 18, 0xFF245434u, 0xFF163620u);
        draw_rect_border(x, y, 18, 18, 0xFF4E8E62u, 0xFF1C4228u);
        /* Haken (Checkmark) in 18x18 */
        for (int i = 0; i < 4; ++i) {
            fill_rect(x + 4 + i, y + 9 + i, 2, 2, 0xFFDCEEE2u);
        }
        for (int i = 0; i < 7; ++i) {
            fill_rect(x + 7 + i, y + 12 - i, 2, 2, 0xFFDCEEE2u);
        }
    } else {
        fill_gradient_v(x, y, 18, 18, 0xFF4E2428u, 0xFF321618u);
        draw_rect_border(x, y, 18, 18, 0xFF8E4A50u, 0xFF3E1C20u);
        /* Kreuz (X) in 18x18 */
        for (int i = 0; i < 9; ++i) {
            fill_rect(x + 4 + i, y + 4 + i, 2, 2, 0xFFE6C8C8u);
            fill_rect(x + 12 - i, y + 4 + i, 2, 2, 0xFFE6C8C8u);
        }
    }
}

static std::string clip_text_left(const std::string &s, size_t max_chars) {
    if (s.size() <= max_chars) return s;
    if (max_chars <= 4) return s.substr(0, max_chars);
    return "..." + s.substr(s.size() - (max_chars - 3));
}

static std::string clip_text_px_left(const std::string &s, int max_px, int font_idx = 1) {
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
    /* 72x72 Emblem mit den charakteristischen Originalfarben der 3 Teile (auch dezent erkennbar, wenn noch nicht aktiv) */
    uint32_t bg_top = (teil == 1) ? (active ? 0xFF381A1Au : 0xFF28181Au)
                    : ((teil == 2) ? (active ? 0xFF16283Eu : 0xFF16202Eu)
                                   : (active ? 0xFF2A2418u : 0xFF221E18u));
    uint32_t bg_bot = (teil == 1) ? (active ? 0xFF1C0D0Du : 0xFF160E10u)
                    : ((teil == 2) ? (active ? 0xFF0C1422u : 0xFF0E131Cu)
                                   : (active ? 0xFF16120Cu : 0xFF12100Cu));
    uint32_t border = (teil == 1) ? (active ? 0xFFC85A4Cu : 0xFF7A4440u)
                    : ((teil == 2) ? (active ? 0xFF5A9CD6u : 0xFF466888u)
                                   : (active ? 0xFFC89A48u : 0xFF786238u));
    uint32_t b_in   = active ? 0xFF464C58u : 0xFF2C3038u;
    fill_gradient_v(x, y, 72, 72, bg_top, bg_bot);
    draw_rect_border(x, y, 72, 72, border, b_in);

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

static void draw_button(int x, int y, int w, int h, const char *label, bool selected, bool enabled, bool primary = false, bool allow_hover = true) {
    uint32_t top, bot, b_out, b_in, tcol;
    if (!enabled) {
        top   = 0xFF1E2128u;
        bot   = 0xFF16181Eu;
        b_out = 0xFF3A3E48u;
        b_in  = 0xFF252830u;
        tcol  = 0xFF666C78u;
    } else if (primary) {
        top   = selected ? 0xFF26543Au : 0xFF1E422Eu;
        bot   = selected ? 0xFF183826u : 0xFF12281Cu;
        b_out = selected ? 0xFF68A880u : 0xFF4E8262u;
        b_in  = selected ? 0xFF346E4Cu : 0xFF28543Au;
        tcol  = 0xFFEBF4EEu;
    } else if (selected) {
        top   = 0xFF343E52u;
        bot   = 0xFF222836u;
        b_out = 0xFF8CA0BCu;
        b_in  = 0xFF48566Eu;
        tcol  = 0xFFF0F4FAu;
    } else {
        top   = 0xFF252A36u;
        bot   = 0xFF181C24u;
        b_out = 0xFF586072u;
        b_in  = 0xFF323846u;
        tcol  = 0xFFD4DCE8u;
    }

    /* Sanfter Hover-Effekt: KEINE Raender aendern, nur die Fuellung dezent aufhellen */
    if (enabled && allow_hover &&
        g_mouse_x >= x && g_mouse_x < x + w &&
        g_mouse_y >= y && g_mouse_y < y + h) {
        top  = lighten_color(top, 24);
        bot  = lighten_color(bot, 20);
        tcol = 0xFFFFFFFFu;
    }

    fill_gradient_v(x, y, w, h, top, bot);
    draw_rect_border(x, y, w, h, b_out, b_in);
    int th = text_height_px(1);
    int ty = y + (h - th) / 2 - 1;
    draw_text_centered(x, ty, w, label, tcol, 1, true);
}

/* =========================================================================
 * 8. Zeichnen der Bildschirme (Ruhige Farbgebung + Originalfarben der 3 Spiele)
 * ========================================================================= */

/* Charakteristische Original-Schriftzugfarben der drei Nordlandtrilogie-Spiele:
 * - Teil 1 (Die Schicksalsklinge, 1992): Klassisches Schicksalsklinge-Glutrot / Karmesin
 * - Teil 2 (Sternenschweif, 1994):       Sternenschweif-Kometen-Eisblau / Silberblau
 * - Teil 3 (Schatten ueber Riva, 1996):  Riva-Bernstein-Gold / Abendrot-Ocker
 */
static uint32_t game_logo_color(int teil, bool verfuegbar) {
    if (teil == 1) {
        return verfuegbar ? 0xFFE86852u : 0xFFB85848u;
    } else if (teil == 2) {
        return verfuegbar ? 0xFF68B6F2u : 0xFF5A96C8u;
    } else {
        return verfuegbar ? 0xFFE2AE52u : 0xFFC29648u;
    }
}

static void render_background_and_header(const char *subtitle) {
    init_gdi_font_canvas();
    bool bg_hover = (!g_help_active && !g_modal_active);
    fill_gradient_v(0, 0, SCREEN_W, SCREEN_H, 0xFF0E121Cu, 0xFF080B12u);
    draw_rect_border(5, 5, SCREEN_W - 10, SCREEN_H - 10, 0xFF566074u, 0xFF282E3Au);
    draw_corner_ornaments(5, 5, SCREEN_W - 10, SCREEN_H - 10, 0xFF8894A8u);

    fill_gradient_v(18, 12, SCREEN_W - 36, 68, 0xFF1A202Eu, 0xFF111520u);
    draw_rect_border(18, 12, SCREEN_W - 36, 68, 0xFF626C80u, 0xFF2E3442u);

    /* Haupttitel "Das Schwarze Auge: Die Nordland-Trilogie" in ruhigem, edlem Pergament-Elfenbein */
    draw_text(34, 13, "Das Schwarze Auge: Die Nordland-Trilogie", 0xFFECE6D8u, 3, true);
    draw_text(34, 51, subtitle, 0xFFA4B0C2u, 1, true);

    /* Rechts oben nur der Hilfe-Knopf (F1) mit reichlich Abstand */
    draw_button(SCREEN_W - 148, 27, 122, 38, "Hilfe (F1)", g_help_active, true, false, bg_hover);
}

static const char *fenster_label_str(const std::string &f) {
    if (f == "2") return "2x (640x480)";
    if (f == "3") return "3x (960x720)";
    if (f == "4") return "4x (1280x960)";
    if (f == "5") return "5x (1600x1200)";
    return "Vollbild (4:3)";
}

static const char *skalierung_label_str(const std::string &s) {
    if (_stricmp(s.c_str(), "ganzzahlig") == 0) return "Ganzzahlig";
    return "Füllend";
}

static const char *stufe_short_label(PatchStufe s) {
    if (s == STUFE_ORIG) return "Original";
    if (s == STUFE_WICHTIG) return "Wichtige Fixes";
    return "Alle Fixes";
}

static void render_screen_select_game(void) {
    bool bg_hover = (!g_help_active && !g_modal_active);
    render_background_and_header(
        "Gemeinsamer Starter – Jedes Spiel direkt auf seiner eigenen Karte einstellen und starten"
    );

    /* Kompakter Hinweis-Balken oben fuer Drag & Drop */
    fill_gradient_v(18, 92, SCREEN_W - 36, 46, 0xFF151B26u, 0xFF10141Eu);
    draw_rect_border(18, 92, SCREEN_W - 36, 46, 0xFF485264u, 0xFF242A36u);
    draw_text(34, 105, "Tipp: GOG-Spielordner, .zip-Archiv oder Port-Ordner einfach ins Fenster ziehen – oder direkt beim jeweiligen Spiel einstellen:", 0xFFC8D2E2u, 1, true);

    /* ---------------------------------------------------------------------
     * Die 3 Spielkarten – ALLES direkt pro Spiel regelbar (keine Fuss-/Statusleiste)
     * --------------------------------------------------------------------- */
    for (size_t i = 0; i < g_games.size() && i < 3; ++i) {
        const GameCatalogEntry &g = g_games[i];
        int cy = 154 + (int)i * 198;
        int cx = 18;
        int cw = SCREEN_W - 36;
        int ch = 180;

        bool is_sel = ((int)i == g_selected_game);
        bool is_hov = bg_hover && ((int)i == g_hover_game);

        uint32_t bg_top, bg_bot, b_out, b_in;
        if (!g.verfuegbar) {
            bg_top = 0xFF151820u;
            bg_bot = 0xFF101218u;
            b_out  = is_sel ? 0xFF606878u : 0xFF3E4554u;
            b_in   = 0xFF222630u;
        } else if (is_sel) {
            bg_top = 0xFF1E2636u;
            bg_bot = 0xFF141926u;
            b_out  = 0xFF8898B2u;
            b_in   = 0xFF424C60u;
        } else {
            bg_top = 0xFF181E2Cu;
            bg_bot = 0xFF111520u;
            b_out  = 0xFF525C6Eu;
            b_in   = 0xFF2C3240u;
        }

        /* Hover auf der Spielkarte: nur Hintergrund minimal aufhellen, keine Raender zufuegen */
        if (is_hov) {
            bg_top = lighten_color(bg_top, 14);
            bg_bot = lighten_color(bg_bot, 12);
        }

        fill_gradient_v(cx, cy, cw, ch, bg_top, bg_bot);
        draw_rect_border(cx, cy, cw, ch, b_out, b_in);
        if (is_sel) {
            draw_corner_ornaments(cx, cy, cw, ch, 0xFFB0BED4u);
        }

        draw_game_emblem(cx + 20, cy + 20, g.teil, g.verfuegbar);

        /* Zeile 1: Spieletitel in der jeweiligen Originalfarbe des Spiel-Schriftzugs + Startknopf rechts oben */
        char title_buf[160];
        snprintf(title_buf, sizeof(title_buf), "%d. %s", g.teil, g.name.c_str());
        uint32_t title_col = game_logo_color(g.teil, g.verfuegbar);
        draw_text(cx + 110, cy + 16, title_buf, title_col, 2, true);

        int title_w = text_width_px(title_buf, 2);
        const char *short_sub = (g.teil == 1) ? "(DSA Teil 1 – 1992)" : ((g.teil == 2) ? "(DSA Teil 2 – 1994)" : "(DSA Teil 3 – 1996)");
        draw_text(cx + 124 + title_w, cy + 22, short_sub, g.verfuegbar ? 0xFFA4B0C2u : 0xFF707888u, 0, true);

        if (g.verfuegbar) {
            draw_button(cx + cw - 236, cy + 18, 216, 42, ">> Spiel starten", true, true, true, bg_hover);
        } else if (!g.has_any_exe) {
            draw_button(cx + cw - 236, cy + 18, 216, 42, "Programm fehlt", false, false, false, false);
        } else {
            draw_button(cx + cw - 236, cy + 18, 216, 42, "Spieldaten fehlen", false, false, false, false);
        }

        /* Zeile 2: Ruhige Status-Checkliste pro Spiel (Haken / Kreuz) */
        uint32_t row_txt_col = g.verfuegbar ? 0xFFD0D8E4u : 0xFF98A2B2u;

        std::string exe_name = (g.chosen_stufe == STUFE_ORIG) ? g.exe_orig
                             : ((g.chosen_stufe == STUFE_WICHTIG) ? g.exe_wichtig : g.exe_alles);
        draw_status_badge(cx + 110, cy + 54, g.has_any_exe);
        std::string prog_lbl = g.has_any_exe ? ("Programm OK (" + exe_name + ")") : ("Programm fehlt (" + exe_name + ")");
        draw_text(cx + 134, cy + 52, prog_lbl.c_str(), row_txt_col, 1, true);

        draw_status_badge(cx + 468, cy + 54, g.gamedir_valid);
        std::string dat_lbl = g.gamedir_valid ? ("Spieldaten OK (" + g.kennungsdatei + ")") : ("Spieldaten fehlen (" + g.kennungsdatei + ")");
        draw_text(cx + 492, cy + 52, dat_lbl.c_str(), row_txt_col, 1, true);

        /* Zeile 3: Erkannter Spielordner & Anzeige-Zusammenfassung dieses Spiels */
        std::string path_info;
        if (g.gamedir_valid) {
            std::string p_prefix = "Spielordner: ";
            int rem_px = 640 - text_width_px(p_prefix.c_str(), 0);
            path_info = p_prefix + clip_text_px_left(g.resolved_gamedir, rem_px > 160 ? rem_px : 160, 0)
                      + "  |  Skalierung: " + skalierung_label_str(g.chosen_skalierung) + " (4:3)";
        } else {
            path_info = "Spielordner noch nicht gesetzt – bitte Ordner/ZIP reinziehen oder unten [Ordner wählen...] klicken.";
        }
        draw_text(cx + 110, cy + 86, path_info.c_str(), 0xFFA4B0C2u, 0, true);

        /* Zeile 4: Alle 6 Bedienknoepfe DIREKT PRO SPIEL auf der Karte (buendig von cx+20 bis cx+cw-20) */
        std::string btn_plog    = std::string("Probenlog: ") + (g.chosen_probenlog ? "AN" : "AUS");
        std::string btn_patches = std::string("Patches: ") + stufe_short_label(g.chosen_stufe);
        std::string btn_fen     = std::string("Fenster: ") + (g.chosen_fenster == "vollbild" ? "Vollbild" : (g.chosen_fenster + "x"));
        std::string btn_intro   = std::string("Intro: ") + (g.chosen_intro ? "AN" : "AUS");

        draw_button(cx + 20,  cy + 124, 152, 38, btn_plog.c_str(),    g.chosen_probenlog, true, g.chosen_probenlog, bg_hover);
        draw_button(cx + 182, cy + 124, 196, 38, btn_patches.c_str(), false,              true, false,              bg_hover);
        draw_button(cx + 388, cy + 124, 154, 38, btn_fen.c_str(),     false,              true, false,              bg_hover);
        draw_button(cx + 552, cy + 124, 122, 38, btn_intro.c_str(),   g.chosen_intro,     true, g.chosen_intro,     bg_hover);
        draw_button(cx + 684, cy + 124, 156, 38, "Ordner wählen...",  false,              true, false,              bg_hover);
        draw_button(cx + 850, cy + 124, 154, 38, "Einstellungen...",  false,              true, false,              bg_hover);
    }
}

static void render_screen_settings(void) {
    if (g_selected_game < 0 || g_selected_game >= (int)g_games.size()) return;
    const GameCatalogEntry &g = g_games[g_selected_game];
    bool bg_hover = (!g_help_active && !g_modal_active);

    char sub_buf[200];
    snprintf(sub_buf, sizeof(sub_buf), "Teil %d: %s – Einstellungen anpassen und Spiel starten", g.teil, g.name.c_str());
    render_background_and_header(sub_buf);

    fill_gradient_v(18, 90, SCREEN_W - 36, 550, 0xFF182030u, 0xFF101622u);
    draw_rect_border(18, 90, SCREEN_W - 36, 550, 0xFF8A7430u, 0xFF3C3418u);

    int row_y[6] = {98, 184, 270, 356, 424, 492};
    int row_h[6] = {80,  80,  80,  62,  62, 138};
    if (g_settings_row >= 0 && g_settings_row < 6) {
        fill_rect(26, row_y[g_settings_row], SCREEN_W - 52, row_h[g_settings_row], 0xFF222C42u);
        draw_rect_border(26, row_y[g_settings_row], SCREEN_W - 52, row_h[g_settings_row], 0xFF7A6828u, 0xFF383018u);
    }

    /* 1. Patches */
    draw_text(40, 112, "1. Patches [S]:", 0xFFF4D068u, 4, true);
    draw_button(276, 105, 224, 38, "Original (0)", g.chosen_stufe == STUFE_ORIG, true, false, bg_hover);
    draw_button(512, 105, 244, 38, "Wichtige Fixes (1)", g.chosen_stufe == STUFE_WICHTIG, true, false, bg_hover);
    draw_button(768, 105, 254, 38, "Alle Fixes (2)", g.chosen_stufe == STUFE_ALLES, true, false, bg_hover);

    std::string stufe_exe = (g.chosen_stufe == STUFE_ORIG) ? g.exe_orig : ((g.chosen_stufe == STUFE_WICHTIG) ? g.exe_wichtig : g.exe_alles);
    bool stufe_exe_ok = (g.chosen_stufe == STUFE_ORIG) ? g.has_exe_orig : ((g.chosen_stufe == STUFE_WICHTIG) ? g.has_exe_wichtig : (g.has_exe_alles || g.has_exe_standard));
    const char *stufe_desc = (g.chosen_stufe == STUFE_ORIG)
        ? "100 % Originalverhalten von 1992/1993"
        : ((g.chosen_stufe == STUFE_WICHTIG) ? "Behebt Abstürze & kritische Spielstand-Fehler" : "Alle 154 Fehlerkorrekturen + Komfort (Empfohlen)");
    draw_status_badge(276, 152, stufe_exe_ok);
    char stufe_sub[240];
    snprintf(stufe_sub, sizeof(stufe_sub), "%s [%s] – %s",
             stufe_exe.c_str(), stufe_exe_ok ? "OK" : "FEHLT", stufe_desc);
    draw_text(302, 150, stufe_sub, stufe_exe_ok ? 0xFFB0C0D8u : 0xFFFF8080u, 1, true);

    /* 2. Anzeige - Fenstergroesse (immer exakt 4:3) */
    draw_text(40, 198, "2. Fenstergröße [F]:", 0xFFF4D068u, 4, true);
    draw_button(276, 191, 140, 38, "2x (640x480)",   g.chosen_fenster == "2", true, false, bg_hover);
    draw_button(424, 191, 140, 38, "3x (960x720)",   g.chosen_fenster == "3", true, false, bg_hover);
    draw_button(572, 191, 150, 38, "4x (1280x960)",  g.chosen_fenster == "4", true, false, bg_hover);
    draw_button(730, 191, 156, 38, "5x (1600x1200)", g.chosen_fenster == "5", true, false, bg_hover);
    draw_button(894, 191, 128, 38, "Vollbild",       g.chosen_fenster == "vollbild", true, false, bg_hover);

    draw_text(276, 236, "Immer exakt 4:3 wie auf dem Röhrenmonitor (Alt+Enter schaltet Vollbild um)", 0xFFB0C0D8u, 1, true);

    /* 3. Anzeige - Skalierung (ganzzahlig vs. fuellend / Sharp Bilinear) */
    draw_text(40, 284, "3. Skalierung [K]:", 0xFFF4D068u, 4, true);
    bool is_ganz = (_stricmp(g.chosen_skalierung.c_str(), "ganzzahlig") == 0);
    draw_button(276, 277, 364, 38, "Ganzzahlig (exakte Vielfache)", is_ganz, true, false, bg_hover);
    draw_button(652, 277, 370, 38, "Füllend (Sharp Bilinear)",     !is_ganz, true, false, bg_hover);

    const char *skal_desc = is_ganz
        ? "Ganzzahlig: Nur ganze Vielfache, alle Pixelspalten exakt gleich breit"
        : "Füllend: Größtmöglich in 4:3 (Nearest-Neighbor + Rest per Sharp-Bilinear)";
    draw_text(276, 322, skal_desc, 0xFFB0C0D8u, 1, true);

    /* 4. Probenlog */
    draw_text(40, 374, "4. Probenlog [P]:", 0xFFF4D068u, 4, true);
    draw_button(276, 367, 234, 38, "Ausgeschaltet", !g.chosen_probenlog, true, false, bg_hover);
    draw_button(522, 367, 500, 38, "Eingeschaltet (PROBEN.CSV + Live-Fenster)", g.chosen_probenlog, true, false, bg_hover);

    /* 5. Vorspann / Intro */
    draw_text(40, 442, "5. Vorspann [I]:", 0xFFF4D068u, 4, true);
    draw_button(276, 435, 346, 38, "Überspringen (--kein-intro)", !g.chosen_intro, true, false, bg_hover);
    draw_button(634, 435, 388, 38, "Abspielen (Original-Vorspann)", g.chosen_intro, true, false, bg_hover);

    /* 6. Spielordner & Status-Checks */
    draw_text(40, 510, "6. Spieldaten [O]:", 0xFFF4D068u, 4, true);
    fill_rect(276, 503, 536, 38, 0xFF0E121Cu);
    draw_rect_border(276, 503, 536, 38, g.gamedir_valid ? 0xFF4C8858u : 0xFF985050u, 0xFF242C3Cu);
    std::string disp_gd = g.gamedir_valid ? clip_text_px_left(g.resolved_gamedir, 512, 1) : "Noch nicht gefunden – Ordner oder .zip ins Fenster ziehen";
    draw_text(288, 511, disp_gd.c_str(), g.gamedir_valid ? 0xFFD8E8D8u : 0xFFFF9898u, 1, true);
    draw_button(824, 503, 198, 38, "Ordner wählen...", g_settings_row == 5, true, false, bg_hover);

    draw_status_badge(276, 556, g.gamedir_valid);
    std::string save_info;
    if (!g.gamedir_valid) {
        save_info = "NOCH NICHT ERLEDIGT – Pflichtdatei " + g.kennungsdatei + " fehlt noch";
    } else if (g.gamedir_writable) {
        save_info = "CHECK OK – " + g.kennungsdatei + " erkannt  |  Spielstände im Spielordner";
    } else {
        save_info = "CHECK OK – " + g.kennungsdatei + " erkannt  |  Spielstände in Spielstaende\\";
    }
    draw_text(302, 554, save_info.c_str(), g.gamedir_valid ? 0xFF8CE09Au : 0xFFFF9090u, 1, true);

    draw_status_badge(276, 590, g.verfuegbar);
    draw_text(302, 588, g.verfuegbar ? "CHECK OK – Spiel ist vollständig startbereit (Originalpixel 4:3)" : "NOCH NICHT BEREIT – Bitte Spieldaten bereitstellen",
              g.verfuegbar ? 0xFF8CE09Au : 0xFFE8C078u, 1, true);

    /* Untere Aktionsleiste */
    fill_gradient_v(18, 694, SCREEN_W - 36, 52, 0xFF181E2Cu, 0xFF10141Eu);
    draw_rect_border(18, 694, SCREEN_W - 36, 52, 0xFF687288u, 0xFF303644u);

    draw_button(30, 701, 264, 38, "< Zurück zur Übersicht", false, true, false, bg_hover);
    draw_button(716, 699, 314, 42, ">> SPIEL STARTEN (Enter)", g_settings_row == 6, true, true, bg_hover);
}

static void render_help_overlay(void) {
    if (!g_help_active) return;
    int mw = 900, mh = 540;
    int mx = (SCREEN_W - mw) / 2;
    int my = (SCREEN_H - mh) / 2;
    fill_gradient_v(mx, my, mw, mh, 0xFF182438u, 0xFF0C121Eu);
    draw_rect_border(mx, my, mw, mh, 0xFFFFDF70u, 0xFF8A7028u);
    draw_corner_ornaments(mx, my, mw, mh, 0xFFFFE484u);

    draw_text_centered(mx, my + 18, mw, "HILFE & BEDIENUNG (F1)", 0xFFFFE484u, 2, true);

    const char *lines_de[] = {
        "SCHRITT 1 – SPIELDATEN BEREITSTELLEN (REINZIEHEN / DRAG & DROP):",
        "   Einfach den GOG-Spielordner oder eine .zip-Datei direkt in dieses",
        "   Starter-Fenster ziehen! Eine .zip wird automatisch mit Fortschrittsbalken",
        "   nach Spieldaten\\<Spiel>\\ entpackt. Alternativ: Knopf [Ordner wählen].",
        "",
        "SCHRITT 2 – ANZEIGE & 4:3-SKALIERUNG [F / K]:",
        "   • Fenstergröße [F]: 2x (640x480), 3x (960x720), 4x (1280x960), 5x (1600x1200)",
        "     oder Vollbild. Seitenverhältnis ist immer exakt 4:3 wie auf dem Röhrenmonitor.",
        "   • Skalierung [K]: Ganzzahlig (exakte Vielfache) oder Füllend (Sharp Bilinear).",
        "",
        "SCHRITT 3 – PATCH-STUFEN [S] & SPIEL STARTEN:",
        "   • Original (0): 100 % Originalverhalten von 1992/1993 ohne Eingriffe",
        "   • Wichtige Fixes (1): Behebt Abstürze & kritische Spielstand-Fehler",
        "   • Alle Fixes (2): Alle 154 Korrekturen + Komfortfunktionen (Empfohlen)",
        "",
        "TASTENKÜRZEL IM STARTER & IM SPIEL:",
        "   Starter: [1–3]=Spiel, [F1]=Hilfe, [F5]=Letztes Spiel sofort starten,",
        "            [Pfeile/Tab/Enter]=Wahl, [S/F/K/P/I/O], [Esc]=Zurück/Beenden",
        "   Im Spiel: [Alt+Enter]=Vollbild/Fenster, [Tab]=Größe, [F10]=Musik, [F11/F12]=Ton"
    };

    for (int i = 0; i < 19; ++i) {
        const char *ln = lines_de[i];
        bool is_hdr = (strncmp(ln, "SCHRITT", 7) == 0 || strncmp(ln, "TASTEN", 6) == 0);
        draw_text(mx + 34, my + 62 + i * 21, ln, is_hdr ? 0xFFF4D068u : 0xFFD8E2F0u, is_hdr ? 4 : 1, true);
    }

    draw_button(mx + (mw - 320) / 2, my + mh - 54, 320, 38, "Hilfe schließen (F1 / Enter)", true, true, true, true);
}

static void render_modal_dialog(void) {
    if (!g_modal_active) return;
    int mw = 840, mh = g_modal_is_crash ? 300 : 260;
    int mx = (SCREEN_W - mw) / 2;
    int my = (SCREEN_H - mh) / 2;
    uint32_t top = g_modal_is_ok ? 0xFF162C20u : 0xFF2A1618u;
    uint32_t bot = g_modal_is_ok ? 0xFF0C1812u : 0xFF160C0Eu;
    uint32_t brd = g_modal_is_ok ? 0xFF68E088u : 0xFFFF6868u;
    uint32_t bin = g_modal_is_ok ? 0xFF287840u : 0xFF983030u;
    fill_gradient_v(mx, my, mw, mh, top, bot);
    draw_rect_border(mx, my, mw, mh, brd, bin);
    draw_corner_ornaments(mx, my, mw, mh, 0xFFFFC060u);

    draw_text_centered(mx, my + 22, mw, g_modal_title.c_str(), g_modal_is_ok ? 0xFF98F4A8u : 0xFFFFB060u, 2, true);
    draw_text_centered(mx + 24, my + 72, mw - 48, g_modal_line1.c_str(), 0xFFF0E8E8u, 1, true);
    draw_text_centered(mx + 24, my + 104, mw - 48, g_modal_line2.c_str(), 0xFFD8E0E8u, 1, true);
    draw_text_centered(mx + 24, my + 136, mw - 48, g_modal_line3.c_str(), 0xFFB8C4D8u, 1, true);
    if (!g_modal_line4.empty()) {
        draw_text_centered(mx + 24, my + 170, mw - 48, g_modal_line4.c_str(), 0xFFFFDF70u, 1, true);
    }

    draw_button(mx + (mw - 280) / 2, my + mh - 56, 280, 38, "OK – Schließen (Enter)", true, true, g_modal_is_ok, true);
}

static void render_progress_modal(const std::string &title, const std::string &sub1, const std::string &cur_file, int percent) {
    if (g_screen == SCREEN_SELECT_GAME) render_screen_select_game();
    else render_screen_settings();

    int mw = 680, mh = 220;
    int mx = (SCREEN_W - mw) / 2;
    int my = (SCREEN_H - mh) / 2;
    fill_gradient_v(mx, my, mw, mh, 0xFF1A263Cu, 0xFF0E1624u);
    draw_rect_border(mx, my, mw, mh, 0xFFFFDF70u, 0xFF8A7028u);
    draw_corner_ornaments(mx, my, mw, mh, 0xFFFFE484u);

    draw_text_centered(mx, my + 22, mw, title.c_str(), 0xFFFFE484u, 2, true);
    draw_text_centered(mx + 20, my + 66, mw - 40, sub1.c_str(), 0xFFD0DCECu, 1, true);

    int bx = mx + 44, by = my + 106, bw = mw - 88, bh = 34;
    fill_rect(bx, by, bw, bh, 0xFF0A0E16u);
    draw_rect_border(bx, by, bw, bh, 0xFFC8A23Cu, 0xFF483A18u);

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    int fill_w = ((bw - 4) * percent) / 100;
    if (fill_w > 0) {
        fill_gradient_v(bx + 2, by + 2, fill_w, bh - 4, 0xFF42A858u, 0xFF246834u);
    }
    char pct_buf[64];
    snprintf(pct_buf, sizeof(pct_buf), "%d %%", percent);
    draw_text_centered(bx, by + 6, bw, pct_buf, 0xFFFFF8D8u, 1, true);

    std::string fmsg = std::string("Schritt: ") + clip_text_px_left(cur_file, mw - 120, 1);
    draw_text_centered(mx + 20, my + 158, mw - 40, fmsg.c_str(), 0xFFA8B8D0u, 1, true);

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
    if (g_help_active) {
        render_help_overlay();
    } else if (g_modal_active) {
        render_modal_dialog();
    }
    SDL_UpdateTexture(g_texture, NULL, g_fb, SCREEN_W * (int)sizeof(uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
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
    g_modal_title    = L("ABSTURZ ERKANNT (absturz.log)", "CRASH DETECTED (absturz.log)");
    g_modal_line1    = game_name + L(" wurde unerwartet beendet. Details aus absturz.log:", " terminated unexpectedly. Details from absturz.log:");
    g_modal_line2    = std::string(L("Ausnahme: ", "Exception: ")) + (exc_code.empty() ? "?" : exc_code)
                     + " @ " + (exc_addr.empty() ? "?" : exc_addr)
                     + L(" | Funktion: ", " | Function: ") + (last_fn.empty() ? "?" : last_fn);
    g_modal_line3    = loc_str.empty() ? (std::string(L("Protokoll: ", "Log file: ")) + clip_text_left(log_path, 50))
                                       : (std::string(L("Spielort: ", "Location: ")) + clip_text_left(loc_str, 50));
    g_modal_line4    = L("Bitte absturz.log unter GitHub Issues (MeowMuhMuhCow/schicksalsklinge) melden!",
                         "Please report & attach absturz.log on GitHub Issues (MeowMuhMuhCow/schicksalsklinge)!");
    g_status_msg     = L("Absturz erkannt - siehe Hinweis zu absturz.log.", "Crash detected - see absturz.log notice.");
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
    if (file_exists_win(join_path(dir, g.kennungsdatei))) return true;
    for (size_t i = 0; i < g.pflichtdateien.size(); ++i) {
        if (file_exists_win(join_path(dir, g.pflichtdateien[i]))) return true;
    }
    if (g.teil == 1 && (file_exists_win(join_path(dir, "BLADEM.EXE")) || file_exists_win(join_path(dir, "BLADE.DAT")))) return true;
    if (g.teil == 2 && (file_exists_win(join_path(dir, "STARTRAIL.EXE")) || file_exists_win(join_path(dir, "ROA2.EXE")))) return true;
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
    int first_matched_idx = -1;

    /* 1. Pruefe, ob im gezogenen/gewaehlten Ordner Port-EXEs (SCHICK_PORT*, SCHWEIF_PORT*, RIVA_PORT*) liegen */
    for (size_t gi = 0; gi < g_games.size(); ++gi) {
        GameCatalogEntry &g = g_games[gi];
        for (size_t ci = 0; ci < candidates.size(); ++ci) {
            const std::string &cand = candidates[ci];
            if (dir_has_port_exe(g, cand)) {
                g.custom_port_ordner = cand;
                refresh_game_status(g);
                unlocked_names.push_back(g.name + " (Port-EXE)");
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
                    unlocked_names.push_back(g.name);
                    if (first_matched_idx < 0) first_matched_idx = (int)gi;
                    break;
                } else if (first_missing_file.empty()) {
                    first_missing_game = g.name;
                    first_missing_file = missing;
                }
            }
        }
    }

    save_einstellungen_ini();

    if (!unlocked_names.empty()) {
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
        g_modal_title    = L("DATEI DER DEUTSCHEN FASSUNG FEHLT", "GERMAN VERSION FILE MISSING");
        g_modal_line1    = std::string(L("Im gewählten Ordner für \"", "In the selected folder for \"")) + first_missing_game + L("\" fehlt:", "\" is missing:");
        g_modal_line2    = std::string(L("Pflichtdatei: ", "Required file: ")) + first_missing_file + L(" (deutsche GOG/CD-Fassung)", " (German GOG/CD version)");
        g_modal_line3    = std::string(L("Ordner: ", "Folder: ")) + clip_text_left(base, 48);
        g_modal_line4.clear();
        g_status_msg     = std::string(L("Fehler: Pflichtdatei ", "Error: Required file ")) + first_missing_file + L(" fehlt.", " missing.");
    } else {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("KEIN SPIELORDNER ERKANNT", "NO GAME FOLDER RECOGNIZED");
        g_modal_line1    = L("Im gezogenen Ordner wurde kein Teil der Nordlandtrilogie gefunden.", "No Realms of Arkania game was found in the dropped folder.");
        g_modal_line2    = L("Erwartet: SCHICK.DAT (Teil 1), SCHWEIF.EXE (Teil 2) oder RIVA.EXE (Teil 3).", "Expected: SCHICK.DAT (Part 1), SCHWEIF.EXE (Part 2) or RIVA.EXE (Part 3).");
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
        g_modal_title    = L("UNGÜLTIGE ZIP-DATEI", "INVALID ZIP ARCHIVE");
        g_modal_line1    = L("Das gezogene Archiv konnte nicht gelesen werden:", "The dropped archive could not be read:");
        g_modal_line2    = clip_text_left(raw_zip_path, 52);
        g_modal_line3    = L("Bitte eine gültige .zip-Datei oder einen Spielordner ziehen.", "Please drop a valid .zip archive or a game folder.");
        g_modal_line4.clear();
        g_status_msg     = L("Fehler beim Lesen der ZIP-Datei.", "Error reading ZIP archive.");
        return;
    }

    std::vector<std::string> unlocked_names;
    std::string first_missing_game;
    std::string first_missing_file;
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

        std::string rel_target_dir = "Spieldaten\\" + g.id;
        std::string abs_target_dir = join_path(g_starter_dir, rel_target_dir);
        mkdir_recursive_win(abs_target_dir);

        uint64_t done_bytes = 0;
        std::string sub_msg = g.name + " -> " + rel_target_dir + "\\";
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
            g.custom_spielpfad = rel_target_dir;
            refresh_game_status(g);
            unlocked_names.push_back(g.name);
            if (first_matched_idx < 0) first_matched_idx = (int)gi;
        }
    }

    fclose(fp);
    save_einstellungen_ini();

    if (!unlocked_names.empty()) {
        if (first_matched_idx >= 0) g_selected_game = first_matched_idx;
        std::string names_joined;
        for (size_t i = 0; i < unlocked_names.size(); ++i) {
            if (i > 0) names_joined += ", ";
            names_joined += unlocked_names[i];
        }
        g_status_msg = std::string(L("ZIP entpackt nach Spieldaten\\ & freigeschaltet: ", "ZIP unpacked to Spieldaten\\ & unlocked: ")) + names_joined;
        g_modal_active = false;
    } else if (!first_missing_file.empty()) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("DATEI DER DEUTSCHEN FASSUNG FEHLT", "GERMAN VERSION FILE MISSING");
        g_modal_line1    = std::string(L("Im ZIP-Archiv für \"", "In the ZIP archive for \"")) + first_missing_game + L("\" fehlt:", "\" is missing:");
        g_modal_line2    = std::string(L("Pflichtdatei: ", "Required file: ")) + first_missing_file + L(" (deutsche GOG/CD-Fassung)", " (German GOG/CD version)");
        g_modal_line3    = std::string(L("Archiv: ", "Archive: ")) + clip_text_left(raw_zip_path, 48);
        g_modal_line4.clear();
        g_status_msg     = std::string(L("Fehler: In der ZIP fehlt ", "Error: Missing in ZIP: ")) + first_missing_file + ".";
    } else {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("KEIN SPIEL IM ARCHIV ERKANNT", "NO GAME FOUND IN ARCHIVE");
        g_modal_line1    = L("In der gezogenen ZIP wurde kein Teil der Nordlandtrilogie gefunden.", "No Realms of Arkania game was found in the dropped ZIP.");
        g_modal_line2    = L("Erwartet: SCHICK.DAT (Teil 1), SCHWEIF.EXE (Teil 2) oder RIVA.EXE (Teil 3).", "Expected: SCHICK.DAT (Part 1), SCHWEIF.EXE (Part 2) or RIVA.EXE (Part 3).");
        g_modal_line3    = std::string(L("Archiv: ", "Archive: ")) + clip_text_left(raw_zip_path, 48);
        g_modal_line4.clear();
        g_status_msg     = L("Kein bekanntes Spiel in der ZIP erkannt.", "No known game found in ZIP.");
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
        g_modal_line3    = L("Bitte einen Spielordner oder eine .zip-Datei ziehen.", "Please drop a game folder or a .zip archive.");
        g_modal_line4.clear();
    }
}

static std::string browse_for_gamedir_dialog(const std::string &prompt_title) {
    char display_name[MAX_PATH] = "";
    std::string wtitle = utf8_to_win1252(prompt_title);

    BROWSEINFOA bi;
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = NULL;
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
    if (file_exists_win(dst)) return;
    CopyFileA(utf8_to_win1252(src).c_str(), utf8_to_win1252(dst).c_str(), TRUE);
}

static void seed_savedir_from_gamedir(const std::string &gamedir, const std::string &savedir) {
    if (_stricmp(gamedir.c_str(), savedir.c_str()) == 0) return;
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

static HANDLE prepare_probenlog(const GameCatalogEntry &g, const std::string &savedir, DWORD *out_plog_pid) {
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

            std::string starter_plogs = join_path(g_starter_dir, "Probenlogs");
            _mkdir(utf8_to_win1252(starter_plogs).c_str());
            std::string dst_starter = join_path(starter_plogs, arch_name);

            std::string save_plogs = join_path(savedir, "Probenlogs");
            if (_stricmp(save_plogs.c_str(), starter_plogs.c_str()) != 0) {
                _mkdir(utf8_to_win1252(save_plogs).c_str());
                std::string dst_save = join_path(save_plogs, arch_name);
                CopyFileA(w_proben_csv.c_str(), utf8_to_win1252(dst_save).c_str(), FALSE);
            }
            CopyFileA(w_proben_csv.c_str(), utf8_to_win1252(dst_starter).c_str(), FALSE);
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

    std::string ps1_candidates[5] = {
        join_path(g_exe_dir, "Probenlog\\probenlog_fenster.ps1"),
        join_path(g_starter_dir, g.probenlog_skript),
        join_path(g_starter_dir, "..\\Port\\release\\Probenlog\\probenlog_fenster.ps1"),
        join_path(g_starter_dir, "..\\Probenlog\\probenlog_fenster.ps1"),
        join_path(g.resolved_port_dir, "..\\release\\Probenlog\\probenlog_fenster.ps1")
    };
    std::string ps1_path;
    for (int i = 0; i < 5; ++i) {
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
    /* SW_SHOWNOACTIVATE: Probenlog-Konsole darf dem Vollbild-Spiel nicht den Fokus stehlen! */
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
        g_modal_title    = L("PROGRAMMDATEI FEHLT", "EXECUTABLE MISSING");
        g_modal_line1    = L("Die gewählte Port-EXE wurde nicht gefunden:", "The selected Port executable was not found:");
        g_modal_line2    = exe_name + " (" + L("Suchpfad: ", "Search path: ") + g.port_ordner + ")";
        g_modal_line3    = L("Bitte zuerst bauen_port.py ausführen oder den Pfad prüfen.", "Please run bauen_port.py first or check the folder path.");
        g_modal_line4.clear();
        g_status_msg     = std::string(L("Fehler: Port-EXE ", "Error: Port EXE ")) + exe_name + L(" fehlt.", " missing.");
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
    if (!g.gamedir_valid) {
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("SPIELORDNER FEHLT", "GAME FOLDER MISSING");
        g_modal_line1    = std::string(L("Der Original-Spielordner für \"", "The game data folder for \"")) + g.name + L("\" fehlt.", "\" is missing.");
        g_modal_line2    = std::string(L("Erwartete Kennungsdatei: ", "Expected key file: ")) + g.kennungsdatei;
        g_modal_line3    = L("Bitte Ordner oder .zip ins Fenster ziehen oder [Ordner...] wählen.", "Please drag & drop folder or .zip or click [Browse...].");
        g_modal_line4.clear();
        g_status_msg     = std::string(L("Fehler: Spielordner (", "Error: Game folder (")) + g.kennungsdatei + L(") nicht gefunden.", ") not found.");
        return -3;
    }

    std::string full_exe = full_win_path(exe_path);
    std::string exe_dir  = full_win_path(g.resolved_port_dir);
    std::string gamedir  = full_win_path(g.resolved_gamedir);
    std::string savedir  = gamedir;

    const char *env_forced_sd = getenv("DSA_SAVE_DIR");
    if (env_forced_sd && env_forced_sd[0]) {
        savedir = full_win_path(env_forced_sd);
    } else if (!g.gamedir_writable) {
        savedir = join_path(g_starter_dir, "Spielstaende");
    }

    _mkdir(utf8_to_win1252(savedir).c_str());
    _mkdir(utf8_to_win1252(join_path(savedir, "TEMP")).c_str());
    if (g.teil >= 2) {
        _mkdir(utf8_to_win1252(join_path(savedir, "GAMES")).c_str());
        _mkdir(utf8_to_win1252(join_path(savedir, "CTEMP")).c_str());
    }
    if (_stricmp(savedir.c_str(), gamedir.c_str()) != 0) {
        seed_savedir_from_gamedir(gamedir, savedir);
    }

    /* Alte absturz.log vor Spielstart entfernen, damit nur echte neue Abstuerze gemeldet werden */
    std::string crash_log_exe  = join_path(exe_dir, "absturz.log");
    std::string crash_log_save = join_path(savedir, "absturz.log");
    DeleteFileA(utf8_to_win1252(crash_log_exe).c_str());
    DeleteFileA(utf8_to_win1252(crash_log_save).c_str());

    DWORD plog_pid = 0;
    HANDLE h_plog = prepare_probenlog(g, savedir, &plog_pid);

    _putenv(("DSA_GAMEDIR=" + utf8_to_win1252(gamedir)).c_str());
    _putenv(("DSA_GAME_DIR=" + utf8_to_win1252(gamedir)).c_str());
    _putenv(("SPIELPFAD=" + utf8_to_win1252(gamedir)).c_str());
    _putenv(("DSA_SAVE_DIR=" + utf8_to_win1252(savedir)).c_str());
    _putenv(g.chosen_intro ? "DSA_INTRO=1" : "DSA_INTRO=0");
    _putenv(("DSA_FENSTER=" + g.chosen_fenster).c_str());
    _putenv(("DSA_SKALIERUNG=" + g.chosen_skalierung).c_str());
    _putenv((g.chosen_fenster == "vollbild") ? "DSA_FULLSCREEN=1" : "DSA_FULLSCREEN=0");
    _putenv((g.chosen_probenlog && g.chosen_stufe != STUFE_ORIG) ? "PROBENLOG=1" : "PROBENLOG=0");
    if (wait_for_exit && timeout_ms > 0) {
        char wd_buf[64];
        snprintf(wd_buf, sizeof(wd_buf), "DSA_WATCHDOG_MS=%lu", (unsigned long)timeout_ms);
        _putenv(wd_buf);
    }

    std::string cmd = "\"" + utf8_to_win1252(full_exe) + "\" --gamedir \"" + utf8_to_win1252(gamedir) + "\"";
    if (g.id == "schicksalsklinge") {
        cmd += " --savedir \"" + utf8_to_win1252(savedir) + "\"";
    }
    if (g.chosen_intro) {
        if (g.id == "schicksalsklinge") cmd += " --intro";
    } else {
        cmd += " --kein-intro";
    }
    cmd += " --fenster=" + g.chosen_fenster;
    cmd += " --skalierung=" + g.chosen_skalierung;
    if (g.chosen_fenster == "vollbild") {
        cmd += " --vollbild";
    }
    for (size_t i = 0; i < extra_port_args.size(); ++i) {
        cmd += " " + extra_port_args[i];
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

    if (!started) {
        if (h_plog) {
            TerminateProcess(h_plog, 0);
            CloseHandle(h_plog);
        }
        g_modal_active   = true;
        g_modal_is_ok    = false;
        g_modal_is_crash = false;
        g_modal_title    = L("START FEHLGESCHLAGEN", "LAUNCH FAILED");
        g_modal_line1    = L("CreateProcessA konnte die Port-EXE nicht starten:", "CreateProcessA could not start the Port executable:");
        g_modal_line2    = exe_name;
        g_modal_line3    = L("Bitte Berechtigungen und Abhängigkeiten prüfen.", "Please check file permissions and dependencies.");
        g_modal_line4.clear();
        return -4;
    }

    CloseHandle(pi.hThread);
    g_status_msg = std::string(L("Gestartet: ", "Started: ")) + exe_name + " (PID " + std::to_string(pi.dwProcessId) + ")";

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

        /* Pruefe nach Spielende, ob absturz.log geschrieben wurde */
        if (!inspect_absturz_log(crash_log_exe, g.name)) {
            inspect_absturz_log(crash_log_save, g.name);
        }
    } else {
        if (h_plog) CloseHandle(h_plog);
        if (g_watched_game_proc) CloseHandle(g_watched_game_proc);
        g_watched_game_proc = pi.hProcess;
        g_watched_exe_dir   = exe_dir;
        g_watched_save_dir  = savedir;
        g_watched_game_name = g.name;
    }

    std::string log_path = join_path(g_exe_dir, "letzter_start.log");
    FILE *lfp = fopen(utf8_to_win1252(log_path).c_str(), "w");
    if (lfp) {
        fprintf(lfp, "zeit=%s\n", current_timestamp_str().c_str());
        fprintf(lfp, "spiel=%s\n", g.id.c_str());
        fprintf(lfp, "stufe=%s\n", stufe_to_str(g.chosen_stufe));
        fprintf(lfp, "exe=%s\n", exe_name.c_str());
        fprintf(lfp, "fenster=%s\n", g.chosen_fenster.c_str());
        fprintf(lfp, "skalierung=%s\n", g.chosen_skalierung.c_str());
        fprintf(lfp, "probenlog=%d\n", (g.chosen_probenlog && g.chosen_stufe != STUFE_ORIG) ? 1 : 0);
        fprintf(lfp, "intro=%d\n", g.chosen_intro ? 1 : 0);
        fprintf(lfp, "pid=%lu\n", (unsigned long)pi.dwProcessId);
        fprintf(lfp, "probenlog_pid=%lu\n", (unsigned long)plog_pid);
        fprintf(lfp, "exit_code=%lu\n", (unsigned long)exit_code);
        fclose(lfp);
    }

    return (int)exit_code;
}

/* =========================================================================
 * 12. Maus- und Tastatur-Ereignisbehandlung (komplett bedienbar)
 * ========================================================================= */

static bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

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
    g_status_msg = std::string("Fenstergröße: ") + fenster_label_str(g.chosen_fenster);
    save_einstellungen_ini();
}

static void toggle_skalierung_mode(GameCatalogEntry &g) {
    if (_stricmp(g.chosen_skalierung.c_str(), "ganzzahlig") == 0) {
        g.chosen_skalierung = "fuellend";
    } else {
        g.chosen_skalierung = "ganzzahlig";
    }
    g_status_msg = std::string("Skalierung: ") + skalierung_label_str(g.chosen_skalierung);
    save_einstellungen_ini();
}

static void handle_mouse_click(int mx, int my, bool &running) {
    /* Header-Knopf Hilfe (F1) */
    if (point_in_rect(mx, my, SCREEN_W - 148, 27, 122, 38)) {
        g_help_active = !g_help_active;
        return;
    }

    if (g_help_active) {
        g_help_active = false;
        return;
    }
    if (g_modal_active) {
        g_modal_active = false;
        return;
    }

    if (g_screen == SCREEN_SELECT_GAME) {
        /* Die 3 Spielkarten (alles direkt pro Spiel regelbar) */
        for (size_t i = 0; i < g_games.size() && i < 3; ++i) {
            int cy = 154 + (int)i * 198;
            int cx = 18;
            int cw = SCREEN_W - 36;
            int ch = 180;

            if (point_in_rect(mx, my, cx, cy, cw, ch)) {
                g_selected_game = (int)i;

                /* 1. Knopf: [Probenlog: AN / AUS] */
                if (point_in_rect(mx, my, cx + 20, cy + 124, 152, 38)) {
                    g_games[i].chosen_probenlog = !g_games[i].chosen_probenlog;
                    g_status_msg = g_games[i].name + " – Probenlogger: " +
                                   (g_games[i].chosen_probenlog ? "EINGESCHALTET (PROBEN.CSV + Live-Fenster)" : "AUSGESCHALTET");
                    save_einstellungen_ini();
                    return;
                }
                /* 2. Knopf: [Patches: Original / Wichtige Fixes / Alle Fixes] */
                if (point_in_rect(mx, my, cx + 182, cy + 124, 196, 38)) {
                    if (g_games[i].chosen_stufe == STUFE_ORIG) {
                        g_games[i].chosen_stufe = STUFE_WICHTIG;
                    } else if (g_games[i].chosen_stufe == STUFE_WICHTIG) {
                        g_games[i].chosen_stufe = STUFE_ALLES;
                    } else {
                        g_games[i].chosen_stufe = STUFE_ORIG;
                    }
                    std::string exe_n = (g_games[i].chosen_stufe == STUFE_ORIG) ? g_games[i].exe_orig
                                      : ((g_games[i].chosen_stufe == STUFE_WICHTIG) ? g_games[i].exe_wichtig : g_games[i].exe_alles);
                    g_status_msg = g_games[i].name + " – Patches: " +
                                   stufe_short_label(g_games[i].chosen_stufe) + " (" + exe_n + ")";
                    save_einstellungen_ini();
                    return;
                }
                /* 3. Knopf: [Fenster: Vollbild / 2x..5x] */
                if (point_in_rect(mx, my, cx + 388, cy + 124, 154, 38)) {
                    cycle_fenster_mode(g_games[i], 1);
                    g_status_msg = g_games[i].name + " – " + g_status_msg;
                    return;
                }
                /* 4. Knopf: [Intro: AN / AUS] */
                if (point_in_rect(mx, my, cx + 552, cy + 124, 122, 38)) {
                    g_games[i].chosen_intro = !g_games[i].chosen_intro;
                    g_status_msg = g_games[i].name + " – Intro: " +
                                   (g_games[i].chosen_intro ? "EINGESCHALTET" : "AUSGESCHALTET");
                    save_einstellungen_ini();
                    return;
                }
                /* 5. Knopf: [Ordner wählen...] */
                if (point_in_rect(mx, my, cx + 684, cy + 124, 156, 38)) {
                    std::string picked = browse_for_gamedir_dialog(
                        (g_games[i].name + " – Spielordner oder Port-Ordner wählen:").c_str());
                    if (!picked.empty()) {
                        handle_dropped_folder(picked);
                    }
                    return;
                }
                /* 6. Knopf: [Einstellungen...] */
                if (point_in_rect(mx, my, cx + 850, cy + 124, 154, 38)) {
                    g_screen = SCREEN_SETTINGS;
                    g_settings_row = 0;
                    g_status_msg = g_games[i].name + " – Einstellungen anpassen und [>> SPIEL STARTEN] klicken.";
                    save_einstellungen_ini();
                    return;
                }
                /* 7. Knopf rechts oben: [>> Spiel starten] (bzw. [Ordner wählen...] falls noch nicht startbereit) */
                if (point_in_rect(mx, my, cx + cw - 236, cy + 18, 216, 42)) {
                    if (g_games[i].verfuegbar) {
                        save_einstellungen_ini();
                        std::vector<std::string> no_extra;
                        launch_selected_game(g_games[i], false, no_extra, false, 0);
                    } else {
                        std::string picked = browse_for_gamedir_dialog(
                            (g_games[i].name + " – Spielordner oder Port-Ordner wählen:").c_str());
                        if (!picked.empty()) {
                            handle_dropped_folder(picked);
                        }
                    }
                    return;
                }

                /* Klick auf die Karte selbst wählt das Spiel aus */
                g_status_msg = g_games[i].name + " ausgewählt.";
                save_einstellungen_ini();
                return;
            }
        }
    } else if (g_screen == SCREEN_SETTINGS) {
        if (g_selected_game < 0 || g_selected_game >= (int)g_games.size()) return;
        GameCatalogEntry &g = g_games[g_selected_game];

        /* 1. Patch-Stufe */
        if (point_in_rect(mx, my, 276, 105, 224, 38)) {
            g.chosen_stufe = STUFE_ORIG;
            g_settings_row = 0;
            g_status_msg = "Patch-Stufe: Original (FIX_BUGS=0)";
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 512, 105, 244, 38)) {
            g.chosen_stufe = STUFE_WICHTIG;
            g_settings_row = 0;
            g_status_msg = "Patch-Stufe: Wichtige Fixes (FIX_BUGS=1)";
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 768, 105, 254, 38)) {
            g.chosen_stufe = STUFE_ALLES;
            g_settings_row = 0;
            g_status_msg = "Patch-Stufe: Alle Fixes (FIX_BUGS=2)";
            save_einstellungen_ini();
            return;
        }

        /* 2. Fenstergroesse (2x, 3x, 4x, 5x, Vollbild) */
        const int fx_x[5] = {276, 424, 572, 730, 894};
        const int fx_w[5] = {140, 140, 150, 156, 128};
        const char *fx_v[5] = {"2", "3", "4", "5", "vollbild"};
        for (int fi = 0; fi < 5; ++fi) {
            if (point_in_rect(mx, my, fx_x[fi], 191, fx_w[fi], 38)) {
                g.chosen_fenster = fx_v[fi];
                g.chosen_vollbild = (g.chosen_fenster == "vollbild");
                g_settings_row = 1;
                g_status_msg = std::string("Fenstergröße: ") + fenster_label_str(g.chosen_fenster);
                save_einstellungen_ini();
                return;
            }
        }

        /* 3. Skalierung (Ganzzahlig vs. Fuellend) */
        if (point_in_rect(mx, my, 276, 277, 364, 38)) {
            g.chosen_skalierung = "ganzzahlig";
            g_settings_row = 2;
            g_status_msg = "Skalierung: Ganzzahlig (exakte Vielfache, alle Pixelspalten gleich breit).";
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 652, 277, 370, 38)) {
            g.chosen_skalierung = "fuellend";
            g_settings_row = 2;
            g_status_msg = "Skalierung: Füllend (Sharp Bilinear, größtmögliches 4:3-Bild).";
            save_einstellungen_ini();
            return;
        }

        /* 4. Probenlog */
        if (point_in_rect(mx, my, 276, 367, 234, 38)) {
            g.chosen_probenlog = false;
            g_settings_row = 3;
            g_status_msg = "Probenlog ausgeschaltet.";
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 522, 367, 500, 38)) {
            g.chosen_probenlog = true;
            g_settings_row = 3;
            g_status_msg = "Probenlog eingeschaltet (PROBEN.CSV + Live-Fenster).";
            save_einstellungen_ini();
            return;
        }

        /* 5. Vorspann */
        if (point_in_rect(mx, my, 276, 435, 346, 38)) {
            g.chosen_intro = false;
            g_settings_row = 4;
            g_status_msg = "Vorspann ausgeschaltet (--kein-intro).";
            save_einstellungen_ini();
            return;
        }
        if (point_in_rect(mx, my, 634, 435, 388, 38)) {
            g.chosen_intro = true;
            g_settings_row = 4;
            g_status_msg = "Vorspann eingeschaltet.";
            save_einstellungen_ini();
            return;
        }

        /* 6. Spielordner */
        if (point_in_rect(mx, my, 824, 503, 198, 38)) {
            g_settings_row = 5;
            std::string picked = browse_for_gamedir_dialog("Spielordner wählen:");
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
            return;
        }

        /* Fusszeile: "< Zurück zur Übersicht" */
        if (point_in_rect(mx, my, 30, 701, 264, 38)) {
            g_screen = SCREEN_SELECT_GAME;
            g_status_msg = "Übersicht: Schritt für Schritt prüfen und Spiel starten.";
            save_einstellungen_ini();
            return;
        }

        /* Fusszeile: ">> SPIEL STARTEN (Enter)" */
        if (point_in_rect(mx, my, 716, 699, 314, 42)) {
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

    if (vk == VK_F1) {
        g_help_active = !g_help_active;
        return;
    }
    if (vk == VK_F4) {
        int next_v = (g_font_variant % 6) + 1;
        apply_font_variant(next_v);
        g_status_msg = std::string("Schrift-Design gewechselt: ") + g_font_specs[g_font_variant - 1].name;
        return;
    }

    if (g_help_active) {
        if (vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) {
            g_help_active = false;
        }
        return;
    }

    if (g_modal_active) {
        if (vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) {
            g_modal_active = false;
        }
        return;
    }

    if (g_screen == SCREEN_SELECT_GAME) {
        int sel_idx = (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) ? g_selected_game : 0;
        if (vk == VK_ESCAPE) {
            save_einstellungen_ini();
            running = false;
            return;
        }
        if (vk == VK_F5) {
            if (g_games[sel_idx].verfuegbar) {
                std::vector<std::string> no_extra;
                launch_selected_game(g_games[sel_idx], false, no_extra, false, 0);
            }
            return;
        }
        if (ch == 'O') {
            std::string picked = browse_for_gamedir_dialog("Spielordner (oder GOG-Oberordner) wählen:");
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
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
        if (ch == 'P') {
            g_games[sel_idx].chosen_probenlog = !g_games[sel_idx].chosen_probenlog;
            g_status_msg = std::string("Probenlogger: ") + (g_games[sel_idx].chosen_probenlog ? "EINGESCHALTET (PROBEN.CSV + Live-Fenster)" : "AUSGESCHALTET");
            save_einstellungen_ini();
            return;
        }
        if (vk == '1' || vk == '2' || vk == '3') {
            int idx = vk - '1';
            if (idx >= 0 && idx < (int)g_games.size()) {
                if (g_games[idx].verfuegbar) {
                    g_selected_game = idx;
                    g_screen = SCREEN_SETTINGS;
                    g_settings_row = 0;
                    save_einstellungen_ini();
                } else {
                    g_status_msg = g_games[idx].name + ": Noch nicht freigeschaltet.";
                }
            }
            return;
        }
        if (vk == VK_UP) {
            for (int step = 1; step <= (int)g_games.size(); ++step) {
                int cand = (g_selected_game - step + (int)g_games.size()) % (int)g_games.size();
                if (g_games[cand].verfuegbar) { g_selected_game = cand; break; }
            }
            save_einstellungen_ini();
            return;
        }
        if (vk == VK_DOWN || vk == VK_TAB) {
            for (int step = 1; step <= (int)g_games.size(); ++step) {
                int cand = (g_selected_game + step) % (int)g_games.size();
                if (g_games[cand].verfuegbar) { g_selected_game = cand; break; }
            }
            save_einstellungen_ini();
            return;
        }
        if (vk == VK_RETURN || vk == VK_SPACE) {
            if (g_games[sel_idx].verfuegbar) {
                g_screen = SCREEN_SETTINGS;
                g_settings_row = 0;
                save_einstellungen_ini();
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
            std::vector<std::string> no_extra;
            launch_selected_game(g, false, no_extra, false, 0);
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
        if (ch == 'O') {
            g_settings_row = 5;
            std::string picked = browse_for_gamedir_dialog("Spielordner wählen:");
            if (!picked.empty()) {
                handle_dropped_folder(picked);
            }
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
                std::string picked = browse_for_gamedir_dialog("Spielordner wählen:");
                if (!picked.empty()) {
                    handle_dropped_folder(picked);
                }
            } else {
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

    std::string spiele_json = join_path(g_starter_dir, "spiele.json");
    if (!load_spiele_json(spiele_json)) {
        GameCatalogEntry s1 = make_default_entry(1);
        s1.id = "schicksalsklinge";
        s1.name = "Die Schicksalsklinge";
        s1.untertitel = "Das Schwarze Auge - Teil 1 (1992)";
        s1.port_ordner = "../Port/build";
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
        }
    }

    load_einstellungen_ini();
    g_lang_en = false;
    refresh_all_games();
    g_status_msg = "Tipp: Spielordner oder .zip einfach in dieses Fenster ziehen! (F1=Hilfe, F5=Schnellstart)";

    std::string screenshot_path;
    std::string test_absturz_path;
    std::vector<std::string> simulated_drops;
    bool auto_start          = false;
    bool auto_quick_start    = false;
    bool auto_quit           = false;
    bool test_missing_exe    = false;
    bool trigger_build_paket = false;
    DWORD timeout_ms         = 20000;
    std::vector<std::string> extra_port_args;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--no-auto-gog" || a == "--no-grafikpaket") {
            continue;
        } else if (a == "--screenshot" && i + 1 < argc) {
            screenshot_path = argv[++i];
        } else if (strncmp(a.c_str(), "--screenshot=", 13) == 0) {
            screenshot_path = a.substr(13);
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
        } else if (strncmp(a.c_str(), "--mouse-pos=", 12) == 0) {
            int mx = 0, my = 0;
            if (sscanf(a.c_str() + 12, "%d,%d", &mx, &my) == 2) {
                g_mouse_x = mx;
                g_mouse_y = my;
                for (size_t gi = 0; gi < g_games.size() && gi < 3; ++gi) {
                    int cy = 154 + (int)gi * 198;
                    if (point_in_rect(g_mouse_x, g_mouse_y, 18, cy, SCREEN_W - 36, 180)) {
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
            /* S3: Alte --grafik=-Optionen ignorieren */
        } else if (strncmp(a.c_str(), "--probenlog=", 12) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_probenlog = (a.substr(12) == "1");
            }
        } else if (strncmp(a.c_str(), "--intro=", 8) == 0) {
            if (g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
                g_games[g_selected_game].chosen_intro = (a.substr(8) == "1");
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
        "Das Schwarze Auge: Die Nordland-Trilogie",
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

    if (!test_absturz_path.empty() && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        inspect_absturz_log(test_absturz_path, g_games[g_selected_game].name);
    }

    render_frame();

    if (!screenshot_path.empty()) {
        save_framebuffer_bmp(screenshot_path);
    }

    if ((auto_start || auto_quick_start) && g_selected_game >= 0 && g_selected_game < (int)g_games.size()) {
        rc = launch_selected_game(g_games[g_selected_game], false, extra_port_args, true, timeout_ms);
        render_frame();
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
                std::string c1 = join_path(g_watched_exe_dir, "absturz.log");
                std::string c2 = join_path(g_watched_save_dir, "absturz.log");
                if (!inspect_absturz_log(c1, g_watched_game_name)) {
                    inspect_absturz_log(c2, g_watched_game_name);
                }
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
                g_hover_game = -1;
                if (g_screen == SCREEN_SELECT_GAME && !g_help_active && !g_modal_active) {
                    for (size_t i = 0; i < g_games.size() && i < 3; ++i) {
                        int cy = 154 + (int)i * 198;
                        if (point_in_rect(g_mouse_x, g_mouse_y, 18, cy, SCREEN_W - 36, 180)) {
                            g_hover_game = (int)i;
                            break;
                        }
                    }
                }
            } else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                g_mouse_x = ev.button.x;
                g_mouse_y = ev.button.y;
                handle_mouse_click(ev.button.x, ev.button.y, running);
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
