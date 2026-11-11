cmake_minimum_required(VERSION 3.16)
project(TPE VERSION 1.5.0 LANGUAGES C CXX)

if(WIN32)
    if(NOT CMAKE_RC_COMPILER OR CMAKE_RC_COMPILER STREQUAL "windres")
        get_filename_component(_C_COMPILER_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
        find_program(_RC_FOUND NAMES windres.exe windres HINTS "${_C_COMPILER_DIR}")
        if(_RC_FOUND)
            set(CMAKE_RC_COMPILER "${_RC_FOUND}" CACHE FILEPATH "Resource compiler" FORCE)
        endif()
    endif()
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc, autorcc, autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Auto-detect common Qt installations on Windows if not already in CMAKE_PREFIX_PATH
if(WIN32)
    file(GLOB _QT_SEARCH_DIRS
        "C:/Qt/*/*/mingw*"
        "C:/Qt/*/mingw*"
        "C:/Qt/*/*/msvc*"
        "C:/Qt/*/msvc*"
        "C:/Qt/*"
        "C:/msys64/mingw64"
        "C:/msys64/ucrt64"
        "$ENV{QTDIR}"
        "$ENV{Qt6_DIR}"
        "$ENV{Qt5_DIR}"
    )
    foreach(_DIR ${_QT_SEARCH_DIRS})
        if(EXISTS "${_DIR}/lib/cmake/Qt6" OR EXISTS "${_DIR}/lib/cmake/Qt5" OR EXISTS "${_DIR}/bin/qmake.exe")
            message(STATUS "Auto-detected Qt directory: ${_DIR}")
            list(APPEND CMAKE_PREFIX_PATH "${_DIR}")
        endif()
    endforeach()
endif()

# Find Qt (support Qt6 or Qt5)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets)

if(NOT QT_FOUND)
    message(FATAL_ERROR
        "\n================================================================================\n"
        " [ERREUR] Qt (Qt5 ou Qt6 avec le module Widgets) n'a pas Ã©tÃ© trouvÃ© !\n\n"
        " Pour rÃ©soudre cette erreur dans CLion :\n\n"
        " 1. Si Qt est dÃ©jÃ  installÃ© sur votre machine (ex: C:/Qt/6.x/mingw_64 ou C:/msys64/mingw64) :\n"
        "    Dans CLion -> File -> Settings -> Build, Execution, Deployment -> CMake\n"
        "    Dans 'CMake options', ajoutez :\n"
        "       -DCMAKE_PREFIX_PATH=C:/chemin/vers/votre/Qt\n"
        "    (Exemple : -DCMAKE_PREFIX_PATH=C:/Qt/6.7.0/mingw_64 ou C:/msys64/mingw64)\n\n"
        " 2. Si Qt n'est pas encore installÃ© pour MinGW :\n"
        "    - Via MSYS2 (recommandÃ© si vous utilisez MinGW) :\n"
        "      pacman -S mingw-w64-x86_64-qt5-base\n"
        "    - Ou via l'installateur officiel Qt (https://www.qt.io/download-qt-installer)\n"
        "      en cochant les composants MinGW correspondant Ã  votre version.\n"
        "================================================================================\n"
    )
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Svg)

message(STATUS "Found Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION}")

# Collect source files
set(SOURCES
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/utils.h
    src/utils.cpp
)

if(WIN32)
    list(APPEND SOURCES src/tpe.rc)
endif()

add_executable(tpe ${SOURCES})
target_compile_definitions(tpe PRIVATE TPE_VERSION="${PROJECT_VERSION}")

# Target properties
if(WIN32)
    set_target_properties(tpe PROPERTIES WIN32_EXECUTABLE ON)
endif()

target_include_directories(tpe PRIVATE
    src
)

target_link_libraries(tpe PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
)

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

# Install rules
install(TARGETS tpe DESTINATION .)
install(DIRECTORY images DESTINATION .)
install(DIRECTORY assets DESTINATION .)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    install(DIRECTORY examples DESTINATION .)
endif()
install(FILES LICENSE README.md DESTINATION .)
