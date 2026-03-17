find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

# Enable Qt automatic processing (MOC / UIC / RCC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(qtraylib STATIC
    qtraylib.h
    qraylibrenderer.h
    qraylibrenderer.cpp
)

target_include_directories(qtraylib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        # Expose raylib / rlgl headers to consumers
        ${CMAKE_SOURCE_DIR}/modules/raylib/src
)

target_link_libraries(qtraylib
    PUBLIC
        raylib
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::OpenGL
)

# Treat source files as UTF-8 on MSVC (suppresses C4819 code-page warnings)
if (MSVC)
    target_compile_options(qtraylib PRIVATE /utf-8)
endif()
