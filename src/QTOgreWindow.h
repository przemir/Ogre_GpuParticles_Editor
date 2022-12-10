#ifndef QTOGREWINDOW_H
#define QTOGREWINDOW_H

#include <QtWidgets/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QWindow>
#include <Ogre.h>

// Source of QTOgreWindow template: http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Integrating+Ogre+into+QT5

class OgreQtAppParticleEditorSystem;
class OgreQtSystem;
class OgreRenderer;

/// With the headers included we now need to inherit from QWindow.
class QTOgreWindow : public QWindow
{
    Q_OBJECT

public:
    explicit QTOgreWindow(OgreQtAppParticleEditorSystem* ogreSystem, OgreRenderer* renderer, QWindow *parent = NULL);
    ~QTOgreWindow();

    OgreQtAppParticleEditorSystem* getOgreSystem() const;

protected:

    virtual void render(QPainter *painter);
    virtual void render();
    virtual void initialize();

    void setAnimating(bool animating);

public slots:

    virtual void renderLater();
    virtual void renderNow();

    /// We use an event filter to be able to capture keyboard/mouse events.
    virtual bool eventFilter(QObject *target, QEvent *event);

protected:
    OgreQtAppParticleEditorSystem* mOgreSystem;
    OgreRenderer* mOgreRenderer;

    bool mUpdatePending;
    bool mAnimating;

    virtual void keyPressEvent(QKeyEvent * e);
    virtual void keyReleaseEvent(QKeyEvent * e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseWheelEvent(QWheelEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void exposeEvent(QExposeEvent *event);
    virtual bool event(QEvent *event);

    void log(Ogre::String msg);
    void log(QString msg);

//    friend class OgreRenderer;
};

#endif
