// ---------------------------------------------------------------------------
// qraylibrenderer.cpp
//
// QTRAYLIB_IMPLEMENTATION must be defined before including any qtraylib header
// to bypass the static_assert interception macros in qtraylib.h and allow this
// file to call raw raylib / rlgl C functions directly.
// ---------------------------------------------------------------------------
#define QTRAYLIB_IMPLEMENTATION

#include "qraylibrenderer.h"

// raylib module feature flags (SUPPORT_MODULE_RTEXT, SUPPORT_MODULE_RSHAPES, ...)
#include <config.h>
// Low-level OpenGL wrapper provided by raylib
#include <rlgl.h>
// raylib math library (MatrixLookAt, MatrixToFloat, etc.)
#include <raymath.h>

#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QByteArray>
#include <QTimer>
#include <QObject>

#include <cstdarg>
#include <cstdio>
#include <cmath>
#include <vector>

// ---------------------------------------------------------------------------
// raylib internal function forward declarations (callable under QTRAYLIB_IMPLEMENTATION)
// ---------------------------------------------------------------------------
// GetScreenToWorldRayEx / GetWorldToScreenEx are implemented in rcore.c and
// declared in raylib.h; this file obtains them via qtraylib.h -> raylib.h.

namespace {

#if SUPPORT_MODULE_RTEXT
extern "C" void LoadFontDefault(void);
extern "C" void UnloadFontDefault(void);
#endif

// Qt-backed OpenGL proc loader for rlLoadExtensions/glad.
void* getQtGlProcAddress(const char* name)
{
    if (!name) return nullptr;

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) return nullptr;

    return reinterpret_cast<void*>(ctx->getProcAddress(QByteArray(name)));
}

} // namespace

// ===========================================================================
// ctor / dtor
// ===========================================================================

QRaylibRenderer::QRaylibRenderer() = default;

QRaylibRenderer::~QRaylibRenderer()
{
    if (state_.ready)
        shutdown();
}

// ===========================================================================
// Initialization / shutdown
// ===========================================================================

bool QRaylibRenderer::initialize(QOpenGLWidget* widget)
{
    if (!widget || state_.ready) return false;

    traceLog(LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);
    traceLog(LOG_INFO, "Platform backend: Qt");

    traceLog(LOG_INFO, "Supported raylib modules:");
    traceLog(LOG_INFO, "    > rcore:..... loaded (mandatory)");
    traceLog(LOG_INFO, "    > rlgl:...... loaded (mandatory)");
#if SUPPORT_MODULE_RSHAPES
    traceLog(LOG_INFO, "    > rshapes:... loaded (optional)");
#else
    traceLog(LOG_INFO, "    > rshapes:... not loaded (optional)");
#endif
#if SUPPORT_MODULE_RTEXTURES
    traceLog(LOG_INFO, "    > rtextures:. loaded (optional)");
#else
    traceLog(LOG_INFO, "    > rtextures:. not loaded (optional)");
#endif
#if SUPPORT_MODULE_RTEXT
    traceLog(LOG_INFO, "    > rtext:..... loaded (optional)");
#else
    traceLog(LOG_INFO, "    > rtext:..... not loaded (optional)");
#endif
#if SUPPORT_MODULE_RMODELS
    traceLog(LOG_INFO, "    > rmodels:... loaded (optional)");
#else
    traceLog(LOG_INFO, "    > rmodels:... not loaded (optional)");
#endif
#if SUPPORT_MODULE_RAUDIO
    traceLog(LOG_INFO, "    > raudio:.... loaded (optional)");
#else
    traceLog(LOG_INFO, "    > raudio:.... not loaded (optional)");
#endif

    int width = widget->width();
    int height = widget->height();
    if (width  <= 0) width  = 1;
    if (height <= 0) height = 1;

    // Load OpenGL entry points before rlgl initialization.
    rlLoadExtensions((void*)getQtGlProcAddress);

    rlglInit(width, height);
    rlSetFramebufferWidth(width);
    rlSetFramebufferHeight(height);

    gl_widget_ = widget;
    state_.render_width       = width;
    state_.render_height      = height;
    state_.current_fbo_width  = width;
    state_.current_fbo_height = height;

    setupViewport(width, height);

#if SUPPORT_MODULE_RTEXT
    // Match InitWindow: load default font so text APIs are ready.
    LoadFontDefault();

    #if SUPPORT_MODULE_RSHAPES
    // Match InitWindow behavior: route shapes through default font white texel.
    Rectangle rec = GetFontDefault().recs[95];
    SetShapesTexture(GetFontDefault().texture,
                     Rectangle{ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
    #endif
#elif SUPPORT_MODULE_RSHAPES
    // rtext disabled: fall back to rlgl default 1x1 white texture for shapes.
    Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    SetShapesTexture(texture, Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
#endif

    state_.time.timer.start();
    state_.time.previous = 0.0;
    state_.ready = true;

    // Drive continuous repaint from inside renderer so setTargetFPS() can adjust it directly.
    frame_timer_ = new QTimer(widget);
    frame_timer_->setTimerType(Qt::PreciseTimer);
    QObject::connect(frame_timer_, &QTimer::timeout, widget, QOverload<>::of(&QOpenGLWidget::update));
    frame_timer_->start(timerIntervalMs());

    return true;
}

void QRaylibRenderer::shutdown()
{
    if (!state_.ready) return;

    if (frame_timer_) {
        frame_timer_->stop();
        delete frame_timer_;
        frame_timer_ = nullptr;
    }

#if SUPPORT_MODULE_RTEXT
    // Match CloseWindow cleanup for default font GPU resources.
    UnloadFontDefault();
#endif

    rlglClose();
    state_ = RenderState{};
    gl_widget_ = nullptr;

    traceLog(LOG_INFO, "Window closed successfully");
}

bool QRaylibRenderer::isReady() const { return state_.ready; }

void QRaylibRenderer::handleResize(int width, int height)
{
    if (width <= 0 || height <= 0) return;

    state_.render_width  = width;
    state_.render_height = height;

    // Only sync currentFbo dimensions when not in FBO mode.
    if (!state_.using_fbo) {
        state_.current_fbo_width  = width;
        state_.current_fbo_height = height;
    }

    rlSetFramebufferWidth(width);
    rlSetFramebufferHeight(height);
    setupViewport(width, height);
}

int QRaylibRenderer::renderWidth()  const { return state_.render_width; }
int QRaylibRenderer::renderHeight() const { return state_.render_height; }

// ===========================================================================
// Drawing control
// ===========================================================================

void QRaylibRenderer::beginDrawing()
{
    double now           = getTime();
    state_.time.update   = now - state_.time.previous;
    state_.time.previous = now;

    // Reset the ModelView matrix.
    // Qt handles HiDPI via devicePixelRatio; no screenScale matrix is needed.
    rlLoadIdentity();
}

void QRaylibRenderer::endDrawing()
{
    // Flush the internal render batch.
    rlDrawRenderBatchActive();

    double now             = getTime();
    state_.time.draw       = now - state_.time.previous;
    state_.time.previous   = now;
    state_.time.frame      = state_.time.update + state_.time.draw;

    // Note: Qt swaps the buffer automatically after paintGL() returns — no SwapScreenBuffer() needed.
    // Note: Qt polls input through its own event loop — no PollInputEvents() needed.
    // Note: frame rate cap is NOT done here — sleeping inside paintGL would cause the swap to miss the
    //       current vsync and block until the next one, halving the effective FPS.
    //       Use setTargetFPS() + timerIntervalMs() to set the QTimer interval instead.

    state_.time.frame_counter++;
}

void QRaylibRenderer::clearBackground(Color color)
{
    rlClearColor(color.r, color.g, color.b, color.a);
    rlClearScreenBuffers();
}

// ===========================================================================
// 2D / 3D camera modes
// ===========================================================================

void QRaylibRenderer::beginMode2D(Camera2D camera)
{
    rlDrawRenderBatchActive();
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(::GetCameraMatrix2D(camera)));
}

void QRaylibRenderer::endMode2D()
{
    rlDrawRenderBatchActive();
    rlLoadIdentity();
    // screenScale is not used in the Qt context; DPI is handled by Qt via devicePixelRatio.
}

void QRaylibRenderer::beginMode3D(Camera3D camera)
{
    rlDrawRenderBatchActive();

    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();

    float aspect = (state_.current_fbo_height > 0)
        ? (float)state_.current_fbo_width / (float)state_.current_fbo_height
        : 1.0f;

    if (camera.projection == CAMERA_PERSPECTIVE) {
        double top   = rlGetCullDistanceNear() * std::tan(camera.fovy * 0.5 * DEG2RAD);
        double right = top * aspect;
        rlFrustum(-right, right, -top, top,
                  rlGetCullDistanceNear(), rlGetCullDistanceFar());
    } else {
        // CAMERA_ORTHOGRAPHIC
        double top   = camera.fovy / 2.0;
        double right = top * aspect;
        rlOrtho(-right, right, -top, top,
                rlGetCullDistanceNear(), rlGetCullDistanceFar());
    }

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    Matrix mat_view = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(mat_view));

    rlEnableDepthTest();
}

void QRaylibRenderer::endMode3D()
{
    rlDrawRenderBatchActive();

    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    // screenScale is not used in the Qt context.

    rlDisableDepthTest();
}

// ===========================================================================
// Off-screen rendering to texture (BeginTextureMode / EndTextureMode)
// ===========================================================================

void QRaylibRenderer::beginTextureMode(RenderTexture2D target)
{
    rlDrawRenderBatchActive();
    rlEnableFramebuffer(target.id);

    rlViewport(0, 0, target.texture.width, target.texture.height);
    rlSetFramebufferWidth(target.texture.width);
    rlSetFramebufferHeight(target.texture.height);

    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    // Texture coordinate origin at top-left.
    rlOrtho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    state_.current_fbo_width  = target.texture.width;
    state_.current_fbo_height = target.texture.height;
    state_.using_fbo          = true;
}

void QRaylibRenderer::endTextureMode()
{
    rlDrawRenderBatchActive();
    rlDisableFramebuffer();

    // Restore the viewport to the main framebuffer.
    setupViewport(state_.render_width, state_.render_height);
    rlSetFramebufferWidth(state_.render_width);
    rlSetFramebufferHeight(state_.render_height);

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    // screenScale is not used in the Qt context.

    state_.current_fbo_width  = state_.render_width;
    state_.current_fbo_height = state_.render_height;
    state_.using_fbo          = false;
}

// ===========================================================================
// Shader / blend / scissor / VR modes
// ===========================================================================

void QRaylibRenderer::beginShaderMode(Shader shader)
{
    rlSetShader(shader.id, shader.locs);
}

void QRaylibRenderer::endShaderMode()
{
    rlSetShader(rlGetShaderIdDefault(), rlGetShaderLocsDefault());
}

void QRaylibRenderer::beginBlendMode(int mode)
{
    rlSetBlendMode(mode);
}

void QRaylibRenderer::endBlendMode()
{
    rlSetBlendMode(RL_BLEND_ALPHA);
}

void QRaylibRenderer::beginScissorMode(int x, int y, int width, int height)
{
    rlDrawRenderBatchActive();
    rlEnableScissorTest();
    // OpenGL scissor origin is at the bottom-left, so the y coordinate must be flipped.
    rlScissor(x, state_.current_fbo_height - (y + height), width, height);
}

void QRaylibRenderer::endScissorMode()
{
    rlDrawRenderBatchActive();
    rlDisableScissorTest();
}

void QRaylibRenderer::beginVrStereoMode(VrStereoConfig config)
{
    // beginVrStereoMode does not access CORE; the raw raylib function can be called directly.
    ::BeginVrStereoMode(config);
}

void QRaylibRenderer::endVrStereoMode()
{
    ::EndVrStereoMode();
}

// ===========================================================================
// VR stereo rendering configuration
// ===========================================================================

VrStereoConfig QRaylibRenderer::loadVrStereoConfig(VrDeviceInfo device)
{
    return ::LoadVrStereoConfig(device);
}

void QRaylibRenderer::unloadVrStereoConfig(VrStereoConfig config)
{
    ::UnloadVrStereoConfig(config);
}

// ===========================================================================
// Shader management (pure GPU operations, no CORE dependency)
// ===========================================================================

Shader QRaylibRenderer::loadShader(const char* vs_file_name, const char* fs_file_name)
{
    return ::LoadShader(vs_file_name, fs_file_name);
}

Shader QRaylibRenderer::loadShaderFromMemory(const char* vs_code, const char* fs_code)
{
    return ::LoadShaderFromMemory(vs_code, fs_code);
}

bool QRaylibRenderer::isShaderValid(Shader shader)
{
    return ::IsShaderValid(shader);
}

int QRaylibRenderer::getShaderLocation(Shader shader, const char* uniform_name)
{
    return ::GetShaderLocation(shader, uniform_name);
}

int QRaylibRenderer::getShaderLocationAttrib(Shader shader, const char* attrib_name)
{
    return ::GetShaderLocationAttrib(shader, attrib_name);
}

void QRaylibRenderer::setShaderValue(Shader shader, int loc_index, const void* value, int uniform_type)
{
    ::SetShaderValue(shader, loc_index, value, uniform_type);
}

void QRaylibRenderer::setShaderValueV(Shader shader, int loc_index, const void* value, int uniform_type, int count)
{
    ::SetShaderValueV(shader, loc_index, value, uniform_type, count);
}

void QRaylibRenderer::setShaderValueMatrix(Shader shader, int loc_index, Matrix mat)
{
    ::SetShaderValueMatrix(shader, loc_index, mat);
}

void QRaylibRenderer::setShaderValueTexture(Shader shader, int loc_index, Texture2D texture)
{
    ::SetShaderValueTexture(shader, loc_index, texture);
}

void QRaylibRenderer::unloadShader(Shader shader)
{
    ::UnloadShader(shader);
}

// ===========================================================================
// Screen-space coordinate conversion
// ===========================================================================

Ray QRaylibRenderer::getScreenToWorldRay(Vector2 position, Camera camera)
{
    // Inject the current Qt render dimensions (replaces CORE.Window.render).
    return ::GetScreenToWorldRayEx(position, camera, state_.render_width, state_.render_height);
}

Ray QRaylibRenderer::getScreenToWorldRayEx(Vector2 position, Camera camera, int width, int height)
{
    return ::GetScreenToWorldRayEx(position, camera, width, height);
}

Vector2 QRaylibRenderer::getWorldToScreen(Vector3 position, Camera camera)
{
    return ::GetWorldToScreenEx(position, camera, state_.render_width, state_.render_height);
}

Vector2 QRaylibRenderer::getWorldToScreenEx(Vector3 position, Camera camera, int width, int height)
{
    return ::GetWorldToScreenEx(position, camera, width, height);
}

Vector2 QRaylibRenderer::getWorldToScreen2D(Vector2 position, Camera2D camera)
{
    return ::GetWorldToScreen2D(position, camera);
}

Vector2 QRaylibRenderer::getScreenToWorld2D(Vector2 position, Camera2D camera)
{
    return ::GetScreenToWorld2D(position, camera);
}

Matrix QRaylibRenderer::getCameraMatrix(Camera camera)
{
    return ::GetCameraMatrix(camera);
}

Matrix QRaylibRenderer::getCameraMatrix2D(Camera2D camera)
{
    return ::GetCameraMatrix2D(camera);
}

// ===========================================================================
// Timing (based on QElapsedTimer, fully replaces CORE.Time)
// ===========================================================================

void QRaylibRenderer::setTargetFPS(int fps)
{
    state_.time.target = (fps > 0) ? (1.0 / (double)fps) : 0.0;

    traceLog(LOG_INFO, "TIMER: Target time per frame: %02.03f milliseconds",
             (float)state_.time.target * 1000.0f);

    if (state_.ready && frame_timer_)
        frame_timer_->start(timerIntervalMs());
}

int QRaylibRenderer::timerIntervalMs() const
{
    if (state_.time.target <= 0.0) return 0;
    // Subtract 1 ms so the timer fires slightly early; vsync or event-loop latency
    // absorbs the remaining fraction and prevents systematic under-shoot.
    int ms = static_cast<int>(state_.time.target * 1000.0) - 1;
    return (ms > 0) ? ms : 0;
}

int QRaylibRenderer::getFPS() const
{
    // Same sliding-average algorithm as raylib's original implementation.
    constexpr int   kCaptureFrames = 30;
    constexpr float kAvgSeconds    = 0.5f;
    constexpr float kStep          = kAvgSeconds / kCaptureFrames;

    static int   index   = 0;
    static float history[kCaptureFrames] = {};
    static float average = 0.0f;
    static float last    = 0.0f;

    float fps_frame = getFrameTime();
    if (fps_frame == 0.0f) return 0;

    float now = (float)getTime();
    if ((now - last) > kStep) {
        last    = now;
        index   = (index + 1) % kCaptureFrames;
        average -= history[index];
        history[index] = fps_frame / kCaptureFrames;
        average += history[index];
    }

    return (average > 0.0f) ? (int)std::roundf(1.0f / average) : 0;
}

float QRaylibRenderer::getFrameTime() const
{
    return (float)state_.time.frame;
}

double QRaylibRenderer::getTime() const
{
    return state_.time.timer.elapsed() / 1000.0;
}

// ===========================================================================
// Logging (direct rcore wrappers)
// ===========================================================================

void QRaylibRenderer::setTraceLogLevel(int log_level)
{
    ::SetTraceLogLevel(log_level);
}

void QRaylibRenderer::setTraceLogCallback(TraceLogCallback callback)
{
    ::SetTraceLogCallback(callback);
}

void QRaylibRenderer::traceLog(int log_level, const char* text, ...)
{
    if (!text) {
        ::TraceLog(log_level, "%s", "(null)");
        return;
    }

    va_list args;
    va_start(args, text);

    va_list args_copy;
    va_copy(args_copy, args);
    int required = std::vsnprintf(nullptr, 0, text, args_copy);
    va_end(args_copy);

    if (required < 0) {
        va_end(args);
        ::TraceLog(log_level, "%s", text);
        return;
    }

    std::vector<char> buffer(static_cast<size_t>(required) + 1U);
    std::vsnprintf(buffer.data(), buffer.size(), text, args);
    va_end(args);

    ::TraceLog(log_level, "%s", buffer.data());
}

// ===========================================================================
// Internal helpers
// ===========================================================================

// Equivalent to SetupViewport in raylib's rcore.c, but uses local parameters instead of CORE.
void QRaylibRenderer::setupViewport(int width, int height)
{
    rlViewport(0, 0, width, height);

    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    // Origin at top-left, consistent with screen coordinate conventions.
    rlOrtho(0, width, height, 0, 0.0f, 1.0f);

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}
