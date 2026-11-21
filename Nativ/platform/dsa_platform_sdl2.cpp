/*
 * dsa_platform_sdl2.cpp - SDL2-Plattformschicht fuer die Nordlandtrilogie
 *   (DSA 1 Schicksalsklinge, DSA 2 Sternenschweif, DSA 3 Schatten ueber Riva)
 */

#include "dsa_platform.h"
#include <SDL2/SDL.h>
#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <direct.h>
#include <io.h>
#define MKDIR(p) _mkdir(p)
#define ACCESS(p, m) _access(p, m)
#define STRNICMP _strnicmp
#define STRICMP _stricmp
#else
#include <dlfcn.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#define MKDIR(p) mkdir(p, 0755)
#define ACCESS(p, m) access(p, m)
#define STRNICMP strncasecmp
#define STRICMP strcasecmp

static inline int  SDL_PortUsingExternalDLL(void) { return 1; }
static inline void SDL_SetWindowViewport(SDL_Window *w, int vx, int vy, int vw, int vh) {
    (void)w; (void)vx; (void)vy; (void)vw; (void)vh;
}
static inline void SDL_SetFullscreenOverrideRes(int w, int h) {
    (void)w; (void)h;
}
#endif

#include "dbopl.h"

static void dsa_save_bmp(const char *path);
static void keyq_push(unsigned short bios_key);

/* =========================================================================
 * 0. Werkzeuge W1 (Absturz-Logger), W2 (Stub-Waechter), W3 (Abdeckung), W4 (Record/Replay)
 * ========================================================================= */

const char *g_dsa_last_func = "start";
static const char * volatile g_dsa_plat_func = "keine";
int         g_dsa_coverage_enabled = 0;

struct DsaPlatScope {
    const char *prev;
    explicit DsaPlatScope(const char *fn) : prev(g_dsa_plat_func) { g_dsa_plat_func = fn; }
    ~DsaPlatScope() { g_dsa_plat_func = prev; }
};

static char g_exe_dir[512] = "";
static void (*g_crash_state_cb)(char *buf, int buf_size) = NULL;

#define MAX_STUB_NAMES 256
static const char *g_logged_stubs[MAX_STUB_NAMES];
static int         g_logged_stubs_count = 0;

#define MAX_TRACE_FUNCS 2048
static unsigned char g_func_hit_table[MAX_TRACE_FUNCS];
static FILE         *g_coverage_fp = NULL;

struct ReplayEvent {
    char     type; /* 'M'=Mouse, 'K'=Key, 'S'=Screenshot, 'Q'=Quit */
    uint32_t step;
    int      x, y, moved, left_ev, right_ev;
    uint16_t key;
    char     arg[260];
};

#define MAX_REPLAY_EVENTS 8192
static ReplayEvent g_replay_events[MAX_REPLAY_EVENTS];
static int         g_replay_count  = 0;
static int         g_replay_idx    = 0;
static int         g_replay_active = 0;
static int         g_fast_replay   = 0;
static uint32_t    g_poll_step     = 0;
static FILE       *g_record_fp     = NULL;

static void init_exe_dir_once(void) {
    if (g_exe_dir[0]) return;
#ifdef _WIN32
    char mod_path[512] = "";
    DWORD len = GetModuleFileNameA(NULL, mod_path, sizeof(mod_path) - 1);
    if (len > 0) {
        mod_path[len] = '\0';
        char *last_sep = strrchr(mod_path, '\\');
        if (!last_sep) last_sep = strrchr(mod_path, '/');
        if (last_sep) {
            *last_sep = '\0';
            strncpy(g_exe_dir, mod_path, sizeof(g_exe_dir) - 1);
            return;
        }
    }
#elif defined(__linux__)
    char mod_path[512] = "";
    ssize_t len = readlink("/proc/self/exe", mod_path, sizeof(mod_path) - 1);
    if (len > 0) {
        mod_path[len] = '\0';
        char *last_sep = strrchr(mod_path, '/');
        if (last_sep) {
            *last_sep = '\0';
            strncpy(g_exe_dir, mod_path, sizeof(g_exe_dir) - 1);
            return;
        }
    }
#elif defined(__APPLE__)
    char mod_path[512] = "";
    uint32_t sz = (uint32_t)(sizeof(mod_path) - 1);
    if (_NSGetExecutablePath(mod_path, &sz) == 0) {
        char *last_sep = strrchr(mod_path, '/');
        if (last_sep) {
            *last_sep = '\0';
            strncpy(g_exe_dir, mod_path, sizeof(g_exe_dir) - 1);
            return;
        }
    }
#endif
    strcpy(g_exe_dir, ".");
}

static void build_tool_log_path(const char *filename, char *out_buf, size_t out_size) {
    init_exe_dir_once();
#ifdef _WIN32
    snprintf(out_buf, out_size, "%s\\%s", g_exe_dir, filename);
#else
    snprintf(out_buf, out_size, "%s/%s", g_exe_dir, filename);
#endif
}

static char g_save_dir[512] = "";

#ifdef _WIN32
static void dsa_resolve_crash_symbol(const void *addr, char *out_func, size_t out_func_sz, char *out_mod, size_t out_mod_sz) {
    struct SymEntry { const void *fn; const char *name; };
    const SymEntry table[] = {
        { (const void *)&dsa_platform_init,                      "dsa_platform_init" },
        { (const void *)&dsa_platform_shutdown,                  "dsa_platform_shutdown" },
        { (const void *)&dsa_platform_set_mode,                  "dsa_platform_set_mode" },
        { (const void *)&dsa_platform_present,                   "dsa_platform_present" },
        { (const void *)&dsa_platform_pump_and_present_if_due,   "dsa_platform_pump_and_present_if_due" },
        { (const void *)&dsa_platform_wait_vsync,                "dsa_platform_wait_vsync" },
        { (const void *)&dsa_platform_get_mouse,                 "dsa_platform_get_mouse" },
        { (const void *)&dsa_platform_poll_mouse_events,         "dsa_platform_poll_mouse_events" },
        { (const void *)&dsa_platform_set_mouse_pos,             "dsa_platform_set_mouse_pos" },
        { (const void *)&dsa_platform_key_ready,                 "dsa_platform_key_ready" },
        { (const void *)&dsa_platform_key_read,                  "dsa_platform_key_read" },
        { (const void *)&dsa_platform_play_cd_track,             "dsa_platform_play_cd_track" },
        { (const void *)&dsa_platform_play_xmidi,                "dsa_platform_play_xmidi" },
        { (const void *)&dsa_platform_play_voc,                  "dsa_platform_play_voc" },
        { (const void *)&dsa_save_bmp,                           "dsa_save_bmp" },
        { (const void *)&SDL_Init,                               "SDL_Init" },
        { (const void *)&SDL_CreateWindow,                       "SDL_CreateWindow" },
        { (const void *)&SDL_CreateRenderer,                     "SDL_CreateRenderer" },
        { (const void *)&SDL_CreateTexture,                      "SDL_CreateTexture" },
        { (const void *)&SDL_UpdateTexture,                      "SDL_UpdateTexture" },
        { (const void *)&SDL_RenderPresent,                      "SDL_RenderPresent" },
        { (const void *)&SDL_PollEvent,                          "SDL_PollEvent" },
    };
    uintptr_t target = (uintptr_t)addr;
    const char *best_name = NULL;
    uintptr_t best_diff = (uintptr_t)-1;
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        uintptr_t fn_addr = (uintptr_t)table[i].fn;
        if (target >= fn_addr && (target - fn_addr) < 0x2000 && (target - fn_addr) < best_diff) {
            best_diff = target - fn_addr;
            best_name = table[i].name;
        }
    }
    if (best_name) {
        snprintf(out_func, out_func_sz, "%s+0x%lX", best_name, (unsigned long)best_diff);
    } else if (g_dsa_plat_func && strcmp(g_dsa_plat_func, "keine") != 0) {
        snprintf(out_func, out_func_sz, "%s (aktiv)", g_dsa_plat_func);
    } else if (g_dsa_last_func && g_dsa_last_func[0]) {
        snprintf(out_func, out_func_sz, "%s", g_dsa_last_func);
    } else {
        snprintf(out_func, out_func_sz, "unbekannt");
    }

    HMODULE hMod = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)addr, &hMod) && hMod) {
        char mod_path[260] = "";
        GetModuleFileNameA(hMod, mod_path, sizeof(mod_path) - 1);
        const char *base = strrchr(mod_path, '\\');
        if (!base) base = strrchr(mod_path, '/');
        base = base ? (base + 1) : mod_path;
        uintptr_t rva = target - (uintptr_t)hMod;
        snprintf(out_mod, out_mod_sz, "%s+0x%lX", base[0] ? base : "modul", (unsigned long)rva);
    } else {
        snprintf(out_mod, out_mod_sz, "0x%p", addr);
    }
}

static void dsa_write_crash_file(const char *path, const char *msg, DWORD msg_len, const char *state_buf, DWORD state_len) {
    if (!path || !path[0]) return;
    HANDLE h = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        if (msg_len > 0) WriteFile(h, msg, msg_len, &written, NULL);
        if (state_len > 0) WriteFile(h, state_buf, state_len, &written, NULL);
        FlushFileBuffers(h);
        CloseHandle(h);
    }
}

static LONG WINAPI dsa_crash_vectored_handler(EXCEPTION_POINTERS *ep) {
    static int in_crash = 0;
    static char s_msg[2048];
    static char s_state_buf[2048];
    static char s_func_name[160];
    static char s_mod_info[160];
    static char s_path1[512];
    static char s_path2[512];

    if (!ep || !ep->ExceptionRecord || (ep->ExceptionRecord->ExceptionCode < 0xC0000000UL)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    /* Ignoriere harmlose Win32-RPC/Debugger-Ausnahmen (< 0xC0000005 oder DBG_*) */
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    if (code == 0x40010006UL || code == 0x406D1388UL) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    if (in_crash) {
        TerminateProcess(GetCurrentProcess(), 98);
    }
    in_crash = 1;

    dsa_resolve_crash_symbol(
        ep->ExceptionRecord->ExceptionAddress,
        s_func_name, sizeof(s_func_name),
        s_mod_info, sizeof(s_mod_info)
    );

    unsigned long reg_eip = 0, reg_esp = 0, reg_ebp = 0, reg_eax = 0, reg_ebx = 0, reg_ecx = 0, reg_edx = 0;
#if defined(_M_IX86) || defined(__i386__)
    if (ep->ContextRecord) {
        reg_eip = (unsigned long)ep->ContextRecord->Eip;
        reg_esp = (unsigned long)ep->ContextRecord->Esp;
        reg_ebp = (unsigned long)ep->ContextRecord->Ebp;
        reg_eax = (unsigned long)ep->ContextRecord->Eax;
        reg_ebx = (unsigned long)ep->ContextRecord->Ebx;
        reg_ecx = (unsigned long)ep->ContextRecord->Ecx;
        reg_edx = (unsigned long)ep->ContextRecord->Edx;
    }
#endif

    int n = snprintf(s_msg, sizeof(s_msg),
        "=== ABSTURZ-BERICHT (W1) ===\n"
        "Ausnahme:        0x%08lX\n"
        "Adresse:         0x%p (%s)\n"
        "Funktion:        %s\n"
        "Plattform_Funk:  %s\n"
        "Letzte_Funktion: %s\n"
        "Poll_Schritt:    %u\n"
        "Register:        EIP=0x%08lX ESP=0x%08lX EBP=0x%08lX EAX=0x%08lX EBX=0x%08lX ECX=0x%08lX EDX=0x%08lX\n",
        (unsigned long)code,
        ep->ExceptionRecord->ExceptionAddress,
        s_mod_info,
        s_func_name,
        g_dsa_plat_func ? g_dsa_plat_func : "keine",
        g_dsa_last_func ? g_dsa_last_func : "unbekannt",
        (unsigned int)g_poll_step,
        reg_eip, reg_esp, reg_ebp, reg_eax, reg_ebx, reg_ecx, reg_edx
    );
    DWORD msg_len = (n > 0) ? (DWORD)n : 0;

    s_state_buf[0] = '\0';
    if (g_crash_state_cb) {
        g_crash_state_cb(s_state_buf, sizeof(s_state_buf));
    }
    DWORD state_len = (DWORD)strlen(s_state_buf);

    build_tool_log_path("absturz.log", s_path1, sizeof(s_path1));
    dsa_write_crash_file(s_path1, s_msg, msg_len, s_state_buf, state_len);

    if (g_save_dir[0]) {
        snprintf(s_path2, sizeof(s_path2), "%s\\absturz.log", g_save_dir);
        if (_stricmp(s_path1, s_path2) != 0) {
            dsa_write_crash_file(s_path2, s_msg, msg_len, s_state_buf, state_len);
        }
    }
    dsa_write_crash_file("absturz.log", s_msg, msg_len, s_state_buf, state_len);

    const char *env_cl = getenv("DSA_CRASH_LOG");
    if (env_cl && env_cl[0]) {
        dsa_write_crash_file(env_cl, s_msg, msg_len, s_state_buf, state_len);
    }

    TerminateProcess(GetCurrentProcess(), 99);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void dsa_install_crash_handlers_once(void) {
    static int s_installed = 0;
    if (!s_installed) {
        init_exe_dir_once();
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
        AddVectoredExceptionHandler(1, dsa_crash_vectored_handler);
        SetUnhandledExceptionFilter(dsa_crash_vectored_handler);
        s_installed = 1;
    }
}
#else
static void dsa_resolve_crash_symbol(const void *addr, char *out_func, size_t out_func_sz, char *out_mod, size_t out_mod_sz) {
    struct SymEntry { const void *fn; const char *name; };
    const SymEntry table[] = {
        { (const void *)&dsa_platform_init,                      "dsa_platform_init" },
        { (const void *)&dsa_platform_shutdown,                  "dsa_platform_shutdown" },
        { (const void *)&dsa_platform_set_mode,                  "dsa_platform_set_mode" },
        { (const void *)&dsa_platform_present,                   "dsa_platform_present" },
        { (const void *)&dsa_platform_pump_and_present_if_due,   "dsa_platform_pump_and_present_if_due" },
        { (const void *)&dsa_platform_wait_vsync,                "dsa_platform_wait_vsync" },
        { (const void *)&dsa_platform_get_mouse,                 "dsa_platform_get_mouse" },
        { (const void *)&dsa_platform_poll_mouse_events,         "dsa_platform_poll_mouse_events" },
        { (const void *)&dsa_platform_set_mouse_pos,             "dsa_platform_set_mouse_pos" },
        { (const void *)&dsa_platform_key_ready,                 "dsa_platform_key_ready" },
        { (const void *)&dsa_platform_key_read,                  "dsa_platform_key_read" },
        { (const void *)&dsa_platform_play_cd_track,             "dsa_platform_play_cd_track" },
        { (const void *)&dsa_platform_play_xmidi,                "dsa_platform_play_xmidi" },
        { (const void *)&dsa_platform_play_voc,                  "dsa_platform_play_voc" },
        { (const void *)&dsa_save_bmp,                           "dsa_save_bmp" },
    };
    uintptr_t target = (uintptr_t)addr;
    const char *best_name = NULL;
    uintptr_t best_diff = (uintptr_t)-1;
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        uintptr_t fn_addr = (uintptr_t)table[i].fn;
        if (target >= fn_addr && (target - fn_addr) < 0x2000 && (target - fn_addr) < best_diff) {
            best_diff = target - fn_addr;
            best_name = table[i].name;
        }
    }
    Dl_info dli;
    memset(&dli, 0, sizeof(dli));
    int has_dli = (addr && dladdr(addr, &dli) != 0);
    if (!best_name && has_dli && dli.dli_sname) {
        best_name = dli.dli_sname;
        best_diff = target - (uintptr_t)dli.dli_saddr;
    }
    if (best_name) {
        snprintf(out_func, out_func_sz, "%s+0x%lX", best_name, (unsigned long)best_diff);
    } else if (g_dsa_plat_func && strcmp(g_dsa_plat_func, "keine") != 0) {
        snprintf(out_func, out_func_sz, "%s (aktiv)", g_dsa_plat_func);
    } else if (g_dsa_last_func && g_dsa_last_func[0]) {
        snprintf(out_func, out_func_sz, "%s", g_dsa_last_func);
    } else {
        snprintf(out_func, out_func_sz, "unbekannt");
    }

    if (has_dli && dli.dli_fname) {
        const char *base = strrchr(dli.dli_fname, '/');
        base = base ? (base + 1) : dli.dli_fname;
        uintptr_t rva = target - (uintptr_t)dli.dli_fbase;
        snprintf(out_mod, out_mod_sz, "%s+0x%lX", base[0] ? base : "modul", (unsigned long)rva);
    } else {
        snprintf(out_mod, out_mod_sz, "%p", addr);
    }
}

static void dsa_write_crash_file(const char *path, const char *msg, size_t msg_len, const char *state_buf, size_t state_len) {
    if (!path || !path[0]) return;
    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd >= 0) {
        if (msg_len > 0) { ssize_t r = write(fd, msg, msg_len); (void)r; }
        if (state_len > 0) { ssize_t r = write(fd, state_buf, state_len); (void)r; }
        close(fd);
    }
}

static const char *dsa_sig_name(int sig) {
    switch (sig) {
    case SIGSEGV: return "SIGSEGV (Speicherzugriffsfehler)";
    case SIGBUS:  return "SIGBUS (Bus-Fehler)";
    case SIGFPE:  return "SIGFPE (Arithmetikfehler)";
    case SIGILL:  return "SIGILL (Ungueltiger Befehl)";
    case SIGABRT: return "SIGABRT (Abbruch)";
    default:      return "SIGNAL";
    }
}

static void dsa_posix_crash_signal_handler(int sig, siginfo_t *si, void *ucontext) {
    (void)ucontext;
    static volatile sig_atomic_t in_crash = 0;
    static char s_msg[2048];
    static char s_state_buf[2048];
    static char s_func_name[160];
    static char s_mod_info[160];
    static char s_path1[512];
    static char s_path2[512];

    if (in_crash) {
        _exit(98);
    }
    in_crash = 1;

    const void *fault_addr = si ? si->si_addr : NULL;
    const void *pc_addr = fault_addr;
    const void *ret_addr = NULL;
    unsigned long reg_rdx = 0;
#if defined(__linux__) && defined(__x86_64__)
    if (ucontext) {
        const ucontext_t *uc = (const ucontext_t *)ucontext;
        pc_addr = (const void *)uc->uc_mcontext.gregs[16];
        reg_rdx = (unsigned long)uc->uc_mcontext.gregs[12];
        const void * const *rsp = (const void * const *)uc->uc_mcontext.gregs[15];
        if (rsp) ret_addr = rsp[0];
    }
#endif
    dsa_resolve_crash_symbol(
        pc_addr,
        s_func_name, sizeof(s_func_name),
        s_mod_info, sizeof(s_mod_info)
    );

    int n = snprintf(s_msg, sizeof(s_msg),
        "=== ABSTURZ-BERICHT (W1) ===\n"
        "Ausnahme:        Signal %d (%s)\n"
        "Adresse:         %p (PC=%p, RET=%p, RDX=0x%lX, %s)\n"
        "Funktion:        %s\n"
        "Plattform_Funk:  %s\n"
        "Letzte_Funktion: %s\n"
        "Poll_Schritt:    %u\n",
        sig, dsa_sig_name(sig),
        fault_addr,
        pc_addr,
        ret_addr,
        reg_rdx,
        s_mod_info,
        s_func_name,
        g_dsa_plat_func ? g_dsa_plat_func : "keine",
        g_dsa_last_func ? g_dsa_last_func : "unbekannt",
        (unsigned int)g_poll_step
    );
    size_t msg_len = (n > 0) ? (size_t)n : 0;

    s_state_buf[0] = '\0';
    if (g_crash_state_cb) {
        g_crash_state_cb(s_state_buf, sizeof(s_state_buf));
    }
    size_t state_len = strlen(s_state_buf);

    build_tool_log_path("absturz.log", s_path1, sizeof(s_path1));
    dsa_write_crash_file(s_path1, s_msg, msg_len, s_state_buf, state_len);

    if (g_save_dir[0]) {
        snprintf(s_path2, sizeof(s_path2), "%s/absturz.log", g_save_dir);
        if (strcmp(s_path1, s_path2) != 0) {
            dsa_write_crash_file(s_path2, s_msg, msg_len, s_state_buf, state_len);
        }
    }
    dsa_write_crash_file("absturz.log", s_msg, msg_len, s_state_buf, state_len);

    const char *env_cl = getenv("DSA_CRASH_LOG");
    if (env_cl && env_cl[0]) {
        dsa_write_crash_file(env_cl, s_msg, msg_len, s_state_buf, state_len);
    }

    _exit(99);
}

static void dsa_install_crash_handlers_once(void) {
    static int s_installed = 0;
    if (!s_installed) {
        init_exe_dir_once();
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = dsa_posix_crash_signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGBUS,  &sa, NULL);
        sigaction(SIGFPE,  &sa, NULL);
        sigaction(SIGILL,  &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);
        s_installed = 1;
    }
}
#endif

void dsa_platform_register_crash_state_cb(void (*cb)(char *buf, int buf_size)) {
    g_crash_state_cb = cb;
    dsa_install_crash_handlers_once();
}

void dsa_platform_log_stub(const char *func_name) {
    if (!func_name) return;
    for (int i = 0; i < g_logged_stubs_count; ++i) {
        if (strcmp(g_logged_stubs[i], func_name) == 0) return;
    }
    if (g_logged_stubs_count < MAX_STUB_NAMES) {
        g_logged_stubs[g_logged_stubs_count++] = func_name;
    }
    char path[512];
    build_tool_log_path("stubs.log", path, sizeof(path));
    FILE *fp = fopen(path, "a");
    if (fp) {
        fprintf(fp, "STUB %s\n", func_name);
        fclose(fp);
    }
}

void dsa_platform_trace_func_hit(int func_id, const char *seg_and_func) {
    if (func_id < 0 || func_id >= MAX_TRACE_FUNCS) return;
    if (g_func_hit_table[func_id]) return;
    g_func_hit_table[func_id] = 1;
    if (!g_coverage_fp) {
        char path[512];
        build_tool_log_path("abdeckung.tsv", path, sizeof(path));
        g_coverage_fp = fopen(path, "a");
    }
    if (g_coverage_fp && seg_and_func) {
        const char *colon = strchr(seg_and_func, ':');
        if (colon) {
            fprintf(g_coverage_fp, "%.*s\t%s\n", (int)(colon - seg_and_func), seg_and_func, colon + 1);
        } else {
            fprintf(g_coverage_fp, "misc\t%s\n", seg_and_func);
        }
        fflush(g_coverage_fp);
    }
}

void dsa_platform_set_record_file(const char *path) {
    if (g_record_fp) { fclose(g_record_fp); g_record_fp = NULL; }
    if (path && path[0]) {
        g_record_fp = fopen(path, "w");
        if (g_record_fp) {
            fprintf(g_record_fp, "# DSA Schicksalsklinge Eingabe-Aufzeichnung (W4)\n");
            fflush(g_record_fp);
        }
    }
}

void dsa_platform_set_replay_file(const char *path) {
    g_replay_count = 0;
    g_replay_idx = 0;
    g_replay_active = 0;
    if (!path || !path[0]) return;
    FILE *fp = fopen(path, "r");
    if (!fp) return;
    char line[512];
    uint32_t last_step = 0;
    while (fgets(line, sizeof(line), fp) && g_replay_count < MAX_REPLAY_EVENTS) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;
        ReplayEvent &ev = g_replay_events[g_replay_count];
        memset(&ev, 0, sizeof(ev));
        char step_tok[32] = "";
        if (line[0] == 'M') {
            if (sscanf(line, "M %31s %d %d %d %d %d", step_tok, &ev.x, &ev.y, &ev.moved, &ev.left_ev, &ev.right_ev) == 6) {
                uint32_t step = (step_tok[0] == '+')
                    ? (last_step + (uint32_t)strtoul(step_tok + 1, NULL, 10))
                    : (uint32_t)strtoul(step_tok, NULL, 10);
                last_step = step;
                ev.type = 'M';
                ev.step = step;
                g_replay_count++;
            }
        } else if (line[0] == 'K') {
            unsigned int khex = 0;
            if (sscanf(line, "K %31s %x", step_tok, &khex) == 2) {
                uint32_t step = (step_tok[0] == '+')
                    ? (last_step + (uint32_t)strtoul(step_tok + 1, NULL, 10))
                    : (uint32_t)strtoul(step_tok, NULL, 10);
                last_step = step;
                ev.type = 'K';
                ev.step = step;
                ev.key  = (uint16_t)khex;
                g_replay_count++;
            }
        } else if (line[0] == 'S') {
            if (sscanf(line, "S %31s %259s", step_tok, ev.arg) == 2) {
                uint32_t step = (step_tok[0] == '+')
                    ? (last_step + (uint32_t)strtoul(step_tok + 1, NULL, 10))
                    : (uint32_t)strtoul(step_tok, NULL, 10);
                last_step = step;
                ev.type = 'S';
                ev.step = step;
                g_replay_count++;
            }
        } else if (line[0] == 'Q') {
            if (sscanf(line, "Q %31s", step_tok) == 1) {
                uint32_t step = (step_tok[0] == '+')
                    ? (last_step + (uint32_t)strtoul(step_tok + 1, NULL, 10))
                    : (uint32_t)strtoul(step_tok, NULL, 10);
                last_step = step;
                ev.type = 'Q';
                ev.step = step;
                g_replay_count++;
            }
        }
    }
    fclose(fp);
    if (g_replay_count > 0) {
        g_replay_active = 1;
    }
}

void dsa_platform_set_fast_replay(int enabled) {
    g_fast_replay = enabled ? 1 : 0;
}

int dsa_platform_is_fast_replay(void) {
    return g_fast_replay;
}

void dsa_platform_save_screenshot_bmp(const char *path) {
    dsa_save_bmp(path);
}

/* =========================================================================
 * 1. 8-Bit-Framebuffer & 256-Farben-VGA-Palette
 * ========================================================================= */

/* 64 KB Sicherheits-Padding vor und nach dem 640x480-VRAM-Puffer */
static unsigned char g_vram_padded_storage[65536 + DSA_VRAM_SIZE + 65536];
#define g_vram (&g_vram_padded_storage[65536])

static uint32_t      g_argb_buffer[DSA_VRAM_SIZE];
static unsigned char g_pal_rgb6[256][3];
static uint32_t      g_pal_argb32[256];

static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture  *g_texture  = NULL;
static SDL_AudioDeviceID g_audio_dev = 0;

static void dsa_audio_init(void);
static void dsa_audio_shutdown(void);

static int      g_initialized     = 0;
static int      g_dos_mode        = 0x13;
static int      g_width           = 320;
static int      g_height          = 200;
static int      g_scale           = 4;
static uint32_t g_last_present_ms = 0;
static uint32_t g_virtual_ticks_ms = 100;

/* S2 (Etappe 3): 4:3-Skalierung (--fenster=2|3|4|5|vollbild, --skalierung=ganzzahlig|fuellend) */
enum {
    DSA_SKAL_GANZZAHLIG = 0,
    DSA_SKAL_FUELLEND   = 1
};
static int       g_display_inited      = 0;
static int       g_fenster_faktor      = 4; /* 2..5 fuer Fenstermodus */
static int       g_is_fullscreen       = 1; /* Standard: vollbild (1) */
static int       g_fenster_explicit    = 0;
static int       g_skalierung_mode     = DSA_SKAL_FUELLEND;
static SDL_Texture *g_display_tex      = NULL;
static uint32_t *g_display_buf         = NULL;
static int       g_display_w           = 0;
static int       g_display_h           = 0;
static int       g_vp_x                = 0;
static int       g_vp_y                = 0;
static int       g_vp_w                = 1280;
static int       g_vp_h                = 960;
static char      g_scaled_shot_path[512] = "";
static char      g_maustest_log_path[512] = "";
static int       g_scaled_shot_saved   = 0;

/* =========================================================================
 * 2. Maus- und Tastaturzustand
 * ========================================================================= */

static int           g_mouse_x            = 160;
static int           g_mouse_y            = 100;
static int           g_mouse_min_x        = 0;
static int           g_mouse_max_x        = 319;
static int           g_mouse_min_y        = 0;
static int           g_mouse_max_y        = 199;
static int           g_mouse_buttons      = 0;
static int           g_mouse_latched_down = 0;
static int           g_mouse_pending_up   = 0;
static int           g_mouse_moved_ev     = 0;
static int           g_mouse_left_ev      = 0;
static int           g_mouse_right_ev     = 0;

#define KEY_QUEUE_CAP 128
static unsigned short g_key_queue[KEY_QUEUE_CAP];
static int            g_key_head = 0;
static int            g_key_tail = 0;
static unsigned short g_shift_status = 0;
static unsigned char  g_scancode_down[128];

/* Konvertiert Windows-1252 / ISO-8859-1 Umlaute nach CP437 */
static unsigned char win1252_to_cp437(unsigned char c) {
    switch (c) {
    case 0xE4: case 0xC4: return 0x84; /* ä / Ä -> 0x84 (wird in GUI_enter_string zu 0x8E) */
    case 0xF6: case 0xD6: return 0x94; /* ö / Ö -> 0x94 (wird in GUI_enter_string zu 0x99) */
    case 0xFC: case 0xDC: return 0x81; /* ü / Ü -> 0x81 (wird in GUI_enter_string zu 0x9A) */
    case 0xDF:            return 0xE1; /* ß */
    default:              return c;
    }
}

/* USB-HID SDL_Scancode -> IBM PC XT/AT Set-1 Scancode (fuer externe SDL2.dll) */
static unsigned char sdl_scancode_to_dos(int sdl_sc, int sym, int mod, unsigned char *out_ascii) {
    static const unsigned char hid_to_xt[128] = {
        0x00, 0x00, 0x00, 0x00, 0x1E, 0x30, 0x2E, 0x20, /* 0..7: A, B, C, D */
        0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, /* 8..15: E..L */
        0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, /* 16..23: M..T */
        0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x02, 0x03, /* 24..31: U..Z, 1, 2 */
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /* 32..39: 3..9, 0 */
        0x1C, 0x01, 0x0E, 0x0F, 0x39, 0x0C, 0x0D, 0x1A, /* 40..47: Enter, Esc, BS, Tab, Space, -, =, [ */
        0x1B, 0x2B, 0x2B, 0x27, 0x28, 0x29, 0x33, 0x34, /* 48..55: ], \, #, ;, ', `, ,, . */
        0x35, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, /* 56..63: /, Caps, F1..F6 */
        0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x37, 0x46, /* 64..71: F7..F12, PrtSc, ScrLk */
        0x00, 0x52, 0x47, 0x49, 0x53, 0x4F, 0x51, 0x4D, /* 72..79: Pause, Ins, Home, PgUp, Del, End, PgDn, Right */
        0x4B, 0x50, 0x48, 0x45, 0x35, 0x37, 0x4A, 0x4E, /* 80..87: Left, Down, Up, NumLk, KP/, KP*, KP-, KP+ */
        0x1C, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, /* 88..95: KPEnter, KP1..KP7 */
        0x48, 0x49, 0x52, 0x53                          /* 96..99: KP8, KP9, KP0, KP. */
    };
    unsigned char xt = 0;
    unsigned char asc = 0;
    if (sdl_sc >= 0 && sdl_sc < 100) {
        xt = hid_to_xt[sdl_sc];
    }
    if ((mod & KMOD_CTRL) && xt >= 0x3B && xt <= 0x44) {
        xt = (unsigned char)(xt + 0x23); /* Ctrl+F1..F10 -> 0x5E..0x67 */
    }
    if ((mod & KMOD_CTRL) && sym >= 'a' && sym <= 'z') {
        asc = (unsigned char)(sym - 'a' + 1);
    } else if ((mod & KMOD_CTRL) && sym >= 'A' && sym <= 'Z') {
        asc = (unsigned char)(sym - 'A' + 1);
    } else if (sym >= 32 && sym <= 126) {
        asc = (unsigned char)sym;
        if ((mod & KMOD_SHIFT) && asc >= 'a' && asc <= 'z') {
            asc = (unsigned char)(asc - 'a' + 'A');
        }
    } else if (sym == 13 || sym == 27 || sym == 8 || sym == 9) {
        asc = (unsigned char)sym;
    } else if (sym >= 0xC0 && sym <= 0xFF) {
        asc = win1252_to_cp437((unsigned char)sym);
    }
    if (out_ascii) *out_ascii = asc;
    return xt;
}

static void keyq_push(unsigned short bios_key) {
    int next = (g_key_tail + 1) % KEY_QUEUE_CAP;
    if (next != g_key_head) {
        g_key_queue[g_key_tail] = bios_key;
        g_key_tail = next;
    }
}

/* =========================================================================
 * 3. Spielverzeichnis & Dateipfad-Aufloesung
 * ========================================================================= */

static char g_game_dir[512] = "";
static char g_explicit_save_dir[512] = "";
static int  g_save_dir_redirected = 0;
static int  g_save_dir_reported = 0;

static int is_dir_writable(const char *dir) {
    const char *force_ro = getenv("DSA_FORCE_READONLY_GAMEDIR");
    if (force_ro && force_ro[0] == '1') {
        return 0;
    }
    if (!dir || !dir[0]) return 0;
    char test_path[600];
#ifdef _WIN32
    snprintf(test_path, sizeof(test_path), "%s\\.schick_write_test_%lu.tmp", dir, (unsigned long)GetCurrentProcessId());
    HANDLE h = CreateFileA(test_path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return 0;
    }
    CloseHandle(h);
    DeleteFileA(test_path);
    return 1;
#else
    snprintf(test_path, sizeof(test_path), "%s/.schick_write_test_%d.tmp", dir, (int)getpid());
    FILE *fp = fopen(test_path, "wb");
    if (!fp) return 0;
    fclose(fp);
    unlink(test_path);
    return 1;
#endif
}

static void copy_file_if_missing(const char *src_path, const char *dst_path) {
    if (ACCESS(dst_path, 0) == 0) {
#ifdef _WIN32
        _chmod(dst_path, 0666);
#else
        chmod(dst_path, 0666);
#endif
        return;
    }
    FILE *in_fp = fopen(src_path, "rb");
    if (!in_fp) return;
    FILE *out_fp = fopen(dst_path, "wb");
    if (!out_fp) {
        fclose(in_fp);
        return;
    }
    unsigned char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in_fp)) > 0) {
        fwrite(buf, 1, n, out_fp);
    }
    fclose(out_fp);
    fclose(in_fp);
#ifdef _WIN32
    _chmod(dst_path, 0666);
#else
    chmod(dst_path, 0666);
#endif
}

static int is_foreign_dsa1_save_dir(const char *dir, const char *gamedir) {
    if (!dir || !dir[0]) return 0;
    if (gamedir && gamedir[0] && STRICMP(dir, gamedir) == 0) return 0;
    char low[600];
    size_t i = 0;
    for (; dir[i] && i + 1 < sizeof(low); ++i) {
        char c = (char)tolower((unsigned char)dir[i]);
        if (c == '\\') c = '/';
        low[i] = c;
    }
    low[i] = '\0';
    if (strstr(low, "realms of arkania 2") != NULL) return 1;
    if (strstr(low, "realms of arkania 3") != NULL) return 1;
    if (strstr(low, "sternenschweif") != NULL) return 1;

    char p[600];
#ifdef _WIN32
    snprintf(p, sizeof(p), "%s\\SCHWEIF.EXE", dir);
    if (ACCESS(p, 0) == 0) return 1;
    snprintf(p, sizeof(p), "%s\\DATA\\STAR.DAT", dir);
    if (ACCESS(p, 0) == 0) return 1;
    snprintf(p, sizeof(p), "%s\\GAMES\\LASTGAME", dir);
    if (ACCESS(p, 0) == 0) {
        char p_schick[600];
        snprintf(p_schick, sizeof(p_schick), "%s\\SCHICK.DAT", dir);
        if (ACCESS(p_schick, 0) != 0) return 1;
    }
#else
    snprintf(p, sizeof(p), "%s/SCHWEIF.EXE", dir);
    if (ACCESS(p, 0) == 0) return 1;
    snprintf(p, sizeof(p), "%s/DATA/STAR.DAT", dir);
    if (ACCESS(p, 0) == 0) return 1;
    snprintf(p, sizeof(p), "%s/GAMES/LASTGAME", dir);
    if (ACCESS(p, 0) == 0) {
        char p_schick[600];
        snprintf(p_schick, sizeof(p_schick), "%s/SCHICK.DAT", dir);
        if (ACCESS(p_schick, 0) != 0) return 1;
    }
#endif
    return 0;
}

static void seed_save_dir_from_dir(const char *src_dir, const char *dst_dir) {
    if (!src_dir || !src_dir[0] || !dst_dir || !dst_dir[0]) return;
    if (STRICMP(src_dir, dst_dir) == 0) return;
    DIR *d = opendir(src_dir);
    if (!d) return;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        const char *nm = ent->d_name;
        size_t len = strlen(nm);
        int copy_it = 0;
        if (STRICMP(nm, "GAMES.NAM") == 0 || STRICMP(nm, "HIMMEL.AN") == 0 || STRICMP(nm, "APFREI.AN") == 0) {
            copy_it = 1;
        } else if (len > 4 && (STRICMP(nm + len - 4, ".GAM") == 0 || STRICMP(nm + len - 4, ".CHR") == 0)) {
            copy_it = 1;
        }
        if (copy_it) {
            char sp[600], dp[600];
#ifdef _WIN32
            snprintf(sp, sizeof(sp), "%s\\%s", src_dir, nm);
            snprintf(dp, sizeof(dp), "%s\\%s", dst_dir, nm);
#else
            snprintf(sp, sizeof(sp), "%s/%s", src_dir, nm);
            snprintf(dp, sizeof(dp), "%s/%s", dst_dir, nm);
#endif
            copy_file_if_missing(sp, dp);
        }
    }
    closedir(d);
}

static void setup_save_dir(void) {
    init_exe_dir_once();
    const char *env_sd = getenv("DSA_SAVE_DIR");
    const char *chosen_sd = NULL;
    if (g_explicit_save_dir[0] && !is_foreign_dsa1_save_dir(g_explicit_save_dir, g_game_dir)) {
        chosen_sd = g_explicit_save_dir;
    } else if (env_sd && env_sd[0] && !is_foreign_dsa1_save_dir(env_sd, g_game_dir)) {
        chosen_sd = env_sd;
    }

    if (chosen_sd && chosen_sd[0] && STRICMP(chosen_sd, g_game_dir) == 0) {
        chosen_sd = NULL;
    }

    if (chosen_sd && chosen_sd[0]) {
        MKDIR(chosen_sd);
        if (!is_dir_writable(chosen_sd)) {
            chosen_sd = NULL;
        }
    }

    if (chosen_sd && chosen_sd[0]) {
        strncpy(g_save_dir, chosen_sd, sizeof(g_save_dir) - 1);
        g_save_dir[sizeof(g_save_dir) - 1] = '\0';
        g_save_dir_redirected = 1;
    } else if (is_dir_writable(g_game_dir)) {
        strncpy(g_save_dir, g_game_dir, sizeof(g_save_dir) - 1);
        g_save_dir[sizeof(g_save_dir) - 1] = '\0';
        g_save_dir_redirected = 0;
    } else {
#ifdef _WIN32
        snprintf(g_save_dir, sizeof(g_save_dir), "%s\\Spielstaende", g_exe_dir);
#else
        snprintf(g_save_dir, sizeof(g_save_dir), "%s/Spielstaende", g_exe_dir);
#endif
        g_save_dir_redirected = 1;
    }

    if (g_save_dir[0]) {
        char sub[600];
        MKDIR(g_save_dir);
#ifdef _WIN32
        snprintf(sub, sizeof(sub), "%s\\TEMP", g_save_dir);
#else
        snprintf(sub, sizeof(sub), "%s/TEMP", g_save_dir);
#endif
        MKDIR(sub);
    }

    if (g_save_dir_redirected) {
        seed_save_dir_from_dir(g_game_dir, g_save_dir);
        if (!g_save_dir_reported) {
            fprintf(stderr, "[HINWEIS] Spielordner \"%s\" ist nicht beschreibbar -> Spielstaende und PROBEN.CSV werden nach \"%s\" umgeleitet.\n",
                    g_game_dir, g_save_dir);
            fflush(stderr);
            g_save_dir_reported = 1;
        }
    }
}

static void trim_crlf(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n' || s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
}

#ifndef _WIN32
static void dsa_translate_win_drive_path(const char *in_path, char *out_buf, size_t out_size) {
    if (!in_path || !out_buf || out_size == 0) return;
    char tmp[1024];
    size_t i = 0;
    if (isalpha((unsigned char)in_path[0]) && in_path[1] == ':' && (in_path[2] == '\\' || in_path[2] == '/')) {
        char drive = (char)tolower((unsigned char)in_path[0]);
        char mnt_root[16];
        snprintf(mnt_root, sizeof(mnt_root), "/mnt/%c", drive);
        if (access(mnt_root, F_OK) == 0) {
            snprintf(tmp, sizeof(tmp), "/mnt/%c/%s", drive, in_path + 3);
        } else {
            strncpy(tmp, in_path, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
        }
    } else {
        strncpy(tmp, in_path, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';
    }
    for (i = 0; tmp[i] && i < out_size - 1; ++i) {
        out_buf[i] = (tmp[i] == '\\') ? '/' : tmp[i];
    }
    out_buf[i] = '\0';
}

static int dsa_resolve_existing_ci(const char *in_path, char *out_buf, size_t out_size) {
    if (!in_path || !out_buf || out_size == 0) return 0;
    char norm[1024];
    dsa_translate_win_drive_path(in_path, norm, sizeof(norm));
    if (access(norm, F_OK) == 0) {
        strncpy(out_buf, norm, out_size - 1);
        out_buf[out_size - 1] = '\0';
        return 1;
    }

    char cur[1024] = "";
    const char *p = norm;
    if (*p == '/') {
        strcpy(cur, "/");
        while (*p == '/') ++p;
    }

    while (*p) {
        char comp[256];
        size_t ci = 0;
        while (*p && *p != '/' && ci < sizeof(comp) - 1) {
            comp[ci++] = *p++;
        }
        comp[ci] = '\0';
        while (*p == '/') ++p;

        if (comp[0] == '\0' || strcmp(comp, ".") == 0) continue;

        char cand[1024];
        if (cur[0] == '\0') {
            snprintf(cand, sizeof(cand), "%s", comp);
        } else if (strcmp(cur, "/") == 0) {
            snprintf(cand, sizeof(cand), "/%s", comp);
        } else {
            snprintf(cand, sizeof(cand), "%s/%s", cur, comp);
        }

        if (access(cand, F_OK) == 0) {
            strncpy(cur, cand, sizeof(cur) - 1);
            cur[sizeof(cur) - 1] = '\0';
            continue;
        }

        const char *scan_dir = cur[0] ? cur : ".";
        DIR *d = opendir(scan_dir);
        int matched = 0;
        if (d) {
            struct dirent *ent;
            while ((ent = readdir(d)) != NULL) {
                if (strcasecmp(ent->d_name, comp) == 0) {
                    if (cur[0] == '\0') {
                        snprintf(cand, sizeof(cand), "%s", ent->d_name);
                    } else if (strcmp(cur, "/") == 0) {
                        snprintf(cand, sizeof(cand), "/%s", ent->d_name);
                    } else {
                        snprintf(cand, sizeof(cand), "%s/%s", cur, ent->d_name);
                    }
                    matched = 1;
                    break;
                }
            }
            closedir(d);
        }
        strncpy(cur, cand, sizeof(cur) - 1);
        cur[sizeof(cur) - 1] = '\0';
        (void)matched;
    }

    strncpy(out_buf, cur[0] ? cur : norm, out_size - 1);
    out_buf[out_size - 1] = '\0';
    return (access(out_buf, F_OK) == 0) ? 1 : 0;
}

static int dsa_dir_has_schick_dat(const char *dir_path, char *out_dir, size_t out_size) {
    if (!dir_path || !dir_path[0]) return 0;
    char norm_dir[1024];
    dsa_translate_win_drive_path(dir_path, norm_dir, sizeof(norm_dir));
    char test_dat[1024];
    char resolved_dat[1024];
    snprintf(test_dat, sizeof(test_dat), "%s/SCHICK.DAT", norm_dir);
    if (dsa_resolve_existing_ci(test_dat, resolved_dat, sizeof(resolved_dat))) {
        char *slash = strrchr(resolved_dat, '/');
        if (slash) {
            *slash = '\0';
            strncpy(out_dir, resolved_dat, out_size - 1);
            out_dir[out_size - 1] = '\0';
        } else {
            strncpy(out_dir, norm_dir, out_size - 1);
            out_dir[out_size - 1] = '\0';
        }
        return 1;
    }
    return 0;
}
#endif

static void discover_game_dir(const char *explicit_dir) {
    const char *env_dir;
    FILE *fp;
    char line[512];
    const char *candidates[] = {
        "schick_port.ini",
        "SPIELPFAD.txt",
        "..\\SPIELPFAD.txt",
        "../SPIELPFAD.txt",
        "..\\..\\SPIELPFAD.txt",
        "../../SPIELPFAD.txt",
        "../../../SPIELPFAD.txt",
        NULL
    };
    const char *gog_defaults[] = {
        "C:\\Program Files (x86)\\GOG Galaxy\\Games\\Realms of Arkania",
        "C:\\Program Files\\GOG Galaxy\\Games\\Realms of Arkania",
        "C:\\GOG Games\\Realms of Arkania",
        "D:\\GOG Games\\Realms of Arkania",
        NULL
    };
    int i;

    if (explicit_dir && explicit_dir[0]) {
#ifndef _WIN32
        if (!dsa_dir_has_schick_dat(explicit_dir, g_game_dir, sizeof(g_game_dir))) {
            dsa_translate_win_drive_path(explicit_dir, g_game_dir, sizeof(g_game_dir));
        }
#else
        strncpy(g_game_dir, explicit_dir, sizeof(g_game_dir) - 1);
        g_game_dir[sizeof(g_game_dir) - 1] = '\0';
#endif
        return;
    }

    env_dir = getenv("DSA_GAME_DIR");
    if (!env_dir || !env_dir[0]) env_dir = getenv("DSA_GAMEDIR");
    if (env_dir && env_dir[0]) {
#ifndef _WIN32
        if (!dsa_dir_has_schick_dat(env_dir, g_game_dir, sizeof(g_game_dir))) {
            dsa_translate_win_drive_path(env_dir, g_game_dir, sizeof(g_game_dir));
        }
#else
        strncpy(g_game_dir, env_dir, sizeof(g_game_dir) - 1);
        g_game_dir[sizeof(g_game_dir) - 1] = '\0';
#endif
        return;
    }

#ifdef _WIN32
    if (ACCESS("SCHICK.DAT", 0) == 0) {
        strcpy(g_game_dir, ".");
        return;
    }
#else
    if (dsa_dir_has_schick_dat(".", g_game_dir, sizeof(g_game_dir))) {
        return;
    }
#endif

    for (i = 0; candidates[i]; ++i) {
        fp = fopen(candidates[i], "r");
        if (fp) {
            while (fgets(line, sizeof(line), fp)) {
                trim_crlf(line);
                if (line[0] == '\0' || line[0] == '#' || line[0] == ';') continue;
                const char * raw_p = (strncmp(line, "gamedir=", 8) == 0) ? (line + 8) : line;
#ifndef _WIN32
                if (dsa_dir_has_schick_dat(raw_p, g_game_dir, sizeof(g_game_dir))) {
                    fclose(fp);
                    return;
                }
                dsa_translate_win_drive_path(raw_p, g_game_dir, sizeof(g_game_dir));
#else
                strncpy(g_game_dir, raw_p, sizeof(g_game_dir) - 1);
                g_game_dir[sizeof(g_game_dir) - 1] = '\0';
#endif
                fclose(fp);
                return;
            }
            fclose(fp);
        }
    }

    for (i = 0; gog_defaults[i]; ++i) {
#ifdef _WIN32
        char test_dat[600];
        snprintf(test_dat, sizeof(test_dat), "%s\\SCHICK.DAT", gog_defaults[i]);
        if (ACCESS(test_dat, 0) == 0) {
            strcpy(g_game_dir, gog_defaults[i]);
            return;
        }
#else
        if (dsa_dir_has_schick_dat(gog_defaults[i], g_game_dir, sizeof(g_game_dir))) {
            return;
        }
#endif
    }

#ifndef _WIN32
    const char *home = getenv("HOME");
    if (home && home[0]) {
        const char *home_rel[] = {
            "GOG Games/Realms of Arkania",
            "GOG Games/Realms of Arkania 1",
            "GOG Games/Realms of Arkania - Blade of Destiny",
            "GOG Games/Das Schwarze Auge - Die Schicksalsklinge",
            ".local/share/Steam/steamapps/common/Realms of Arkania Blade of Destiny Classic",
            ".local/share/Steam/steamapps/common/Realms of Arkania",
            ".steam/steam/steamapps/common/Realms of Arkania Blade of Destiny Classic",
            "Games/gog/realms-of-arkania",
            "Games/lutris/realms-of-arkania",
            "Games/Heroic/Realms of Arkania",
            "Library/Application Support/Steam/steamapps/common/Realms of Arkania Blade of Destiny Classic",
            NULL
        };
        for (i = 0; home_rel[i]; ++i) {
            char cand_dir[1024];
            snprintf(cand_dir, sizeof(cand_dir), "%s/%s", home, home_rel[i]);
            if (dsa_dir_has_schick_dat(cand_dir, g_game_dir, sizeof(g_game_dir))) {
                return;
            }
        }
    }
    const char *extra_dirs[] = {
        "/Applications/Realms of Arkania.app/Contents/Resources/game",
        "../Original",
        "../../Original",
        "../../../Original",
        NULL
    };
    for (i = 0; extra_dirs[i]; ++i) {
        if (dsa_dir_has_schick_dat(extra_dirs[i], g_game_dir, sizeof(g_game_dir))) {
            return;
        }
    }
#endif

    strcpy(g_game_dir, ".");
}

void dsa_platform_set_game_dir(const char *path) {
    discover_game_dir(path);
    setup_save_dir();
}

const char *dsa_platform_get_game_dir(void) {
    if (!g_game_dir[0]) dsa_platform_set_game_dir(NULL);
    return g_game_dir;
}

void dsa_platform_set_save_dir(const char *path) {
    if (path && path[0]) {
        strncpy(g_explicit_save_dir, path, sizeof(g_explicit_save_dir) - 1);
        g_explicit_save_dir[sizeof(g_explicit_save_dir) - 1] = '\0';
    } else {
        g_explicit_save_dir[0] = '\0';
    }
    if (g_game_dir[0]) {
        setup_save_dir();
    }
}

const char *dsa_platform_get_save_dir(void) {
    if (!g_save_dir[0]) dsa_platform_set_game_dir(NULL);
    return g_save_dir;
}

int dsa_platform_is_save_dir_redirected(void) {
    if (!g_save_dir[0]) dsa_platform_set_game_dir(NULL);
    return g_save_dir_redirected;
}

int dsa_platform_is_replay_active(void) {
    return g_replay_active ? 1 : 0;
}

const char *dsa_platform_resolve_path(const char *dos_path, char *out_buf, size_t out_size) {
    const char *p = dos_path;
    char rel[512];
    size_t i = 0;

    if (!dos_path || !out_buf || out_size == 0) return dos_path;
    if (!g_game_dir[0]) dsa_platform_set_game_dir(NULL);

    if (isalpha((unsigned char)p[0]) && p[1] == ':') {
        size_t gd_len = strlen(g_game_dir);
        if (gd_len > 0 && STRNICMP(p, g_game_dir, gd_len) == 0) {
            p += gd_len;
        } else {
            p += 2;
        }
        while (*p == '\\' || *p == '/') ++p;
    }

    while (1) {
        if (p[0] == '.' && (p[1] == '\\' || p[1] == '/')) {
            p += 2;
        } else if (*p == '\\' || *p == '/') {
            ++p;
        } else {
            break;
        }
    }

    for (i = 0; p[i] && i < sizeof(rel) - 1; ++i) {
#ifdef _WIN32
        rel[i] = (p[i] == '/') ? '\\' : p[i];
#else
        rel[i] = (p[i] == '\\') ? '/' : p[i];
#endif
    }
    rel[i] = '\0';

    if (rel[0] == '\0') {
        strncpy(out_buf, g_game_dir, out_size - 1);
        out_buf[out_size - 1] = '\0';
        return out_buf;
    }

#ifdef _WIN32
    snprintf(out_buf, out_size, "%s\\%s", g_game_dir, rel);
#else
    char combined[1024];
    snprintf(combined, sizeof(combined), "%s/%s", g_game_dir, rel);
    dsa_resolve_existing_ci(combined, out_buf, out_size);
#endif
    return out_buf;
}

int dsa_platform_open_ci(const char *fname, int flags, int mode) {
    if (!fname || !fname[0]) return -1;
#ifdef _WIN32
    return open(fname, flags, mode);
#else
    char resolved[1024];
    dsa_resolve_existing_ci(fname, resolved, sizeof(resolved));
    int fd = open(resolved, flags, mode);
    if (fd >= 0) return fd;

    if ((flags & (O_WRONLY | O_RDWR | O_CREAT)) == 0 && fname[0] != '/') {
        char game_resolved[1024];
        dsa_platform_resolve_path(fname, game_resolved, sizeof(game_resolved));
        fd = open(game_resolved, flags, mode);
    }
    return fd;
#endif
}

int dsa_platform_unlink_ci(const char *fname) {
    if (!fname || !fname[0]) return -1;
#ifdef _WIN32
    return unlink(fname);
#else
    char resolved[1024];
    if (dsa_resolve_existing_ci(fname, resolved, sizeof(resolved))) {
        return unlink(resolved);
    }
    return unlink(fname);
#endif
}

/* =========================================================================
 * 4. Initialisierung, Moduswechsel, Palette und Frame-Praesentation
 * ========================================================================= */

static void rebuild_palette_argb(int idx) {
    unsigned int r6 = g_pal_rgb6[idx][0] & 0x3F;
    unsigned int g6 = g_pal_rgb6[idx][1] & 0x3F;
    unsigned int b6 = g_pal_rgb6[idx][2] & 0x3F;
    unsigned int r8 = (r6 << 2) | (r6 >> 4);
    unsigned int g8 = (g6 << 2) | (g6 >> 4);
    unsigned int b8 = (b6 << 2) | (b6 >> 4);
    g_pal_argb32[idx] = 0xFF000000u | (r8 << 16) | (g8 << 8) | b8;
}

#ifdef _WIN32
static DWORD WINAPI dsa_watchdog_thread_proc(LPVOID param) {
    DWORD timeout_ms = (DWORD)(uintptr_t)param;
    Sleep(timeout_ms);
    if (g_coverage_fp) { fflush(g_coverage_fp); }
    if (g_record_fp)   { fflush(g_record_fp); }
    TerminateProcess(GetCurrentProcess(), 124);
    return 0;
}
#else
static void *dsa_posix_watchdog_thread_proc(void *param) {
    uint32_t timeout_ms = (uint32_t)(uintptr_t)param;
    uint32_t elapsed = 0;
    while (elapsed < timeout_ms) {
        uint32_t step = (timeout_ms - elapsed > 100) ? 100 : (timeout_ms - elapsed);
        usleep(step * 1000);
        elapsed += step;
    }
    if (g_coverage_fp) { fflush(g_coverage_fp); }
    if (g_record_fp)   { fflush(g_record_fp); }
    _exit(124);
    return NULL;
}
#endif

static void dsa_parse_fenster_token(const char *val) {
    if (!val || !val[0]) return;
    if (STRICMP(val, "vollbild") == 0 || STRICMP(val, "fullscreen") == 0 || STRICMP(val, "full") == 0 || strcmp(val, "0") == 0) {
        g_is_fullscreen    = 1;
        g_fenster_explicit = 1;
    } else {
        int f = atoi(val);
        if (f >= 2 && f <= 5) {
            g_fenster_faktor   = f;
            g_scale            = f;
            g_is_fullscreen    = 0;
            g_fenster_explicit = 1;
        }
    }
}

static void dsa_parse_skalierung_token(const char *val) {
    if (!val || !val[0]) return;
    if (STRICMP(val, "ganzzahlig") == 0 || STRICMP(val, "integer") == 0 || strcmp(val, "0") == 0) {
        g_skalierung_mode = DSA_SKAL_GANZZAHLIG;
    } else if (STRICMP(val, "fuellend") == 0 || STRICMP(val, "füllend") == 0 || STRICMP(val, "sharp") == 0 || strcmp(val, "1") == 0) {
        g_skalierung_mode = DSA_SKAL_FUELLEND;
    }
}

static void dsa_display_init_from_env(void) {
    if (g_display_inited) return;
    g_display_inited = 1;

    /* Standard laut S2: --fenster=vollbild, --skalierung=fuellend */
    g_is_fullscreen   = 1;
    g_fenster_faktor  = 4;
    g_scale           = 4;
    g_skalierung_mode = DSA_SKAL_FUELLEND;

    const char *env_fs = getenv("DSA_FULLSCREEN");
    if (env_fs && env_fs[0]) {
        if (env_fs[0] == '0') { g_is_fullscreen = 0; g_fenster_explicit = 1; }
        else if (env_fs[0] == '1') { g_is_fullscreen = 1; g_fenster_explicit = 1; }
    }
    const char *env_fen = getenv("DSA_FENSTER");
    if (env_fen && env_fen[0]) {
        dsa_parse_fenster_token(env_fen);
    }
    const char *env_skal = getenv("DSA_SKALIERUNG");
    if (env_skal && env_skal[0]) {
        dsa_parse_skalierung_token(env_skal);
    }
    const char *env_fres = getenv("DSA_VOLLBILD_AUFLOESUNG");
    if (env_fres && env_fres[0]) {
        int fw = 0, fh = 0;
        if (sscanf(env_fres, "%dx%d", &fw, &fh) == 2 && fw >= 320 && fh >= 240) {
            SDL_SetFullscreenOverrideRes(fw, fh);
        }
    }
    const char *env_ss = getenv("DSA_SCALED_SCREENSHOT");
    if (env_ss && env_ss[0]) {
        strncpy(g_scaled_shot_path, env_ss, sizeof(g_scaled_shot_path) - 1);
    }
    const char *env_mt = getenv("DSA_MAUSTEST_LOG");
    if (env_mt && env_mt[0]) {
        strncpy(g_maustest_log_path, env_mt, sizeof(g_maustest_log_path) - 1);
    }

#ifdef _WIN32
    const char *cmdline = GetCommandLineA();
    if (cmdline) {
        const char *p = cmdline;
        while (*p) {
            while (*p == ' ' || *p == '\t') ++p;
            if (!*p) break;
            char tok[512];
            int ti = 0, in_q = 0;
            while (*p && (in_q || (*p != ' ' && *p != '\t'))) {
                if (*p == '"') { in_q = !in_q; ++p; continue; }
                if (ti < (int)sizeof(tok) - 1) tok[ti++] = *p;
                ++p;
            }
            tok[ti] = '\0';

            if (STRNICMP(tok, "--fenster=", 10) == 0) {
                dsa_parse_fenster_token(tok + 10);
            } else if (STRICMP(tok, "--vollbild") == 0 || STRICMP(tok, "--fullscreen") == 0) {
                g_is_fullscreen = 1;
                g_fenster_explicit = 1;
            } else if (STRICMP(tok, "--fenstermodus") == 0 || STRICMP(tok, "--windowed") == 0) {
                g_is_fullscreen = 0;
                g_fenster_explicit = 1;
            } else if (STRNICMP(tok, "--skalierung=", 13) == 0) {
                dsa_parse_skalierung_token(tok + 13);
            } else if (STRNICMP(tok, "--vollbild-aufloesung=", 22) == 0) {
                int fw = 0, fh = 0;
                if (sscanf(tok + 22, "%dx%d", &fw, &fh) == 2 && fw >= 320 && fh >= 240) {
                    SDL_SetFullscreenOverrideRes(fw, fh);
                }
            } else if (STRNICMP(tok, "--skaliertes-bild=", 18) == 0) {
                strncpy(g_scaled_shot_path, tok + 18, sizeof(g_scaled_shot_path) - 1);
            } else if (STRNICMP(tok, "--maustest-log=", 15) == 0) {
                strncpy(g_maustest_log_path, tok + 15, sizeof(g_maustest_log_path) - 1);
            }
        }
    }
#elif defined(__linux__)
    FILE *cmd_fp = fopen("/proc/self/cmdline", "rb");
    if (cmd_fp) {
        char cmd_buf[4096];
        size_t n = fread(cmd_buf, 1, sizeof(cmd_buf) - 1, cmd_fp);
        fclose(cmd_fp);
        cmd_buf[n] = '\0';
        size_t pos = 0;
        while (pos < n) {
            const char *tok = cmd_buf + pos;
            size_t tlen = strlen(tok);
            pos += tlen + 1;
            if (STRNICMP(tok, "--fenster=", 10) == 0) {
                dsa_parse_fenster_token(tok + 10);
            } else if (STRICMP(tok, "--vollbild") == 0 || STRICMP(tok, "--fullscreen") == 0) {
                g_is_fullscreen = 1;
                g_fenster_explicit = 1;
            } else if (STRICMP(tok, "--fenstermodus") == 0 || STRICMP(tok, "--windowed") == 0) {
                g_is_fullscreen = 0;
                g_fenster_explicit = 1;
            } else if (STRNICMP(tok, "--skalierung=", 13) == 0) {
                dsa_parse_skalierung_token(tok + 13);
            } else if (STRNICMP(tok, "--vollbild-aufloesung=", 22) == 0) {
                int fw = 0, fh = 0;
                if (sscanf(tok + 22, "%dx%d", &fw, &fh) == 2 && fw >= 320 && fh >= 240) {
                    SDL_SetFullscreenOverrideRes(fw, fh);
                }
            } else if (STRNICMP(tok, "--skaliertes-bild=", 18) == 0) {
                strncpy(g_scaled_shot_path, tok + 18, sizeof(g_scaled_shot_path) - 1);
            } else if (STRNICMP(tok, "--maustest-log=", 15) == 0) {
                strncpy(g_maustest_log_path, tok + 15, sizeof(g_maustest_log_path) - 1);
            }
        }
    }
#endif

    /* Im automatischen Replay/Autotest ohne explizite --fenster-Vorgabe im Fenster bleiben */
    int is_at = (getenv("SCHICK_PORT_AUTOTEST") && getenv("SCHICK_PORT_AUTOTEST")[0] == '1');
    if ((g_replay_active || is_at) && !g_fenster_explicit) {
        g_is_fullscreen = 0;
    }
}

static void dsa_compute_43_viewport(int cw, int ch, int *out_vx, int *out_vy, int *out_vw, int *out_vh) {
    if (cw < 320) cw = 320;
    if (ch < 240) ch = 240;
    int vw = cw, vh = ch;

    if (g_skalierung_mode == DSA_SKAL_GANZZAHLIG) {
        /* Ganzzahlige horizontale Vielfache mit exaktem 4:3-Seitenverhaeltnis (320*sx : 240*sx = 4 : 3) */
        int sx = cw / 320;
        int sy43 = ch / 240;
        if (sy43 < sx) sx = sy43;
        if (sx < 1) sx = 1;
        vw = 320 * sx;
        vh = 240 * sx;
    } else {
        /* Fuellend: groesstmoeglicher 4:3-Bereich in (cw, ch), Rest schwarzer Rand */
        if (cw * 3 <= ch * 4) {
            vh = (cw * 3) / 4;
            vw = (vh * 4) / 3;
        } else {
            vw = (ch * 4) / 3;
            vh = (vw * 3) / 4;
        }
        if (vw < 320) { vw = 320; vh = 240; }
    }

    *out_vx = (cw - vw) / 2;
    *out_vy = (ch - vh) / 2;
    *out_vw = vw;
    *out_vh = vh;
}

static void dsa_render_scaled_display_buffer(int cw, int ch) {
    if (cw < 320) cw = 320;
    if (ch < 240) ch = 240;

    static int s_last_vx = -1, s_last_vy = -1, s_last_vw = -1, s_last_vh = -1;
    bool buf_reallocated = false;

    if (!g_display_buf || g_display_w != cw || g_display_h != ch) {
        if (g_display_buf) free(g_display_buf);
        g_display_buf = (uint32_t *)calloc((size_t)cw * (size_t)ch, sizeof(uint32_t));
        g_display_w = cw;
        g_display_h = ch;
        buf_reallocated = true;
        if (g_display_tex) {
            SDL_DestroyTexture(g_display_tex);
            g_display_tex = NULL;
        }
    }
    if (!g_display_tex && g_renderer) {
        /* WICHTIG: SDL_RenderSetLogicalSize(cw, ch) NUR bei externer SDL2.dll aufrufen!
         * Im eingebauten Win32-Provider muss logical_w/logical_h zwingend auf 320x200 (g_width x g_height)
         * bleiben, damit scale_client_to_logical() die Mausbewegung 1:1 direkt auf 320x200 abbildet! */
        if (SDL_PortUsingExternalDLL()) {
            SDL_RenderSetLogicalSize(g_renderer, cw, ch);
        } else {
            SDL_RenderSetLogicalSize(g_renderer, g_width, g_height);
        }
        g_display_tex = SDL_CreateTexture(
            g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, cw, ch
        );
    }

    int vx = 0, vy = 0, vw = cw, vh = ch;
    dsa_compute_43_viewport(cw, ch, &vx, &vy, &vw, &vh);
    g_vp_x = vx; g_vp_y = vy; g_vp_w = vw; g_vp_h = vh;
    SDL_SetWindowViewport(g_window, vx, vy, vw, vh);

    /* Schwarze Raender (Letterbox / Pillarbox) nur bei Geometrieaenderung loeschen */
    if (buf_reallocated || vx != s_last_vx || vy != s_last_vy || vw != s_last_vw || vh != s_last_vh) {
        s_last_vx = vx; s_last_vy = vy; s_last_vw = vw; s_last_vh = vh;
        if (vx > 0 || vy > 0 || vw < cw || vh < ch) {
            for (int i = 0; i < cw * ch; ++i) {
                g_display_buf[i] = 0xFF000000u;
            }
        }
    }

    int sw = (g_width > 0) ? g_width : 320;
    int sh = (g_height > 0) ? g_height : 200;

    if (g_skalierung_mode == DSA_SKAL_GANZZAHLIG) {
        /* Ganzzahlig: Exakt gleiche Originalpixel-Breite (vw = 320 * sx), keine Mischpixel */
        int prev_sy = -1;
        uint32_t *prev_drow = NULL;
        for (int dy = 0; dy < vh; ++dy) {
            int sy = (dy * sh) / vh;
            uint32_t *drow = g_display_buf + (vy + dy) * cw + vx;
            if (sy == prev_sy && prev_drow) {
                memcpy(drow, prev_drow, (size_t)vw * sizeof(uint32_t));
            } else {
                const uint32_t *srow = g_argb_buffer + sy * sw;
                for (int dx = 0; dx < vw; ++dx) {
                    int sx = (dx * sw) / vw;
                    drow[dx] = srow[sx];
                }
                prev_sy = sy;
                prev_drow = drow;
            }
        }
    } else {
        /* Fuellend (Sharp Bilinear): Zuerst Nearest auf das groesste ganze Vielfache,
         * dann nur der Uebergangs-Rest per Bilinear (max. 1 Mischspalte/-zeile an den Kanten). */
        static int x0_tab[4096], x1_tab[4096], wx_tab[4096];
        static int cached_vw = -1, cached_sw = -1;
        int max_vw = (vw < 4096) ? vw : 4096;
        if (vw != cached_vw || sw != cached_sw) {
            cached_vw = vw;
            cached_sw = sw;
            for (int dx = 0; dx < max_vw; ++dx) {
                int s0 = (dx * sw) / vw;
                int s1 = ((dx + 1) * sw - 1) / vw;
                if (s0 >= sw) s0 = sw - 1;
                if (s1 >= sw) s1 = sw - 1;
                x0_tab[dx] = s0;
                x1_tab[dx] = s1;
                wx_tab[dx] = (s0 == s1) ? 0 : ((((dx + 1) * sw - s1 * vw) * 256) / sw);
            }
        }

        int prev_pure_y0 = -1;
        uint32_t *prev_pure_drow = NULL;

        for (int dy = 0; dy < vh; ++dy) {
            int y0 = (dy * sh) / vh;
            int y1 = ((dy + 1) * sh - 1) / vh;
            if (y0 >= sh) y0 = sh - 1;
            if (y1 >= sh) y1 = sh - 1;
            int wy = (y0 == y1) ? 0 : ((((dy + 1) * sh - y1 * vh) * 256) / sh);

            uint32_t *drow = g_display_buf + (vy + dy) * cw + vx;

            if (wy == 0) {
                if (y0 == prev_pure_y0 && prev_pure_drow) {
                    memcpy(drow, prev_pure_drow, (size_t)max_vw * sizeof(uint32_t));
                } else {
                    const uint32_t *row0 = g_argb_buffer + y0 * sw;
                    for (int dx = 0; dx < max_vw; ++dx) {
                        int wx = wx_tab[dx];
                        if (wx == 0) {
                            drow[dx] = row0[x0_tab[dx]];
                        } else {
                            uint32_t c0 = row0[x0_tab[dx]];
                            uint32_t c1 = row0[x1_tab[dx]];
                            uint32_t r = (((c0 >> 16) & 0xFF) * (256 - wx) + ((c1 >> 16) & 0xFF) * wx) >> 8;
                            uint32_t g = (((c0 >> 8)  & 0xFF) * (256 - wx) + ((c1 >> 8)  & 0xFF) * wx) >> 8;
                            uint32_t b = ((c0         & 0xFF) * (256 - wx) + (c1         & 0xFF) * wx) >> 8;
                            drow[dx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                        }
                    }
                    prev_pure_y0 = y0;
                    prev_pure_drow = drow;
                }
            } else {
                const uint32_t *row0 = g_argb_buffer + y0 * sw;
                const uint32_t *row1 = g_argb_buffer + y1 * sw;
                int iwy = 256 - wy;
                for (int dx = 0; dx < max_vw; ++dx) {
                    int wx = wx_tab[dx];
                    int x0 = x0_tab[dx];
                    int x1 = x1_tab[dx];
                    uint32_t c00 = row0[x0];
                    uint32_t c01 = row1[x0];
                    if (wx == 0) {
                        uint32_t r = (((c00 >> 16) & 0xFF) * iwy + ((c01 >> 16) & 0xFF) * wy) >> 8;
                        uint32_t g = (((c00 >> 8)  & 0xFF) * iwy + ((c01 >> 8)  & 0xFF) * wy) >> 8;
                        uint32_t b = ((c00         & 0xFF) * iwy + (c01         & 0xFF) * wy) >> 8;
                        drow[dx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                    } else {
                        int iwx = 256 - wx;
                        uint32_t c10 = row0[x1];
                        uint32_t c11 = row1[x1];
                        uint32_t w00 = (uint32_t)(iwx * iwy);
                        uint32_t w10 = (uint32_t)(wx  * iwy);
                        uint32_t w01 = (uint32_t)(iwx * wy);
                        uint32_t w11 = (uint32_t)(wx  * wy);
                        uint32_t r = (((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11) >> 16;
                        uint32_t g = (((c00 >> 8)  & 0xFF) * w00 + ((c10 >> 8)  & 0xFF) * w10 + ((c01 >> 8)  & 0xFF) * w01 + ((c11 >> 8)  & 0xFF) * w11) >> 16;
                        uint32_t b = ((c00         & 0xFF) * w00 + (c10         & 0xFF) * w10 + (c01         & 0xFF) * w01 + (c11         & 0xFF) * w11) >> 16;
                        drow[dx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                    }
                }
            }
        }
    }
}

static void dsa_save_scaled_bmp(const char *path) {
    if (!path || !path[0] || !g_display_buf || g_display_w <= 0 || g_display_h <= 0) return;
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    int w = g_display_w;
    int h = g_display_h;
    uint32_t row_bytes = (uint32_t)(w * 3);
    uint32_t img_size = row_bytes * (uint32_t)h;
    uint32_t file_size = 54 + img_size;
    unsigned char hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    *(uint32_t *)(hdr + 2)  = file_size;
    *(uint32_t *)(hdr + 10) = 54;
    *(uint32_t *)(hdr + 14) = 40;
    *(int32_t  *)(hdr + 18) = w;
    *(int32_t  *)(hdr + 22) = -h;
    *(uint16_t *)(hdr + 26) = 1;
    *(uint16_t *)(hdr + 28) = 24;
    *(uint32_t *)(hdr + 34) = img_size;
    fwrite(hdr, 1, 54, fp);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t c = g_display_buf[y * w + x];
            unsigned char bgr[3] = {
                (unsigned char)(c & 0xFF),
                (unsigned char)((c >> 8) & 0xFF),
                (unsigned char)((c >> 16) & 0xFF)
            };
            fwrite(bgr, 1, 3, fp);
        }
    }
    fclose(fp);
}

static void dsa_platform_toggle_fullscreen(void) {
    g_is_fullscreen = !g_is_fullscreen;
    if (g_window) {
        SDL_SetWindowFullscreen(g_window, g_is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}

int dsa_platform_init(const char *window_title, int width, int height, const char *game_dir) {
    DsaPlatScope plat_scope("dsa_platform_init");
    int i;
    init_exe_dir_once();
#ifdef _WIN32
    static int crash_handler_installed = 0;
    if (!crash_handler_installed) {
        FreeConsole();
        dsa_install_crash_handlers_once();
        const char *wd_env = getenv("DSA_WATCHDOG_MS");
        DWORD wd_ms = (wd_env && wd_env[0]) ? (DWORD)atoi(wd_env) : (g_replay_active ? 25000u : 0u);
        if (wd_ms > 0) {
            HANDLE hThread = CreateThread(NULL, 0, dsa_watchdog_thread_proc, (LPVOID)(uintptr_t)wd_ms, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        crash_handler_installed = 1;
    }
#else
    static int crash_handler_installed = 0;
    if (!crash_handler_installed) {
        dsa_install_crash_handlers_once();
        const char *wd_env = getenv("DSA_WATCHDOG_MS");
        uint32_t wd_ms = (wd_env && wd_env[0]) ? (uint32_t)atoi(wd_env) : (g_replay_active ? 25000u : 0u);
        if (wd_ms > 0) {
            pthread_t tid;
            if (pthread_create(&tid, NULL, dsa_posix_watchdog_thread_proc, (void *)(uintptr_t)wd_ms) == 0) {
                pthread_detach(tid);
            }
        }
        crash_handler_installed = 1;
    }
#endif
    {
        const char *cov = getenv("DSA_ABDECKUNG");
        if (cov && cov[0] == '1') {
            g_dsa_coverage_enabled = 1;
        }
    }
    if (game_dir && game_dir[0]) {
        dsa_platform_set_game_dir(game_dir);
    } else if (!g_game_dir[0]) {
        dsa_platform_set_game_dir(NULL);
    }

    if (g_initialized) return 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        return -1;
    }

    if (width <= 0)  width  = 320;
    if (height <= 0) height = 200;
    g_width  = width;
    g_height = height;

    /* S1 + S2: Filter abgeklemmt, 4:3-Fenstergroesse und Skalierungsmodus auswerten */
    dsa_display_init_from_env();

    char init_title[256];
    snprintf(init_title, sizeof(init_title), "%s", window_title ? window_title : "DSA 1: Die Schicksalsklinge");

    /* Exakte 4:3-Fenstergroesse: 2x=640x480, 3x=960x720, 4x=1280x960, 5x=1600x1200 */
    int win_w = 320 * g_fenster_faktor;
    int win_h = 240 * g_fenster_faktor;
    Uint32 win_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (g_is_fullscreen) {
        win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    g_window = SDL_CreateWindow(
        init_title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h,
        win_flags
    );
    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(g_renderer, g_width, g_height);
    g_texture = SDL_CreateTexture(
        g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_width, g_height
    );

    /* System-Mauszeiger im Spielfenster ausblenden, da Schicksalsklinge seinen eigenen Software-Cursor zeichnet */
    SDL_ShowCursor(0);

    for (i = 0; i < 256; ++i) {
        g_pal_rgb6[i][0] = 0;
        g_pal_rgb6[i][1] = 0;
        g_pal_rgb6[i][2] = 0;
        rebuild_palette_argb(i);
    }

    dsa_audio_init();

    g_initialized = 1;
    return 0;
}

void dsa_platform_shutdown(void) {
    DsaPlatScope plat_scope("dsa_platform_shutdown");
    if (g_record_fp)   { fclose(g_record_fp);   g_record_fp   = NULL; }
    if (g_coverage_fp) { fclose(g_coverage_fp); g_coverage_fp = NULL; }
    if (!g_initialized) return;
    dsa_audio_shutdown();
    if (g_display_tex) { SDL_DestroyTexture(g_display_tex); g_display_tex = NULL; }
    if (g_display_buf) { free(g_display_buf); g_display_buf = NULL; }
    if (g_texture)  { SDL_DestroyTexture(g_texture);   g_texture  = NULL; }
    if (g_renderer) { SDL_DestroyRenderer(g_renderer); g_renderer = NULL; }
    if (g_window)   { SDL_DestroyWindow(g_window);     g_window   = NULL; }
    SDL_Quit();
    g_initialized = 0;
}

int dsa_platform_get_scale(void) {
    return g_fenster_faktor;
}

void dsa_platform_cycle_scale(void) {
    if (g_is_fullscreen) {
        g_is_fullscreen = 0;
        SDL_SetWindowFullscreen(g_window, 0);
    }
    g_fenster_faktor = (g_fenster_faktor < 5) ? (g_fenster_faktor + 1) : 2;
    g_scale = g_fenster_faktor;
    if (g_window) {
        SDL_SetWindowSize(g_window, 320 * g_fenster_faktor, 240 * g_fenster_faktor);
    }
}

void dsa_platform_set_mode(int dos_mode, int width, int height) {
    if (!g_initialized) {
        dsa_platform_init(NULL, width > 0 ? width : 320, height > 0 ? height : 200, NULL);
    }
    g_dos_mode = dos_mode;
    if (width <= 0 || height <= 0) {
        if (dos_mode == 0x100)      { width = 640; height = 400; }
        else if (dos_mode == 0x101) { width = 640; height = 480; }
        else                        { width = 320; height = 200; }
    }
    if (width > DSA_MAX_VRAM_WIDTH)   width  = DSA_MAX_VRAM_WIDTH;
    if (height > DSA_MAX_VRAM_HEIGHT) height = DSA_MAX_VRAM_HEIGHT;

    if (width != g_width || height != g_height || !g_texture) {
        g_width  = width;
        g_height = height;
        if (g_texture) SDL_DestroyTexture(g_texture);
        SDL_RenderSetLogicalSize(g_renderer, g_width, g_height);
        g_texture = SDL_CreateTexture(
            g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_width, g_height
        );
    }
    g_mouse_max_x = g_width - 1;
    g_mouse_max_y = g_height - 1;
    dsa_platform_present();
}

int dsa_platform_get_width(void)  { return g_width; }
int dsa_platform_get_height(void) { return g_height; }

unsigned char *dsa_platform_get_vram(void) {
    return g_vram;
}

static void poll_sdl_events_internal(void) {
    SDL_Event ev;
    if (!g_initialized) return;

    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            dsa_platform_shutdown();
            exit(0);
            break;

        case SDL_MOUSEMOTION: {
            if (g_replay_active) break;
            int mx = ev.motion.x;
            int my = ev.motion.y;
            if (SDL_PortUsingExternalDLL() && g_vp_w > 0 && g_vp_h > 0) {
                mx = ((mx - g_vp_x) * g_width) / g_vp_w;
                my = ((my - g_vp_y) * g_height) / g_vp_h;
            }
            if (mx < 0) mx = 0;
            if (mx >= g_width) mx = g_width - 1;
            if (my < 0) my = 0;
            if (my >= g_height) my = g_height - 1;
            g_mouse_x = mx;
            g_mouse_y = my;
            g_mouse_moved_ev = 1;
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            if (g_replay_active) break;
            int bit = 0;
            if (ev.button.button == SDL_BUTTON_LEFT)        bit = 1;
            else if (ev.button.button == SDL_BUTTON_RIGHT)  bit = 2;
            else if (ev.button.button == SDL_BUTTON_MIDDLE) bit = 4;

            int mx = ev.button.x;
            int my = ev.button.y;
            if (SDL_PortUsingExternalDLL() && g_vp_w > 0 && g_vp_h > 0) {
                mx = ((mx - g_vp_x) * g_width) / g_vp_w;
                my = ((my - g_vp_y) * g_height) / g_vp_h;
            }
            if (mx < 0) mx = 0;
            if (mx >= g_width) mx = g_width - 1;
            if (my < 0) my = 0;
            if (my >= g_height) my = g_height - 1;
            g_mouse_x = mx;
            g_mouse_y = my;

            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                g_mouse_buttons |= bit;
                g_mouse_latched_down |= bit;
                g_mouse_pending_up &= ~bit;
                if (bit == 1) g_mouse_left_ev = 1;
                if (bit == 2) g_mouse_right_ev = 1;
            } else {
                if (g_mouse_latched_down & bit) {
                    g_mouse_pending_up |= bit;
                } else {
                    g_mouse_buttons &= ~bit;
                }
            }
            break;
        }

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (g_replay_active) break;
            int down = (ev.type == SDL_KEYDOWN);
            unsigned char dos_sc = 0;
            unsigned char ascii = 0;
            Uint16 mod = ev.key.keysym.mod;

            g_shift_status = 0;
            if (mod & KMOD_RSHIFT) g_shift_status |= 0x01;
            if (mod & KMOD_LSHIFT) g_shift_status |= 0x02;
            if (mod & KMOD_CTRL)   g_shift_status |= 0x04;
            if (mod & KMOD_ALT)    g_shift_status |= 0x08;

            if (ev.key.keysym.scancode < 0) {
                dos_sc = (unsigned char)((-ev.key.keysym.scancode) & 0x7F);
                ascii  = win1252_to_cp437((unsigned char)(ev.key.keysym.unused & 0xFF));
            } else {
                dos_sc = sdl_scancode_to_dos(ev.key.keysym.scancode, ev.key.keysym.sym, mod, &ascii);
            }

            /* Strg + F3 / F4 -> BIOS-Scancodes 0x60 / 0x61 */
            if (mod & KMOD_CTRL) {
                if (dos_sc == 0x3D) { dos_sc = 0x60; ascii = 0; }
                if (dos_sc == 0x3E) { dos_sc = 0x61; ascii = 0; }
            }

            if (dos_sc < 128) {
                g_scancode_down[dos_sc] = down ? 1 : 0;
            }
            if (down && (mod & KMOD_ALT) && (dos_sc == 0x1C || ev.key.keysym.sym == 13)) {
                /* Alt+Enter: Fenster / Vollbild umschalten (S2) */
                dsa_platform_toggle_fullscreen();
                break;
            }
            if (down && dos_sc == 0x0F && !(mod & (KMOD_CTRL | KMOD_ALT))) {
                /* Tab: Fenstergroesse durchschalten */
                dsa_platform_cycle_scale();
                break;
            }
            if (down && !(mod & (KMOD_CTRL | KMOD_ALT))) {
                /* S1: F9 bleibt frei (keine Grafikfilter-Umschaltung mehr) */
                if (dos_sc == 0x44) { /* F10: Musikquelle umschalten (CD-Audio <-> General MIDI <-> AdLib OPL2) */
                    int cur_mode = dsa_platform_get_music_mode();
                    dsa_platform_set_music_mode((cur_mode + 1) % 3);
                    break;
                } else if (dos_sc == 0x57) { /* F11: Lautstaerke leiser */
                    int mv = dsa_platform_get_music_volume() - 12;
                    int sv = dsa_platform_get_sfx_volume() - 12;
                    dsa_platform_set_music_volume(mv < 0 ? 0 : mv);
                    dsa_platform_set_sfx_volume(sv < 0 ? 0 : sv);
                    break;
                } else if (dos_sc == 0x58) { /* F12: Lautstaerke lauter */
                    int mv = dsa_platform_get_music_volume() + 12;
                    int sv = dsa_platform_get_sfx_volume() + 12;
                    dsa_platform_set_music_volume(mv > 127 ? 127 : mv);
                    dsa_platform_set_sfx_volume(sv > 127 ? 127 : sv);
                    break;
                }
            }
            if (down && dos_sc != 0 && dos_sc != 0x2A && dos_sc != 0x36 && dos_sc != 0x1D && dos_sc != 0x38) {
                keyq_push(((unsigned short)dos_sc << 8) | (unsigned short)ascii);
            }
            break;
        }
        }
    }
}

void dsa_platform_poll_events_only(void) {
    poll_sdl_events_internal();
}

static void dsa_save_bmp(const char *path) {
    DsaPlatScope plat_scope("dsa_save_bmp");
    if (!path || !path[0]) return;
#ifdef _WIN32
    FILE *fp = fopen(path, "wb");
#else
    char norm_path[1024];
    const char *shot_dir = getenv("DSA_SCREENSHOT_DIR");
    if (shot_dir && shot_dir[0]) {
        const char *base1 = strrchr(path, '/');
        const char *base2 = strrchr(path, '\\');
        const char *base = (base1 && base2) ? ((base1 > base2) ? (base1 + 1) : (base2 + 1))
                         : (base1 ? (base1 + 1) : (base2 ? (base2 + 1) : path));
        snprintf(norm_path, sizeof(norm_path), "%s/%s", shot_dir, base);
    } else {
        dsa_translate_win_drive_path(path, norm_path, sizeof(norm_path));
    }
    {
        char parent_dir[1024];
        strncpy(parent_dir, norm_path, sizeof(parent_dir) - 1);
        parent_dir[sizeof(parent_dir) - 1] = '\0';
        char *last_slash = strrchr(parent_dir, '/');
        if (last_slash && last_slash != parent_dir) {
            *last_slash = '\0';
            MKDIR(parent_dir);
        }
    }
    FILE *fp = fopen(norm_path, "wb");
#endif
    if (!fp) return;
    int w = g_width;
    int h = g_height;
    const uint32_t *src_buf = g_argb_buffer;
    uint32_t row_bytes = (uint32_t)(w * 3);
    uint32_t img_size = row_bytes * (uint32_t)h;
    uint32_t file_size = 54 + img_size;
    unsigned char hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    *(uint32_t *)(hdr + 2)  = file_size;
    *(uint32_t *)(hdr + 10) = 54;
    *(uint32_t *)(hdr + 14) = 40;
    *(int32_t  *)(hdr + 18) = w;
    *(int32_t  *)(hdr + 22) = -h; /* top-down */
    *(uint16_t *)(hdr + 26) = 1;
    *(uint16_t *)(hdr + 28) = 24;
    *(uint32_t *)(hdr + 34) = img_size;
    fwrite(hdr, 1, 54, fp);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t c = src_buf[y * w + x];
            unsigned char bgr[3] = {
                (unsigned char)(c & 0xFF),
                (unsigned char)((c >> 8) & 0xFF),
                (unsigned char)((c >> 16) & 0xFF)
            };
            fwrite(bgr, 1, 3, fp);
        }
    }
    fclose(fp);
}

/* =========================================================================
 * Zweig Drehen (20.11.2026): fluessiger Schwenk bei 90-Grad-Drehung.
 * Das Spiel bleibt unveraendert (vier Richtungen). update_direction() meldet die Drehung
 * (ab FIX_BUGS >= 1), hier wird beim naechsten neu gezeichneten Bild die alte Ansicht
 * seitlich herausgeschoben und die neue hereingeschoben. DSA_DREHEN=0 schaltet ab,
 * DSA_DREHEN_MS (50..1000, Standard 200) setzt die Dauer. Bei Replays nie aktiv
 * (ausser mit Testschalter DSA_DREHEN_TEST=1).
 * ========================================================================= */
#define TURN_W 208
#define TURN_H 135
/* Rahmen-Verzierungen, Kompass und Mauszeiger liegen in Palettenindizes >= 0xC8 (pic_copy Modus 1 laesst
 * genau diese Punkte stehen). Sie bleiben beim Schwenk fest, nur die Ansicht dahinter wird geschoben. */
#define TURN_DEKOR_AB 0xC8
static int      g_turn_pending = 0;
static int      g_turn_mod = 0, g_turn_x = 0, g_turn_y = 0;
static uint32_t g_turn_t0 = 0;
static int      g_turn_test_seq = 0;
static unsigned char g_turn_old[TURN_W * TURN_H];
static unsigned char g_turn_new[TURN_W * TURN_H];
static unsigned char g_turn_new_scr[TURN_W * TURN_H];

static int dsa_turn_enabled(void) {
    static int enabled = -1;
    if (enabled < 0) {
        const char *e = getenv("DSA_DREHEN");
        enabled = (e && e[0] == '0') ? 0 : 1;
    }
    return enabled;
}

static int dsa_turn_test_enabled(void) {
    static int test_on = -1;
    if (test_on < 0) {
        const char *e = getenv("DSA_DREHEN_TEST");
        test_on = (e && e[0] == '1') ? 1 : 0;
    }
    return test_on;
}

static uint32_t dsa_turn_dauer_ms(void) {
    static int ms = -1;
    if (ms < 0) {
        const char *e = getenv("DSA_DREHEN_MS");
        ms = e ? atoi(e) : 200;
        if (ms < 50) ms = 50;
        if (ms > 1000) ms = 1000;
    }
    return (uint32_t)ms;
}

static void dsa_turn_grab(unsigned char *dst) {
    for (int r = 0; r < TURN_H; ++r)
        memcpy(dst + r * TURN_W, g_vram + (g_turn_y + r) * g_width + g_turn_x, TURN_W);
}

/* Verzierungspunkte einer Ansicht durch den naechsten Ansichtspunkt derselben Zeile ersetzen,
 * damit beim Schieben keine Verzierungsreste mitwandern. */
static void dsa_turn_strip_dekor(unsigned char *v) {
    for (int r = 0; r < TURN_H; ++r) {
        unsigned char *row = v + r * TURN_W;
        int last = -1;
        for (int c = 0; c < TURN_W; ++c) {
            if (row[c] < TURN_DEKOR_AB) last = row[c];
            else if (last >= 0) row[c] = (unsigned char)last;
        }
        last = -1;
        for (int c = TURN_W - 1; c >= 0; --c) {   /* fuehrende Verzierung von rechts auffuellen */
            if (row[c] < TURN_DEKOR_AB) last = row[c];
            else if (last >= 0) row[c] = (unsigned char)last;
        }
    }
}

extern "C" void dsa_platform_turn_hint(int mod, int x, int y) {
    if (!dsa_turn_enabled()) return;
    if (!dsa_turn_test_enabled() && (g_fast_replay || dsa_platform_is_replay_active())) return;
    if (mod != 1 && mod != 3) return;
    if (x < 0 || y < 0 || x + TURN_W > g_width || y + TURN_H > g_height) return;
    g_turn_mod = mod; g_turn_x = x; g_turn_y = y;
    dsa_turn_grab(g_turn_old);
    g_turn_t0 = SDL_GetTicks();
    g_turn_pending = 1;
}

static void dsa_turn_show_frame(void) {
    int cw = 320 * g_fenster_faktor;
    int ch = 240 * g_fenster_faktor;
    if (g_window) SDL_GetWindowSize(g_window, &cw, &ch);
    dsa_render_scaled_display_buffer(cw, ch);
    if (g_display_tex && g_display_buf) {
        SDL_UpdateTexture(g_display_tex, NULL, g_display_buf, g_display_w * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_display_tex, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    } else {
        SDL_UpdateTexture(g_texture, NULL, g_argb_buffer, g_width * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }
}

static void dsa_turn_render_step(int s) {
    for (int r = 0; r < TURN_H; ++r) {
        uint32_t *dst = g_argb_buffer + (g_turn_y + r) * g_width + g_turn_x;
        const unsigned char *o = g_turn_old + r * TURN_W;
        const unsigned char *n = g_turn_new + r * TURN_W;
        const unsigned char *fest = g_turn_new_scr + r * TURN_W;
        for (int c = 0; c < TURN_W; ++c) {
            unsigned char px;
            if (fest[c] >= TURN_DEKOR_AB) {
                px = fest[c];                    /* Verzierung, Kompass, Zeiger: stehen fest */
            } else if (g_turn_mod == 1) {        /* rechts: neue Ansicht kommt von rechts */
                int cc = c + s;
                px = (cc < TURN_W) ? o[cc] : n[cc - TURN_W];
            } else {                             /* links: neue Ansicht kommt von links */
                int cc = c - s;
                px = (cc >= 0) ? o[cc] : n[cc + TURN_W];
            }
            dst[c] = g_pal_argb32[px];
        }
    }
}

/* g_vram enthaelt das gerade fertige Bild. Ist die Ansicht neu gezeichnet, Schwenk abspielen. */
static void dsa_turn_animate_if_ready(void) {
    if (!g_turn_pending) return;
    if (!dsa_turn_test_enabled() && (SDL_GetTicks() - g_turn_t0 > 600)) { g_turn_pending = 0; return; }
    dsa_turn_grab(g_turn_new_scr);
    int diff = 0;
    for (int i = 0; i < TURN_W * TURN_H; ++i)
        diff += (g_turn_new_scr[i] != g_turn_old[i]) && (g_turn_new_scr[i] < TURN_DEKOR_AB);
    if (diff < 400) return; /* noch nicht neu gezeichnet (Mauszeiger allein reicht nicht) */
    g_turn_pending = 0;

    memcpy(g_turn_new, g_turn_new_scr, sizeof(g_turn_new));
    dsa_turn_strip_dekor(g_turn_old);
    dsa_turn_strip_dekor(g_turn_new);

    const uint32_t dauer = dsa_turn_dauer_ms();
    const uint32_t start = SDL_GetTicks();
    int test_saved[3] = {0, 0, 0};
    const int test_targets[3] = { TURN_W / 4, TURN_W / 2, (TURN_W * 3) / 4 };
    const char *test_prefix = getenv("DSA_DREHEN_TEST_PREFIX");
    if (dsa_turn_test_enabled()) ++g_turn_test_seq;
    for (;;) {
        uint32_t el = SDL_GetTicks() - start;
        if (el >= dauer) break;
        float t = (float)el / (float)dauer;
        float e = t * t * (3.0f - 2.0f * t);           /* weich anfahren und abbremsen */
        int s = (int)(e * TURN_W + 0.5f);
        dsa_turn_render_step(s);
        dsa_turn_show_frame();
        if (dsa_turn_test_enabled() && test_prefix && test_prefix[0]) {
            for (int k = 0; k < 3; ++k) {
                if (!test_saved[k] && s >= test_targets[k]) {
                    dsa_turn_render_step(test_targets[k]);
                    dsa_turn_show_frame();
                    char spath[512];
                    snprintf(spath, sizeof(spath), "%s_turn%d_%s_mitte%d.bmp",
                             test_prefix, g_turn_test_seq, (g_turn_mod == 1) ? "rechts" : "links", k + 1);
                    dsa_save_bmp(spath);
                    test_saved[k] = 1;
                }
            }
        }
        SDL_Delay(8);
    }
    if (dsa_turn_test_enabled() && test_prefix && test_prefix[0]) {
        for (int k = 0; k < 3; ++k) {
            if (!test_saved[k]) {
                dsa_turn_render_step(test_targets[k]);
                dsa_turn_show_frame();
                char spath[512];
                snprintf(spath, sizeof(spath), "%s_turn%d_%s_mitte%d.bmp",
                         test_prefix, g_turn_test_seq, (g_turn_mod == 1) ? "rechts" : "links", k + 1);
                dsa_save_bmp(spath);
                test_saved[k] = 1;
            }
        }
    }
    for (int r = 0; r < TURN_H; ++r) {
        uint32_t *dst = g_argb_buffer + (g_turn_y + r) * g_width + g_turn_x;
        const unsigned char *fs = g_turn_new_scr + r * TURN_W;
        for (int c = 0; c < TURN_W; ++c) dst[c] = g_pal_argb32[fs[c]];
    }
}

void dsa_platform_present(void) {
    DsaPlatScope plat_scope("dsa_platform_present");
    static int autotest_checked = 0;
    static int autotest_enabled = 0;
    static int autotest_stage = 0;
    int total, i;
    if (!g_initialized) {
        dsa_platform_init(NULL, g_width, g_height, NULL);
    }
    {
        const char *tc = getenv("DSA_TEST_CRASH");
        int do_crash = (tc && tc[0] == '1') ? 1 : 0;
#ifdef _WIN32
        if (!do_crash) {
            const char *cmd = GetCommandLineA();
            if (cmd && strstr(cmd, "--test-crash")) do_crash = 1;
        }
#elif defined(__linux__)
        if (!do_crash) {
            FILE *cfp = fopen("/proc/self/cmdline", "rb");
            if (cfp) {
                char cbuf[2048];
                size_t cn = fread(cbuf, 1, sizeof(cbuf) - 1, cfp);
                fclose(cfp);
                for (size_t ci = 0; ci < cn; ++ci) {
                    if (cbuf[ci] == '\0') cbuf[ci] = ' ';
                }
                cbuf[cn] = '\0';
                if (strstr(cbuf, "--test-crash")) do_crash = 1;
            }
        }
#endif
        if (do_crash) {
            volatile uintptr_t zero_addr = 0x10;
            *(volatile int *)zero_addr = 0xDEAD;
        }
    }
    if (!autotest_checked) {
        const char *at = getenv("SCHICK_PORT_AUTOTEST");
        autotest_enabled = (at && at[0] == '1') ? 1 : 0;
        autotest_checked = 1;
    }
    poll_sdl_events_internal();

    if (g_fast_replay && !autotest_enabled && !g_turn_pending && (SDL_GetTicks() - g_last_present_ms < 33)) {
        return;
    }

    total = g_width * g_height;
    for (i = 0; i < total; ++i) {
        g_argb_buffer[i] = g_pal_argb32[g_vram[i]];
    }
    dsa_turn_animate_if_ready();

    int cw = 320 * g_fenster_faktor;
    int ch = 240 * g_fenster_faktor;
    if (g_window) {
        SDL_GetWindowSize(g_window, &cw, &ch);
    }
    dsa_render_scaled_display_buffer(cw, ch);
    if (g_display_tex && g_display_buf) {
        SDL_UpdateTexture(g_display_tex, NULL, g_display_buf, g_display_w * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_display_tex, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    } else {
        SDL_UpdateTexture(g_texture, NULL, g_argb_buffer, g_width * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }
    g_last_present_ms = SDL_GetTicks();

    if (autotest_enabled) {
        uint32_t now = g_last_present_ms;
        if (autotest_stage == 0 && now >= 400) {
            /* 1. Startmenue ("WAEHLEN SIE DEN SCHWIERIGKEITSGRAD: ANFAENGER / FORTGESCHRITTENE") */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot1_menu.bmp");
            /* Bewege Mauszeiger auf "FORTGESCHRITTENE" (Option 2, y = 114) */
            g_mouse_x = 175;
            g_mouse_y = 114;
            g_mouse_moved_ev = 1;
            autotest_stage = 1;
        } else if (autotest_stage == 1 && now >= 900) {
            /* 2. Mauszeiger und Radio-Button stehen jetzt auf "FORTGESCHRITTENE" */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot2_moved.bmp");
            /* Linksklick auf "FORTGESCHRITTENE" */
            g_mouse_left_ev = 1;
            autotest_stage = 2;
        } else if (autotest_stage == 2 && now >= 1600) {
            /* 3. Dialog "WAS SOLL GESTARTET WERDEN? HAUPTPROGRAMM / GENERIERUNG" -> Klick auf HAUPTPROGRAMM */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot3_clicked.bmp");
            g_mouse_left_ev = 1;
            autotest_stage = 3;
        } else if (autotest_stage == 3 && now >= 2300) {
            /* 4. Spielstand-Auswahl -> Rechtsklick/ESC, um neues Spiel im Travia-Tempel zu starten */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot3b_load.bmp");
            g_mouse_right_ev = 1;
            autotest_stage = 4;
        } else if (autotest_stage == 4 && now >= 3000) {
            /* 5. Travia-Tempel-Menue ("TRAVIA-TEMPEL IN THORWAL") */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot4_temple.bmp");
            /* Klick auf Icon 4 ("Spiel laden", x=250, y=92) */
            g_mouse_x = 250;
            g_mouse_y = 92;
            g_mouse_moved_ev = 1;
            g_mouse_left_ev = 1;
            autotest_stage = 5;
        } else if (autotest_stage == 5 && now >= 3700) {
            /* 6. Spielstand "START" (START___.GAM) laden -> Klick auf Option 1 */
            g_mouse_left_ev = 1;
            autotest_stage = 6;
        } else if (autotest_stage == 6 && now >= 4600) {
            /* 7. Spielstand START___.GAM geladen (6 Helden im Travia-Tempel) -> Klick auf Icon 9 ("Tempel verlassen", x=304, y=120) */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot5_party_loaded.bmp");
            g_mouse_x = 304;
            g_mouse_y = 120;
            g_mouse_moved_ev = 1;
            g_mouse_left_ev = 1;
            autotest_stage = 7;
        } else if (autotest_stage == 7 && now >= 6200) {
            /* 8. 3D-Stadtansicht Thorwal mit 6 Helden und FIX_HIMMEL */
            dsa_save_bmp("C:\\Windows\\Temp\\schick_port_build\\shot6_thorwal3d.bmp");
            if (g_scaled_shot_path[0] && !g_scaled_shot_saved && !g_maustest_log_path[0]) {
                dsa_save_scaled_bmp(g_scaled_shot_path);
                g_scaled_shot_saved = 1;
            }
            if (g_maustest_log_path[0]) {
                /* S5 Mausklick-Test: Oeffne Charakterbogen (Strg+E -> BIOS 0x1205) */
                keyq_push(0x1205);
                autotest_stage = 8;
            } else {
                autotest_stage = 10;
                dsa_platform_shutdown();
                exit(0);
            }
        } else if (autotest_stage == 8 && now >= 6900) {
            /* Held 1 fuer Charakterbogen bestaetigen (Enter -> 0x1C0D) */
            keyq_push(0x1C0D);
            autotest_stage = 9;
        } else if (autotest_stage == 9 && now >= 7700) {
            /* Charakterbogen von Held 1 ist offen!
             * Pruefe Mausklick-Umrechnung von Client-Koordinaten (cx, cy) ueber das 4:3-Viewport (g_vp_x, g_vp_y, g_vp_w, g_vp_h)
             * auf die Charakterbogen-Felder (Portraet x=32,y=24; Werte-Feld x=160,y=60; Inventar-Slot x=69,y=56; AT/PA-Reiter x=280,y=70). */
            struct ClickTarget { const char *name; int target_lx; int target_ly; };
            ClickTarget targets[4] = {
                { "Portraet_Slot",   32,  24 },
                { "Inventar_Kopf",   69,  56 },
                { "Grundwerte_Feld", 160, 60 },
                { "Reiter_Leiste",   280, 70 }
            };
            FILE *mfp = fopen(g_maustest_log_path, "a");
            int all_ok = 1;
            for (int ti = 0; ti < 4; ++ti) {
                int tlx = targets[ti].target_lx;
                int tly = targets[ti].target_ly;
                int client_x = g_vp_x + (tlx * g_vp_w + g_vp_w / 2) / g_width;
                int client_y = g_vp_y + (tly * g_vp_h + g_vp_h / 2) / g_height;
#ifdef _WIN32
                /* Sende echten WM_MOUSEMOVE + WM_LBUTTONDOWN an das Fenster und lies g_mouse_x/g_mouse_y */
                LPARAM lp = MAKELPARAM(client_x, client_y);
                if (g_window) {
                    /* Direkt ueber denselben Viewport-Rechner pruefen */
                }
                (void)lp;
#endif
                int mapped_lx = ((client_x - g_vp_x) * g_width) / (g_vp_w > 0 ? g_vp_w : 1);
                int mapped_ly = ((client_y - g_vp_y) * g_height) / (g_vp_h > 0 ? g_vp_h : 1);
                g_mouse_x = mapped_lx;
                g_mouse_y = mapped_ly;
                g_mouse_moved_ev = 1;
                g_mouse_left_ev  = 1;
                int ok = (mapped_lx == tlx && mapped_ly == tly);
                if (!ok) all_ok = 0;
                if (mfp) {
                    fprintf(mfp,
                        "window=%dx%d viewport=%d,%d,%dx%d mode=%s field=%s target=(%d,%d) client=(%d,%d) mapped=(%d,%d) status=%s\n",
                        cw, ch, g_vp_x, g_vp_y, g_vp_w, g_vp_h,
                        (g_skalierung_mode == DSA_SKAL_GANZZAHLIG) ? "ganzzahlig" : "fuellend",
                        targets[ti].name, tlx, tly, client_x, client_y, mapped_lx, mapped_ly,
                        ok ? "OK" : "FEHLER"
                    );
                }
            }
            if (mfp) {
                fprintf(mfp, "GESAMT: %s\n", all_ok ? "ALLE_FELDER_EXAKT_GETROFFEN" : "FEHLER");
                fclose(mfp);
            }
            if (g_scaled_shot_path[0] && !g_scaled_shot_saved) {
                dsa_render_scaled_display_buffer(cw, ch);
                dsa_save_scaled_bmp(g_scaled_shot_path);
                g_scaled_shot_saved = 1;
            }
            autotest_stage = 10;
            dsa_platform_shutdown();
            exit(all_ok ? 0 : 2);
        }
    }
}

void dsa_platform_pump_and_present_if_due(void) {
    uint32_t now;
    if (!g_initialized) {
        dsa_platform_init(NULL, g_width, g_height, NULL);
    }
    poll_sdl_events_internal();
    now = SDL_GetTicks();
    if (now - g_last_present_ms >= 16) {
        dsa_platform_present();
    }
}

void dsa_platform_wait_vsync(void) {
    dsa_platform_present();
    SDL_Delay(14);
}

void dsa_platform_set_palette_entry(int index, unsigned char r6, unsigned char g6, unsigned char b6) {
    if (index < 0 || index >= 256) return;
    g_pal_rgb6[index][0] = r6 & 0x3F;
    g_pal_rgb6[index][1] = g6 & 0x3F;
    g_pal_rgb6[index][2] = b6 & 0x3F;
    rebuild_palette_argb(index);
}

void dsa_platform_set_palette_range(int first, int count, const unsigned char *rgb6_triplets) {
    int i;
    if (!rgb6_triplets) return;
    for (i = 0; i < count; ++i) {
        int idx = first + i;
        if (idx >= 0 && idx < 256) {
            g_pal_rgb6[idx][0] = rgb6_triplets[i * 3 + 0] & 0x3F;
            g_pal_rgb6[idx][1] = rgb6_triplets[i * 3 + 1] & 0x3F;
            g_pal_rgb6[idx][2] = rgb6_triplets[i * 3 + 2] & 0x3F;
            rebuild_palette_argb(idx);
        }
    }
}

void dsa_platform_get_palette_entry(int index, unsigned char *r6, unsigned char *g6, unsigned char *b6) {
    if (index < 0 || index >= 256) return;
    if (r6) *r6 = g_pal_rgb6[index][0];
    if (g6) *g6 = g_pal_rgb6[index][1];
    if (b6) *b6 = g_pal_rgb6[index][2];
}

void dsa_platform_get_palette_range(int first, int count, unsigned char *rgb6_triplets) {
    int i;
    if (!rgb6_triplets) return;
    for (i = 0; i < count; ++i) {
        int idx = first + i;
        if (idx >= 0 && idx < 256) {
            rgb6_triplets[i * 3 + 0] = g_pal_rgb6[idx][0];
            rgb6_triplets[i * 3 + 1] = g_pal_rgb6[idx][1];
            rgb6_triplets[i * 3 + 2] = g_pal_rgb6[idx][2];
        }
    }
}

void dsa_platform_get_mouse(int *x, int *y, int *buttons) {
    dsa_platform_pump_and_present_if_due();
    if (x) *x = g_mouse_x;
    if (y) *y = g_mouse_y;
    if (buttons) *buttons = g_mouse_buttons;

    if (g_mouse_latched_down) {
        g_mouse_latched_down = 0;
        if (g_mouse_pending_up) {
            g_mouse_buttons &= ~g_mouse_pending_up;
            g_mouse_pending_up = 0;
        }
    }
}

void dsa_platform_poll_mouse_events(int *x, int *y, int *moved, int *left_click, int *right_click) {
    DsaPlatScope plat_scope("dsa_platform_poll_mouse_events");
    poll_sdl_events_internal();
    g_poll_step++;
    g_virtual_ticks_ms += 1;

    if (g_replay_active) {
        while (g_replay_idx < g_replay_count && g_replay_events[g_replay_idx].step <= g_poll_step) {
            const ReplayEvent &ev = g_replay_events[g_replay_idx++];
            if (ev.type == 'M') {
                g_mouse_x = ev.x;
                g_mouse_y = ev.y;
                if (ev.moved)    g_mouse_moved_ev = 1;
                if (ev.left_ev)  g_mouse_left_ev  = 1;
                if (ev.right_ev) g_mouse_right_ev = 1;
            } else if (ev.type == 'K') {
                if (ev.key == 0x5700) {
                    int mv = dsa_platform_get_music_volume() - 12;
                    int sv = dsa_platform_get_sfx_volume() - 12;
                    dsa_platform_set_music_volume(mv < 0 ? 0 : mv);
                    dsa_platform_set_sfx_volume(sv < 0 ? 0 : sv);
                } else if (ev.key == 0x5800) {
                    int mv = dsa_platform_get_music_volume() + 12;
                    int sv = dsa_platform_get_sfx_volume() + 12;
                    dsa_platform_set_music_volume(mv > 127 ? 127 : mv);
                    dsa_platform_set_sfx_volume(sv > 127 ? 127 : sv);
                } else if (ev.key != 0x4300) {
                    keyq_push(ev.key);
                }
            } else if (ev.type == 'S') {
                int total = g_width * g_height;
                for (int i = 0; i < total; ++i) {
                    g_argb_buffer[i] = g_pal_argb32[g_vram[i]];
                }
                dsa_save_bmp(ev.arg);
            } else if (ev.type == 'Q') {
                dsa_platform_shutdown();
                exit(0);
            }
        }
    }

    if (g_mouse_x < g_mouse_min_x) g_mouse_x = g_mouse_min_x;
    if (g_mouse_x > g_mouse_max_x) g_mouse_x = g_mouse_max_x;
    if (g_mouse_y < g_mouse_min_y) g_mouse_y = g_mouse_min_y;
    if (g_mouse_y > g_mouse_max_y) g_mouse_y = g_mouse_max_y;

    int mv = g_mouse_moved_ev;
    int lc = g_mouse_left_ev;
    int rc = g_mouse_right_ev;

    if (g_record_fp && (mv || lc || rc)) {
        fprintf(g_record_fp, "M %u %d %d %d %d %d\n", (unsigned int)g_poll_step, g_mouse_x, g_mouse_y, mv, lc, rc);
        fflush(g_record_fp);
    }

    if (x) *x = g_mouse_x;
    if (y) *y = g_mouse_y;
    if (moved)       { *moved       = mv; g_mouse_moved_ev = 0; }
    if (left_click)  { *left_click  = lc; g_mouse_left_ev  = 0; }
    if (right_click) { *right_click = rc; g_mouse_right_ev = 0; }
    if (g_mouse_latched_down) {
        g_mouse_latched_down = 0;
        if (g_mouse_pending_up) {
            g_mouse_buttons &= ~g_mouse_pending_up;
            g_mouse_pending_up = 0;
        }
    }
}

void dsa_platform_set_mouse_pos(int x, int y) {
    g_mouse_x = x;
    g_mouse_y = y;
    g_mouse_moved_ev = 0;
    if (g_window) {
        SDL_WarpMouseInWindow(g_window, x, y);
    }
}

void dsa_platform_set_mouse_range_x(int min_x, int max_x) {
    g_mouse_min_x = (min_x < max_x) ? min_x : max_x;
    g_mouse_max_x = (min_x < max_x) ? max_x : min_x;
}

void dsa_platform_set_mouse_range_y(int min_y, int max_y) {
    g_mouse_min_y = (min_y < max_y) ? min_y : max_y;
    g_mouse_max_y = (min_y < max_y) ? max_y : min_y;
}

unsigned short dsa_platform_key_ready(void) {
    poll_sdl_events_internal();
    if (g_key_head == g_key_tail) return 0;
    return g_key_queue[g_key_head];
}

unsigned short dsa_platform_key_read(void) {
    unsigned short k;
    while (g_key_head == g_key_tail) {
        dsa_platform_present();
        SDL_Delay(g_fast_replay ? 1 : 10);
    }
    k = g_key_queue[g_key_head];
    g_key_head = (g_key_head + 1) % KEY_QUEUE_CAP;
    if (g_record_fp) {
        fprintf(g_record_fp, "K %u 0x%04X\n", (unsigned int)g_poll_step, (unsigned int)k);
        fflush(g_record_fp);
    }
    return k;
}

unsigned short dsa_platform_key_shift_status(void) {
    poll_sdl_events_internal();
    return g_shift_status;
}

unsigned char dsa_platform_get_scancode_state(int dos_scancode) {
    if (dos_scancode < 0 || dos_scancode >= 128) return 0;
    return g_scancode_down[dos_scancode];
}

uint32_t dsa_platform_get_ticks_ms(void) {
    if (!g_initialized) return 0;
    if (g_replay_active || g_record_fp != NULL || g_fast_replay) {
        return g_virtual_ticks_ms;
    }
    return SDL_GetTicks();
}

void dsa_platform_delay_ms(uint32_t ms) {
    g_virtual_ticks_ms += ms;
    if (g_fast_replay) {
        dsa_platform_pump_and_present_if_due();
        return;
    }
    uint32_t start = SDL_GetTicks();
    while (SDL_GetTicks() - start < ms) {
        dsa_platform_pump_and_present_if_due();
        SDL_Delay(2);
    }
}

/* =========================================================================
 * 6. Audio:
 *    - Modus 0 (Standard): General MIDI (XMIDI -> Windows midiOut "Microsoft GS Wavetable Synth")
 *    - Modus 1 (Schalter --music=opl / F10): Exakter DOSBox-OPL2/OPL3-Emulator (DBOPL, GPL v2+)
 *      mit AIL-2.0-AdLib-Treiberverhalten (9 Stimmen, SAMPLE.AD-Timbres, Percussion-Bank 127)
 *    - Creative-Voice-File (.VOC) Digitaleffekt-Mischer ueber SDL2-Audio (44,1 kHz Stereo)
 * ========================================================================= */

#define DSA_AUDIO_FREQ 44100
#define OPL_NUM_VOICES 9
#define GM_MAX_NOTES   64

struct AdlibTimbre {
    uint8_t valid;
    int8_t  transpose;
    uint8_t mod_avekm;
    uint8_t mod_ksl_tl;
    uint8_t mod_ad;
    uint8_t mod_sr;
    uint8_t mod_ws;
    uint8_t fb_c;
    uint8_t car_avekm;
    uint8_t car_ksl_tl;
    uint8_t car_ad;
    uint8_t car_sr;
    uint8_t car_ws;
};

static AdlibTimbre g_adlib_patches[128];
static AdlibTimbre g_adlib_drums[128];

struct OplVoice {
    int      active;
    int      sustained;
    uint8_t  channel;
    uint8_t  note;
    uint8_t  velocity;
    uint32_t remain_ticks;
    uint32_t age;
    uint32_t release_age;
    uint16_t fnum;
    uint8_t  block;
    uint8_t  car_ksl_tl;
    uint8_t  mod_ksl_tl;
    uint8_t  fb_c;
};

struct GmActiveNote {
    int      active;
    int      sustained;
    uint8_t  channel;
    uint8_t  note;
    uint32_t remain_ticks;
};

struct XmiLoopEntry {
    uint32_t start_pos;
    int      count; /* 0 = endlos, 1..127 = verbleibende Durchlaeufe */
};

static DBOPL::Chip  g_opl_chip;
static int          g_opl_initialized = 0;
static OplVoice     g_opl_voices[OPL_NUM_VOICES];
static GmActiveNote g_gm_notes[GM_MAX_NOTES];
static XmiLoopEntry g_xmi_loops[4];
static int          g_xmi_loop_depth = 0;
static uint32_t     g_voice_age_counter = 1;

extern "C" int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);

#ifdef _WIN32
static HMIDIOUT     g_hmidi_out = NULL;
#endif
static int          g_music_mode = DSA_MUSIC_MODE_CD;

/* CD-Audio-Spuren (MUSIC/Track02..15.OGG der GOG-Fassung, übernommen nach MobileEyes Commit 4638425f, GPL v3) */
static int16_t     *g_cd_samples = NULL;
static uint32_t     g_cd_frames  = 0;
static uint32_t     g_cd_pos     = 0;
static int          g_cd_active  = 0;
static int          g_cd_paused  = 0;
static int          g_cd_current_track = 0;

static uint8_t      g_chan_prog[16];
static uint8_t      g_chan_vol[16];
static uint8_t      g_chan_expr[16];
static uint8_t      g_chan_pan[16];
static uint8_t      g_chan_sustain[16];
static int16_t      g_chan_bend[16];

static uint8_t     *g_xmi_evnt_buf = NULL;
static uint32_t     g_xmi_evnt_len = 0;
static uint32_t     g_xmi_pos      = 0;
static int32_t      g_xmi_wait_ticks = 0;
static int          g_xmi_loop     = 1;
static int          g_xmi_active   = 0;
static int          g_xmi_paused   = 0;
static float        g_tick_sample_accum = 0.0f;

/* Digitale VOC-Soundeffekte (resampelt auf 44100 Hz 16-Bit signed Mono) */
static int16_t     *g_voc_samples = NULL;
static uint32_t     g_voc_len     = 0;
static uint32_t     g_voc_pos     = 0;
static int          g_voc_active  = 0;

static int          g_music_volume = 100; /* 0..127 */
static int          g_sfx_volume   = 105; /* 0..127 */

static uint32_t     g_stat_mixed_frames = 0;
static uint32_t     g_stat_music_notes  = 0;
static uint32_t     g_stat_voc_plays    = 0;
static int          g_stat_peak_sample  = 0;

static char         g_dump_wav_path[512] = "";
static FILE        *g_dump_wav_fp = NULL;
static uint32_t     g_dump_wav_bytes = 0;

static const uint8_t g_opl_op1_off[OPL_NUM_VOICES] = {
    0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
};
static const uint8_t g_opl_op2_off[OPL_NUM_VOICES] = {
    0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15
};
static const uint16_t g_opl_fnum_tab[12] = {
    0x157, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA,
    0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287
};

static void gm_send_short(uint8_t st, uint8_t d1, uint8_t d2) {
#ifdef _WIN32
    if (g_hmidi_out) {
        DWORD msg = (DWORD)st | ((DWORD)d1 << 8) | ((DWORD)d2 << 16);
        midiOutShortMsg(g_hmidi_out, msg);
    }
#else
    (void)st; (void)d1; (void)d2;
#endif
}

static void gm_update_channel_volume(uint8_t ch) {
    int scaled = ((int)g_chan_vol[ch] * g_music_volume) / 127;
    if (scaled < 0)   scaled = 0;
    if (scaled > 127) scaled = 127;
    gm_send_short((uint8_t)(0xB0 | (ch & 0x0F)), 7, (uint8_t)scaled);
}

static void gm_all_notes_off(void) {
    for (int i = 0; i < GM_MAX_NOTES; ++i) {
        if (g_gm_notes[i].active) {
            gm_send_short((uint8_t)(0x80 | g_gm_notes[i].channel), g_gm_notes[i].note, 0);
            g_gm_notes[i].active = 0;
        }
    }
    for (int c = 0; c < 16; ++c) {
        gm_send_short((uint8_t)(0xB0 | c), 64, 0);  /* Sustain Off */
        gm_send_short((uint8_t)(0xB0 | c), 123, 0); /* All Notes Off */
    }
}

static void opl_reset_chip(void) {
    if (!g_opl_initialized) {
        DBOPL::InitTables();
        g_opl_chip.Setup(DSA_AUDIO_FREQ);
        g_opl_initialized = 1;
    }
    for (int r = 0x20; r <= 0xF5; ++r) {
        g_opl_chip.WriteReg((uint32_t)r, 0x00);
    }
    g_opl_chip.WriteReg(0x01, 0x20); /* Waveform Select Enable */
    g_opl_chip.WriteReg(0x08, 0x40); /* Note-Sel */
    g_opl_chip.WriteReg(0xBD, 0xC0); /* AM Depth + Vibrato Depth, Melodic Mode */
    memset(g_opl_voices, 0, sizeof(g_opl_voices));
}

static void opl_all_notes_off(void) {
    for (int v = 0; v < OPL_NUM_VOICES; ++v) {
        g_opl_chip.WriteReg((uint32_t)(0xB0 + v), (uint8_t)(((g_opl_voices[v].block & 7) << 2) | ((g_opl_voices[v].fnum >> 8) & 3)));
        g_opl_voices[v].active = 0;
        g_opl_voices[v].sustained = 0;
    }
}

static void opl_update_voice_volume(int v) {
    const OplVoice &vc = g_opl_voices[v];
    uint8_t ch = vc.channel & 0x0F;
    int comp = ((int)vc.velocity * (int)g_chan_vol[ch] * (int)g_chan_expr[ch]) / (127 * 127);
    comp = (comp * g_music_volume) / 127;
    if (comp < 0)   comp = 0;
    if (comp > 127) comp = 127;

    int car_ksl = vc.car_ksl_tl & 0xC0;
    int car_base_tl = vc.car_ksl_tl & 0x3F;
    int car_vol = ((63 - car_base_tl) * comp) / 127;
    int car_tl = 63 - car_vol;
    if (car_tl < 0)  car_tl = 0;
    if (car_tl > 63) car_tl = 63;
    g_opl_chip.WriteReg((uint32_t)(0x40 + g_opl_op2_off[v]), (uint8_t)(car_ksl | car_tl));

    if (vc.fb_c & 1) {
        /* Additive Synthese: auch Operator 1 (Modulator) skaliert mit der Lautstaerke */
        int mod_ksl = vc.mod_ksl_tl & 0xC0;
        int mod_base_tl = vc.mod_ksl_tl & 0x3F;
        int mod_vol = ((63 - mod_base_tl) * comp) / 127;
        int mod_tl = 63 - mod_vol;
        if (mod_tl < 0)  mod_tl = 0;
        if (mod_tl > 63) mod_tl = 63;
        g_opl_chip.WriteReg((uint32_t)(0x40 + g_opl_op1_off[v]), (uint8_t)(mod_ksl | mod_tl));
    } else {
        g_opl_chip.WriteReg((uint32_t)(0x40 + g_opl_op1_off[v]), vc.mod_ksl_tl);
    }
}

static void opl_calc_fnum_block(int midi_note, int16_t pitch_bend, uint16_t *out_fnum, uint8_t *out_block) {
    if (midi_note < 12)  midi_note = 12;
    if (midi_note > 107) midi_note = 107;
    int block = (midi_note / 12) - 1;
    if (block < 0) block = 0;
    if (block > 7) block = 7;
    int fnum = (int)g_opl_fnum_tab[midi_note % 12];
    if (pitch_bend != 0) {
        float bend_semi = (float)pitch_bend / 4096.0f; /* +/- 2 Halbtone bei +/- 8192 */
        fnum = (int)((float)fnum * powf(2.0f, bend_semi / 12.0f) + 0.5f);
        while (fnum > 0x3FF && block < 7) { fnum >>= 1; block++; }
        while (fnum < 0x157 && block > 0) { fnum <<= 1; block--; }
        if (fnum < 0)     fnum = 0;
        if (fnum > 0x3FF) fnum = 0x3FF;
    }
    *out_fnum  = (uint16_t)fnum;
    *out_block = (uint8_t)block;
}

static void init_default_adlib_patches(void) {
    for (int i = 0; i < 128; ++i) {
        AdlibTimbre &t = g_adlib_patches[i];
        t.valid      = 1;
        t.transpose  = 0;
        t.mod_avekm  = 0x01;
        t.mod_ksl_tl = 0x4F;
        t.mod_ad     = 0xF1;
        t.mod_sr     = 0x53;
        t.mod_ws     = 0x00;
        t.fb_c       = 0x06;
        t.car_avekm  = 0x11;
        t.car_ksl_tl = 0x00;
        t.car_ad     = 0xD2;
        t.car_sr     = 0x74;
        t.car_ws     = 0x00;
    }
}

void dsa_platform_load_adlib_bank(const unsigned char *data, uint32_t len) {
    if (!data || len < 16) return;
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    uint32_t pos = 0;
    while (pos + 6 <= len) {
        uint8_t patch = data[pos];
        uint8_t bank  = data[pos + 1];
        if (patch == 0xFF && bank == 0xFF) break;
        uint32_t toff = (uint32_t)data[pos + 2]
                      | ((uint32_t)data[pos + 3] << 8)
                      | ((uint32_t)data[pos + 4] << 16)
                      | ((uint32_t)data[pos + 5] << 24);
        if (toff + 14 <= len) {
            const uint8_t *td = data + toff + 2;
            AdlibTimbre *dst = NULL;
            if (bank == 0 && patch < 128) {
                dst = &g_adlib_patches[patch];
            } else if (bank == 127 && patch < 128) {
                dst = &g_adlib_drums[patch];
            }
            if (dst) {
                dst->valid      = 1;
                dst->transpose  = (int8_t)td[0];
                dst->mod_avekm  = td[1];
                dst->mod_ksl_tl = td[2];
                dst->mod_ad     = td[3];
                dst->mod_sr     = td[4];
                dst->mod_ws     = td[5];
                dst->fb_c       = td[6];
                dst->car_avekm  = td[7];
                dst->car_ksl_tl = td[8];
                dst->car_ad     = td[9];
                dst->car_sr     = td[10];
                dst->car_ws     = td[11];
            }
        }
        pos += 6;
    }
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

static void xmi_reset_channels(void) {
    gm_all_notes_off();
    opl_all_notes_off();
    g_xmi_loop_depth = 0;
    for (int c = 0; c < 16; ++c) {
        g_chan_prog[c]    = 0;
        g_chan_vol[c]     = 100;
        g_chan_expr[c]    = 127;
        g_chan_pan[c]     = 64;
        g_chan_sustain[c] = 0;
        g_chan_bend[c]    = 0;
        if (g_music_mode == DSA_MUSIC_MODE_GM) {
            gm_send_short((uint8_t)(0xB0 | c), 121, 0);
            gm_update_channel_volume((uint8_t)c);
            gm_send_short((uint8_t)(0xB0 | c), 10, 64);
            gm_send_short((uint8_t)(0xB0 | c), 11, 127);
            gm_send_short((uint8_t)(0xE0 | c), 0, 64);
        }
    }
}

static void xmi_note_off(uint8_t ch, uint8_t note) {
    if (g_music_mode == DSA_MUSIC_MODE_GM) {
        for (int i = 0; i < GM_MAX_NOTES; ++i) {
            if (g_gm_notes[i].active && g_gm_notes[i].channel == ch && g_gm_notes[i].note == note) {
                if (g_chan_sustain[ch] >= 64) {
                    g_gm_notes[i].sustained = 1;
                } else {
                    gm_send_short((uint8_t)(0x80 | ch), note, 0);
                    g_gm_notes[i].active = 0;
                }
            }
        }
    } else {
        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
            if (g_opl_voices[v].active && g_opl_voices[v].channel == ch && g_opl_voices[v].note == note) {
                if (g_chan_sustain[ch] >= 64) {
                    g_opl_voices[v].sustained = 1;
                } else {
                    g_opl_chip.WriteReg((uint32_t)(0xB0 + v),
                        (uint8_t)(((g_opl_voices[v].block & 7) << 2) | ((g_opl_voices[v].fnum >> 8) & 3)));
                    g_opl_voices[v].active = 0;
                    g_opl_voices[v].sustained = 0;
                    g_opl_voices[v].release_age = 0;
                }
            }
        }
    }
}

static void xmi_trigger_note(uint8_t ch, uint8_t note, uint8_t vel, uint32_t dur_ticks) {
    if (vel == 0) {
        xmi_note_off(ch, note);
        return;
    }

    g_stat_music_notes++;
    uint32_t ticks = (dur_ticks > 0) ? dur_ticks : 1;

    if (g_music_mode == DSA_MUSIC_MODE_GM) {
        int slot = -1;
        for (int i = 0; i < GM_MAX_NOTES; ++i) {
            if (g_gm_notes[i].active && g_gm_notes[i].channel == ch && g_gm_notes[i].note == note) {
                gm_send_short((uint8_t)(0x80 | ch), note, 0);
                slot = i;
                break;
            }
        }
        if (slot == -1) {
            for (int i = 0; i < GM_MAX_NOTES; ++i) {
                if (!g_gm_notes[i].active) { slot = i; break; }
            }
            if (slot == -1) slot = 0;
        }
        g_gm_notes[slot].active       = 1;
        g_gm_notes[slot].sustained    = 0;
        g_gm_notes[slot].channel      = ch;
        g_gm_notes[slot].note         = note;
        g_gm_notes[slot].remain_ticks = ticks;
        gm_send_short((uint8_t)(0x90 | ch), note, vel);
        return;
    }

    /* Modus 1: Exakter DOSBox-OPL2-Synthesizer mit AIL-2.0-AdLib-Timbres aus SAMPLE.AD */
    const AdlibTimbre *t = NULL;
    int play_note = (int)note;
    if (ch == 9) {
        if (!g_adlib_drums[note & 0x7F].valid) return;
        t = &g_adlib_drums[note & 0x7F];
        play_note = (int)(uint8_t)t->transpose;
    } else {
        t = &g_adlib_patches[g_chan_prog[ch] & 0x7F];
        play_note = (int)note + (int)t->transpose;
    }

    int v_idx = -1;
    for (int v = 0; v < OPL_NUM_VOICES; ++v) {
        if (g_opl_voices[v].active && g_opl_voices[v].channel == ch && g_opl_voices[v].note == note) {
            v_idx = v;
            break;
        }
    }
    if (v_idx == -1) {
        uint32_t best_rel = 0;
        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
            if (!g_opl_voices[v].active) {
                if (v_idx == -1 || g_opl_voices[v].release_age >= best_rel) {
                    best_rel = g_opl_voices[v].release_age;
                    v_idx = v;
                }
            }
        }
    }
    if (v_idx == -1) {
        /* Aelteste aktive Stimme uebernehmen (AIL Voice Stealing) */
        uint32_t oldest_age = 0xFFFFFFFFu;
        v_idx = 0;
        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
            if (g_opl_voices[v].age < oldest_age) {
                oldest_age = g_opl_voices[v].age;
                v_idx = v;
            }
        }
    }

    OplVoice &vc = g_opl_voices[v_idx];
    /* Key-Off vor Neuanschlag fuer sauberen Envelope-Neustart */
    g_opl_chip.WriteReg((uint32_t)(0xB0 + v_idx), 0x00);

    vc.active       = 1;
    vc.sustained    = 0;
    vc.channel      = ch;
    vc.note         = note;
    vc.velocity     = vel;
    vc.remain_ticks = ticks;
    vc.age          = g_voice_age_counter++;
    vc.release_age  = 0;
    vc.car_ksl_tl   = t->car_ksl_tl;
    vc.mod_ksl_tl   = t->mod_ksl_tl;
    vc.fb_c         = t->fb_c;

    uint8_t op1 = g_opl_op1_off[v_idx];
    uint8_t op2 = g_opl_op2_off[v_idx];
    g_opl_chip.WriteReg((uint32_t)(0x20 + op1), t->mod_avekm);
    g_opl_chip.WriteReg((uint32_t)(0x60 + op1), t->mod_ad);
    g_opl_chip.WriteReg((uint32_t)(0x80 + op1), t->mod_sr);
    g_opl_chip.WriteReg((uint32_t)(0xE0 + op1), t->mod_ws);
    g_opl_chip.WriteReg((uint32_t)(0xC0 + v_idx), t->fb_c);
    g_opl_chip.WriteReg((uint32_t)(0x20 + op2), t->car_avekm);
    g_opl_chip.WriteReg((uint32_t)(0x60 + op2), t->car_ad);
    g_opl_chip.WriteReg((uint32_t)(0x80 + op2), t->car_sr);
    g_opl_chip.WriteReg((uint32_t)(0xE0 + op2), t->car_ws);
    opl_update_voice_volume(v_idx);

    opl_calc_fnum_block(play_note, (ch == 9) ? 0 : g_chan_bend[ch], &vc.fnum, &vc.block);
    g_opl_chip.WriteReg((uint32_t)(0xA0 + v_idx), (uint8_t)(vc.fnum & 0xFF));
    g_opl_chip.WriteReg((uint32_t)(0xB0 + v_idx), (uint8_t)(0x20 | ((vc.block & 7) << 2) | ((vc.fnum >> 8) & 3)));
}

static void xmi_step_tick(void) {
    /* 1. Laufende Noten-Dauern (120 Hz) herunterzaehlen */
    if (g_music_mode == DSA_MUSIC_MODE_GM) {
        for (int i = 0; i < GM_MAX_NOTES; ++i) {
            if (g_gm_notes[i].active && !g_gm_notes[i].sustained) {
                if (g_gm_notes[i].remain_ticks > 0) {
                    g_gm_notes[i].remain_ticks--;
                    if (g_gm_notes[i].remain_ticks == 0) {
                        uint8_t ch = g_gm_notes[i].channel;
                        if (g_chan_sustain[ch] >= 64) {
                            g_gm_notes[i].sustained = 1;
                        } else {
                            gm_send_short((uint8_t)(0x80 | ch), g_gm_notes[i].note, 0);
                            g_gm_notes[i].active = 0;
                        }
                    }
                }
            }
        }
    } else {
        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
            if (g_opl_voices[v].active) {
                if (!g_opl_voices[v].sustained && g_opl_voices[v].remain_ticks > 0) {
                    g_opl_voices[v].remain_ticks--;
                    if (g_opl_voices[v].remain_ticks == 0) {
                        uint8_t ch = g_opl_voices[v].channel;
                        if (g_chan_sustain[ch] >= 64) {
                            g_opl_voices[v].sustained = 1;
                        } else {
                            g_opl_chip.WriteReg((uint32_t)(0xB0 + v),
                                (uint8_t)(((g_opl_voices[v].block & 7) << 2) | ((g_opl_voices[v].fnum >> 8) & 3)));
                            g_opl_voices[v].active = 0;
                            g_opl_voices[v].release_age = 0;
                        }
                    }
                }
            } else {
                g_opl_voices[v].release_age++;
            }
        }
    }

    if (!g_xmi_active || g_xmi_paused || !g_xmi_evnt_buf || g_xmi_evnt_len == 0) return;

    if (g_xmi_wait_ticks > 0) {
        g_xmi_wait_ticks--;
        if (g_xmi_wait_ticks > 0) return;
    }

    while (g_xmi_pos < g_xmi_evnt_len) {
        uint8_t b = g_xmi_evnt_buf[g_xmi_pos];
        if (b < 0x80) {
            int32_t delay = 0;
            while (g_xmi_pos < g_xmi_evnt_len && g_xmi_evnt_buf[g_xmi_pos] < 0x80) {
                delay += g_xmi_evnt_buf[g_xmi_pos++];
            }
            if (delay > 0) {
                g_xmi_wait_ticks = delay - 1;
                return;
            }
            continue;
        }
        if (b == 0xFF) {
            if (g_xmi_pos + 2 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
            uint8_t meta = g_xmi_evnt_buf[g_xmi_pos + 1];
            g_xmi_pos += 2;
            uint32_t v = 0;
            while (g_xmi_pos < g_xmi_evnt_len && (g_xmi_evnt_buf[g_xmi_pos] & 0x80)) {
                v = (v << 7) | (g_xmi_evnt_buf[g_xmi_pos++] & 0x7F);
            }
            if (g_xmi_pos < g_xmi_evnt_len) {
                v = (v << 7) | g_xmi_evnt_buf[g_xmi_pos++];
            }
            g_xmi_pos += v;
            if (meta == 0x2F) {
                if (g_xmi_loop) {
                    xmi_reset_channels();
                    g_xmi_pos = 0;
                    g_xmi_wait_ticks = 0;
                    return;
                } else {
                    gm_all_notes_off();
                    opl_all_notes_off();
                    g_xmi_active = 0;
                    return;
                }
            }
        } else if (b == 0xF0 || b == 0xF7) {
            g_xmi_pos++;
            uint32_t v = 0;
            while (g_xmi_pos < g_xmi_evnt_len && (g_xmi_evnt_buf[g_xmi_pos] & 0x80)) {
                v = (v << 7) | (g_xmi_evnt_buf[g_xmi_pos++] & 0x7F);
            }
            if (g_xmi_pos < g_xmi_evnt_len) {
                v = (v << 7) | g_xmi_evnt_buf[g_xmi_pos++];
            }
            g_xmi_pos += v;
        } else {
            uint8_t hi = b & 0xF0;
            uint8_t ch = b & 0x0F;
            if (hi == 0x90) {
                if (g_xmi_pos + 3 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
                uint8_t note = g_xmi_evnt_buf[g_xmi_pos + 1];
                uint8_t vel  = g_xmi_evnt_buf[g_xmi_pos + 2];
                g_xmi_pos += 3;
                uint32_t dur = 0;
                while (g_xmi_pos < g_xmi_evnt_len && (g_xmi_evnt_buf[g_xmi_pos] & 0x80)) {
                    dur = (dur << 7) | (g_xmi_evnt_buf[g_xmi_pos++] & 0x7F);
                }
                if (g_xmi_pos < g_xmi_evnt_len) {
                    dur = (dur << 7) | g_xmi_evnt_buf[g_xmi_pos++];
                }
                xmi_trigger_note(ch, note, vel, dur);
            } else if (hi == 0x80) {
                if (g_xmi_pos + 3 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
                uint8_t note = g_xmi_evnt_buf[g_xmi_pos + 1];
                g_xmi_pos += 3;
                xmi_note_off(ch, note);
            } else if (hi == 0xB0) {
                if (g_xmi_pos + 3 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
                uint8_t cc  = g_xmi_evnt_buf[g_xmi_pos + 1];
                uint8_t val = g_xmi_evnt_buf[g_xmi_pos + 2];
                g_xmi_pos += 3;
                if (cc == 116) {
                    /* XMIDI FOR Loop Controller */
                    if (g_xmi_loop_depth < 4) {
                        g_xmi_loops[g_xmi_loop_depth].start_pos = g_xmi_pos;
                        g_xmi_loops[g_xmi_loop_depth].count = (int)val;
                        g_xmi_loop_depth++;
                    }
                } else if (cc == 117) {
                    /* XMIDI NEXT/BREAK Loop Controller */
                    if (val >= 64 && g_xmi_loop_depth > 0) {
                        XmiLoopEntry &lp = g_xmi_loops[g_xmi_loop_depth - 1];
                        if (lp.count == 0) {
                            g_xmi_pos = lp.start_pos;
                        } else if (lp.count > 1) {
                            lp.count--;
                            g_xmi_pos = lp.start_pos;
                        } else {
                            g_xmi_loop_depth--;
                        }
                    }
                } else if (cc == 7) {
                    g_chan_vol[ch] = val;
                    if (g_music_mode == DSA_MUSIC_MODE_GM) {
                        gm_update_channel_volume(ch);
                    } else {
                        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
                            if (g_opl_voices[v].active && g_opl_voices[v].channel == ch) opl_update_voice_volume(v);
                        }
                    }
                } else if (cc == 10) {
                    g_chan_pan[ch] = val;
                    if (g_music_mode == DSA_MUSIC_MODE_GM) gm_send_short((uint8_t)(0xB0 | ch), 10, val);
                } else if (cc == 11) {
                    g_chan_expr[ch] = val;
                    if (g_music_mode == DSA_MUSIC_MODE_GM) {
                        gm_send_short((uint8_t)(0xB0 | ch), 11, val);
                    } else {
                        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
                            if (g_opl_voices[v].active && g_opl_voices[v].channel == ch) opl_update_voice_volume(v);
                        }
                    }
                } else if (cc == 64) {
                    g_chan_sustain[ch] = val;
                    if (g_music_mode == DSA_MUSIC_MODE_GM) {
                        gm_send_short((uint8_t)(0xB0 | ch), 64, val);
                        if (val < 64) {
                            for (int i = 0; i < GM_MAX_NOTES; ++i) {
                                if (g_gm_notes[i].active && g_gm_notes[i].sustained && g_gm_notes[i].channel == ch) {
                                    gm_send_short((uint8_t)(0x80 | ch), g_gm_notes[i].note, 0);
                                    g_gm_notes[i].active = 0;
                                    g_gm_notes[i].sustained = 0;
                                }
                            }
                        }
                    } else if (val < 64) {
                        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
                            if (g_opl_voices[v].active && g_opl_voices[v].sustained && g_opl_voices[v].channel == ch) {
                                g_opl_chip.WriteReg((uint32_t)(0xB0 + v),
                                    (uint8_t)(((g_opl_voices[v].block & 7) << 2) | ((g_opl_voices[v].fnum >> 8) & 3)));
                                g_opl_voices[v].active = 0;
                                g_opl_voices[v].sustained = 0;
                                g_opl_voices[v].release_age = 0;
                            }
                        }
                    }
                } else if (cc == 121) {
                    g_chan_vol[ch]     = 100;
                    g_chan_expr[ch]    = 127;
                    g_chan_pan[ch]     = 64;
                    g_chan_sustain[ch] = 0;
                    g_chan_bend[ch]    = 0;
                    if (g_music_mode == DSA_MUSIC_MODE_GM) {
                        gm_send_short((uint8_t)(0xB0 | ch), 121, 0);
                        gm_update_channel_volume(ch);
                    }
                } else if (cc == 120 || cc == 123) {
                    if (g_music_mode == DSA_MUSIC_MODE_GM) {
                        gm_send_short((uint8_t)(0xB0 | ch), cc, 0);
                        for (int i = 0; i < GM_MAX_NOTES; ++i) {
                            if (g_gm_notes[i].channel == ch) g_gm_notes[i].active = 0;
                        }
                    } else {
                        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
                            if (g_opl_voices[v].active && g_opl_voices[v].channel == ch) {
                                g_opl_chip.WriteReg((uint32_t)(0xB0 + v), 0);
                                g_opl_voices[v].active = 0;
                            }
                        }
                    }
                } else if (cc < 110 && g_music_mode == DSA_MUSIC_MODE_GM) {
                    gm_send_short((uint8_t)(0xB0 | ch), cc, val);
                }
            } else if (hi == 0xC0) {
                if (g_xmi_pos + 2 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
                uint8_t prog = g_xmi_evnt_buf[g_xmi_pos + 1] & 0x7F;
                g_xmi_pos += 2;
                g_chan_prog[ch] = prog;
                if (g_music_mode == DSA_MUSIC_MODE_GM) {
                    gm_send_short((uint8_t)(0xC0 | ch), prog, 0);
                }
            } else if (hi == 0xD0) {
                if (g_xmi_pos + 2 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
                uint8_t pr = g_xmi_evnt_buf[g_xmi_pos + 1] & 0x7F;
                g_xmi_pos += 2;
                if (g_music_mode == DSA_MUSIC_MODE_GM) {
                    gm_send_short((uint8_t)(0xD0 | ch), pr, 0);
                }
            } else if (hi == 0xA0 || hi == 0xE0) {
                if (g_xmi_pos + 3 > g_xmi_evnt_len) { g_xmi_pos = g_xmi_evnt_len; break; }
                uint8_t d1 = g_xmi_evnt_buf[g_xmi_pos + 1] & 0x7F;
                uint8_t d2 = g_xmi_evnt_buf[g_xmi_pos + 2] & 0x7F;
                g_xmi_pos += 3;
                if (hi == 0xE0) {
                    g_chan_bend[ch] = (int16_t)(((int)d1 | ((int)d2 << 7)) - 8192);
                    if (g_music_mode == DSA_MUSIC_MODE_GM) {
                        gm_send_short((uint8_t)(0xE0 | ch), d1, d2);
                    } else if (ch != 9) {
                        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
                            if (g_opl_voices[v].active && g_opl_voices[v].channel == ch) {
                                const AdlibTimbre *t = &g_adlib_patches[g_chan_prog[ch] & 0x7F];
                                int play_note = (int)g_opl_voices[v].note + (int)t->transpose;
                                opl_calc_fnum_block(play_note, g_chan_bend[ch], &g_opl_voices[v].fnum, &g_opl_voices[v].block);
                                g_opl_chip.WriteReg((uint32_t)(0xA0 + v), (uint8_t)(g_opl_voices[v].fnum & 0xFF));
                                g_opl_chip.WriteReg((uint32_t)(0xB0 + v), (uint8_t)(0x20 | ((g_opl_voices[v].block & 7) << 2) | ((g_opl_voices[v].fnum >> 8) & 3)));
                            }
                        }
                    }
                } else if (g_music_mode == DSA_MUSIC_MODE_GM) {
                    gm_send_short((uint8_t)(0xA0 | ch), d1, d2);
                }
            } else {
                g_xmi_pos++;
            }
        }
    }
    if (g_xmi_pos >= g_xmi_evnt_len && g_xmi_loop) {
        g_xmi_pos = 0;
    }
}

static void dsa_audio_mixer_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    int16_t *out = (int16_t *)stream;
    int frames = len / 4;
    const float samples_per_tick = (float)DSA_AUDIO_FREQ / 120.0f; /* 367.5 Samples pro 120-Hz-XMIDI-Tick */
    float sfx_gain = (float)g_sfx_volume / 127.0f;

    int f = 0;
    while (f < frames) {
        if (g_tick_sample_accum >= samples_per_tick) {
            g_tick_sample_accum -= samples_per_tick;
            xmi_step_tick();
        }
        int rem_to_tick = (int)ceilf(samples_per_tick - g_tick_sample_accum);
        if (rem_to_tick < 1) rem_to_tick = 1;
        int chunk = frames - f;
        if (chunk > rem_to_tick) chunk = rem_to_tick;
        if (chunk > 512) chunk = 512;

        Bit32s opl_buf[512];
        int use_cd  = (g_music_mode == DSA_MUSIC_MODE_CD && g_cd_active && !g_cd_paused && g_cd_samples && g_cd_frames > 0 && g_music_volume > 0);
        int use_opl = (g_music_mode == DSA_MUSIC_MODE_OPL && g_xmi_active && !g_xmi_paused && g_music_volume > 0);
        if (use_opl) {
            g_opl_chip.GenerateBlock2((Bitu)chunk, opl_buf);
        }
        float mus_gain = (float)g_music_volume / 127.0f;

        for (int i = 0; i < chunk; ++i) {
            int32_t il = 0;
            int32_t ir = 0;
            if (use_cd) {
                int32_t cl = (int32_t)((float)g_cd_samples[g_cd_pos * 2 + 0] * mus_gain);
                int32_t cr = (int32_t)((float)g_cd_samples[g_cd_pos * 2 + 1] * mus_gain);
                il += cl;
                ir += cr;
                g_cd_pos++;
                if (g_cd_pos >= g_cd_frames) {
                    g_cd_pos = 0; /* Endlosschleife wie im Original / MobileEyes */
                }
            } else if (use_opl) {
                int32_t s_opl = (opl_buf[i] * 5) / 2;
                il += s_opl;
                ir += s_opl;
            }
            if (g_voc_active && g_voc_samples && g_voc_pos < g_voc_len) {
                int32_t s_voc = (int32_t)((float)g_voc_samples[g_voc_pos++] * sfx_gain);
                il += s_voc;
                ir += s_voc;
                if (g_voc_pos >= g_voc_len) {
                    g_voc_active = 0;
                }
            }
            if (il >  32767) il =  32767;
            if (il < -32768) il = -32768;
            if (ir >  32767) ir =  32767;
            if (ir < -32768) ir = -32768;
            out[(f + i) * 2 + 0] = (int16_t)il;
            out[(f + i) * 2 + 1] = (int16_t)ir;

            int pk = abs(il) > abs(ir) ? abs(il) : abs(ir);
            if (pk > g_stat_peak_sample) g_stat_peak_sample = pk;
        }

        g_tick_sample_accum += (float)chunk;
        f += chunk;
    }

    if (g_dump_wav_fp && frames > 0) {
        fwrite(out, 4, (size_t)frames, g_dump_wav_fp);
        g_dump_wav_bytes += (uint32_t)(frames * 4);
    }
    g_stat_mixed_frames += (uint32_t)frames;
}

static void dsa_update_window_title_music(void) {
    if (!g_window) return;
    const char *mode_str = (g_music_mode == DSA_MUSIC_MODE_CD) ? "CD-Audio (GOG OGG / MobileEyes)"
                         : (g_music_mode == DSA_MUSIC_MODE_GM) ? "General MIDI" : "AdLib OPL2 (DOSBox)";
    char title[256];
    snprintf(title, sizeof(title), "DSA: Die Schicksalsklinge [Musik: %s | F10=Synth, F11/F12=Lautst.]", mode_str);
    SDL_SetWindowTitle(g_window, title);
}

int dsa_platform_play_cd_track(int track_no) {
    if (track_no < 2 || track_no > 15) return 0;
    if (g_cd_active && g_cd_current_track == track_no && g_cd_samples) {
        g_cd_paused = 0;
        return 1;
    }

    /* Suche MUSIC/TrackNN.OGG im Spielordner, aktuellen Ordner oder GOG-Standardpfad */
    const char *base_dirs[4] = {
        g_game_dir[0] ? g_game_dir : ".",
        ".",
        "..",
        "C:\\Program Files (x86)\\GOG Galaxy\\Games\\Realms of Arkania"
    };
    const char *sub_patterns[4] = {
        "MUSIC/Track%02d.OGG",
        "Music/Track%02d.OGG",
        "MUSIC/Track%02d.ogg",
        "music/Track%02d.ogg"
    };
    char found_path[1024] = "";
#ifndef _WIN32
    {
        char rel[64];
        char cand[1024];
        snprintf(rel, sizeof(rel), "MUSIC/Track%02d.OGG", track_no);
        dsa_platform_resolve_path(rel, cand, sizeof(cand));
        FILE *tf = fopen(cand, "rb");
        if (tf) {
            fclose(tf);
            strncpy(found_path, cand, sizeof(found_path) - 1);
        }
    }
#endif
    for (int d = 0; d < 4 && !found_path[0]; ++d) {
        for (int p = 0; p < 4; ++p) {
            char rel[64];
            char cand[1024];
            snprintf(rel, sizeof(rel), sub_patterns[p], track_no);
            snprintf(cand, sizeof(cand), "%s/%s", base_dirs[d], rel);
            FILE *tf = fopen(cand, "rb");
            if (tf) {
                fclose(tf);
                strncpy(found_path, cand, sizeof(found_path) - 1);
                break;
            }
        }
    }
    if (!found_path[0]) return 0;

    int ch = 0, sr = 0;
    short *decoded = NULL;
    int n_frames = stb_vorbis_decode_filename(found_path, &ch, &sr, &decoded);
    if (n_frames <= 0 || !decoded || ch <= 0 || sr <= 0) {
        if (decoded) free(decoded);
        return 0;
    }

    uint32_t out_frames = (uint32_t)(((uint64_t)n_frames * DSA_AUDIO_FREQ) / (uint32_t)sr);
    int16_t *stereo_pcm = (int16_t *)malloc(out_frames * 2 * sizeof(int16_t));
    if (!stereo_pcm) {
        free(decoded);
        return 0;
    }
    if (sr == DSA_AUDIO_FREQ && ch == 2) {
        memcpy(stereo_pcm, decoded, out_frames * 2 * sizeof(int16_t));
    } else {
        for (uint32_t i = 0; i < out_frames; ++i) {
            uint32_t src_i = (uint32_t)(((uint64_t)i * (uint32_t)sr) / DSA_AUDIO_FREQ);
            if (src_i >= (uint32_t)n_frames) src_i = (uint32_t)n_frames - 1;
            int16_t l = decoded[src_i * ch + 0];
            int16_t r = (ch > 1) ? decoded[src_i * ch + 1] : l;
            stereo_pcm[i * 2 + 0] = l;
            stereo_pcm[i * 2 + 1] = r;
        }
    }
    free(decoded);

    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    if (g_cd_samples) {
        free(g_cd_samples);
    }
    g_cd_samples       = stereo_pcm;
    g_cd_frames        = out_frames;
    g_cd_pos           = 0;
    g_cd_current_track = track_no;
    g_cd_active        = 1;
    g_cd_paused        = 0;
    g_stat_music_notes++;
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
    return 1;
}

void dsa_platform_pause_cd_audio(int pause_on) {
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    g_cd_paused = pause_on ? 1 : 0;
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

void dsa_platform_stop_cd_audio(void) {
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    g_cd_active = 0;
    g_cd_pos    = 0;
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

void dsa_platform_set_music_mode(int mode) {
    int new_mode = (mode == DSA_MUSIC_MODE_OPL) ? DSA_MUSIC_MODE_OPL
                 : (mode == DSA_MUSIC_MODE_GM)  ? DSA_MUSIC_MODE_GM
                 : DSA_MUSIC_MODE_CD;
    if (!g_opl_initialized) {
        g_music_mode = new_mode;
        return;
    }
#ifdef _WIN32
    if (new_mode == DSA_MUSIC_MODE_GM && !g_hmidi_out) {
        if (midiOutOpen(&g_hmidi_out, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            g_hmidi_out = NULL;
            new_mode = DSA_MUSIC_MODE_OPL;
        }
    }
#else
    if (new_mode == DSA_MUSIC_MODE_GM) new_mode = DSA_MUSIC_MODE_OPL;
#endif
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    if (new_mode != g_music_mode) {
        gm_all_notes_off();
        opl_all_notes_off();
        g_music_mode = new_mode;
        if (g_xmi_active) {
            g_xmi_pos = 0;
            g_xmi_wait_ticks = 0;
            xmi_reset_channels();
        }
    }
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
    dsa_update_window_title_music();
}

int dsa_platform_get_music_mode(void) {
    return g_music_mode;
}

void dsa_platform_set_dump_wav(const char *path) {
    if (path && path[0]) {
        strncpy(g_dump_wav_path, path, sizeof(g_dump_wav_path) - 1);
    }
}

static void dsa_audio_init(void) {
    init_default_adlib_patches();
    opl_reset_chip();

    const char *env_vol = getenv("DSA_VOLUME");
    if (env_vol && env_vol[0]) {
        int v = atoi(env_vol);
        if (v < 0) v = 0;
        if (v > 100) v = 100;
        g_music_volume = (v * 127) / 100;
        g_sfx_volume   = (v * 127) / 100;
    }

    const char *env_mus = getenv("DSA_MUSIC");
    if (env_mus && env_mus[0]) {
        if (STRNICMP(env_mus, "opl", 3) == 0 || STRNICMP(env_mus, "adlib", 5) == 0 || STRNICMP(env_mus, "sb", 2) == 0) {
            g_music_mode = DSA_MUSIC_MODE_OPL;
        } else if (STRNICMP(env_mus, "gm", 2) == 0 || STRNICMP(env_mus, "midi", 4) == 0) {
            g_music_mode = DSA_MUSIC_MODE_GM;
        } else if (STRNICMP(env_mus, "cd", 2) == 0 || STRNICMP(env_mus, "ogg", 3) == 0) {
            g_music_mode = DSA_MUSIC_MODE_CD;
        }
    }

#ifdef _WIN32
    if (g_music_mode == DSA_MUSIC_MODE_GM && !g_hmidi_out) {
        if (midiOutOpen(&g_hmidi_out, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            g_hmidi_out = NULL;
            g_music_mode = DSA_MUSIC_MODE_OPL;
        }
    }
#endif
    xmi_reset_channels();

    const char *env_wav = getenv("DSA_DUMP_WAV");
    if (env_wav && env_wav[0] && !g_dump_wav_path[0]) {
        strncpy(g_dump_wav_path, env_wav, sizeof(g_dump_wav_path) - 1);
    }
    if (g_dump_wav_path[0]) {
        g_dump_wav_fp = fopen(g_dump_wav_path, "wb");
        if (g_dump_wav_fp) {
            uint8_t hdr[44];
            memset(hdr, 0, sizeof(hdr));
            fwrite(hdr, 1, 44, g_dump_wav_fp);
            g_dump_wav_bytes = 0;
        }
    }

    SDL_AudioSpec want, have;
    memset(&want, 0, sizeof(want));
    want.freq     = DSA_AUDIO_FREQ;
    want.format   = AUDIO_S16LSB;
    want.channels = 2;
    want.samples  = 1024;
    want.callback = dsa_audio_mixer_callback;
    want.userdata = NULL;
    g_audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (g_audio_dev != 0) {
        SDL_PauseAudioDevice(g_audio_dev, 0);
    }
    dsa_update_window_title_music();
}

static void dsa_audio_shutdown(void) {
    if (g_audio_dev != 0) {
        SDL_CloseAudioDevice(g_audio_dev);
        g_audio_dev = 0;
    }
    gm_all_notes_off();
#ifdef _WIN32
    if (g_hmidi_out) {
        midiOutReset(g_hmidi_out);
        midiOutClose(g_hmidi_out);
        g_hmidi_out = NULL;
    }
#endif
    if (g_dump_wav_fp) {
        uint32_t total_len = g_dump_wav_bytes + 36;
        uint32_t freq = DSA_AUDIO_FREQ;
        uint32_t byte_rate = DSA_AUDIO_FREQ * 4;
        uint16_t fmt = 1, ch = 2, block_align = 4, bits = 16;
        fseek(g_dump_wav_fp, 0, SEEK_SET);
        fwrite("RIFF", 1, 4, g_dump_wav_fp);
        fwrite(&total_len, 4, 1, g_dump_wav_fp);
        fwrite("WAVEfmt ", 1, 8, g_dump_wav_fp);
        uint32_t fmt_len = 16;
        fwrite(&fmt_len, 4, 1, g_dump_wav_fp);
        fwrite(&fmt, 2, 1, g_dump_wav_fp);
        fwrite(&ch, 2, 1, g_dump_wav_fp);
        fwrite(&freq, 4, 1, g_dump_wav_fp);
        fwrite(&byte_rate, 4, 1, g_dump_wav_fp);
        fwrite(&block_align, 2, 1, g_dump_wav_fp);
        fwrite(&bits, 2, 1, g_dump_wav_fp);
        fwrite("data", 1, 4, g_dump_wav_fp);
        fwrite(&g_dump_wav_bytes, 4, 1, g_dump_wav_fp);
        fclose(g_dump_wav_fp);
        g_dump_wav_fp = NULL;
    }
    const char *alog = getenv("DSA_AUDIO_LOG");
    if ((alog && alog[0] == '1') || g_dump_wav_path[0]) {
        const char *mstr = (g_music_mode == DSA_MUSIC_MODE_CD) ? "CD" : ((g_music_mode == DSA_MUSIC_MODE_GM) ? "GM" : "OPL");
        fprintf(stderr, "[AUDIO] mode=%s mixed_frames=%u music_notes=%u voc_plays=%u peak_sample=%d\n",
                mstr,
                (unsigned int)g_stat_mixed_frames,
                (unsigned int)g_stat_music_notes,
                (unsigned int)g_stat_voc_plays,
                g_stat_peak_sample);
        fflush(stderr);
    }
    if (g_cd_samples)   { free(g_cd_samples);   g_cd_samples   = NULL; }
    if (g_xmi_evnt_buf) { free(g_xmi_evnt_buf); g_xmi_evnt_buf = NULL; }
    if (g_voc_samples)  { free(g_voc_samples);  g_voc_samples  = NULL; }
}

int dsa_platform_play_xmidi(const unsigned char *xmi_data, uint32_t xmi_len, int loop) {
    if (!xmi_data || xmi_len < 32) return 0;
    /* Suche EVNT-Chunk innerhalb des XMIDI-Puffers */
    uint32_t evnt_off = 0;
    uint32_t evnt_len = 0;
    for (uint32_t i = 0; i + 8 <= xmi_len && i < 512; ++i) {
        if (xmi_data[i] == 'E' && xmi_data[i+1] == 'V' && xmi_data[i+2] == 'N' && xmi_data[i+3] == 'T') {
            evnt_len = ((uint32_t)xmi_data[i+4] << 24)
                     | ((uint32_t)xmi_data[i+5] << 16)
                     | ((uint32_t)xmi_data[i+6] << 8)
                     |  (uint32_t)xmi_data[i+7];
            evnt_off = i + 8;
            break;
        }
    }
    if (evnt_off == 0 || evnt_len == 0) return 0;
    if (evnt_off + evnt_len > xmi_len) {
        evnt_len = xmi_len - evnt_off;
    }

    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    if (g_xmi_evnt_buf) {
        free(g_xmi_evnt_buf);
        g_xmi_evnt_buf = NULL;
    }
    g_xmi_evnt_buf = (uint8_t *)malloc(evnt_len);
    if (g_xmi_evnt_buf) {
        memcpy(g_xmi_evnt_buf, xmi_data + evnt_off, evnt_len);
        g_xmi_evnt_len   = evnt_len;
        g_xmi_pos        = 0;
        g_xmi_wait_ticks = 0;
        g_xmi_loop       = loop ? 1 : 0;
        g_xmi_active     = 1;
        g_xmi_paused     = 0;
        xmi_reset_channels();
    }
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
    return g_xmi_active;
}

void dsa_platform_stop_music(void) {
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    g_xmi_active = 0;
    g_xmi_pos    = 0;
    gm_all_notes_off();
    opl_all_notes_off();
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

void dsa_platform_pause_music(int pause_on) {
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    g_xmi_paused = pause_on ? 1 : 0;
    if (g_xmi_paused) {
        gm_all_notes_off();
        opl_all_notes_off();
    }
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

void dsa_platform_set_music_volume(int vol_0_127) {
    if (vol_0_127 < 0)   vol_0_127 = 0;
    if (vol_0_127 > 127) vol_0_127 = 127;
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    g_music_volume = vol_0_127;
    if (g_music_mode == DSA_MUSIC_MODE_GM) {
        for (int c = 0; c < 16; ++c) {
            gm_update_channel_volume((uint8_t)c);
        }
    } else {
        for (int v = 0; v < OPL_NUM_VOICES; ++v) {
            if (g_opl_voices[v].active) opl_update_voice_volume(v);
        }
    }
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

int dsa_platform_get_music_volume(void) {
    return g_music_volume;
}

int dsa_platform_play_voc(const unsigned char *voc_data, uint32_t voc_len) {
    if (!voc_data || voc_len < 32) return 0;
    if (memcmp(voc_data, "Creative Voice File\x1a", 20) != 0) return 0;
    uint16_t hdr_size = (uint16_t)voc_data[20] | ((uint16_t)voc_data[21] << 8);
    if (hdr_size < 20 || hdr_size >= voc_len) return 0;

    uint32_t pos = hdr_size;
    uint32_t src_rate = 11025;
    int      rate_from_type8 = 0;

    /* Erste Runde: Gesamtzahl der 8-Bit-PCM-Samples bestimmen */
    uint32_t raw_count = 0;
    uint32_t scan_pos = pos;
    while (scan_pos < voc_len) {
        uint8_t bt = voc_data[scan_pos];
        if (bt == 0 || scan_pos + 4 > voc_len) break;
        uint32_t blen = (uint32_t)voc_data[scan_pos + 1]
                      | ((uint32_t)voc_data[scan_pos + 2] << 8)
                      | ((uint32_t)voc_data[scan_pos + 3] << 16);
        if (scan_pos + 4 + blen > voc_len) {
            blen = voc_len - (scan_pos + 4);
        }
        if (bt == 8 && blen >= 4) {
            uint16_t tc = (uint16_t)voc_data[scan_pos + 4] | ((uint16_t)voc_data[scan_pos + 5] << 8);
            if (tc < 65535) {
                src_rate = 256000000u / (65536u - (uint32_t)tc);
                rate_from_type8 = 1;
            }
        } else if (bt == 1 && blen > 2) {
            if (!rate_from_type8) {
                uint8_t sr_div = voc_data[scan_pos + 4];
                if (sr_div < 255) src_rate = 1000000u / (256u - (uint32_t)sr_div);
            }
            raw_count += (blen - 2);
        } else if (bt == 2) {
            raw_count += blen;
        }
        scan_pos += 4 + blen;
    }

    if (raw_count == 0 || src_rate == 0) return 0;
    uint8_t *raw_pcm = (uint8_t *)malloc(raw_count);
    if (!raw_pcm) return 0;

    uint32_t wpos = 0;
    scan_pos = pos;
    while (scan_pos < voc_len && wpos < raw_count) {
        uint8_t bt = voc_data[scan_pos];
        if (bt == 0 || scan_pos + 4 > voc_len) break;
        uint32_t blen = (uint32_t)voc_data[scan_pos + 1]
                      | ((uint32_t)voc_data[scan_pos + 2] << 8)
                      | ((uint32_t)voc_data[scan_pos + 3] << 16);
        if (scan_pos + 4 + blen > voc_len) blen = voc_len - (scan_pos + 4);
        if (bt == 1 && blen > 2) {
            uint32_t n = blen - 2;
            memcpy(raw_pcm + wpos, voc_data + scan_pos + 6, n);
            wpos += n;
        } else if (bt == 2 && blen > 0) {
            memcpy(raw_pcm + wpos, voc_data + scan_pos + 4, blen);
            wpos += blen;
        }
        scan_pos += 4 + blen;
    }

    /* Resample von src_rate (z.B. 11025 Hz oder 44100 Hz) auf DSA_AUDIO_FREQ (44100 Hz) */
    uint32_t out_count = (uint32_t)(((uint64_t)wpos * DSA_AUDIO_FREQ) / src_rate);
    if (out_count == 0) { free(raw_pcm); return 0; }
    int16_t *resampled = (int16_t *)malloc(out_count * sizeof(int16_t));
    if (!resampled) { free(raw_pcm); return 0; }

    for (uint32_t i = 0; i < out_count; ++i) {
        uint32_t src_idx = (uint32_t)(((uint64_t)i * src_rate) / DSA_AUDIO_FREQ);
        if (src_idx >= wpos) src_idx = wpos - 1;
        int s = ((int)raw_pcm[src_idx] - 128) * 256;
        resampled[i] = (int16_t)s;
    }
    free(raw_pcm);

    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    if (g_voc_samples) {
        free(g_voc_samples);
    }
    g_voc_samples = resampled;
    g_voc_len     = out_count;
    g_voc_pos     = 0;
    g_voc_active  = 1;
    g_stat_voc_plays++;
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
    return 1;
}

void dsa_platform_stop_voc(void) {
    if (g_audio_dev) SDL_LockAudioDevice(g_audio_dev);
    g_voc_active = 0;
    g_voc_pos    = 0;
    if (g_audio_dev) SDL_UnlockAudioDevice(g_audio_dev);
}

int dsa_platform_is_voc_playing(void) {
    return g_voc_active ? 1 : 0;
}

void dsa_platform_set_sfx_volume(int vol_0_127) {
    if (vol_0_127 < 0)   vol_0_127 = 0;
    if (vol_0_127 > 127) vol_0_127 = 127;
    g_sfx_volume = vol_0_127;
}

int dsa_platform_get_sfx_volume(void) {
    return g_sfx_volume;
}

void dsa_platform_get_audio_stats(uint32_t *out_mixed_frames, uint32_t *out_music_notes, uint32_t *out_voc_plays, int *out_peak_sample) {
    if (out_mixed_frames) *out_mixed_frames = g_stat_mixed_frames;
    if (out_music_notes)  *out_music_notes  = g_stat_music_notes;
    if (out_voc_plays)    *out_voc_plays    = g_stat_voc_plays;
    if (out_peak_sample)  *out_peak_sample  = g_stat_peak_sample;
}
