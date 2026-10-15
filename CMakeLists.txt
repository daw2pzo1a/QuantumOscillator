cmake_minimum_required(VERSION 3.20)

project(QuantumOscillator
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

# Dear ImGui

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.7
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_include_directories(imgui
    PUBLIC
    ${imgui_SOURCE_DIR}
)

# rlImGui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG Raylib_5_5
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(rlimgui)

if(NOT rlimgui_POPULATED)
    FetchContent_Populate(rlimgui)
endif()

add_library(rlimgui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

target_include_directories(rlimgui
    PUBLIC
    ${rlimgui_SOURCE_DIR}
)

target_link_libraries(rlimgui
    PUBLIC
    raylib
    imgui
)

# ImPlot
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.17
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(implot)

if(NOT implot_POPULATED)
    FetchContent_Populate(implot)
endif()

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(implot
    PUBLIC
    ${implot_SOURCE_DIR}
)

target_link_libraries(implot
    PUBLIC
    imgui
)

# приложение
add_executable(QuantumOscillator
    src/main.cpp
    src/core/QuantumSystem.cpp
    src/math/Grid.cpp
    src/math/FiniteDifference.cpp
    src/math/TrigSweepSolver.cpp
    src/math/SpectrumSolver.cpp
    src/math/WaveFunctionSolver.cpp
)

target_link_libraries(QuantumOscillator
    PRIVATE
    raylib
    imgui
    rlimgui
    implot
)