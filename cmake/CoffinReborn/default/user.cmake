# Customization hook included by the generated CMakeLists.txt. Unlike
# .generated/*.cmake this file is NOT overwritten when MCC regenerates, and
# .gitignore whitelists it, so project-specific build additions belong here.
#
# The object-library target name below is deterministic. Only the *executable*
# target carries a random suffix (CoffinReborn_default_image_xz_...), which is
# why the release section below finds it by pattern instead of by name.
set(CR_OBJ_TARGET CoffinReborn_default_default_XC32_compile)

# --- Build flavour -----------------------------------------------------------
# OFF: the generated debug build (-g, __DEBUG, console printfs).
# ON:  no debugging flags at all - see "Release build" at the bottom. Same
#      out/CoffinReborn/default.hex either way. Saving this file re-runs CMake
#      on the next build, so flip it and build/flash as usual.
set(CR_RELEASE ON)

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

# --- Release build -----------------------------------------------------------
# The extension's tasks only generate a debug build ("buildType": "debug" in
# tasks.json - "production" is a separate generated tree this file does not
# cover). So instead, strip what .generated/rule.cmake adds for debugging from
# the targets it has already set up:
#   -g, --gdwarf-2                        debug info
#   __DEBUG, __MPLAB_DEBUG (+ linker)     fault handlers hit a software
#                                         breakpoint, which with no debugger
#                                         attached is just a second fault
# and define NDEBUG, which compiles out every console printf. The linker script
# does not reference either debug symbol, so dropping them is safe.
if(CR_RELEASE)
    message(STATUS "CR_RELEASE is ON - building without debugging flags.")

    set(cr_debug_syms "(,--defsym=__MPLAB_DEBUG=1|,--defsym=__DEBUG=1|,--gdwarf-2)")

    get_property(cr_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    list(FILTER cr_targets INCLUDE REGEX "^CoffinReborn_default_(default_XC32_|image_)")

    foreach(t IN LISTS cr_targets)
        foreach(prop COMPILE_OPTIONS LINK_OPTIONS)
            get_target_property(opts ${t} ${prop})
            if(opts)
                list(REMOVE_ITEM opts "-g")
                list(TRANSFORM opts REPLACE "${cr_debug_syms}" "")
                set_target_properties(${t} PROPERTIES ${prop} "${opts}")
            endif()
        endforeach()

        get_target_property(defs ${t} COMPILE_DEFINITIONS)
        if(defs)
            list(REMOVE_ITEM defs "__DEBUG" "__DEBUG=1")
            set_target_properties(${t} PROPERTIES COMPILE_DEFINITIONS "${defs}")
        endif()
    endforeach()

    target_compile_definitions(${CR_OBJ_TARGET} PRIVATE NDEBUG)
endif()
