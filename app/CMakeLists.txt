cmake_minimum_required(VERSION 3.10)
project(degiro-app)

find_package(glfw3 REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

set(SOURCES
    main.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl2.cpp
    imgui/backends/imgui_impl_glfw.cpp
    implot/implot.cpp
    implot/implot_items.cpp
    implot/implot_demo.cpp
)

add_executable(degiro-app ${SOURCES})

target_include_directories(degiro-app PRIVATE imgui imgui/backends implot)

target_link_libraries(degiro-app PRIVATE glfw ${OPENGL_LIBRARIES} degiro)
