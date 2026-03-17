# qtraylib

[English](README.md)

一个小型 C++ 库，让你能在 Qt5 `QOpenGLWidget` 中使用 raylib 的绘图 API。

raylib 本身会接管窗口、OpenGL 上下文和事件循环。qtraylib 将这部分交由 Qt 处理，只保留 GPU 渲染能力（通过 `rlgl`）。将 `QRaylibRenderer` 嵌入任意 `QOpenGLWidget`，无需 `InitWindow`、`WindowShouldClose`，也不依赖 GLFW。

## 工作原理

raylib 的 `core` 模块负责窗口、OpenGL 上下文和事件循环——qtraylib 将这部分替换为 Qt。输入（键盘、鼠标、触屏）直接用 Qt 事件（`QKeyEvent`、`QMouseEvent` 等）处理，无需额外实现。`rshapes`、`rtext`、`rtextures`、`rmodels`、`raudio` 等其他模块不受影响，原有的 raylib 绘图调用可以直接在 `paintGL()` 中使用。

`qtraylib.h` 在 `raylib.h` 基础上，将与 Qt 冲突的 `core` API（`InitWindow`、`BeginDrawing`、`SetTargetFPS` 等）替换为编译期错误，错误信息会提示对应的替代写法。需要感知渲染尺寸的 API（坐标转换、相机模式、计时等）通过 `QRaylibRenderer` 提供。

## 环境要求

- CMake ≥ 3.16
- C++ ≥ 11
- Qt 5（Core、Gui、Widgets、OpenGL）
- raylib（位于 `modules/raylib` 的 git 子模块）
- MSVC（v143）或 MinGW/GCC

## 构建

```sh
git clone --recurse-submodules <repo-url>
cd qtraylib
```

设置 Qt5 路径（Windows / PowerShell）：

```sh
$env:Qt5_PREFIX_PATH = "C:/Qt/5.15.2/msvc2019_64"
```

配置并构建：

```sh
# MSVC
cmake --preset msvc
cmake --build build --config Debug

# MinGW
cmake --preset mingw
cmake --build build --config Debug
```

在 Windows 上，`example` 目标会自动运行 `windeployqt`。

## 使用

> 在 Windows 上，`qraylibrenderer.h`（或 `qtraylib.h`）必须在每个翻译单元中**最先被包含**，以确保 `windows.h` 在所有 Qt 头文件之前完成预处理。

```cpp
#include "qraylibrenderer.h"   // Windows 上必须最先包含

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

| 类别 | 方法 |
|---|---|
| 初始化 / 销毁 | `initialize(widget)`、`shutdown()`、`isReady()` |
| 尺寸调整 | `handleResize(w, h)`、`renderWidth()`、`renderHeight()` |
| 绘制 | `beginDrawing()`、`endDrawing()`、`clearBackground(color)` |
| 2D 摄像机 | `beginMode2D(cam)`、`endMode2D()` |
| 3D 摄像机 | `beginMode3D(cam)`、`endMode3D()` |
| 渲染纹理 | `beginTextureMode(rt)`、`endTextureMode()` |
| 着色器 | `beginShaderMode(s)`、`endShaderMode()`、`loadShader(…)` 等 |
| 混合 / 裁剪 | `beginBlendMode(m)`、`beginScissorMode(…)` 等 |
| VR 立体 | `beginVrStereoMode(cfg)`、`loadVrStereoConfig(dev)` 等 |
| 坐标转换 | `getScreenToWorld2D(…)`、`getWorldToScreen(…)` 等 |
| 计时 | `setTargetFPS(fps)`、`getFPS()`、`getFrameTime()`、`getTime()` |
| 日志 | `traceLog(level, fmt, …)`、`setTraceLogLevel(…)`、`setTraceLogCallback(…)` |

## 示例

`example/main.cpp` — 带鼠标缩放的 2D 摄像机（滚轮或右键拖动缩放，按 `1`/`2` 切换模式）。

## 目录结构

```
src/
  qtraylib.h          — raylib.h + 编译期拦截宏
  qraylibrenderer.h   — QRaylibRenderer 声明
  qraylibrenderer.cpp — QRaylibRenderer 实现（rlgl 后端）
example/
  main.cpp            — 2D 摄像机鼠标缩放演示
modules/
  raylib/             — raylib 子模块
```

## 许可证

qtraylib 采用 [MIT 许可证](LICENSE)。

raylib 采用 [zlib 许可证](modules/raylib/LICENSE)。
