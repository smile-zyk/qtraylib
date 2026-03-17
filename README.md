# qtraylib

[中文](README.zh.md)

A C++ bridge library that embeds [raylib](https://www.raylib.com/) rendering inside a Qt5 `QOpenGLWidget`.

raylib normally owns the window, OpenGL context, and event loop. qtraylib hands all of that to Qt and keeps only the GPU rendering pipeline (via `rlgl`). Drop `QRaylibRenderer` into any `QOpenGLWidget` — no `InitWindow`, no `WindowShouldClose`, no GLFW.

## How it works

Only raylib's `core` module — window, OpenGL context, event loop — is replaced by Qt. Input (keyboard, mouse, touch) is handled directly by Qt events (`QKeyEvent`, `QMouseEvent`, etc.); nothing needs to be re-implemented. All other modules (`rshapes`, `rtext`, `rtextures`, `rmodels`, `raudio`, …) are unaffected and their draw calls work as usual inside `paintGL()`.

`qtraylib.h` includes `raylib.h` and replaces conflicting `core` APIs (`InitWindow`, `BeginDrawing`, `SetTargetFPS`, …) with **compile-time errors** that point to the correct replacement. APIs that need render-size awareness (coordinate conversion, camera modes, timing) are provided by `QRaylibRenderer`.

## Requirements

- CMake ≥ 3.16
- C++ ≥ 11
- Qt 5 (Core, Gui, Widgets, OpenGL)
- raylib (git submodule at `modules/raylib`)
- MSVC (v143) or MinGW/GCC

## Build

```sh
git clone --recurse-submodules <repo-url>
cd qtraylib
```

Set your Qt5 prefix (Windows / PowerShell):

```sh
$env:Qt5_PREFIX_PATH = "C:/Qt/5.15.2/msvc2019_64"
```

Configure and build:

```sh
# MSVC
cmake --preset msvc
cmake --build build --config Debug

# MinGW
cmake --preset mingw
cmake --build build --config Debug
```

The `example` target runs `windeployqt` automatically on Windows.

## Usage

> `qraylibrenderer.h` (or `qtraylib.h`) **must be the first include** in every translation unit on Windows so that `windows.h` is preprocessed before any Qt headers.

```cpp
#include "qraylibrenderer.h"   // must be first on Windows

#include <QApplication>
#include <QOpenGLWidget>

class MyWidget : public QOpenGLWidget
{
    QRaylibRenderer m_renderer;

protected:
    void initializeGL() override
    {
        m_renderer.initialize(this);
        m_renderer.setTargetFPS(60);
    }

    void resizeGL(int w, int h) override
    {
        m_renderer.handleResize(w, h);
    }

    void paintGL() override
    {
        m_renderer.beginDrawing();
        m_renderer.clearBackground(RAYWHITE);

        DrawText("Hello from raylib inside Qt!", 20, 20, 20, DARKGRAY);

        m_renderer.endDrawing();
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MyWidget w;
    w.show();
    return app.exec();
}
```

### API

| Category | Methods |
|---|---|
| Init / shutdown | `initialize(widget)`, `shutdown()`, `isReady()` |
| Resize | `handleResize(w, h)`, `renderWidth()`, `renderHeight()` |
| Drawing | `beginDrawing()`, `endDrawing()`, `clearBackground(color)` |
| 2D camera | `beginMode2D(cam)`, `endMode2D()` |
| 3D camera | `beginMode3D(cam)`, `endMode3D()` |
| Render texture | `beginTextureMode(rt)`, `endTextureMode()` |
| Shader | `beginShaderMode(s)`, `endShaderMode()`, `loadShader(…)`, … |
| Blend / scissor | `beginBlendMode(m)`, `beginScissorMode(…)`, … |
| VR stereo | `beginVrStereoMode(cfg)`, `loadVrStereoConfig(dev)`, … |
| Coordinate conversion | `getScreenToWorld2D(…)`, `getWorldToScreen(…)`, … |
| Timing | `setTargetFPS(fps)`, `getFPS()`, `getFrameTime()`, `getTime()` |
| Logging | `traceLog(level, fmt, …)`, `setTraceLogLevel(…)`, `setTraceLogCallback(…)` |

## Example

`example/main.cpp` — 2D camera with mouse zoom (wheel or right-button-drag, toggle with `1`/`2`).

## Structure

```
src/
  qtraylib.h          — raylib.h + compile-time interception macros
  qraylibrenderer.h   — QRaylibRenderer declaration
  qraylibrenderer.cpp — QRaylibRenderer implementation (rlgl backend)
example/
  main.cpp            — 2D camera mouse-zoom demo
modules/
  raylib/             — raylib submodule
```

## License

qtraylib is licensed under the [MIT License](LICENSE).

raylib is licensed under the [zlib License](modules/raylib/LICENSE).