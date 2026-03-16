cmake_minimum_required(VERSION 3.16)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" OBLIVION_VERSION
    LIMIT_COUNT 1
    REGEX "^[0-9]+\.[0-9]+\.[0-9]+$")

if(OBLIVION_VERSION STREQUAL "")
    message(FATAL_ERROR "VERSION must contain a semantic version like 0.1.0")
endif()

project(oblivion_game VERSION ${OBLIVION_VERSION} LANGUAGES C)

# Ship a release-configured build by default so the produced DLL matches
# the original Quake II release binaries unless the user explicitly asks
# for another configuration.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Collect every C source under src/game and src/common.
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.c"
)

if(GAME_SOURCES STREQUAL "")
    message(FATAL_ERROR "No source files found under src/game")
endif()

add_library(oblivion_game MODULE ${GAME_SOURCES})

target_include_directories(oblivion_game
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/game
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

target_compile_definitions(oblivion_game
    PRIVATE
        _GNU_SOURCE
)

option(OBLIVION_ENABLE_LOCAL_DEPLOY "Copy Windows builds to a local Quake II test install when available" ON)
set(OBLIVION_LOCAL_DEPLOY_DIR "C:/q2Clean/oblivion-re" CACHE PATH
    "Local Quake II game directory used for optional Windows post-build deployment")
set(OBLIVION_TARGET_ARCH "" CACHE STRING
    "Target architecture label (x86, x64, arm64); inferred automatically when left empty")
set(OBLIVION_WINDOWS_OUTPUT_NAME "" CACHE STRING
    "Windows game module filename stem; inferred from OBLIVION_TARGET_ARCH when left empty")

if(OBLIVION_TARGET_ARCH STREQUAL "")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(OBLIVION_TARGET_ARCH "x64")
        else()
            set(OBLIVION_TARGET_ARCH "x86")
        endif()
    elseif(APPLE)
        if(CMAKE_OSX_ARCHITECTURES)
            list(GET CMAKE_OSX_ARCHITECTURES 0 OBLIVION_OSX_ARCH)
        else()
            set(OBLIVION_OSX_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
        endif()

        if(OBLIVION_OSX_ARCH MATCHES "^(x86_64|amd64)$")
            set(OBLIVION_TARGET_ARCH "x64")
        elseif(OBLIVION_OSX_ARCH MATCHES "^(arm64|aarch64)$")
            set(OBLIVION_TARGET_ARCH "arm64")
        elseif(OBLIVION_OSX_ARCH MATCHES "^(i[3-6]86|x86)$")
            set(OBLIVION_TARGET_ARCH "x86")
        endif()
    else()
        if(CMAKE_C_FLAGS MATCHES "(^| )-m32($| )" OR CMAKE_SHARED_LINKER_FLAGS MATCHES "(^| )-m32($| )")
            set(OBLIVION_TARGET_ARCH "x86")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
            set(OBLIVION_TARGET_ARCH "x64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
            set(OBLIVION_TARGET_ARCH "arm64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(i[3-6]86|x86)$")
            set(OBLIVION_TARGET_ARCH "x86")
        endif()
    endif()
endif()

if(OBLIVION_TARGET_ARCH STREQUAL "")
    message(FATAL_ERROR
        "Unable to infer OBLIVION_TARGET_ARCH. Set -DOBLIVION_TARGET_ARCH=x86|x64|arm64 explicitly.")
endif()

if(WIN32)
    if(OBLIVION_WINDOWS_OUTPUT_NAME STREQUAL "")
        if(OBLIVION_TARGET_ARCH STREQUAL "x64")
            set(OBLIVION_WINDOWS_OUTPUT_NAME "gamex86_64")
        elseif(OBLIVION_TARGET_ARCH STREQUAL "x86")
            set(OBLIVION_WINDOWS_OUTPUT_NAME "gamex86")
        else()
            set(OBLIVION_WINDOWS_OUTPUT_NAME "game${OBLIVION_TARGET_ARCH}")
        endif()
    endif()

    # game.def lists the exports that the engine expects on Windows builds.
    target_sources(oblivion_game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/game/game.def)
    set_target_properties(oblivion_game PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${OBLIVION_WINDOWS_OUTPUT_NAME}"
        RUNTIME_OUTPUT_NAME "${OBLIVION_WINDOWS_OUTPUT_NAME}"
        ARCHIVE_OUTPUT_NAME "${OBLIVION_WINDOWS_OUTPUT_NAME}"
        SUFFIX ".dll"
    )

    get_filename_component(OBLIVION_LOCAL_DEPLOY_PARENT "${OBLIVION_LOCAL_DEPLOY_DIR}" DIRECTORY)
    if(OBLIVION_ENABLE_LOCAL_DEPLOY AND EXISTS "${OBLIVION_LOCAL_DEPLOY_PARENT}")
        add_custom_command(TARGET oblivion_game POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OBLIVION_LOCAL_DEPLOY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:oblivion_game>"
                "${OBLIVION_LOCAL_DEPLOY_DIR}/$<TARGET_FILE_NAME:oblivion_game>"
        )
    endif()
else()
    if(OBLIVION_TARGET_ARCH STREQUAL "x64")
        set(OBLIVION_UNIX_OUTPUT_NAME "gamex86_64")
    elseif(OBLIVION_TARGET_ARCH STREQUAL "x86")
        set(OBLIVION_UNIX_OUTPUT_NAME "gamei386")
    elseif(OBLIVION_TARGET_ARCH STREQUAL "arm64")
        set(OBLIVION_UNIX_OUTPUT_NAME "gamearm64")
    else()
        set(OBLIVION_UNIX_OUTPUT_NAME "game${OBLIVION_TARGET_ARCH}")
    endif()

    set_target_properties(oblivion_game PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${OBLIVION_UNIX_OUTPUT_NAME}"
    )

    if(APPLE)
        set_target_properties(oblivion_game PROPERTIES
            SUFFIX ".dylib"
        )
    endif()
endif()

install(TARGETS oblivion_game
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
