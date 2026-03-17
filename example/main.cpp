// qraylibrenderer.h must be included FIRST (before any Qt / Windows headers)
// so that windows.h is pre-processed with NOGDI, preventing the Rectangle
// GDI function declaration from conflicting with raylib's Rectangle struct.
#include "qraylibrenderer.h"

#include <rlgl.h>

#include <algorithm>
#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QWheelEvent>

#include <cmath>

// ---------------------------------------------------------------------------
// 2D camera mouse zoom example implemented on top of QRaylibRenderer + Qt events
// ---------------------------------------------------------------------------
class DemoWidget : public QOpenGLWidget
{
public:
    explicit DemoWidget(QWidget* parent = nullptr)
        : QOpenGLWidget(parent)
    {
        setWindowTitle("qtraylib demo - 2d camera mouse zoom");
        resize(800, 450);
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);

        m_camera.zoom = 1.0f;
    }

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

    void keyPressEvent(QKeyEvent* event) override
    {
        if (!event->isAutoRepeat()) {
            if (event->key() == Qt::Key_1) m_zoomMode = 0;
            else if (event->key() == Qt::Key_2) m_zoomMode = 1;
        }

        QOpenGLWidget::keyPressEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        m_lastMousePos = event->pos();
        m_mousePos = event->pos();

        if ((event->button() == Qt::RightButton) && (m_zoomMode == 1)) {
            const Vector2 mousePos = toVector2(event->pos());
            const Vector2 mouseWorldPos = m_renderer.getScreenToWorld2D(mousePos, m_camera);

            m_camera.offset = mousePos;
            m_camera.target = mouseWorldPos;
        }

        QOpenGLWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        m_mousePos = event->pos();

        const QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();

        if (event->buttons().testFlag(Qt::LeftButton)) {
            Vector2 worldDelta = { -(float)delta.x() / m_camera.zoom, -(float)delta.y() / m_camera.zoom };
            m_camera.target.x += worldDelta.x;
            m_camera.target.y += worldDelta.y;
        }

        if ((m_zoomMode == 1) && event->buttons().testFlag(Qt::RightButton)) {
            const float scale = 0.005f * (float)delta.x();
            m_camera.zoom = zoomFromScale(m_camera.zoom, scale);
        }

        QOpenGLWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        m_mousePos = event->pos();
        m_lastMousePos = event->pos();
        QOpenGLWidget::mouseReleaseEvent(event);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        m_mousePos = event->pos();
        m_lastMousePos = event->pos();

        if (m_zoomMode == 0) {
            const float wheel = (float)event->angleDelta().y() / 120.0f;
            if (wheel != 0.0f) {
                const Vector2 mousePos = toVector2(event->pos());
                const Vector2 mouseWorldPos = m_renderer.getScreenToWorld2D(mousePos, m_camera);

                m_camera.offset = mousePos;
                m_camera.target = mouseWorldPos;

                const float scale = 0.2f * wheel;
                m_camera.zoom = zoomFromScale(m_camera.zoom, scale);
            }
        }

        QOpenGLWidget::wheelEvent(event);
    }

    void paintGL() override
    {
        m_renderer.beginDrawing();
        m_renderer.clearBackground(RAYWHITE);

        m_renderer.beginMode2D(m_camera);
        rlPushMatrix();
        rlTranslatef(0, 25 * 50, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(100, 50);
        rlPopMatrix();

        DrawCircle(m_renderer.renderWidth() / 2, m_renderer.renderHeight() / 2, 50, MAROON);
        m_renderer.endMode2D();

        const Vector2 mousePos = toVector2(m_mousePos);
        DrawCircleV(mousePos, 4, DARKGRAY);
        DrawTextEx(
            GetFontDefault(),
            TextFormat("[%i, %i]", m_mousePos.x(), m_mousePos.y()),
            Vector2{ mousePos.x - 44.0f, mousePos.y - 24.0f },
            20,
            2,
            BLACK
        );

        DrawText("[1][2] Select mouse zoom mode (Wheel or Move)", 20, 20, 20, DARKGRAY);
        if (m_zoomMode == 0) {
            DrawText("Mouse left button drag to move, mouse wheel to zoom", 20, 50, 20, DARKGRAY);
        } else {
            DrawText("Mouse left button drag to move, mouse press and move to zoom", 20, 50, 20, DARKGRAY);
        }
        DrawText(TextFormat("FPS: %d", m_renderer.getFPS()), 20, 80, 20, DARKGRAY);

        m_renderer.endDrawing();
    }

private:
    static Vector2 toVector2(const QPoint& point)
    {
        return Vector2{ (float)point.x(), (float)point.y() };
    }

    static float zoomFromScale(float currentZoom, float scale)
    {
        const float nextZoom = (float)std::exp(std::log(currentZoom) + scale);
        return std::max(0.125f, std::min(64.0f, nextZoom));
    }

    QRaylibRenderer m_renderer;
    Camera2D m_camera = {};
    QPoint m_mousePos;
    QPoint m_lastMousePos;
    int m_zoomMode = 0;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DemoWidget   w;
    w.show();
    return app.exec();
}
