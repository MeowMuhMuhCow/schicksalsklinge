cmake_minimum_required(VERSION 3.16)
project(SchickPort VERSION 0.1.0 LANGUAGES C CXX)

# ============================================================================
# DSA 1: Die Schicksalsklinge (v3.02 de) - Nativer Windows-Port (Etappe 1)
# ============================================================================
# Spielcode: Rekonstruierter C-Quelltext dieser Fassung (MooMoo, Quelle/, inkl.
#            s_fix.h mit 284 Bugfixes, FIX_HIMMEL, FIX_0437..0439, FIX_SPEICHERN,
#            FIX_PROBENLOG).
# Plattform: Gemeinsame SDL2-Plattformschicht der Nordlandtrilogie
#            (Port/platform/dsa_platform_sdl2.cpp + Port/platform/vgalib.c)
#            sowie C-Ersatz fuer seg008..011 und nativer Spielstand-Adapter
#            (datseg_status.h) nach BrightEyes-SDL2 / MobileEyes (GPL v3).
# ============================================================================

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(FIX_BUGS "2" CACHE STRING "Fehlerbehebungsstufe fuer s_fix.h: 0=Original, 1=Wichtig, 2=Alles")
set(SKIP_COPY_PROTECTION "1" CACHE STRING "Kopierschutzabfrage standardmaessig ueberspringen (1=ja, 0=nein)")

# 1. Port/src/ aus Quelle/ vorbereiten, falls noch nicht geschehen
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/datseg_status.h")
    message(STATUS "Bereite Schicksalsklinge-Quellen in Port/src/ vor...")
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_sources.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE PREP_RES
    )
    if(NOT PREP_RES EQUAL 0)
        message(FATAL_ERROR "prepare_sources.py fehlgeschlagen")
    endif()
endif()

# 2. Alle 110 C-Module des Spiels sammeln (datseg.c + seg001.c .. seg122.c)
file(GLOB SCHICK_GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/datseg.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ngen_embed.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/seg*.c"
)

find_package(SDL2 QUIET)

add_executable(SCHICK_PORT
    ${SCHICK_GAME_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/vgalib.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/dsa_platform_sdl2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/dsa_intro.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/sdl2_win32_provider.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb_vorbis/stb_vorbis.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/dbopl/dbopl.cpp"
)

target_include_directories(SCHICK_PORT PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/AIL"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/dbopl"
)

target_compile_definitions(SCHICK_PORT PRIVATE
    FIX_BUGS=${FIX_BUGS}
    SKIP_COPY_PROTECTION=${SKIP_COPY_PROTECTION}
)

target_compile_options(SCHICK_PORT PRIVATE
    -m32
    -O2
    -fpack-struct=1
    -fsigned-char
    -Wno-unused-result
)

if(WIN32)
    target_link_options(SCHICK_PORT PRIVATE -m32 -mwindows)
    target_link_libraries(SCHICK_PORT PRIVATE gdi32 user32 winmm)
else()
    target_link_options(SCHICK_PORT PRIVATE -m32)
endif()

if(SDL2_FOUND)
    target_link_libraries(SCHICK_PORT PRIVATE ${SDL2_LIBRARIES})
endif()
