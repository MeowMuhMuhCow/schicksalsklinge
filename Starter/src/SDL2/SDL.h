/*
 * SDL.h - SDL2-Header fuer den gemeinsamen Starter der Nordlandtrilogie.
 * Kompatibel mit 32-Bit-SDL2.dll sowie dem eingebauten Win32-GDI/User32-Provider
 * (sdl2_win32_provider.c), inkl. SDL_DROPFILE (Ziehen und Ablegen).
 */

#ifndef STARTER_SDL2_H
#define STARTER_SDL2_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t  Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef int32_t  Sint32;

/* SDL_Init Flags */
#define SDL_INIT_TIMER          0x00000001u
#define SDL_INIT_AUDIO          0x00000010u
#define SDL_INIT_VIDEO          0x00000020u
#define SDL_INIT_EVENTS         0x00004000u

/* SDL_WindowFlags */
#define SDL_WINDOW_FULLSCREEN         0x00000001u
#define SDL_WINDOW_SHOWN              0x00000004u
#define SDL_WINDOW_HIDDEN             0x00000008u
#define SDL_WINDOW_RESIZABLE          0x00000020u
#define SDL_WINDOW_FULLSCREEN_DESKTOP (SDL_WINDOW_FULLSCREEN | 0x00001000u)

#define SDL_WINDOWPOS_UNDEFINED       0x1FFF0000u
#define SDL_WINDOWPOS_CENTERED        0x2FFF0000u

/* SDL_RendererFlags */
#define SDL_RENDERER_SOFTWARE         0x00000001u
#define SDL_RENDERER_ACCELERATED      0x00000002u
#define SDL_RENDERER_PRESENTVSYNC     0x00000004u

/* SDL_PixelFormatEnum & SDL_TextureAccess */
#define SDL_PIXELFORMAT_ARGB8888      0x16362004u
#define SDL_TEXTUREACCESS_STATIC      0
#define SDL_TEXTUREACCESS_STREAMING   1
#define SDL_TEXTUREACCESS_TARGET      2

/* Maus-Buttons */
#define SDL_BUTTON_LEFT               1
#define SDL_BUTTON_MIDDLE             2
#define SDL_BUTTON_RIGHT              3
#define SDL_BUTTON_LMASK              (1u << (SDL_BUTTON_LEFT - 1))
#define SDL_BUTTON_MMASK              (1u << (SDL_BUTTON_MIDDLE - 1))
#define SDL_BUTTON_RMASK              (1u << (SDL_BUTTON_RIGHT - 1))

/* SDL_EventType */
#define SDL_QUIT                      0x100u
#define SDL_KEYDOWN                   0x300u
#define SDL_KEYUP                     0x301u
#define SDL_MOUSEMOTION               0x400u
#define SDL_MOUSEBUTTONDOWN           0x401u
#define SDL_MOUSEBUTTONUP             0x402u
#define SDL_MOUSEWHEEL                0x403u
#define SDL_DROPFILE                  0x1000u

/* SDL_Keymod */
#define KMOD_NONE                     0x0000
#define KMOD_LSHIFT                   0x0001
#define KMOD_RSHIFT                   0x0002
#define KMOD_LCTRL                    0x0040
#define KMOD_RCTRL                    0x0080
#define KMOD_LALT                     0x0100
#define KMOD_RALT                     0x0200
#define KMOD_SHIFT                    (KMOD_LSHIFT | KMOD_RSHIFT)
#define KMOD_CTRL                     (KMOD_LCTRL | KMOD_RCTRL)
#define KMOD_ALT                      (KMOD_LALT | KMOD_RALT)

typedef struct SDL_Window   SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture  SDL_Texture;

typedef struct SDL_Rect {
    int x, y, w, h;
} SDL_Rect;

typedef int32_t SDL_Scancode;
typedef int32_t SDL_Keycode;

typedef struct SDL_Keysym {
    SDL_Scancode scancode;
    SDL_Keycode  sym;
    Uint16       mod;
    Uint32       unused;
} SDL_Keysym;

typedef struct SDL_KeyboardEvent {
    Uint32     type;
    Uint32     timestamp;
    Uint32     windowID;
    Uint8      state;
    Uint8      repeat;
    Uint8      padding2;
    Uint8      padding3;
    SDL_Keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_MouseMotionEvent {
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Uint32 state;
    Sint32 x;
    Sint32 y;
    Sint32 xrel;
    Sint32 yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_MouseButtonEvent {
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Uint8  button;
    Uint8  state;
    Uint8  clicks;
    Uint8  padding1;
    Sint32 x;
    Sint32 y;
} SDL_MouseButtonEvent;

typedef struct SDL_MouseWheelEvent {
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Sint32 x;
    Sint32 y;
    Uint32 direction;
} SDL_MouseWheelEvent;

typedef struct SDL_DropEvent {
    Uint32 type;
    Uint32 timestamp;
    char  *file;
    Uint32 windowID;
} SDL_DropEvent;

typedef union SDL_Event {
    Uint32               type;
    SDL_KeyboardEvent    key;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_MouseWheelEvent  wheel;
    SDL_DropEvent        drop;
    Uint8                padding[56];
} SDL_Event;

int           SDL_Init(Uint32 flags);
void          SDL_Quit(void);
const char   *SDL_GetError(void);
void          SDL_free(void *mem);

SDL_Window   *SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags);
void          SDL_DestroyWindow(SDL_Window *window);
void          SDL_SetWindowTitle(SDL_Window *window, const char *title);
void          SDL_SetWindowSize(SDL_Window *window, int w, int h);
void          SDL_GetWindowSize(SDL_Window *window, int *w, int *h);
void          SDL_SetWindowPosition(SDL_Window *window, int x, int y);
void          SDL_GetWindowPosition(SDL_Window *window, int *x, int *y);
int           SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags);
void          SDL_HideWindow(SDL_Window *window);
void          SDL_ShowWindow(SDL_Window *window);
void          SDL_RaiseWindow(SDL_Window *window);

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags);
void          SDL_DestroyRenderer(SDL_Renderer *renderer);
int           SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h);
int           SDL_RenderClear(SDL_Renderer *renderer);
int           SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect);
void          SDL_RenderPresent(SDL_Renderer *renderer);

SDL_Texture  *SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format, int access, int w, int h);
void          SDL_DestroyTexture(SDL_Texture *texture);
int           SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch);

int           SDL_PollEvent(SDL_Event *event);
int           SDL_PushEvent(SDL_Event *event);
Uint32        SDL_GetMouseState(int *x, int *y);
void          SDL_WarpMouseInWindow(SDL_Window *window, int x, int y);
int           SDL_ShowCursor(int toggle);
void          SDL_SetHandCursor(int hand);

Uint32        SDL_GetTicks(void);
void          SDL_Delay(Uint32 ms);

#ifdef __cplusplus
}
#endif

#endif /* STARTER_SDL2_H */
