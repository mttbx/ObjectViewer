#include "window.h"

#include "../objects/scene.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QTimer>

/**
 * @brief Constructeur paramétré
 *
 * Permet d'initialiser la fenêtre et les propriétés de la zone de rendu OpenGL
 *
 * @param screen Propriétés de l'écran
 */
Window::Window(QScreen *screen)
    : QWindow(screen),
      m_scene(new Scene(this)),
      m_leftButtonPressed(false),
      m_cameraSpeed(200.0),
      m_cameraSensitivity(0.2)
{
    // On définit le type de la zone de rendu, dans notre cas il
    // s'agit d'une zone OpenGL
    setSurfaceType(QSurface::OpenGLSurface);

    // Puis on définit les propriétés de la zone de rendu
    QSurfaceFormat format;

    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4); // Multisampling x4
    format.setProfile(QSurfaceFormat::CoreProfile); // Fonctions obsolètes d'OpenGL non disponibles
    format.setOption(QSurfaceFormat::DebugContext);

    // On applique le format et on créer la fenêtre
    setFormat(format);
    create();
    resize(800, 600);
    setTitle("ObjectViewer");

    // On créer le contexte OpenGL et on définit son format
    m_context = new QOpenGLContext;
    m_context->setFormat(format);
    m_context->create();
    m_context->makeCurrent(this);

    // On définit le contexte OpenGL de la scène
    m_scene->setContext(m_context);

    m_renderTimer.invalidate(); // Timer pour la zone de rendu (animation... etc)
    m_updateTimer.start(); // Timer pour la mise à jour de la scène (camera... etc)

    initializeGL();

    connect(this, SIGNAL(widthChanged(int)), this, SLOT(resizeGL()));
    connect(this, SIGNAL(heightChanged(int)), this, SLOT(resizeGL()));

    resizeGL();

    // Création d'un timer permettant la mise à jour de la zone de rendu 60 fois par seconde
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateScene()));
    timer->start(16); // f = 1 / 16.10e-3 = 60Hz
}

/**
 * @brief Initialisation de la zone de rendu
 */
void Window::initializeGL()
{
    m_context->makeCurrent(this);
    m_scene->initialize();
}

/**
 * @brief Mise à jour de la zone de rendu (redessine la scène)
 */
void Window::paintGL()
{
    m_context->makeCurrent(this);
    m_scene->render(static_cast<double>(m_renderTimer.elapsed())/1000);
    m_context->swapBuffers(this);

    emit updateFramerate();
}

/**
 * @brief Permet de redimensionner la zone de rendu
 */
void Window::resizeGL()
{
    m_context->makeCurrent(this);
    m_scene->resize(width(), height());
}

/**
 * @brief Mise à jour de la scène
 */
void Window::updateScene()
{
    m_scene->update(static_cast<float>(m_updateTimer.elapsed())/1000.0f);
    paintGL();
}

/**
 * @brief Vérification de l'animation de la scène
 */
void Window::checkAnimate(int state)
{
    if(state == Qt::Checked)
        m_renderTimer.start();

    if(state == Qt::Unchecked)
        m_renderTimer.invalidate();
}

Scene* Window::getScene()
{
    Scene* scene = static_cast<Scene*>(m_scene);

    return scene;
}

void Window::keyPressEvent(QKeyEvent* e)
{
    Scene* scene = static_cast<Scene*>(m_scene);

    switch (e->key())
    {
    case Qt::Key_Escape:

        QCoreApplication::instance()->quit();
        break;

    case Qt::Key_Right:
        scene->setSideSpeed(m_cameraSpeed);
        break;

    case Qt::Key_Left:
        scene->setSideSpeed(-m_cameraSpeed);
        break;

    case Qt::Key_Up:
        scene->setForwardSpeed(m_cameraSpeed);
        break;

    case Qt::Key_Down:
        scene->setForwardSpeed(-m_cameraSpeed);
        break;

    case Qt::Key_PageUp:
        scene->setVerticalSpeed(m_cameraSpeed);
        break;

    case Qt::Key_PageDown:
        scene->setVerticalSpeed(-m_cameraSpeed);
        break;

    case Qt::Key_Shift:
        scene->setViewCenterFixed(true);
        break;

    default:
        QWindow::keyPressEvent(e);
    }
}

void Window::keyReleaseEvent(QKeyEvent* e)
{
    Scene* scene = static_cast<Scene*>(m_scene);

    switch (e->key())
    {
        case Qt::Key_Right:
        case Qt::Key_Left:
            scene->setSideSpeed(0.0f);
            break;

        case Qt::Key_Up:
        case Qt::Key_Down:
            scene->setForwardSpeed(0.0f);
            break;

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            scene->setVerticalSpeed(0.0f);
            break;

        case Qt::Key_Shift:
            scene->setViewCenterFixed(false);
            break;

        default:
            QWindow::keyReleaseEvent(e);
    }
}

void Window::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        m_leftButtonPressed = true;
        m_pos = m_prevPos = e->pos();
    }

    QWindow::mousePressEvent(e);
}

void Window::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        m_leftButtonPressed = false;
    }

    QWindow::mouseReleaseEvent(e);
}

void Window::mouseMoveEvent(QMouseEvent* e)
{
    if(m_leftButtonPressed)
    {
        m_pos = e->pos();

        float dx = m_cameraSensitivity * (m_pos.x() - m_prevPos.x());
        float dy = -m_cameraSensitivity * (m_pos.y() - m_prevPos.y());

        m_prevPos = m_pos;

        Scene* scene = static_cast<Scene*>(m_scene);

        scene->pan(dx);
        scene->tilt(dy);
    }

    QWindow::mouseMoveEvent(e);
}

void Window::setCameraSpeed(double speed)
{
    m_cameraSpeed = speed;
}

void Window::setCameraSensitivity(double sensitivity)
{
    m_cameraSensitivity = sensitivity;
}

