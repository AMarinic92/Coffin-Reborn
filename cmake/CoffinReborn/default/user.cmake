# Customization hook included by the generated CMakeLists.txt. Unlike
# .generated/*.cmake this file is NOT overwritten when MCC regenerates, and
# .gitignore whitelists it, so project-specific build additions belong here.
#
# The object-library target name below is deterministic. Only the *executable*
# target carries a random suffix (CoffinReborn_default_image_xz_...), which is
# why nothing here refers to it.
set(CR_OBJ_TARGET CoffinReborn_default_default_XC32_compile)

# --- lib/ws2812-spi (git submodule) ------------------------------------------
# Not in the .mplab.json fileSet, so MCC will never emit these into file.cmake.
set(WS2812_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../lib/ws2812-spi")

target_sources(${CR_OBJ_TARGET} PRIVATE
    "${WS2812_DIR}/src/ws2812.c"
    "${WS2812_DIR}/src/ws2812_fx.c"
    "${WS2812_DIR}/src/ws2812_freertos.c")

target_include_directories(${CR_OBJ_TARGET} PRIVATE "${WS2812_DIR}/include")

# ws2812_freertos.c compiles to nothing without this.
target_compile_definitions(${CR_OBJ_TARGET} PRIVATE WS2812_ENABLE_FREERTOS)

# --- lib/dy_sound_same51 (git submodule) --------------------------------------
# DY-HV20T / DY-SV MP3 module over a SERCOM USART. Same arrangement as ws2812
# above: MCC configures the SERCOM, the driver only drives it.
set(DY_SOUND_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../lib/dy_sound_same51")

# Guarded on the sources actually being there. A submodule that has been added
# but not yet populated (cloned before its branch was pushed, or a fresh clone
# without --recursive) would otherwise break the whole build instead of just
# leaving the sound out. src/sound.c keys off DY_SOUND_AVAILABLE and compiles to
# stubs without it, so main.c needs no #ifdef either way.
if(EXISTS "${DY_SOUND_DIR}/src/dy_sound.c")
    target_sources(${CR_OBJ_TARGET} PRIVATE
        "${DY_SOUND_DIR}/src/dy_sound.c"
        "${DY_SOUND_DIR}/src/dy_sound_bank.c"
        "${DY_SOUND_DIR}/src/dy_sound_freertos.c")

    target_include_directories(${CR_OBJ_TARGET} PRIVATE "${DY_SOUND_DIR}/include")

    # dy_sound_freertos.c compiles to nothing without the first of these.
    target_compile_definitions(${CR_OBJ_TARGET} PRIVATE
        DY_SOUND_ENABLE_FREERTOS
        DY_SOUND_AVAILABLE)
else()
    message(STATUS
        "dy_sound_same51 submodule is empty - sound stubbed out. "
        "Run: git submodule update --init --remote lib/dy_sound_same51")
endif()
