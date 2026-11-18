/*
 * sdl2_win32_provider.c - Dynamischer SDL2.dll-Lader mit eingebautem Win32-Fallback
 *
 * Wenn eine 32-Bit-SDL2.dll im Programmverzeichnis oder Systempfad liegt, werden
 * alle SDL_*-Aufrufe direkt an SDL2.dll delegiert. Falls keine SDL2.dll vorhanden
 * ist, stellt dieses Modul dieselbe SDL2-API direkt ueber Win32 (user32/gdi32/winmm)
 * bereit, sodass die Port-EXE ohne externe Abhaengigkeiten startet.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"

static DWORD g_autoquit_ms = 0;

static int dsa_is_headless_run(void) {
    if ((getenv("SCHICK_PORT_AUTOTEST") && getenv("SCHICK_PORT_AUTOTEST")[0] == '1') ||
        (getenv("RIVA_PORT_AUTOTEST") && getenv("RIVA_PORT_AUTOTEST")[0] == '1') ||
        (getenv("DSA_HIDDEN") && getenv("DSA_HIDDEN")[0] == '1') ||
        (getenv("SCHICK_PORT_HIDDEN") && getenv("SCHICK_PORT_HIDDEN")[0] == '1')) {
        return 1;
    }
    const char *cmd = GetCommandLineA();
    if (cmd && (strstr(cmd, "--replay") != NULL || strstr(cmd, "--headless") != NULL)) {
        return 1;
    }
    return 0;
}

static DWORD WINAPI dsa_autoquit_thread(LPVOID param) {
    DWORD ms = (DWORD)(uintptr_t)param;
    Sleep(ms);
    ExitProcess(0);
    return 0;
}

static LONG WINAPI dsa_seh_handler(EXCEPTION_POINTERS *ep) {
    const char *cf = getenv("SCHICK_PORT_CRASH_FILE");
    if (!cf) cf = getenv("RIVA_PORT_CRASH_FILE");
    FILE *fp = fopen(cf ? cf : "C:\\Windows\\Temp\\schick_port_build\\schick_port_crash.log", "w");
    if (fp && ep && ep->ExceptionRecord && ep->ContextRecord) {
        CONTEXT *c = ep->ContextRecord;
        fprintf(
            fp,
            "CRASH code=0x%08lX addr=%p EIP=0x%08lX EAX=0x%08lX EBX=0x%08lX ECX=0x%08lX EDX=0x%08lX ESI=0x%08lX EDI=0x%08lX EBP=0x%08lX ESP=0x%08lX\n",
            (unsigned long)ep->ExceptionRecord->ExceptionCode,
            ep->ExceptionRecord->ExceptionAddress,
            (unsigned long)c->Eip,
            (unsigned long)c->Eax,
            (unsigned long)c->Ebx,
            (unsigned long)c->Ecx,
            (unsigned long)c->Edx,
            (unsigned long)c->Esi,
            (unsigned long)c->Edi,
            (unsigned long)c->Ebp,
            (unsigned long)c->Esp
        );
        if (c->Esp > 0x10000) {
            unsigned long *sp = (unsigned long *)(uintptr_t)c->Esp;
            int k;
            fprintf(fp, "Stack:");
            for (k = 0; k < 256; ++k) {
                if (sp[k] >= 0x00401000 && sp[k] < 0x005B9000) fprintf(fp, " [+%d]=%08lX", k*4, sp[k]);
            }
            fprintf(fp, "\n");
        }
        fflush(fp);
        fclose(fp);
    }
    ExitProcess(1);
    return EXCEPTION_EXECUTE_HANDLER;
}

/* Funktionszeiger fuer dynamisch geladene SDL2.dll */
static HMODULE g_sdl_dll = NULL;
static int     g_using_dll = 0;

static int           (__cdecl *p_SDL_Init)(Uint32) = NULL;
static void          (__cdecl *p_SDL_Quit)(void) = NULL;
static const char *  (__cdecl *p_SDL_GetError)(void) = NULL;
static SDL_Window *  (__cdecl *p_SDL_CreateWindow)(const char *, int, int, int, int, Uint32) = NULL;
static void          (__cdecl *p_SDL_DestroyWindow)(SDL_Window *) = NULL;
static void          (__cdecl *p_SDL_SetWindowTitle)(SDL_Window *, const char *) = NULL;
static void          (__cdecl *p_SDL_SetWindowSize)(SDL_Window *, int, int) = NULL;
static void          (__cdecl *p_SDL_GetWindowSize)(SDL_Window *, int *, int *) = NULL;
static int           (__cdecl *p_SDL_SetWindowFullscreen)(SDL_Window *, Uint32) = NULL;
static SDL_Renderer *(__cdecl *p_SDL_CreateRenderer)(SDL_Window *, int, Uint32) = NULL;
static void          (__cdecl *p_SDL_DestroyRenderer)(SDL_Renderer *) = NULL;
static int           (__cdecl *p_SDL_RenderSetLogicalSize)(SDL_Renderer *, int, int) = NULL;
static int           (__cdecl *p_SDL_RenderClear)(SDL_Renderer *) = NULL;
static int           (__cdecl *p_SDL_RenderCopy)(SDL_Renderer *, SDL_Texture *, const SDL_Rect *, const SDL_Rect *) = NULL;
static void          (__cdecl *p_SDL_RenderPresent)(SDL_Renderer *) = NULL;
static SDL_Texture * (__cdecl *p_SDL_CreateTexture)(SDL_Renderer *, Uint32, int, int, int) = NULL;
static void          (__cdecl *p_SDL_DestroyTexture)(SDL_Texture *) = NULL;
static int           (__cdecl *p_SDL_UpdateTexture)(SDL_Texture *, const SDL_Rect *, const void *, int) = NULL;
static int           (__cdecl *p_SDL_PollEvent)(SDL_Event *) = NULL;
static Uint32        (__cdecl *p_SDL_GetMouseState)(int *, int *) = NULL;
static void          (__cdecl *p_SDL_WarpMouseInWindow)(SDL_Window *, int, int) = NULL;
static int           (__cdecl *p_SDL_ShowCursor)(int) = NULL;
static Uint32        (__cdecl *p_SDL_GetTicks)(void) = NULL;
static void          (__cdecl *p_SDL_Delay)(Uint32) = NULL;
static SDL_AudioDeviceID (__cdecl *p_SDL_OpenAudioDevice)(const char *, int, const SDL_AudioSpec *, SDL_AudioSpec *, int) = NULL;
static void          (__cdecl *p_SDL_CloseAudioDevice)(SDL_AudioDeviceID) = NULL;
static void          (__cdecl *p_SDL_PauseAudioDevice)(SDL_AudioDeviceID, int) = NULL;
static void          (__cdecl *p_SDL_LockAudioDevice)(SDL_AudioDeviceID) = NULL;
static void          (__cdecl *p_SDL_UnlockAudioDevice)(SDL_AudioDeviceID) = NULL;

/* =========================================================================
 * Eingebauter Win32-Provider (falls keine SDL2.dll geladen wurde)
 * ========================================================================= */

struct SDL_Window {
    HWND hwnd;
    HDC  hdc;
    int  win_w;
    int  win_h;
    int  saved_win_w;
    int  saved_win_h;
    int  logical_w;
    int  logical_h;
    int  vp_x;
    int  vp_y;
    int  vp_w;
    int  vp_h;
    int  fullscreen;
};

static SDL_Window *g_main_win = NULL;
static int g_fs_override_w = 0;
static int g_fs_override_h = 0;

void SDL_SetFullscreenOverrideRes(int w, int h) {
    g_fs_override_w = w;
    g_fs_override_h = h;
}

void SDL_SetWindowViewport(SDL_Window *window, int vx, int vy, int vw, int vh) {
    if (!window) window = g_main_win;
    if (window) {
        window->vp_x = vx;
        window->vp_y = vy;
        window->vp_w = vw;
        window->vp_h = vh;
    }
}

struct SDL_Renderer {
    SDL_Window  *window;
    SDL_Texture *active_tex;
    int          logical_w;
    int          logical_h;
};

struct SDL_Texture {
    int        w;
    int        h;
    uint32_t  *pixels;
    BITMAPINFO bmi;
};

#define EVQ_CAP 256
static SDL_Event   g_evq[EVQ_CAP];
static int         g_evq_head = 0;
static int         g_evq_tail = 0;
static int         g_mouse_x = 0;
static int         g_mouse_y = 0;
static Uint32      g_mouse_buttons = 0;
static int         g_show_cursor = 1;
static DWORD       g_start_ticks = 0;

static void evq_push(const SDL_Event *ev) {
    int next = (g_evq_tail + 1) % EVQ_CAP;
    if (next != g_evq_head) {
        g_evq[g_evq_tail] = *ev;
        g_evq_tail = next;
    }
}

static void scale_client_to_logical(HWND hwnd, int cx, int cy, int *lx, int *ly) {
    RECT rc;
    int cw, ch, log_w, log_h, vx, vy, vw, vh;
    GetClientRect(hwnd, &rc);
    cw = rc.right - rc.left;
    ch = rc.bottom - rc.top;
    log_w = (g_main_win && g_main_win->logical_w > 0) ? g_main_win->logical_w : 320;
    log_h = (g_main_win && g_main_win->logical_h > 0) ? g_main_win->logical_h : 200;
    if (cw <= 0) cw = 1;
    if (ch <= 0) ch = 1;

    if (g_main_win && g_main_win->vp_w > 0 && g_main_win->vp_h > 0) {
        vx = g_main_win->vp_x;
        vy = g_main_win->vp_y;
        vw = g_main_win->vp_w;
        vh = g_main_win->vp_h;
    } else {
        /* 4:3 Standard-Viewport in (cw, ch) */
        if (cw * 3 <= ch * 4) {
            vw = cw;
            vh = (cw * 3 + 2) / 4;
        } else {
            vh = ch;
            vw = (ch * 4 + 1) / 3;
        }
        if (vw <= 0) vw = 1;
        if (vh <= 0) vh = 1;
        vx = (cw - vw) / 2;
        vy = (ch - vh) / 2;
    }

    *lx = ((cx - vx) * log_w) / vw;
    *ly = ((cy - vy) * log_h) / vh;
    if (*lx < 0) *lx = 0;
    if (*ly < 0) *ly = 0;
    if (*lx >= log_w) *lx = log_w - 1;
    if (*ly >= log_h) *ly = log_h - 1;
}

static Uint16 get_win32_kmod(void) {
    Uint16 mod = KMOD_NONE;
    if (GetKeyState(VK_LSHIFT) & 0x8000)   mod |= KMOD_LSHIFT;
    if (GetKeyState(VK_RSHIFT) & 0x8000)   mod |= KMOD_RSHIFT;
    if (GetKeyState(VK_LCONTROL) & 0x8000) mod |= KMOD_LCTRL;
    if (GetKeyState(VK_RCONTROL) & 0x8000) mod |= KMOD_RCTRL;
    if (GetKeyState(VK_LMENU) & 0x8000)    mod |= KMOD_LALT;
    if (GetKeyState(VK_RMENU) & 0x8000)    mod |= KMOD_RALT;
    return mod;
}

static LRESULT CALLBACK dsa_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SDL_Event ev;
    memset(&ev, 0, sizeof(ev));
    ev.key.timestamp = GetTickCount() - g_start_ticks;

    switch (msg) {
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && !g_show_cursor) {
            SetCursor(NULL);
            return TRUE;
        }
        break;

    case WM_MOUSEMOVE: {
        int lx, ly;
        scale_client_to_logical(hwnd, (short)LOWORD(lParam), (short)HIWORD(lParam), &lx, &ly);
        ev.type = SDL_MOUSEMOTION;
        ev.motion.x = lx;
        ev.motion.y = ly;
        ev.motion.xrel = lx - g_mouse_x;
        ev.motion.yrel = ly - g_mouse_y;
        ev.motion.state = g_mouse_buttons;
        g_mouse_x = lx;
        g_mouse_y = ly;
        evq_push(&ev);
        return 0;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP: {
        int lx, ly;
        Uint8 btn = SDL_BUTTON_LEFT;
        int down = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN);
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) btn = SDL_BUTTON_RIGHT;
        else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) btn = SDL_BUTTON_MIDDLE;

        scale_client_to_logical(hwnd, (short)LOWORD(lParam), (short)HIWORD(lParam), &lx, &ly);
        g_mouse_x = lx;
        g_mouse_y = ly;
        if (down) g_mouse_buttons |= (1u << (btn - 1));
        else      g_mouse_buttons &= ~(1u << (btn - 1));

        ev.type = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
        ev.button.button = btn;
        ev.button.state = down ? 1 : 0;
        ev.button.clicks = 1;
        ev.button.x = lx;
        ev.button.y = ly;
        evq_push(&ev);
        return 0;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        int down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        int dos_scan = (int)((lParam >> 16) & 0x7F);
        BYTE kb_state[256];
        WORD ascii_word = 0;
        int ascii_len = 0;
        unsigned char ascii_ch = 0;

        if (GetKeyboardState(kb_state)) {
            ascii_len = ToAscii((UINT)wParam, (UINT)((lParam >> 16) & 0xFF), kb_state, &ascii_word, 0);
            if (ascii_len == 1) {
                ascii_ch = (unsigned char)(ascii_word & 0xFF);
            }
        }
        /* Sondertasten wie Pfeiltasten, Entf, Pos1, Ende, BildAuf/Ab haben im BIOS ASCII 0 */
        if ((lParam & (1 << 24)) && wParam != VK_RETURN && wParam != VK_DIVIDE) {
            ascii_ch = 0;
        }

        ev.type = down ? SDL_KEYDOWN : SDL_KEYUP;
        ev.key.state = down ? 1 : 0;
        ev.key.repeat = (down && (lParam & (1 << 30))) ? 1 : 0;
        ev.key.keysym.scancode = -dos_scan; /* Negativ markiert direkten DOS-Set-1-Scancode */
        ev.key.keysym.sym = (SDL_Keycode)wParam;
        ev.key.keysym.mod = get_win32_kmod();
        if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP || (lParam & (1 << 29))) {
            ev.key.keysym.mod |= KMOD_LALT;
        }
        ev.key.keysym.unused = ((Uint32)(dos_scan & 0xFF) << 8) | (Uint32)ascii_ch;
        evq_push(&ev);
        return 0;
    }

    case WM_SYSCHAR:
        /* Verhindert System-Signalton bei Alt+Enter */
        if (wParam == VK_RETURN) return 0;
        break;

    case WM_ACTIVATEAPP:
        /* Im echten Vollbild: Bei Fokusverlust (Alt+Tab) Topmost freigeben und minimieren,
         * bei Reaktivierung sofort wieder rahmenlos ueber den gesamten Monitor inkl. Taskleiste legen. */
        if (g_main_win && g_main_win->hwnd == hwnd && g_main_win->fullscreen) {
            int is_at = dsa_is_headless_run();
            if (!is_at) {
                if (wParam) {
                    int sw = g_main_win->win_w > 0 ? g_main_win->win_w : GetSystemMetrics(SM_CXSCREEN);
                    int sh = g_main_win->win_h > 0 ? g_main_win->win_h : GetSystemMetrics(SM_CYSCREEN);
                    SetWindowLongA(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, sw, sh, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
                } else {
                    SetWindowLongA(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
                    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    ShowWindow(hwnd, SW_MINIMIZE);
                }
            }
        }
        break;

    case WM_CLOSE:
    case WM_DESTROY:
        ev.type = SDL_QUIT;
        evq_push(&ev);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void enable_win32_dpi_awareness(void) {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *PFN_SetProcessDpiAwarenessContext)(HANDLE);
        typedef BOOL (WINAPI *PFN_SetProcessDPIAware)(void);
        PFN_SetProcessDpiAwarenessContext pCtx =
            (PFN_SetProcessDpiAwarenessContext)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pCtx) {
            /* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = (HANDLE)-4 */
            if (pCtx((HANDLE)(intptr_t)-4)) return;
            /* DPI_AWARENESS_CONTEXT_SYSTEM_AWARE = (HANDLE)-2 */
            if (pCtx((HANDLE)(intptr_t)-2)) return;
        }
        PFN_SetProcessDPIAware pAware =
            (PFN_SetProcessDPIAware)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pAware) {
            pAware();
        }
    }
}

static void query_fullscreen_monitor_rect(HWND hwnd, int *out_x, int *out_y, int *out_w, int *out_h) {
    if (g_fs_override_w > 0 && g_fs_override_h > 0) {
        *out_x = 0;
        *out_y = 0;
        *out_w = g_fs_override_w;
        *out_h = g_fs_override_h;
        return;
    }
    HMONITOR hmon = hwnd
        ? MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST)
        : MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY);
    if (hmon) {
        MONITORINFO mi;
        memset(&mi, 0, sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoA(hmon, &mi)) {
            int mw = mi.rcMonitor.right - mi.rcMonitor.left;
            int mh = mi.rcMonitor.bottom - mi.rcMonitor.top;
            if (mw > 0 && mh > 0) {
                *out_x = (int)mi.rcMonitor.left;
                *out_y = (int)mi.rcMonitor.top;
                *out_w = mw;
                *out_h = mh;
                return;
            }
        }
    }
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    *out_x = 0;
    *out_y = 0;
    *out_w = (sw > 0) ? sw : 1920;
    *out_h = (sh > 0) ? sh : 1080;
}

int SDL_PortUsingExternalDLL(void) {
    return g_using_dll;
}

int SDL_Init(Uint32 flags) {
    const char *aq = getenv("SCHICK_PORT_AUTOQUIT_MS");
    if (!aq) aq = getenv("RIVA_PORT_AUTOQUIT_MS");
    if (!aq && ((getenv("SCHICK_PORT_AUTOTEST") && getenv("SCHICK_PORT_AUTOTEST")[0] == '1') ||
                (getenv("RIVA_PORT_AUTOTEST") && getenv("RIVA_PORT_AUTOTEST")[0] == '1'))) {
        aq = "15000";
    }
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    (void)dsa_seh_handler;
    enable_win32_dpi_awareness();
    if (aq && aq[0]) {
        g_autoquit_ms = (DWORD)atoi(aq);
        if (g_autoquit_ms > 0) {
            CreateThread(NULL, 0, dsa_autoquit_thread, (LPVOID)(uintptr_t)g_autoquit_ms, 0, NULL);
        }
    }
    g_start_ticks = GetTickCount();
    if (!g_sdl_dll) {
        g_sdl_dll = LoadLibraryA("SDL2.dll");
        if (g_sdl_dll) {
            p_SDL_Init                 = (void *)GetProcAddress(g_sdl_dll, "SDL_Init");
            p_SDL_Quit                 = (void *)GetProcAddress(g_sdl_dll, "SDL_Quit");
            p_SDL_GetError             = (void *)GetProcAddress(g_sdl_dll, "SDL_GetError");
            p_SDL_CreateWindow         = (void *)GetProcAddress(g_sdl_dll, "SDL_CreateWindow");
            p_SDL_DestroyWindow        = (void *)GetProcAddress(g_sdl_dll, "SDL_DestroyWindow");
            p_SDL_SetWindowTitle       = (void *)GetProcAddress(g_sdl_dll, "SDL_SetWindowTitle");
            p_SDL_SetWindowSize        = (void *)GetProcAddress(g_sdl_dll, "SDL_SetWindowSize");
            p_SDL_GetWindowSize        = (void *)GetProcAddress(g_sdl_dll, "SDL_GetWindowSize");
            p_SDL_SetWindowFullscreen  = (void *)GetProcAddress(g_sdl_dll, "SDL_SetWindowFullscreen");
            p_SDL_CreateRenderer       = (void *)GetProcAddress(g_sdl_dll, "SDL_CreateRenderer");
            p_SDL_DestroyRenderer      = (void *)GetProcAddress(g_sdl_dll, "SDL_DestroyRenderer");
            p_SDL_RenderSetLogicalSize = (void *)GetProcAddress(g_sdl_dll, "SDL_RenderSetLogicalSize");
            p_SDL_RenderClear          = (void *)GetProcAddress(g_sdl_dll, "SDL_RenderClear");
            p_SDL_RenderCopy           = (void *)GetProcAddress(g_sdl_dll, "SDL_RenderCopy");
            p_SDL_RenderPresent        = (void *)GetProcAddress(g_sdl_dll, "SDL_RenderPresent");
            p_SDL_CreateTexture        = (void *)GetProcAddress(g_sdl_dll, "SDL_CreateTexture");
            p_SDL_DestroyTexture       = (void *)GetProcAddress(g_sdl_dll, "SDL_DestroyTexture");
            p_SDL_UpdateTexture        = (void *)GetProcAddress(g_sdl_dll, "SDL_UpdateTexture");
            p_SDL_PollEvent            = (void *)GetProcAddress(g_sdl_dll, "SDL_PollEvent");
            p_SDL_GetMouseState        = (void *)GetProcAddress(g_sdl_dll, "SDL_GetMouseState");
            p_SDL_WarpMouseInWindow    = (void *)GetProcAddress(g_sdl_dll, "SDL_WarpMouseInWindow");
            p_SDL_ShowCursor           = (void *)GetProcAddress(g_sdl_dll, "SDL_ShowCursor");
            p_SDL_GetTicks             = (void *)GetProcAddress(g_sdl_dll, "SDL_GetTicks");
            p_SDL_Delay                = (void *)GetProcAddress(g_sdl_dll, "SDL_Delay");
            p_SDL_OpenAudioDevice      = (void *)GetProcAddress(g_sdl_dll, "SDL_OpenAudioDevice");
            p_SDL_CloseAudioDevice     = (void *)GetProcAddress(g_sdl_dll, "SDL_CloseAudioDevice");
            p_SDL_PauseAudioDevice     = (void *)GetProcAddress(g_sdl_dll, "SDL_PauseAudioDevice");
            p_SDL_LockAudioDevice      = (void *)GetProcAddress(g_sdl_dll, "SDL_LockAudioDevice");
            p_SDL_UnlockAudioDevice    = (void *)GetProcAddress(g_sdl_dll, "SDL_UnlockAudioDevice");

            if (p_SDL_Init && p_SDL_CreateWindow && p_SDL_CreateRenderer && p_SDL_CreateTexture && p_SDL_PollEvent) {
                if (p_SDL_Init(flags) == 0) {
                    g_using_dll = 1;
                    return 0;
                }
            }
        }
    }
    g_using_dll = 0;
    return 0;
}

void SDL_Quit(void) {
    if (g_using_dll && p_SDL_Quit) {
        p_SDL_Quit();
        return;
    }
}

const char *SDL_GetError(void) {
    if (g_using_dll && p_SDL_GetError) return p_SDL_GetError();
    return "";
}

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags) {
    WNDCLASSEXA wc;
    RECT rc;
    int is_at = dsa_is_headless_run();
    int want_fs = (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
    DWORD vis = is_at ? 0u : WS_VISIBLE;
    DWORD style = want_fs ? (WS_POPUP | vis | WS_CLIPCHILDREN | WS_CLIPSIBLINGS) : (WS_OVERLAPPEDWINDOW | vis);
    DWORD ex_style = want_fs ? (is_at ? 0u : (WS_EX_APPWINDOW | WS_EX_TOPMOST)) : WS_EX_APPWINDOW;
    SDL_Window *win;
    (void)x; (void)y;

    if (g_using_dll && p_SDL_CreateWindow) {
        return p_SDL_CreateWindow(title, x, y, w, h, flags);
    }

    memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = dsa_wndproc;
    wc.hInstance     = GetModuleHandleA(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "DSA_SDL2_WindowClass";
    RegisterClassExA(&wc);

    win = (SDL_Window *)calloc(1, sizeof(SDL_Window));
    win->saved_win_w = w;
    win->saved_win_h = h;
    win->logical_w   = 320;
    win->logical_h   = 200;

    int create_x = CW_USEDEFAULT, create_y = CW_USEDEFAULT;
    int create_w = w, create_h = h;

    if (want_fs) {
        int mx = 0, my = 0, mw = 1920, mh = 1080;
        query_fullscreen_monitor_rect(NULL, &mx, &my, &mw, &mh);
        create_x = mx;
        create_y = my;
        create_w = mw;
        create_h = mh;
        win->win_w = mw;
        win->win_h = mh;
        win->vp_x  = 0;
        win->vp_y  = 0;
        win->vp_w  = mw;
        win->vp_h  = mh;
        win->fullscreen = 1;
    } else {
        rc.left = 0; rc.top = 0; rc.right = w; rc.bottom = h;
        AdjustWindowRectEx(&rc, style, FALSE, ex_style);
        create_w = rc.right - rc.left;
        create_h = rc.bottom - rc.top;
        int mx = 0, my = 0, mw = 1920, mh = 1080;
        query_fullscreen_monitor_rect(NULL, &mx, &my, &mw, &mh);
        create_x = mx + (mw > create_w ? (mw - create_w) / 2 : 0);
        create_y = my + (mh > create_h ? (mh - create_h) / 2 : 0);
        win->win_w = w;
        win->win_h = h;
        win->vp_x  = 0;
        win->vp_y  = 0;
        win->vp_w  = w;
        win->vp_h  = h;
        win->fullscreen = 0;
    }

    win->hwnd = CreateWindowExA(
        ex_style, "DSA_SDL2_WindowClass", title ? title : "DSA - Die Schicksalsklinge",
        style, create_x, create_y,
        create_w, create_h,
        NULL, NULL, wc.hInstance, NULL
    );
    win->hdc = GetDC(win->hwnd);
    g_main_win = win;

    if (want_fs) {
        SDL_SetWindowFullscreen(win, flags);
    } else if (!is_at) {
        ShowWindow(win->hwnd, SW_SHOW);
        UpdateWindow(win->hwnd);
        SetForegroundWindow(win->hwnd);
        SetFocus(win->hwnd);
    }
    return win;
}

void SDL_DestroyWindow(SDL_Window *window) {
    if (g_using_dll && p_SDL_DestroyWindow) {
        p_SDL_DestroyWindow(window);
        return;
    }
    if (window) {
        if (window->hwnd && window->hdc) ReleaseDC(window->hwnd, window->hdc);
        if (window->hwnd) DestroyWindow(window->hwnd);
        if (g_main_win == window) g_main_win = NULL;
        free(window);
    }
}

void SDL_SetWindowTitle(SDL_Window *window, const char *title) {
    if (g_using_dll && p_SDL_SetWindowTitle) {
        p_SDL_SetWindowTitle(window, title);
        return;
    }
    if (window && window->hwnd && title) SetWindowTextA(window->hwnd, title);
}

void SDL_SetWindowSize(SDL_Window *window, int w, int h) {
    if (g_using_dll && p_SDL_SetWindowSize) {
        p_SDL_SetWindowSize(window, w, h);
        return;
    }
    if (window && window->hwnd) {
        DWORD style = (DWORD)GetWindowLongA(window->hwnd, GWL_STYLE);
        DWORD ex_style = (DWORD)GetWindowLongA(window->hwnd, GWL_EXSTYLE);
        RECT rc = {0, 0, w, h};
        AdjustWindowRectEx(&rc, style, FALSE, ex_style);
        int full_w = rc.right - rc.left;
        int full_h = rc.bottom - rc.top;
        int mx = 0, my = 0, mw = 1920, mh = 1080;
        query_fullscreen_monitor_rect(window->hwnd, &mx, &my, &mw, &mh);
        int sx = mx + (mw > full_w ? (mw - full_w) / 2 : 0);
        int sy = my + (mh > full_h ? (mh - full_h) / 2 : 0);
        SetWindowPos(window->hwnd, NULL, sx, sy, full_w, full_h, SWP_NOZORDER);
        window->win_w = w;
        window->win_h = h;
        window->saved_win_w = w;
        window->saved_win_h = h;
    }
}

void SDL_GetWindowSize(SDL_Window *window, int *w, int *h) {
    if (g_using_dll && p_SDL_GetWindowSize) {
        p_SDL_GetWindowSize(window, w, h);
        return;
    }
    if (window) {
        if (window->fullscreen && g_fs_override_w > 0 && g_fs_override_h > 0) {
            if (w) *w = g_fs_override_w;
            if (h) *h = g_fs_override_h;
            return;
        }
        if (window->hwnd) {
            RECT rc;
            GetClientRect(window->hwnd, &rc);
            int cw = rc.right - rc.left;
            int ch = rc.bottom - rc.top;
            if (cw > 0 && ch > 0) {
                if (w) *w = cw;
                if (h) *h = ch;
                return;
            }
        }
        if (w) *w = window->win_w;
        if (h) *h = window->win_h;
    }
}

int SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags) {
    if (g_using_dll && p_SDL_SetWindowFullscreen) {
        return p_SDL_SetWindowFullscreen(window, flags);
    }
    if (!window || !window->hwnd) return -1;
    int is_at = dsa_is_headless_run();
    DWORD vis = is_at ? 0u : WS_VISIBLE;

    if (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        if (!window->fullscreen) {
            RECT crc;
            if (GetClientRect(window->hwnd, &crc) && (crc.right - crc.left) >= 320 && (crc.bottom - crc.top) >= 200) {
                window->saved_win_w = crc.right - crc.left;
                window->saved_win_h = crc.bottom - crc.top;
            }
        }
        int mx = 0, my = 0, sw = 1920, sh = 1080;
        query_fullscreen_monitor_rect(window->hwnd, &mx, &my, &sw, &sh);

        /* Alle Fensterrahmen und Extended-Border-Styles (WS_EX_WINDOWEDGE) entfernen,
         * WS_POPUP + HWND_TOPMOST setzen, damit auch die Windows-Taskleiste komplett verdeckt wird! */
        SetWindowLongA(window->hwnd, GWL_STYLE, (LONG)(WS_POPUP | vis | WS_CLIPCHILDREN | WS_CLIPSIBLINGS));
        SetWindowLongA(window->hwnd, GWL_EXSTYLE, (LONG)(is_at ? 0u : (WS_EX_APPWINDOW | WS_EX_TOPMOST)));
        SetWindowPos(
            window->hwnd,
            is_at ? HWND_TOP : HWND_TOPMOST,
            mx, my, sw, sh,
            SWP_FRAMECHANGED | (is_at ? SWP_NOACTIVATE : SWP_SHOWWINDOW)
        );
        if (!is_at) {
            ShowWindow(window->hwnd, SW_SHOW);
            UpdateWindow(window->hwnd);
            SetForegroundWindow(window->hwnd);
            SetFocus(window->hwnd);
        }
        window->win_w = sw;
        window->win_h = sh;
        window->fullscreen = 1;
    } else {
        int ww = (window->saved_win_w >= 320) ? window->saved_win_w : 1280;
        int wh = (window->saved_win_h >= 240) ? window->saved_win_h : 960;
        DWORD style = WS_OVERLAPPEDWINDOW | vis;
        DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        RECT rc = {0, 0, ww, wh};
        AdjustWindowRectEx(&rc, style, FALSE, ex_style);
        int full_w = rc.right - rc.left;
        int full_h = rc.bottom - rc.top;
        int mx = 0, my = 0, mw = 1920, mh = 1080;
        query_fullscreen_monitor_rect(window->hwnd, &mx, &my, &mw, &mh);
        int sx = mx + (mw > full_w ? (mw - full_w) / 2 : 0);
        int sy = my + (mh > full_h ? (mh - full_h) / 2 : 0);

        SetWindowLongA(window->hwnd, GWL_EXSTYLE, (LONG)ex_style);
        SetWindowLongA(window->hwnd, GWL_STYLE, (LONG)style);
        SetWindowPos(
            window->hwnd,
            HWND_NOTOPMOST,
            sx, sy, full_w, full_h,
            SWP_FRAMECHANGED | (is_at ? SWP_NOACTIVATE : SWP_SHOWWINDOW)
        );
        if (!is_at) {
            ShowWindow(window->hwnd, SW_SHOWNORMAL);
            UpdateWindow(window->hwnd);
            SetForegroundWindow(window->hwnd);
            SetFocus(window->hwnd);
        }
        window->win_w = ww;
        window->win_h = wh;
        window->fullscreen = 0;
    }
    return 0;
}

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags) {
    SDL_Renderer *ren;
    if (g_using_dll && p_SDL_CreateRenderer) {
        return p_SDL_CreateRenderer(window, index, flags);
    }
    ren = (SDL_Renderer *)calloc(1, sizeof(SDL_Renderer));
    ren->window = window;
    ren->logical_w = window ? window->logical_w : 320;
    ren->logical_h = window ? window->logical_h : 200;
    return ren;
}

void SDL_DestroyRenderer(SDL_Renderer *renderer) {
    if (g_using_dll && p_SDL_DestroyRenderer) {
        p_SDL_DestroyRenderer(renderer);
        return;
    }
    if (renderer) free(renderer);
}

int SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h) {
    if (g_using_dll && p_SDL_RenderSetLogicalSize) {
        return p_SDL_RenderSetLogicalSize(renderer, w, h);
    }
    if (renderer) {
        renderer->logical_w = w;
        renderer->logical_h = h;
        if (renderer->window) {
            renderer->window->logical_w = w;
            renderer->window->logical_h = h;
        }
    }
    return 0;
}

int SDL_RenderClear(SDL_Renderer *renderer) {
    if (g_using_dll && p_SDL_RenderClear) return p_SDL_RenderClear(renderer);
    return 0;
}

int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect) {
    (void)srcrect; (void)dstrect;
    if (g_using_dll && p_SDL_RenderCopy) {
        return p_SDL_RenderCopy(renderer, texture, srcrect, dstrect);
    }
    if (renderer) renderer->active_tex = texture;
    return 0;
}

void SDL_RenderPresent(SDL_Renderer *renderer) {
    RECT rc;
    int cw, ch;
    SDL_Texture *tex;
    if (g_using_dll && p_SDL_RenderPresent) {
        p_SDL_RenderPresent(renderer);
        return;
    }
    if (!renderer || !renderer->window || !renderer->window->hwnd || !renderer->active_tex) return;
    tex = renderer->active_tex;
    GetClientRect(renderer->window->hwnd, &rc);
    cw = rc.right - rc.left;
    ch = rc.bottom - rc.top;
    if (cw > 0 && ch > 0 && tex->pixels) {
        StretchDIBits(
            renderer->window->hdc,
            0, 0, cw, ch,
            0, 0, tex->w, tex->h,
            tex->pixels, &tex->bmi,
            DIB_RGB_COLORS, SRCCOPY
        );
    }
}

SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h) {
    SDL_Texture *tex;
    (void)format; (void)access;
    if (g_using_dll && p_SDL_CreateTexture) {
        return p_SDL_CreateTexture(renderer, format, access, w, h);
    }
    tex = (SDL_Texture *)calloc(1, sizeof(SDL_Texture));
    tex->w = w;
    tex->h = h;
    tex->pixels = (uint32_t *)calloc((size_t)(w * h), sizeof(uint32_t));
    tex->bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    tex->bmi.bmiHeader.biWidth       = w;
    tex->bmi.bmiHeader.biHeight      = -h; /* Top-Down DIB */
    tex->bmi.bmiHeader.biPlanes      = 1;
    tex->bmi.bmiHeader.biBitCount    = 32;
    tex->bmi.bmiHeader.biCompression = BI_RGB;
    if (renderer) renderer->active_tex = tex;
    return tex;
}

void SDL_DestroyTexture(SDL_Texture *texture) {
    if (g_using_dll && p_SDL_DestroyTexture) {
        p_SDL_DestroyTexture(texture);
        return;
    }
    if (texture) {
        free(texture->pixels);
        free(texture);
    }
}

int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch) {
    int y;
    const uint8_t *src = (const uint8_t *)pixels;
    if (g_using_dll && p_SDL_UpdateTexture) {
        return p_SDL_UpdateTexture(texture, rect, pixels, pitch);
    }
    if (!texture || !texture->pixels || !pixels) return -1;
    for (y = 0; y < texture->h; ++y) {
        memcpy(texture->pixels + y * texture->w, src + y * pitch, (size_t)texture->w * 4);
    }
    return 0;
}

int SDL_PollEvent(SDL_Event *event) {
    MSG msg;
    if (g_using_dll && p_SDL_PollEvent) {
        return p_SDL_PollEvent(event);
    }
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            SDL_Event qev;
            memset(&qev, 0, sizeof(qev));
            qev.type = SDL_QUIT;
            evq_push(&qev);
        } else {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (g_evq_head != g_evq_tail) {
        if (event) *event = g_evq[g_evq_head];
        g_evq_head = (g_evq_head + 1) % EVQ_CAP;
        return 1;
    }
    return 0;
}

Uint32 SDL_GetMouseState(int *x, int *y) {
    if (g_using_dll && p_SDL_GetMouseState) {
        return p_SDL_GetMouseState(x, y);
    }
    if (x) *x = g_mouse_x;
    if (y) *y = g_mouse_y;
    return g_mouse_buttons;
}

void SDL_WarpMouseInWindow(SDL_Window *window, int x, int y) {
    if (g_using_dll && p_SDL_WarpMouseInWindow) {
        p_SDL_WarpMouseInWindow(window, x, y);
        return;
    }
    g_mouse_x = x;
    g_mouse_y = y;
}

int SDL_ShowCursor(int toggle) {
    if (g_using_dll && p_SDL_ShowCursor) {
        return p_SDL_ShowCursor(toggle);
    }
    if (toggle >= 0) {
        g_show_cursor = toggle ? 1 : 0;
    }
    return g_show_cursor;
}

Uint32 SDL_GetTicks(void) {
    Uint32 elapsed = (Uint32)(GetTickCount() - g_start_ticks);
    if (g_autoquit_ms > 0 && elapsed >= g_autoquit_ms) {
        ExitProcess(0);
    }
    if (g_using_dll && p_SDL_GetTicks) {
        return p_SDL_GetTicks();
    }
    return elapsed;
}

void SDL_Delay(Uint32 ms) {
    if (g_using_dll && p_SDL_Delay) {
        p_SDL_Delay(ms);
        return;
    }
    Sleep(ms);
}

/* =========================================================================
 * SDL2-Audio-Implementierung ueber Win32 waveOut (winmm.dll)
 * ========================================================================= */

#define AUDIO_NUM_BUFS 4
static HWAVEOUT          g_wave_out = NULL;
static WAVEHDR           g_wave_hdrs[AUDIO_NUM_BUFS];
static uint8_t          *g_wave_bufs[AUDIO_NUM_BUFS];
static SDL_AudioSpec     g_audio_spec;
static CRITICAL_SECTION  g_audio_cs;
static int               g_audio_cs_init = 0;
static volatile int      g_audio_running = 0;
static volatile int      g_audio_paused  = 1;
static HANDLE            g_audio_thread  = NULL;

static DWORD WINAPI win32_audio_thread_proc(LPVOID param) {
    int i;
    uint32_t buf_bytes = g_audio_spec.size ? g_audio_spec.size : 4096;
    uint8_t *dummy_buf = (uint8_t *)malloc(buf_bytes);
    (void)param;

    while (g_audio_running) {
        if (!g_audio_paused && g_audio_spec.callback) {
            if (g_wave_out) {
                for (i = 0; i < AUDIO_NUM_BUFS; ++i) {
                    if (g_wave_hdrs[i].dwFlags & WHDR_DONE) {
                        EnterCriticalSection(&g_audio_cs);
                        if (g_audio_spec.callback) {
                            g_audio_spec.callback(g_audio_spec.userdata, g_wave_bufs[i], (int)buf_bytes);
                        } else {
                            memset(g_wave_bufs[i], 0, buf_bytes);
                        }
                        LeaveCriticalSection(&g_audio_cs);
                        waveOutWrite(g_wave_out, &g_wave_hdrs[i], sizeof(WAVEHDR));
                    }
                }
            } else if (dummy_buf) {
                /* Headless-/Null-Audio-Taktung (ca. 44,1 kHz -> 1024 Samples alle ~23 ms) */
                EnterCriticalSection(&g_audio_cs);
                if (g_audio_spec.callback) {
                    g_audio_spec.callback(g_audio_spec.userdata, dummy_buf, (int)buf_bytes);
                }
                LeaveCriticalSection(&g_audio_cs);
                Sleep(15);
            }
        }
        Sleep(5);
    }
    if (dummy_buf) free(dummy_buf);
    return 0;
}

SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device, int iscapture, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained, int allowed_changes) {
    int i;
    uint32_t buf_bytes;
    if (g_using_dll && p_SDL_OpenAudioDevice) {
        return p_SDL_OpenAudioDevice(device, iscapture, desired, obtained, allowed_changes);
    }
    if (!desired || iscapture) return 0;
    if (!g_audio_cs_init) {
        InitializeCriticalSection(&g_audio_cs);
        g_audio_cs_init = 1;
    }
    g_audio_spec = *desired;
    if (g_audio_spec.freq <= 0) g_audio_spec.freq = 44100;
    if (g_audio_spec.channels <= 0) g_audio_spec.channels = 2;
    if (g_audio_spec.samples == 0) g_audio_spec.samples = 1024;
    buf_bytes = (uint32_t)g_audio_spec.samples * (uint32_t)g_audio_spec.channels * 2u;
    g_audio_spec.size = buf_bytes;
    g_audio_spec.silence = 0;
    if (obtained) *obtained = g_audio_spec;

    g_wave_out = NULL;
    int is_at = dsa_is_headless_run();
    if (!is_at) {
        WAVEFORMATEX wfx;
        memset(&wfx, 0, sizeof(wfx));
        wfx.wFormatTag      = WAVE_FORMAT_PCM;
        wfx.nChannels       = g_audio_spec.channels;
        wfx.nSamplesPerSec  = (DWORD)g_audio_spec.freq;
        wfx.wBitsPerSample  = 16;
        wfx.nBlockAlign     = (WORD)(wfx.nChannels * (wfx.wBitsPerSample / 8));
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        if (waveOutOpen(&g_wave_out, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) == MMSYSERR_NOERROR) {
            for (i = 0; i < AUDIO_NUM_BUFS; ++i) {
                g_wave_bufs[i] = (uint8_t *)calloc(1, buf_bytes);
                memset(&g_wave_hdrs[i], 0, sizeof(WAVEHDR));
                g_wave_hdrs[i].lpData         = (LPSTR)g_wave_bufs[i];
                g_wave_hdrs[i].dwBufferLength = buf_bytes;
                waveOutPrepareHeader(g_wave_out, &g_wave_hdrs[i], sizeof(WAVEHDR));
                g_wave_hdrs[i].dwFlags |= WHDR_DONE;
            }
        } else {
            g_wave_out = NULL;
        }
    }

    g_audio_paused  = 1;
    g_audio_running = 1;
    g_audio_thread  = CreateThread(NULL, 0, win32_audio_thread_proc, NULL, 0, NULL);
    return 1;
}

void SDL_CloseAudioDevice(SDL_AudioDeviceID dev) {
    int i;
    if (g_using_dll && p_SDL_CloseAudioDevice) {
        p_SDL_CloseAudioDevice(dev);
        return;
    }
    if (!g_audio_running) return;
    g_audio_running = 0;
    g_audio_paused  = 1;
    if (g_audio_thread) {
        WaitForSingleObject(g_audio_thread, 500);
        CloseHandle(g_audio_thread);
        g_audio_thread = NULL;
    }
    if (g_wave_out) {
        waveOutReset(g_wave_out);
        for (i = 0; i < AUDIO_NUM_BUFS; ++i) {
            if (g_wave_bufs[i]) {
                waveOutUnprepareHeader(g_wave_out, &g_wave_hdrs[i], sizeof(WAVEHDR));
                free(g_wave_bufs[i]);
                g_wave_bufs[i] = NULL;
            }
        }
        waveOutClose(g_wave_out);
        g_wave_out = NULL;
    }
}

void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on) {
    if (g_using_dll && p_SDL_PauseAudioDevice) {
        p_SDL_PauseAudioDevice(dev, pause_on);
        return;
    }
    g_audio_paused = pause_on ? 1 : 0;
}

void SDL_LockAudioDevice(SDL_AudioDeviceID dev) {
    if (g_using_dll && p_SDL_LockAudioDevice) {
        p_SDL_LockAudioDevice(dev);
        return;
    }
    if (g_audio_cs_init) EnterCriticalSection(&g_audio_cs);
}

void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev) {
    if (g_using_dll && p_SDL_UnlockAudioDevice) {
        p_SDL_UnlockAudioDevice(dev);
        return;
    }
    if (g_audio_cs_init) LeaveCriticalSection(&g_audio_cs);
}
