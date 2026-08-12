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
