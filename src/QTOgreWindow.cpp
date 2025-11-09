#include "QTOgreWindow.h"
#include "OgreRenderer.h"
#include <Common/OgreQtSystem.h>

#include "OgreQtAppParticleEditorSystem.h"

#include <OgreWindow.h>

QTOgreWindow::QTOgreWindow(OgreQtAppParticleEditorSystem* ogreSystem,
                           OgreRenderer* renderer,
                           QWindow *parent)
    : QWindow(parent)
    , mOgreSystem(ogreSystem)
    , mOgreRenderer(renderer)
    , mUpdatePending(false)
    , mAnimating(false)
{
    mOgreSystem->setWindow(this);

    setAnimating(true);
    installEventFilter(this);
}

QTOgreWindow::~QTOgreWindow()
{
    mOgreSystem->deinitialize();
}

// In case any drawing surface backing stores (QRasterWindow or QOpenGLWindow) of Qt are supplied to this class in any way we inform Qt that they will be unused.
void QTOgreWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void QTOgreWindow::render()
{
    // How we tied in the render function for OGre3D with QWindow's render function. This is what gets call repeatedly. Note that we don't call this function directly; rather we use the renderNow() function to call this method as we don't want to render the Ogre3D scene unless everything is set up first.
    // That is what renderNow() does.
    // Theoretically you can have one function that does this check but from my experience it seems better to keep things separate and keep the render function as simple as possible.

    // Commented because I don't see difference in usage and It causes crash when resizing because of callstack loop.
    // Ogre::WindowEventUtilities::messagePump();

    mOgreSystem->getRoot()->renderOneFrame();
}

void QTOgreWindow::initialize()
{
    mOgreSystem->initialize("QT Window");

    mOgreRenderer->initialize(this, mOgreSystem->getSceneManager(), mOgreSystem->getCamera());
    mOgreRenderer->createScene();

    mOgreSystem->getRoot()->addFrameListener(mOgreRenderer);
}

void QTOgreWindow::renderLater()
{
    // This function forces QWindow to keep rendering. Omitting this causes the renderNow() function to only get called when the window is resized, moved, etc. as opposed to all of the time; which is generally what we need.
    if (!mUpdatePending)
    {
        mUpdatePending = true;
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool QTOgreWindow::event(QEvent *event)
{
    // QWindow's "message pump". The base method that handles all QWindow events. As you will see there are other methods that actually process the keyboard/other events of Qt and the underlying OS.
    // Note that we call the renderNow() function which checks to see if everything is initialized, etc. before calling the render() function.
    switch (event->type())
    {
    case QEvent::UpdateRequest:
        mUpdatePending = false;
        renderNow();
        return true;

    default:
        return QWindow::event(event);
    }
}

// Called after the QWindow is reopened or when the QWindow is first opened.
void QTOgreWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}


// The renderNow() function calls the initialize() function when needed and if the QWindow is already initialized and prepped calls the render() method.
void QTOgreWindow::renderNow()
{
    if (!isExposed())
        return;

    if (mOgreSystem->getRoot() == NULL)
    {
        initialize();
    }

    render();

    if (mAnimating)
        renderLater();
}


// Our event filter; handles the resizing of the QWindow. When the size of the QWindow changes note the call to the Ogre3D window and camera. This keeps the Ogre3D scene looking correct.
bool QTOgreWindow::eventFilter(QObject *target, QEvent *event)
{
    if (target == this)
    {
        if (event->type() == QEvent::Resize)
        {
            if (isExposed() && mOgreSystem->getRenderWindow() != NULL)
            {
                // Only OpenGL on linux needs Ogre::Window::requestResolution as camera will be rotated somehow.
                // For OpenGL on windows and Direct3D11 this is not necessary.
                // For Vulkan it causes crash.
                if(mOgreSystem->getRealRenderer() == OgreQtSystem::Renderer::OpenGL) {
                    mOgreSystem->getRenderWindow()->requestResolution(this->width(), this->height());
                }

                mOgreSystem->getRenderWindow()->windowMovedOrResized();
                mOgreSystem->getCamera()->setAspectRatio(Ogre::Real(mOgreSystem->getRenderWindow()->getWidth()) / Ogre::Real(mOgreSystem->getRenderWindow()->getHeight()));
            }
        }
    }

    return false;
}

OgreQtAppParticleEditorSystem* QTOgreWindow::getOgreSystem() const
{
    return mOgreSystem;
}

void QTOgreWindow::keyPressEvent(QKeyEvent * e)
{
    mOgreRenderer->keyPressEvent(e);
}

void QTOgreWindow::keyReleaseEvent(QKeyEvent * e)
{
    mOgreRenderer->keyReleaseEvent(e);
}

void QTOgreWindow::mouseMoveEvent( QMouseEvent* e )
{
    mOgreRenderer->mouseMoveEvent(e);
}

void QTOgreWindow::mouseWheelEvent(QWheelEvent *e)
{
    mOgreRenderer->mouseWheelEvent(e);
}

void QTOgreWindow::mousePressEvent( QMouseEvent* e )
{
    mOgreRenderer->mousePressEvent(e);
}

void QTOgreWindow::mouseReleaseEvent( QMouseEvent* e )
{
    mOgreRenderer->mouseReleaseEvent(e);
}

// Function to keep track of when we should and shouldn't redraw the window; we wouldn't want to do rendering when the QWindow is minimized. This takes care of those scenarios.
void QTOgreWindow::setAnimating(bool animating)
{
    mAnimating = animating;

    if (animating)
        renderLater();
}

void QTOgreWindow::log(Ogre::String msg)
{
    if (Ogre::LogManager::getSingletonPtr() != NULL) Ogre::LogManager::getSingletonPtr()->logMessage(msg);
}

void QTOgreWindow::log(QString msg)
{
    log(Ogre::String(msg.toStdString().c_str()));
}
