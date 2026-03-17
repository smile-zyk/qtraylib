#pragma once

// ---------------------------------------------------------------------------
// qraylibrenderer.h
//
// QRaylibRenderer — replaces raylib's window/context/input management with Qt
// while preserving raylib's GPU rendering capabilities through rlgl.
//
// Usage example (QOpenGLWidget):
//
//   class MyGLWidget : public QOpenGLWidget {
//       QRaylibRenderer m_renderer;
//   protected:
//       void initializeGL() override { m_renderer.initialize(this); }
//       void resizeGL(int w, int h) override { m_renderer.handleResize(w, h); }
//       void paintGL() override {
//           m_renderer.beginDrawing();
//           m_renderer.clearBackground(RAYWHITE);
//           // ... drawing calls ...
//           m_renderer.endDrawing();
//       }
//   };
//
// ---------------------------------------------------------------------------

#include "qtraylib.h"       // raylib types + Qt interception macros

#include <QElapsedTimer>

QT_FORWARD_DECLARE_CLASS(QOpenGLWidget)
QT_FORWARD_DECLARE_CLASS(QTimer)

class QRaylibRenderer
{
public:
    QRaylibRenderer();
    ~QRaylibRenderer();

    // Non-copyable
    QRaylibRenderer(const QRaylibRenderer&)            = delete;
    QRaylibRenderer& operator=(const QRaylibRenderer&) = delete;

    // -----------------------------------------------------------------------
    // Initialization / shutdown
    // Call from QOpenGLWidget::initializeGL().
    // -----------------------------------------------------------------------
    bool initialize(QOpenGLWidget* widget);
    void shutdown();
    bool isReady() const;

    // Call from resizeGL(w, h) to keep rlgl's internal framebuffer size in sync with Qt.
    void handleResize(int width, int height);

    // Current render dimensions (replaces GetRenderWidth / GetRenderHeight).
    int renderWidth()  const;
    int renderHeight() const;

    // -----------------------------------------------------------------------
    // Drawing control
    // Call BeginDrawing() / EndDrawing() from paintGL().
    // Qt swaps the buffer automatically after paintGL() returns — do not call SwapScreenBuffer.
    // -----------------------------------------------------------------------
    void beginDrawing();
    void endDrawing();
    void clearBackground(Color color);

    // 2D camera mode
    void beginMode2D(Camera2D camera);
    void endMode2D();

    // 3D camera mode
    void beginMode3D(Camera3D camera);
    void endMode3D();

    // Off-screen rendering to texture
    void beginTextureMode(RenderTexture2D target);
    void endTextureMode();

    // Shader / blend / scissor / VR modes (thin rlgl wrappers)
    void beginShaderMode(Shader shader);
    void endShaderMode();
    void beginBlendMode(int mode);
    void endBlendMode();
    void beginScissorMode(int x, int y, int width, int height);
    void endScissorMode();
    void beginVrStereoMode(VrStereoConfig config);
    void endVrStereoMode();

    // -----------------------------------------------------------------------
    // VR stereo rendering configuration
    // -----------------------------------------------------------------------
    VrStereoConfig loadVrStereoConfig(VrDeviceInfo device);
    void           unloadVrStereoConfig(VrStereoConfig config);

    // -----------------------------------------------------------------------
    // Shader management (direct rlgl calls, no CORE dependency)
    // -----------------------------------------------------------------------
    Shader loadShader(const char* vs_file_name, const char* fs_file_name);
    Shader loadShaderFromMemory(const char* vs_code, const char* fs_code);
    bool   isShaderValid(Shader shader);
    int    getShaderLocation(Shader shader, const char* uniform_name);
    int    getShaderLocationAttrib(Shader shader, const char* attrib_name);
    void   setShaderValue(Shader shader, int loc_index, const void* value, int uniform_type);
    void   setShaderValueV(Shader shader, int loc_index, const void* value, int uniform_type, int count);
    void   setShaderValueMatrix(Shader shader, int loc_index, Matrix mat);
    void   setShaderValueTexture(Shader shader, int loc_index, Texture2D texture);
    void   unloadShader(Shader shader);

    // -----------------------------------------------------------------------
    // Screen-space coordinate conversion
    // The no-argument overloads automatically use the current Qt render dimensions
    // (replacing CORE.Window reads); Ex variants accept explicit width/height.
    // -----------------------------------------------------------------------
    Ray     getScreenToWorldRay(Vector2 position, Camera camera);
    Ray     getScreenToWorldRayEx(Vector2 position, Camera camera, int width, int height);
    Vector2 getWorldToScreen(Vector3 position, Camera camera);
    Vector2 getWorldToScreenEx(Vector3 position, Camera camera, int width, int height);
    Vector2 getWorldToScreen2D(Vector2 position, Camera2D camera);
    Vector2 getScreenToWorld2D(Vector2 position, Camera2D camera);
    Matrix  getCameraMatrix(Camera camera);
    Matrix  getCameraMatrix2D(Camera2D camera);

    // -----------------------------------------------------------------------
    // Timing (based on QElapsedTimer, no GLFW / platform dependency)
    // -----------------------------------------------------------------------
    void   setTargetFPS(int fps);   // 0 = uncapped
    int    getFPS()       const;
    float  getFrameTime() const;    // last frame duration (seconds)
    double getTime()      const;    // seconds elapsed since initialize()

    // Returns the internal QTimer interval (milliseconds) that matches the target FPS.
    // Renderer-owned timer drives QWidget::update() at this cadence, keeping frame rate
    // control outside paintGL to avoid conflicts with vsync.
    // Returns 0 (fire immediately) when target FPS is 0/uncapped or >= ~1000.
    int    timerIntervalMs() const;

    // -----------------------------------------------------------------------
    // Logging (direct rcore wrappers)
    // -----------------------------------------------------------------------
    void setTraceLogLevel(int log_level);
    void setTraceLogCallback(TraceLogCallback callback);
    void traceLog(int log_level, const char* text, ...);

private:
    // -----------------------------------------------------------------------
    // Internal render state (replaces raylib's global CORE struct)
    // -----------------------------------------------------------------------
    struct RenderState
    {
        bool ready = false;

        // Framebuffer dimensions (updated by HandleResize and BeginTextureMode).
        int render_width  = 0;
        int render_height = 0;

        // Active FBO dimensions (same as render_* when not in texture mode).
        int current_fbo_width  = 0;
        int current_fbo_height = 0;

        bool using_fbo = false;

        // Timing sub-structe
        struct TimingState
        {
            QElapsedTimer timer;              // started at Initialize()
            double previous      = 0.0;      // time point of the last BeginDrawing()
            double update        = 0.0;      // time spent in the update phase
            double draw          = 0.0;      // time spent in the draw phase
            double frame         = 0.0;      // total duration of the last frame (seconds)
            double target        = 0.0;      // target frame time; 0 = uncapped
            unsigned int frame_counter = 0;
        } time;
    } state_;

    // Qt objects owned/managed by renderer lifetime, not by render state snapshot.
    QOpenGLWidget* gl_widget_ = nullptr;
    QTimer* frame_timer_ = nullptr;

    // Internal helpers
    void setupViewport(int width, int height);
};
