/*
 * sdl2_win32_provider.c - Dynamischer SDL2.dll-Lader mit eingebautem Win32-Fallback
 * fuer den gemeinsamen Starter der Nordlandtrilogie (640x400), inkl. WM_DROPFILES
 * -> SDL_DROPFILE (Ziehen und Ablegen von Ordnern und Zip-Archiven).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"

static HMODULE g_sdl_dll   = NULL;
static int     g_using_dll = 0;

static int           (__cdecl *p_SDL_Init)(Uint32) = NULL;
static void          (__cdecl *p_SDL_Quit)(void) = NULL;
static const char *  (__cdecl *p_SDL_GetError)(void) = NULL;
static void          (__cdecl *p_SDL_free)(void *) = NULL;
static Uint8         (__cdecl *p_SDL_EventState)(Uint32, int) = NULL;
static SDL_Window *  (__cdecl *p_SDL_CreateWindow)(const char *, int, int, int, int, Uint32) = NULL;
static void          (__cdecl *p_SDL_DestroyWindow)(SDL_Window *) = NULL;
static void          (__cdecl *p_SDL_SetWindowTitle)(SDL_Window *, const char *) = NULL;
static void          (__cdecl *p_SDL_SetWindowSize)(SDL_Window *, int, int) = NULL;
static void          (__cdecl *p_SDL_GetWindowSize)(SDL_Window *, int *, int *) = NULL;
static void          (__cdecl *p_SDL_SetWindowPosition)(SDL_Window *, int, int) = NULL;
static void          (__cdecl *p_SDL_GetWindowPosition)(SDL_Window *, int *, int *) = NULL;
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
static int           (__cdecl *p_SDL_PushEvent)(SDL_Event *) = NULL;
static Uint32        (__cdecl *p_SDL_GetMouseState)(int *, int *) = NULL;
static void          (__cdecl *p_SDL_WarpMouseInWindow)(SDL_Window *, int, int) = NULL;
static int           (__cdecl *p_SDL_ShowCursor)(int) = NULL;
static Uint32        (__cdecl *p_SDL_GetTicks)(void) = NULL;
static void          (__cdecl *p_SDL_Delay)(Uint32) = NULL;

struct SDL_Window {
    HWND   hwnd;
    HDC    hdc;
    int    win_w;
    int    win_h;
    int    logical_w;
    int    logical_h;
    int    fullscreen;
    LONG   saved_style;
    RECT   saved_rect;
};

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
static SDL_Window *g_main_win = NULL;
static int         g_mouse_x = 0;
static int         g_mouse_y = 0;
static Uint32      g_mouse_buttons = 0;
static int         g_show_cursor = 1;
static int         g_hand_cursor = 0;
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
    int cw, ch, log_w, log_h;
    GetClientRect(hwnd, &rc);
    cw = rc.right - rc.left;
    ch = rc.bottom - rc.top;
    log_w = (g_main_win && g_main_win->logical_w > 0) ? g_main_win->logical_w : 640;
    log_h = (g_main_win && g_main_win->logical_h > 0) ? g_main_win->logical_h : 400;
    if (cw <= 0) cw = 1;
    if (ch <= 0) ch = 1;
    *lx = (cx * log_w) / cw;
    *ly = (cy * log_h) / ch;
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

static LRESULT CALLBACK starter_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SDL_Event ev;
    memset(&ev, 0, sizeof(ev));
    ev.key.timestamp = GetTickCount() - g_start_ticks;

    switch (msg) {
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            if (!g_show_cursor) {
                SetCursor(NULL);
                return TRUE;
            }
            SetCursor(LoadCursor(NULL, g_hand_cursor ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        break;

    case WM_MOUSEWHEEL: {
        short delta = (short)HIWORD(wParam);
        ev.type = SDL_MOUSEWHEEL;
        ev.wheel.timestamp = GetTickCount() - g_start_ticks;
        ev.wheel.x = 0;
        if (delta >= 120 || delta <= -120) {
            ev.wheel.y = delta / 120;
        } else {
            ev.wheel.y = (delta > 0) ? 1 : ((delta < 0) ? -1 : 0);
        }
        evq_push(&ev);
        return 0;
    }

    case WM_DROPFILES: {
        HDROP hDrop = (HDROP)wParam;
        UINT count = DragQueryFileA(hDrop, 0xFFFFFFFFu, NULL, 0);
        for (UINT i = 0; i < count; ++i) {
            char path_buf[MAX_PATH * 2] = "";
            if (DragQueryFileA(hDrop, i, path_buf, sizeof(path_buf) - 1) > 0) {
                SDL_Event dev;
                memset(&dev, 0, sizeof(dev));
                dev.type = SDL_DROPFILE;
                dev.drop.timestamp = GetTickCount() - g_start_ticks;
                dev.drop.file = (char *)malloc(strlen(path_buf) + 1);
                if (dev.drop.file) {
                    strcpy(dev.drop.file, path_buf);
                    evq_push(&dev);
                }
            }
        }
        DragFinish(hDrop);
        return 0;
    }

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
        if ((lParam & (1 << 24)) && wParam != VK_RETURN && wParam != VK_DIVIDE) {
            ascii_ch = 0;
        }

        ev.type = down ? SDL_KEYDOWN : SDL_KEYUP;
        ev.key.state = down ? 1 : 0;
        ev.key.repeat = (down && (lParam & (1 << 30))) ? 1 : 0;
        ev.key.keysym.scancode = -dos_scan;
        ev.key.keysym.sym = (SDL_Keycode)wParam;
        ev.key.keysym.mod = get_win32_kmod();
        ev.key.keysym.unused = ((Uint32)(dos_scan & 0xFF) << 8) | (Uint32)ascii_ch;
        evq_push(&ev);
        return 0;
    }

    case WM_CLOSE:
    case WM_DESTROY:
        ev.type = SDL_QUIT;
        evq_push(&ev);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static void enable_starter_dpi_awareness(void) {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *PFN_SetProcessDpiAwarenessContext)(HANDLE);
        typedef BOOL (WINAPI *PFN_SetProcessDPIAware)(void);
        PFN_SetProcessDpiAwarenessContext pCtx =
            (PFN_SetProcessDpiAwarenessContext)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pCtx) {
            if (pCtx((HANDLE)(intptr_t)-4)) return;
            if (pCtx((HANDLE)(intptr_t)-2)) return;
        }
        PFN_SetProcessDPIAware pAware =
            (PFN_SetProcessDPIAware)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pAware) {
            pAware();
        }
    }
}

int SDL_Init(Uint32 flags) {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    enable_starter_dpi_awareness();
    g_start_ticks = GetTickCount();
    if (!g_sdl_dll) {
        g_sdl_dll = LoadLibraryA("SDL2.dll");
        if (g_sdl_dll) {
            p_SDL_Init                 = (void *)GetProcAddress(g_sdl_dll, "SDL_Init");
            p_SDL_Quit                 = (void *)GetProcAddress(g_sdl_dll, "SDL_Quit");
            p_SDL_GetError             = (void *)GetProcAddress(g_sdl_dll, "SDL_GetError");
            p_SDL_free                 = (void *)GetProcAddress(g_sdl_dll, "SDL_free");
            p_SDL_EventState           = (void *)GetProcAddress(g_sdl_dll, "SDL_EventState");
            p_SDL_CreateWindow         = (void *)GetProcAddress(g_sdl_dll, "SDL_CreateWindow");
            p_SDL_DestroyWindow        = (void *)GetProcAddress(g_sdl_dll, "SDL_DestroyWindow");
            p_SDL_SetWindowTitle       = (void *)GetProcAddress(g_sdl_dll, "SDL_SetWindowTitle");
            p_SDL_SetWindowSize        = (void *)GetProcAddress(g_sdl_dll, "SDL_SetWindowSize");
            p_SDL_GetWindowSize        = (void *)GetProcAddress(g_sdl_dll, "SDL_GetWindowSize");
            p_SDL_SetWindowPosition    = (void *)GetProcAddress(g_sdl_dll, "SDL_SetWindowPosition");
            p_SDL_GetWindowPosition    = (void *)GetProcAddress(g_sdl_dll, "SDL_GetWindowPosition");
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
            p_SDL_PushEvent            = (void *)GetProcAddress(g_sdl_dll, "SDL_PushEvent");
            p_SDL_GetMouseState        = (void *)GetProcAddress(g_sdl_dll, "SDL_GetMouseState");
            p_SDL_WarpMouseInWindow    = (void *)GetProcAddress(g_sdl_dll, "SDL_WarpMouseInWindow");
            p_SDL_ShowCursor           = (void *)GetProcAddress(g_sdl_dll, "SDL_ShowCursor");
            p_SDL_GetTicks             = (void *)GetProcAddress(g_sdl_dll, "SDL_GetTicks");
            p_SDL_Delay                = (void *)GetProcAddress(g_sdl_dll, "SDL_Delay");

            if (p_SDL_Init && p_SDL_CreateWindow && p_SDL_CreateRenderer && p_SDL_CreateTexture && p_SDL_PollEvent) {
                if (p_SDL_Init(flags) == 0) {
                    if (p_SDL_EventState) p_SDL_EventState(SDL_DROPFILE, 1);
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
    }
}

const char *SDL_GetError(void) {
    if (g_using_dll && p_SDL_GetError) return p_SDL_GetError();
    return "";
}

void SDL_free(void *mem) {
    if (!mem) return;
    if (g_using_dll && p_SDL_free) {
        p_SDL_free(mem);
        return;
    }
    free(mem);
}

static BOOL CALLBACK apply_starter_icon_enum_proc(HWND hwnd, LPARAM lParam) {
    HINSTANCE hInst = (HINSTANCE)lParam;
    HICON hBig   = (HICON)LoadImageA(hInst, MAKEINTRESOURCEA(1), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    HICON hSmall = (HICON)LoadImageA(hInst, MAKEINTRESOURCEA(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    if (hBig)   SendMessageA(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hBig);
    if (hSmall) SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
    return TRUE;
}

SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags) {
    WNDCLASSEXA wc;
    RECT rc;
    const char *hl = getenv("STARTER_HEADLESS");
    int hidden = ((flags & SDL_WINDOW_HIDDEN) != 0) || (hl && hl[0] == '1');
    DWORD style = hidden ? WS_OVERLAPPEDWINDOW : (WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SDL_Window *win;
    HINSTANCE hInst = GetModuleHandleA(NULL);
    HICON hIconBig   = (HICON)LoadImageA(hInst, MAKEINTRESOURCEA(1), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    HICON hIconSmall = (HICON)LoadImageA(hInst, MAKEINTRESOURCEA(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    (void)x; (void)y;

    if (g_using_dll && p_SDL_CreateWindow) {
        win = p_SDL_CreateWindow(title, x, y, w, h, flags);
        EnumThreadWindows(GetCurrentThreadId(), apply_starter_icon_enum_proc, (LPARAM)hInst);
        return win;
    }

    memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = starter_wndproc;
    wc.hInstance     = hInst;
    wc.hIcon         = hIconBig;
    wc.hIconSm       = hIconSmall;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "DSA_Starter_SDL2_Wnd";
    RegisterClassExA(&wc);

    rc.left = 0; rc.top = 0; rc.right = w; rc.bottom = h;
    AdjustWindowRect(&rc, style, FALSE);

    win = (SDL_Window *)calloc(1, sizeof(SDL_Window));
    win->win_w = w;
    win->win_h = h;
    win->logical_w = 640;
    win->logical_h = 400;

    int win_total_w = rc.right - rc.left;
    int win_total_h = rc.bottom - rc.top;
    int scr_w = GetSystemMetrics(SM_CXSCREEN);
    int scr_h = GetSystemMetrics(SM_CYSCREEN);
    int pos_x = (x != (int)SDL_WINDOWPOS_CENTERED && x != (int)SDL_WINDOWPOS_UNDEFINED)
        ? x : ((scr_w > win_total_w) ? (scr_w - win_total_w) / 2 : CW_USEDEFAULT);
    int pos_y = (y != (int)SDL_WINDOWPOS_CENTERED && y != (int)SDL_WINDOWPOS_UNDEFINED)
        ? y : ((scr_h > win_total_h) ? (scr_h - win_total_h) / 2 : CW_USEDEFAULT);

    win->hwnd = CreateWindowExA(
        WS_EX_ACCEPTFILES, "DSA_Starter_SDL2_Wnd", title ? title : "Nordlandtrilogie",
        style, pos_x, pos_y,
        win_total_w, win_total_h,
        NULL, NULL, wc.hInstance, NULL
    );
    if (hIconBig)   SendMessageA(win->hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hIconBig);
    if (hIconSmall) SendMessageA(win->hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    DragAcceptFiles(win->hwnd, TRUE);
    win->hdc = GetDC(win->hwnd);
    if (!hidden) {
        ShowWindow(win->hwnd, SW_SHOW);
        UpdateWindow(win->hwnd);
    }
    g_main_win = win;
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
        RECT rc = {0, 0, w, h};
        AdjustWindowRect(&rc, (DWORD)GetWindowLongA(window->hwnd, GWL_STYLE), FALSE);
        SetWindowPos(window->hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
        window->win_w = w;
        window->win_h = h;
    }
}

void SDL_GetWindowSize(SDL_Window *window, int *w, int *h) {
    if (g_using_dll && p_SDL_GetWindowSize) {
        p_SDL_GetWindowSize(window, w, h);
        return;
    }
    if (window && window->hwnd) {
        RECT rc;
        GetClientRect(window->hwnd, &rc);
        if (w) *w = rc.right - rc.left;
        if (h) *h = rc.bottom - rc.top;
    }
}

void SDL_SetWindowPosition(SDL_Window *window, int x, int y) {
    if (g_using_dll && p_SDL_SetWindowPosition) {
        p_SDL_SetWindowPosition(window, x, y);
        return;
    }
    if (window && window->hwnd && !window->fullscreen) {
        SetWindowPos(window->hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void SDL_GetWindowPosition(SDL_Window *window, int *x, int *y) {
    if (g_using_dll && p_SDL_GetWindowPosition) {
        p_SDL_GetWindowPosition(window, x, y);
        return;
    }
    if (window && window->hwnd) {
        RECT rc;
        if (GetWindowRect(window->hwnd, &rc)) {
            if (x) *x = (int)rc.left;
            if (y) *y = (int)rc.top;
        }
    }
}

int SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags) {
    if (g_using_dll && p_SDL_SetWindowFullscreen) {
        return p_SDL_SetWindowFullscreen(window, flags);
    }
    if (!window || !window->hwnd) return -1;
    int want_fs = (flags & SDL_WINDOW_FULLSCREEN) != 0;
    if (want_fs && !window->fullscreen) {
        window->saved_style = GetWindowLongA(window->hwnd, GWL_STYLE);
        GetWindowRect(window->hwnd, &window->saved_rect);
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        SetWindowLongA(window->hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(window->hwnd, HWND_TOP, 0, 0, sw, sh, SWP_FRAMECHANGED);
        window->fullscreen = 1;
    } else if (!want_fs && window->fullscreen) {
        SetWindowLongA(window->hwnd, GWL_STYLE, window->saved_style);
        SetWindowPos(window->hwnd, NULL,
                     window->saved_rect.left, window->saved_rect.top,
                     window->saved_rect.right - window->saved_rect.left,
                     window->saved_rect.bottom - window->saved_rect.top,
                     SWP_FRAMECHANGED | SWP_NOZORDER);
        window->fullscreen = 0;
    }
    return 0;
}

void SDL_HideWindow(SDL_Window *window) {
    if (window && !g_using_dll && window->hwnd) {
        ShowWindow(window->hwnd, SW_HIDE);
    }
}

void SDL_ShowWindow(SDL_Window *window) {
    if (window && !g_using_dll && window->hwnd) {
        ShowWindow(window->hwnd, SW_SHOW);
    }
}

void SDL_RaiseWindow(SDL_Window *window) {
    if (window && !g_using_dll && window->hwnd) {
        SetForegroundWindow(window->hwnd);
    }
}

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags) {
    SDL_Renderer *ren;
    if (g_using_dll && p_SDL_CreateRenderer) {
        return p_SDL_CreateRenderer(window, index, flags);
    }
    ren = (SDL_Renderer *)calloc(1, sizeof(SDL_Renderer));
    ren->window = window;
    ren->logical_w = window ? window->logical_w : 640;
    ren->logical_h = window ? window->logical_h : 400;
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
    tex->bmi.bmiHeader.biHeight      = -h;
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

int SDL_PushEvent(SDL_Event *event) {
    if (!event) return 0;
    if (g_using_dll && p_SDL_PushEvent) {
        return p_SDL_PushEvent(event);
    }
    evq_push(event);
    return 1;
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

void SDL_SetHandCursor(int hand) {
    int h = hand ? 1 : 0;
    if (g_hand_cursor != h) {
        g_hand_cursor = h;
        if (!g_using_dll && g_show_cursor) {
            SetCursor(LoadCursor(NULL, g_hand_cursor ? IDC_HAND : IDC_ARROW));
        }
    }
}

Uint32 SDL_GetTicks(void) {
    if (g_using_dll && p_SDL_GetTicks) {
        return p_SDL_GetTicks();
    }
    return (Uint32)(GetTickCount() - g_start_ticks);
}

void SDL_Delay(Uint32 ms) {
    if (g_using_dll && p_SDL_Delay) {
        p_SDL_Delay(ms);
        return;
    }
    Sleep(ms);
}
