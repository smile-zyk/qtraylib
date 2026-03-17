#pragma once
// ---------------------------------------------------------------------------
// qtraylib.h
//
// Includes raylib.h for all types and non-intercepted APIs, then applies
// compile-time interception to the following functions:
//
//   (A) Fully replaced by Qt APIs (window, cursor, keyboard, mouse, touch, logging)
//   (B) Must be called via QRaylibRenderer (drawing, shaders, timing, etc.)
//
// The QRaylibRenderer .cpp file defines QTRAYLIB_IMPLEMENTATION before including
// this header to bypass interception and call raw raylib / rlgl C functions directly.
//
// IMPORTANT (Windows): qtraylib.h must be included BEFORE any Qt (or other Windows)
// headers in every translation unit. This ensures windows.h is first processed with
// WIN32_LEAN_AND_MEAN + NOGDI + NOUSER, which prevents the Win32 API declarations
// that conflict with raylib names (Rectangle, CloseWindow, ShowCursor, PlaySound, ...)
// from ever being emitted in this translation unit.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Windows: pre-include windows.h with the three suppression macros before
// raylib to prevent conflicting Win32 API declarations:
//   - WIN32_LEAN_AND_MEAN  excludes mmsystem.h  → PlaySound() / PlaySoundA()
//   - NOGDI               excludes wingdi.h     → Rectangle(), DrawText(), DrawTextEx(), ...
//   - NOUSER              excludes winuser.h    → CloseWindow(), ShowCursor(), LoadImage(), ...
// Because windows.h is include-guarded (_WINDOWS_), this first inclusion
// determines which sub-headers are pulled in; any later Qt / system
// #include <windows.h> is a no-op.
// Qt is unaffected: Qt public headers forward-declare Win32 handle types
// (HWND, HMENU, ...) themselves and do not require the full winuser.h.
// ---------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX
#include <windows.h>
#endif

#include <raylib.h>

#ifndef QTRAYLIB_IMPLEMENTATION

// ===========================================================================
// 1. Window management  →  Qt / QRaylibRenderer::initialize()
// ===========================================================================
#define InitWindow(...)               static_assert(false, "[qtraylib] Use QRaylibRenderer::Initialize() instead of InitWindow()")
#define CloseWindow(...)              static_assert(false, "[qtraylib] Use QRaylibRenderer::ShutDown() instead of CloseWindow()")
#define WindowShouldClose(...)        static_assert(false, "[qtraylib] Use QApplication::quit() or QWindow::close()")
#define IsWindowReady(...)            static_assert(false, "[qtraylib] Use QRaylibRenderer::IsReady()")
#define IsWindowFullscreen(...)       static_assert(false, "[qtraylib] Use QWindow::windowState() & Qt::WindowFullScreen")
#define IsWindowHidden(...)           static_assert(false, "[qtraylib] Use !QWidget::isVisible()")
#define IsWindowMinimized(...)        static_assert(false, "[qtraylib] Use QWidget::isMinimized()")
#define IsWindowMaximized(...)        static_assert(false, "[qtraylib] Use QWidget::isMaximized()")
#define IsWindowFocused(...)          static_assert(false, "[qtraylib] Use QWidget::hasFocus()")
#define IsWindowResized(...)          static_assert(false, "[qtraylib] Override resizeGL() and call QRaylibRenderer::HandleResize()")
#define IsWindowState(...)            static_assert(false, "[qtraylib] Use Qt window state / flags APIs")
#define SetWindowState(...)           static_assert(false, "[qtraylib] Use Qt window state / flags APIs")
#define ClearWindowState(...)         static_assert(false, "[qtraylib] Use Qt window state / flags APIs")
#define ToggleFullscreen(...)         static_assert(false, "[qtraylib] Use QWidget::showFullScreen() / showNormal()")
#define ToggleBorderlessWindowed(...) static_assert(false, "[qtraylib] Use Qt::FramelessWindowHint flag")
#define MaximizeWindow(...)           static_assert(false, "[qtraylib] Use QWidget::showMaximized()")
#define MinimizeWindow(...)           static_assert(false, "[qtraylib] Use QWidget::showMinimized()")
#define RestoreWindow(...)            static_assert(false, "[qtraylib] Use QWidget::showNormal()")
#define SetWindowIcon(...)            static_assert(false, "[qtraylib] Use QWindow::setIcon()")
#define SetWindowIcons(...)           static_assert(false, "[qtraylib] Use QWindow::setIcon()")
#define SetWindowTitle(...)           static_assert(false, "[qtraylib] Use QWidget::setWindowTitle()")
#define SetWindowPosition(...)        static_assert(false, "[qtraylib] Use QWidget::move()")
#define SetWindowMonitor(...)         static_assert(false, "[qtraylib] Use QWindow::setScreen()")
#define SetWindowMinSize(...)         static_assert(false, "[qtraylib] Use QWidget::setMinimumSize()")
#define SetWindowMaxSize(...)         static_assert(false, "[qtraylib] Use QWidget::setMaximumSize()")
#define SetWindowSize(...)            static_assert(false, "[qtraylib] Use QWidget::resize()")
#define SetWindowOpacity(...)         static_assert(false, "[qtraylib] Use QWidget::setWindowOpacity()")
#define SetWindowFocused(...)         static_assert(false, "[qtraylib] Use QWidget::setFocus()")
#define GetWindowHandle(...)          static_assert(false, "[qtraylib] Use QWidget::winId() or QWindow::winId()")
#define GetScreenWidth(...)           static_assert(false, "[qtraylib] Use QRaylibRenderer::RenderWidth()")
#define GetScreenHeight(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::RenderHeight()")
#define GetRenderWidth(...)           static_assert(false, "[qtraylib] Use QRaylibRenderer::RenderWidth()")
#define GetRenderHeight(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::RenderHeight()")
#define GetMonitorCount(...)          static_assert(false, "[qtraylib] Use QGuiApplication::screens().count()")
#define GetCurrentMonitor(...)        static_assert(false, "[qtraylib] Use QWindow::screen()")
#define GetMonitorPosition(...)       static_assert(false, "[qtraylib] Use QScreen::geometry().topLeft()")
#define GetMonitorWidth(...)          static_assert(false, "[qtraylib] Use QScreen::size().width()")
#define GetMonitorHeight(...)         static_assert(false, "[qtraylib] Use QScreen::size().height()")
#define GetMonitorPhysicalWidth(...)  static_assert(false, "[qtraylib] Use QScreen::physicalSize().width()")
#define GetMonitorPhysicalHeight(...) static_assert(false, "[qtraylib] Use QScreen::physicalSize().height()")
#define GetMonitorRefreshRate(...)    static_assert(false, "[qtraylib] Use QScreen::refreshRate()")
#define GetWindowPosition(...)        static_assert(false, "[qtraylib] Use QWidget::pos()")
#define GetWindowScaleDPI(...)        static_assert(false, "[qtraylib] Use QWindow::devicePixelRatio()")
#define GetMonitorName(...)           static_assert(false, "[qtraylib] Use QScreen::name()")
#define SetClipboardText(...)         static_assert(false, "[qtraylib] Use QGuiApplication::clipboard()->setText()")
#define GetClipboardText(...)         static_assert(false, "[qtraylib] Use QGuiApplication::clipboard()->text()")
#define GetClipboardImage(...)        static_assert(false, "[qtraylib] Use QGuiApplication::clipboard()->image()")
#define EnableEventWaiting(...)       static_assert(false, "[qtraylib] Qt event loop manages this automatically")
#define DisableEventWaiting(...)      static_assert(false, "[qtraylib] Qt event loop manages this automatically")

// ===========================================================================
// 2. Cursor management  →  Qt QApplication::setOverrideCursor() / QWidget::setCursor()
// ===========================================================================
#define ShowCursor(...)               static_assert(false, "[qtraylib] Use QApplication::restoreOverrideCursor()")
#define HideCursor(...)               static_assert(false, "[qtraylib] Use QApplication::setOverrideCursor(Qt::BlankCursor)")
#define IsCursorHidden(...)           static_assert(false, "[qtraylib] Check QApplication::overrideCursor() != nullptr")
#define IsCursorOnScreen(...)         static_assert(false, "[qtraylib] Use widget->underMouse()")
#define EnableCursor(...)             static_assert(false, "[qtraylib] Use widget->releaseMouse() and restore cursor")
#define DisableCursor(...)            static_assert(false, "[qtraylib] Use widget->grabMouse() and QApplication::setOverrideCursor(Qt::BlankCursor)")

// ===========================================================================
// 3. Drawing control  →  QRaylibRenderer::beginDrawing() etc.
// ===========================================================================
#define BeginDrawing(...)             static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginDrawing()")
#define EndDrawing(...)               static_assert(false, "[qtraylib] Use QRaylibRenderer::EndDrawing()")
#define ClearBackground(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::ClearBackground()")
#define BeginMode2D(...)              static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginMode2D()")
#define EndMode2D(...)                static_assert(false, "[qtraylib] Use QRaylibRenderer::EndMode2D()")
#define BeginMode3D(...)              static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginMode3D()")
#define EndMode3D(...)                static_assert(false, "[qtraylib] Use QRaylibRenderer::EndMode3D()")
#define BeginTextureMode(...)         static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginTextureMode()")
#define EndTextureMode(...)           static_assert(false, "[qtraylib] Use QRaylibRenderer::EndTextureMode()")
#define BeginShaderMode(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginShaderMode()")
#define EndShaderMode(...)            static_assert(false, "[qtraylib] Use QRaylibRenderer::EndShaderMode()")
#define BeginBlendMode(...)           static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginBlendMode()")
#define EndBlendMode(...)             static_assert(false, "[qtraylib] Use QRaylibRenderer::EndBlendMode()")
#define BeginScissorMode(...)         static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginScissorMode()")
#define EndScissorMode(...)           static_assert(false, "[qtraylib] Use QRaylibRenderer::EndScissorMode()")
#define BeginVrStereoMode(...)        static_assert(false, "[qtraylib] Use QRaylibRenderer::BeginVrStereoMode()")
#define EndVrStereoMode(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::EndVrStereoMode()")

// ===========================================================================
// 4. VR stereo rendering config  →  QRaylibRenderer
// ===========================================================================
#define LoadVrStereoConfig(...)       static_assert(false, "[qtraylib] Use QRaylibRenderer::LoadVrStereoConfig()")
#define UnloadVrStereoConfig(...)     static_assert(false, "[qtraylib] Use QRaylibRenderer::UnloadVrStereoConfig()")

// ===========================================================================
// 5. Shader management  →  QRaylibRenderer
// ===========================================================================
#define LoadShader(...)               static_assert(false, "[qtraylib] Use QRaylibRenderer::LoadShader()")
#define LoadShaderFromMemory(...)     static_assert(false, "[qtraylib] Use QRaylibRenderer::LoadShaderFromMemory()")
#define IsShaderValid(...)            static_assert(false, "[qtraylib] Use QRaylibRenderer::IsShaderValid()")
#define GetShaderLocation(...)        static_assert(false, "[qtraylib] Use QRaylibRenderer::GetShaderLocation()")
#define GetShaderLocationAttrib(...)  static_assert(false, "[qtraylib] Use QRaylibRenderer::GetShaderLocationAttrib()")
#define SetShaderValue(...)           static_assert(false, "[qtraylib] Use QRaylibRenderer::SetShaderValue()")
#define SetShaderValueV(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::SetShaderValueV()")
#define SetShaderValueMatrix(...)     static_assert(false, "[qtraylib] Use QRaylibRenderer::SetShaderValueMatrix()")
#define SetShaderValueTexture(...)    static_assert(false, "[qtraylib] Use QRaylibRenderer::SetShaderValueTexture()")
#define UnloadShader(...)             static_assert(false, "[qtraylib] Use QRaylibRenderer::UnloadShader()")

// ===========================================================================
// 6. Screen-space coordinate conversion  →  QRaylibRenderer (injects Qt render dimensions)
// ===========================================================================
#define GetScreenToWorldRay(...)      static_assert(false, "[qtraylib] Use QRaylibRenderer::GetScreenToWorldRay()")
#define GetScreenToWorldRayEx(...)    static_assert(false, "[qtraylib] Use QRaylibRenderer::GetScreenToWorldRayEx()")
#define GetWorldToScreen(...)         static_assert(false, "[qtraylib] Use QRaylibRenderer::GetWorldToScreen()")
#define GetWorldToScreenEx(...)       static_assert(false, "[qtraylib] Use QRaylibRenderer::GetWorldToScreenEx()")
#define GetWorldToScreen2D(...)       static_assert(false, "[qtraylib] Use QRaylibRenderer::GetWorldToScreen2D()")
#define GetScreenToWorld2D(...)       static_assert(false, "[qtraylib] Use QRaylibRenderer::GetScreenToWorld2D()")
#define GetCameraMatrix(...)          static_assert(false, "[qtraylib] Use QRaylibRenderer::GetCameraMatrix()")
#define GetCameraMatrix2D(...)        static_assert(false, "[qtraylib] Use QRaylibRenderer::GetCameraMatrix2D()")

// ===========================================================================
// 7. Timing  →  QRaylibRenderer (based on QElapsedTimer, no GLFW dependency)
// ===========================================================================
#define SetTargetFPS(...)             static_assert(false, "[qtraylib] Use QRaylibRenderer::SetTargetFPS()")
#define GetFPS(...)                   static_assert(false, "[qtraylib] Use QRaylibRenderer::GetFPS()")
#define GetFrameTime(...)             static_assert(false, "[qtraylib] Use QRaylibRenderer::GetFrameTime()")
#define GetTime(...)                  static_assert(false, "[qtraylib] Use QRaylibRenderer::GetTime()")

// ===========================================================================
// 8. Custom frame control  →  handled automatically by the Qt event loop
// ===========================================================================
#define SwapScreenBuffer(...)         static_assert(false, "[qtraylib] Qt swaps the buffer automatically after paintGL()")
#define PollInputEvents(...)          static_assert(false, "[qtraylib] Qt polls events through its own event loop")
#define WaitTime(...)                 static_assert(false, "[qtraylib] Use QTimer/QElapsedTimer-driven frame pacing (or QThread::msleep() for non-render tasks)")

// ===========================================================================
// 9. Keyboard input  →  QKeyEvent in keyPressEvent() / keyReleaseEvent()
// ===========================================================================
#define IsKeyPressed(...)             static_assert(false, "[qtraylib] Track via QKeyEvent in keyPressEvent()")
#define IsKeyPressedRepeat(...)       static_assert(false, "[qtraylib] Use QKeyEvent::isAutoRepeat() in keyPressEvent()")
#define IsKeyDown(...)                static_assert(false, "[qtraylib] Track via QKeyEvent in keyPressEvent()")
#define IsKeyReleased(...)            static_assert(false, "[qtraylib] Track via QKeyEvent in keyReleaseEvent()")
#define IsKeyUp(...)                  static_assert(false, "[qtraylib] Track via QKeyEvent in keyReleaseEvent()")
#define GetKeyPressed(...)            static_assert(false, "[qtraylib] Use QKeyEvent::key() in keyPressEvent()")
#define GetCharPressed(...)           static_assert(false, "[qtraylib] Use QKeyEvent::text() in keyPressEvent()")
#define GetKeyName(...)               static_assert(false, "[qtraylib] Use QKeySequence(key).toString()")
#define SetExitKey(...)               static_assert(false, "[qtraylib] Use QShortcut or override keyPressEvent()")

// ===========================================================================
// 10. Mouse input  →  QMouseEvent / QWheelEvent
// ===========================================================================
#define IsMouseButtonPressed(...)     static_assert(false, "[qtraylib] Use QMouseEvent in mousePressEvent()")
#define IsMouseButtonDown(...)        static_assert(false, "[qtraylib] Use QMouseEvent::buttons()")
#define IsMouseButtonReleased(...)    static_assert(false, "[qtraylib] Use QMouseEvent in mouseReleaseEvent()")
#define IsMouseButtonUp(...)          static_assert(false, "[qtraylib] Use QMouseEvent::buttons() (bit NOT set)")
#define GetMouseX(...)                static_assert(false, "[qtraylib] Use QMouseEvent::pos().x()")
#define GetMouseY(...)                static_assert(false, "[qtraylib] Use QMouseEvent::pos().y()")
#define GetMousePosition(...)         static_assert(false, "[qtraylib] Use QMouseEvent::pos()")
#define GetMouseDelta(...)            static_assert(false, "[qtraylib] Track delta from successive QMouseEvent positions")
#define SetMousePosition(...)         static_assert(false, "[qtraylib] Use QCursor::setPos()")
#define SetMouseOffset(...)           static_assert(false, "[qtraylib] Not applicable in Qt; handle in event logic")
#define SetMouseScale(...)            static_assert(false, "[qtraylib] Not applicable in Qt; handle in event logic")
#define GetMouseWheelMove(...)        static_assert(false, "[qtraylib] Use QWheelEvent::angleDelta().y() in wheelEvent()")
#define GetMouseWheelMoveV(...)       static_assert(false, "[qtraylib] Use QWheelEvent::angleDelta() in wheelEvent()")
#define SetMouseCursor(...)           static_assert(false, "[qtraylib] Use QWidget::setCursor()")

// ===========================================================================
// 11. Touch input  →  QTouchEvent in event(QEvent*)
// ===========================================================================
#define GetTouchX(...)                static_assert(false, "[qtraylib] Use QTouchEvent::touchPoints() in touchEvent()")
#define GetTouchY(...)                static_assert(false, "[qtraylib] Use QTouchEvent::touchPoints() in touchEvent()")
#define GetTouchPosition(...)         static_assert(false, "[qtraylib] Use QTouchEvent::touchPoints()[i].pos()")
#define GetTouchPointId(...)          static_assert(false, "[qtraylib] Use QTouchEvent::touchPoints()[i].id()")
#define GetTouchPointCount(...)       static_assert(false, "[qtraylib] Use QTouchEvent::touchPoints().count()")

// ===========================================================================
// 12. Gamepad input  →  use Qt5::Gamepad directly (QGamepad / QGamepadManager)
//     QGamepad provides signals/slots and properties equivalent to raylib's gamepad API.
//     Use it directly in your Qt application; no need to route through QRaylibRenderer.
// ===========================================================================
#define IsGamepadAvailable(...)       static_assert(false, "[qtraylib] Use QGamepadManager::instance()->connectedGamepads()")
#define GetGamepadName(...)           static_assert(false, "[qtraylib] Use QGamepadManager::instance()->gamepadName(id)")
#define IsGamepadButtonPressed(...)   static_assert(false, "[qtraylib] Connect to QGamepad signals, e.g. QGamepad::buttonAChanged")
#define IsGamepadButtonDown(...)      static_assert(false, "[qtraylib] Use QGamepad::buttonA() etc. (poll properties)")
#define IsGamepadButtonReleased(...)  static_assert(false, "[qtraylib] Connect to QGamepad signals for release detection")
#define IsGamepadButtonUp(...)        static_assert(false, "[qtraylib] Use !QGamepad::buttonA() etc.")
#define GetGamepadButtonPressed(...)  static_assert(false, "[qtraylib] Connect to QGamepad button signals")
#define GetGamepadAxisCount(...)      static_assert(false, "[qtraylib] QGamepad exposes axisLeftX/Y and axisRightX/Y properties")
#define GetGamepadAxisMovement(...)   static_assert(false, "[qtraylib] Use QGamepad::axisLeftX() / axisRightX() etc.")
#define SetGamepadMappings(...)       static_assert(false, "[qtraylib] Use QGamepadManager for controller configuration")
#define SetGamepadVibration(...)      static_assert(false, "[qtraylib] Qt5Gamepad does not expose vibration; use platform APIs (e.g. XInput)")

// ===========================================================================
// 13. Logging  ->  QRaylibRenderer
// ===========================================================================
#define SetTraceLogLevel(...)         static_assert(false, "[qtraylib] Use QRaylibRenderer::setTraceLogLevel()")
#define TraceLog(...)                 static_assert(false, "[qtraylib] Use QRaylibRenderer::traceLog()")
#define SetTraceLogCallback(...)      static_assert(false, "[qtraylib] Use QRaylibRenderer::setTraceLogCallback()")

#endif // QTRAYLIB_IMPLEMENTATION
