cmake_minimum_required(VERSION 3.16)
project(TPE VERSION 1.5.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc, autorcc, autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt (support Qt6 or Qt5)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

message(STATUS "Found Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION}")

# Find SDL 1.2
find_package(SDL)

if(NOT SDL_FOUND)
    # Check bundled SDL 1.2 in cmake/sdl12
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl12/include/SDL/SDL.h")
        message(STATUS "Using bundled SDL 1.2 headers from cmake/sdl12")
        set(SDL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl12/include/SDL")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SDL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl12/lib/x64")
        else()
            set(SDL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl12/lib/x86")
        endif()

        find_library(SDL_LIBRARY NAMES SDL SDL-1.2 libSDL PATHS ${SDL_LIB_DIR} NO_DEFAULT_PATH)
        find_library(SDLMAIN_LIBRARY NAMES SDLmain libSDLmain PATHS ${SDL_LIB_DIR} NO_DEFAULT_PATH)
        if(SDL_LIBRARY)
            set(SDL_FOUND TRUE)
        endif()
    endif()
endif()

if(NOT SDL_FOUND)
    message(FATAL_ERROR "SDL 1.2 not found. Please install libsdl1.2-dev or provide SDL 1.2 libraries.")
endif()

message(STATUS "SDL include: ${SDL_INCLUDE_DIR}")
message(STATUS "SDL library: ${SDL_LIBRARY}")

# Collect source files
set(SOURCES
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/sdl_presentation.h
    src/sdl_presentation.cpp
    src/utils.h
    src/utils.cpp
)

if(WIN32)
    list(APPEND SOURCES src/tpe.rc)
endif()

add_executable(tpe ${SOURCES})

# Target properties
if(WIN32)
    set_target_properties(tpe PROPERTIES WIN32_EXECUTABLE ON)
endif()

target_include_directories(tpe PRIVATE
    src
    ${SDL_INCLUDE_DIR}
)

target_link_libraries(tpe PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${SDL_LIBRARY}
)

if(SDLMAIN_LIBRARY)
    target_link_libraries(tpe PRIVATE ${SDLMAIN_LIBRARY})
endif()

# Copy images and assets to the build output directory for CLion / out-of-source builds
add_custom_command(TARGET tpe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "$<TARGET_FILE_DIR:tpe>/images"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:tpe>/assets"
    COMMENT "Copying images and assets to build output directory"
)

# On Windows, copy SDL DLLs to target folder if bundled
if(WIN32 AND DEFINED SDL_LIB_DIR)
    file(GLOB BUNDLED_DLLS "${SDL_LIB_DIR}/*.dll")
    foreach(DLL_FILE ${BUNDLED_DLLS})
        add_custom_command(TARGET tpe POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_FILE}"
                "$<TARGET_FILE_DIR:tpe>/"
        )
    endforeach()
endif()

# Install rules
install(TARGETS tpe DESTINATION .)
install(DIRECTORY images DESTINATION .)
install(DIRECTORY assets DESTINATION .)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    install(DIRECTORY examples DESTINATION .)
endif()
install(FILES LICENSE OFL.txt README.md DESTINATION .)
