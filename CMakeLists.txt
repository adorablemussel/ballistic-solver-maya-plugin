#########################
# KONFIGURACJA PROJEKTU #
#########################

# minimalna wersja Cmake
cmake_minimum_required(VERSION 3.27)

# nazwa projektu i użytych języków (cpp i cuda)
project(BallisticSolver LANGUAGES CXX CUDA)

# cuda jest samodzielnie wykrywana przez CMake - nie potrzebne jest podawanie ścieżki
# ustawienie architektury CUDA na 8.6 (dla kart Ampere, w tym mojej karty RTX3060)
set(CMAKE_CUDA_ARCHITECTURES 86)

# wymuszenie standardów (cpp 17 i cuda 17 dla Mayi 2026)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)

###########
# ŚCIEŻKI #
###########

# definicje ścieżek
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
set(MAYA_DEVKIT_BASE_DIR "${THIRD_PARTY_DIR}/MayaDevkitBase2026")
set(GMSH_SDK_DIR "${THIRD_PARTY_DIR}/GmshSDK")

###################
# ŚRODOWISKO MAYI #
###################

# pakiety Mayi
list(APPEND CMAKE_MODULE_PATH "${MAYA_DEVKIT_BASE_DIR}/cmake/modules")
set(MAYA_LOCATION "${MAYA_DEVKIT_BASE_DIR}")
find_package(Maya REQUIRED)

##################
# PLIKI PROJEKTU #
##################

set(SOURCE_FILES
    "src/main.cpp"
    "src/HelloWorldCommand.cpp"
    "src/HelloWorldNode.cpp" 
    "src/HelloWorldDrawOverride.cpp"
)

set(HEADER_FILES
    "include/HelloWorldCommand.h"
    "include/HelloWorldNode.h"
    "include/HelloWorldDrawOverride.h"
)

###############
# BUILDOWANIE #
###############

# budowanie pluginu
add_library(${PROJECT_NAME} SHARED
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

# zmiana rozszerzenia z domyślnego .dll na .mll
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".mll"
)

##############
# LINKOWANIE #
##############

# nagłówki projektu
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# nagłówki i biblioteki Mayi
target_include_directories(${PROJECT_NAME} PRIVATE "${MAYA_DEVKIT_BASE_DIR}/include")
target_link_directories(${PROJECT_NAME} PRIVATE "${MAYA_DEVKIT_BASE_DIR}/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Foundation.lib 
    OpenMaya.lib
    OpenMayaRender.lib
    OpenMayaUI.lib
)

# nagłówki i biblioteki Gmsh
target_include_directories(${PROJECT_NAME} PRIVATE "${GMSH_SDK_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE "${GMSH_SDK_DIR}/lib/msvc-x64/gmsh.dll.lib")

#################
# AUTOMATYZACJA #
#################

# target: wyładowanie pluginu w Mayi (przed kompilacją)
add_custom_target(unload_plugin
    COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/dev/unload_plugin_via_port.py"
    VERBATIM
)

# zależność odłączenia pluginu zawsze przed kompilacją
add_dependencies(${PROJECT_NAME} unload_plugin)

# kopiowanie gmsh.dll i wczytanie ponownie pluginu w Mayi
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    # kopiowanie biblioteki Gmsh obok pluginu
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GMSH_SDK_DIR}/lib/msvc-x64/gmsh.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>

    # załadowanie skompilowanej z powrotem wtyczki do Mayi
    COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/dev/load_plugin_via_port.py"
    VERBATIM
)