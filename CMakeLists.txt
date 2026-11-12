#########################
# KONFIGURACJA PROJEKTU #
#########################

cmake_minimum_required(VERSION 3.27)
project(BallisticSolver LANGUAGES CXX CUDA)

set(CMAKE_CUDA_ARCHITECTURES 86)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)

###########
# ŚCIEŻKI #
###########

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
set(MAYA_DEVKIT_BASE_DIR "${THIRD_PARTY_DIR}/MayaDevkitBase2026")
set(MAYA_PLUGINS_DIR "C:/Users/Szymon/Documents/maya/2026/plug-ins")

if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GMSH_SDK_DIR "${THIRD_PARTY_DIR}/GmshSDK-Release")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GMSH_SDK_DIR "${THIRD_PARTY_DIR}/GmshSDK-Debug")
endif ()

###################
# ŚRODOWISKO MAYI #
###################

list(APPEND CMAKE_MODULE_PATH "${MAYA_DEVKIT_BASE_DIR}/cmake/modules")
set(MAYA_LOCATION "${MAYA_DEVKIT_BASE_DIR}")
find_package(Maya REQUIRED)

##################
# PLIKI PROJEKTU #
##################

set(HEADER_FILES 
    "include/bMaterial.h" 
    "include/bBodyData.h" 
    "include/bMaterialNode.h" 
    "include/bMaterialData.h" 
    "include/bMeshData.h" 
    "include/bMesherNode.h" 
    "include/bMeshVisualizationNode.h" 
    "include/bMeshVisualizationData.h" 
    "include/bMeshVisualizationDrawOverride.h" 
    "include/bBodyNode.h" 
    "include/bSolverNode.h" 
    "include/bSolverEngine.h" 
    "include/bBodyState.h")

set(SOURCE_FILES
    "src/main.cpp"
    "src/bBodyData.cpp" 
    "src/bMaterialNode.cpp" 
    "src/bMaterialData.cpp" 
    "src/bMeshData.cpp" 
    "src/bMesherNode.cpp" 
    "src/bMeshVisualizationNode.cpp" 
    "src/bMeshVisualizationData.cpp" 
    "src/bMeshVisualizationDrawOverride.cpp" 
    "src/bBodyNode.cpp" 
    "src/bSolverNode.cpp" 
    "src/bSolverEngine.cpp")

###############
# BUILDOWANIE #
###############

add_library(${PROJECT_NAME} SHARED
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".mll"
)

##############
# LINKOWANIE #
##############

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_include_directories(${PROJECT_NAME} PRIVATE "${MAYA_DEVKIT_BASE_DIR}/include")
target_link_directories(${PROJECT_NAME} PRIVATE "${MAYA_DEVKIT_BASE_DIR}/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Foundation.lib 
    OpenMaya.lib
    OpenMayaRender.lib
    OpenMayaUI.lib
    OpenMayaAnim.lib
    delayimp.lib
)

target_include_directories(${PROJECT_NAME} PRIVATE "${GMSH_SDK_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE "${GMSH_SDK_DIR}/lib/gmsh.dll.lib")

target_link_options(${PROJECT_NAME} PRIVATE "/DELAYLOAD:gmsh.dll")

#################
# AUTOMATYZACJA #
#################

add_custom_target(unload_plugin
    COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/dev/unload_plugin_via_port.py"
    VERBATIM
)

add_dependencies(${PROJECT_NAME} unload_plugin)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GMSH_SDK_DIR}/lib/gmsh.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:${PROJECT_NAME}>
    "${MAYA_PLUGINS_DIR}"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GMSH_SDK_DIR}/lib/gmsh.dll"
    "${MAYA_PLUGINS_DIR}"

    COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/dev/load_plugin_via_port.py"
    VERBATIM
)