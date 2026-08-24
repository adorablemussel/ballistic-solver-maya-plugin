# minimalna wersja Cmake
cmake_minimum_required(VERSION 3.27)

# definicja nazwy projektu i użytych języków (cpp i cuda)
project(BallisticSolver LANGUAGES CXX CUDA)

# cuda jest samodzielnie wykrywana przez CMake - nie potrzebne jest podawanie ścieżki
# ustawienie architektury CUDA na 8.6 (dla kart Ampere, w tym mojej karty RTX3060)
set(CMAKE_CUDA_ARCHITECTURES 86)

# wymuszenie standardów cpp 17 i cuda 17 dla Mayi 2026
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)

# lokalizacja devkitu Mayi
set(MAYA_DEVKIT_BASE "C:/Users/Szymon/Desktop/PRACA_INZYNIERSKA/BallisticSolver/third-party/MayaDevkitBase2026")
list(APPEND CMAKE_MODULE_PATH "${MAYA_DEVKIT_BASE}/cmake/modules")

# znalezienie pakietu Mayi (wymagane do kompilacji pluginu)
set(MAYA_LOCATION "${MAYA_DEVKIT_BASE}")
find_package(Maya REQUIRED)

# definicja plików źródłowych i nagłówkowych pluginu
add_library(${PROJECT_NAME} SHARED 
    "src/main.cpp"
    "src/HelloWorldCommand.cpp" 
    "include/HelloWorldCommand.h"
    "include/HelloWorldNode.h"
    "src/HelloWorldNode.cpp" "src/HelloWorldDrawOverride.cpp" "include/HelloWorldDrawOverride.h")

# zmiana rozszerzenia z domyślnego .dll na .mll
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".mll"
)

# pliki nagłówkowe .h i .cuh
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# linkowanie devkitu Mayi
target_include_directories(${PROJECT_NAME} PRIVATE "${MAYA_DEVKIT_BASE}/include")
target_link_directories(${PROJECT_NAME} PRIVATE "${MAYA_DEVKIT_BASE}/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Foundation.lib 
    OpenMaya.lib
    OpenMayaRender.lib
    OpenMayaUI.lib
)

# podłączenie Gmsh SDK
set(GMSH_SDK_DIR "C:/Users/Szymon/Desktop/PRACA_INZYNIERSKA/BallisticSolver/third-party/GmshSDK")
target_include_directories(${PROJECT_NAME} PRIVATE "${GMSH_SDK_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE "${GMSH_SDK_DIR}/lib/msvc-x64/gmsh.dll.lib")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GMSH_SDK_DIR}/lib/msvc-x64/gmsh.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

