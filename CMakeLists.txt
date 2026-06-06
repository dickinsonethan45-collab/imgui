cmake_minimum_required(VERSION 3.22.1)
project(cheesemenu)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui)

add_library(cheesemenu SHARED
    src/main.cpp
    src/ModMenu.cpp
    src/hooks/UnityHook.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(cheesemenu PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    src
)

find_library(log-lib  log)
find_library(egl-lib  EGL)
find_library(gles-lib GLESv3)

target_link_libraries(cheesemenu
    ${log-lib}
    ${egl-lib}
    ${gles-lib}
)

target_compile_options(cheesemenu PRIVATE
    -std=c++17
    -O2
    -fvisibility=hidden
    -fexceptions
    -frtti
)
