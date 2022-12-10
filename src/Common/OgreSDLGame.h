#ifndef OGRESDLGAME_H
#define OGRESDLGAME_H

#if OGRE_USE_SDL2
#include <SDL.h>
#endif
#include <OgreFrameListener.h>

class OgreSDLGame : public Ogre::FrameListener
{
public:
    OgreSDLGame();
    virtual ~OgreSDLGame();

    virtual void initialize(void) {}
    virtual void deinitialize(void) {}

    virtual void createScene01(void) {}
    virtual void createScene02(void) {}
    virtual void createScene03(void) {}

    virtual void destroyScene(void) {}

    virtual bool frameStarted(const Ogre::FrameEvent& evt);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool frameEnded(const Ogre::FrameEvent& evt);


    virtual void update( float timeSinceLast ) {}
    virtual void finishFrameParallel(void) {}
    virtual void finishFrame(void) {}


#ifdef OGRE_USE_SDL2
    virtual void mouseMoved( const SDL_MouseMotionEvent &arg ) {}
    virtual void mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) {}
    virtual void mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id ) {}
    virtual void mouseWheel( const SDL_MouseWheelEvent &arg ) {}

    virtual void textEditing( const SDL_TextEditingEvent& arg ) {}
    virtual void textInput( const SDL_TextInputEvent& arg ) {}
    virtual void keyPressed( const SDL_KeyboardEvent &arg ) {}
    virtual void keyReleased (const SDL_KeyboardEvent &arg ) {}

    virtual void joyButtonPressed( const SDL_JoyButtonEvent &evt, int button ) {}
    virtual void joyButtonReleased( const SDL_JoyButtonEvent &evt, int button ) {}
    virtual void joyAxisMoved( const SDL_JoyAxisEvent &arg, int axis ) {}
    virtual void joyPovMoved( const SDL_JoyHatEvent &arg, int index ) {}
#endif
};

#endif
