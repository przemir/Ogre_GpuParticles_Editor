#include "OgreSDLGame.h"

OgreSDLGame::OgreSDLGame()
{

}

OgreSDLGame::~OgreSDLGame()
{

}

bool OgreSDLGame::frameStarted(const Ogre::FrameEvent& evt)
{
    return true;
}

bool OgreSDLGame::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    return true;
}

bool OgreSDLGame::frameEnded(const Ogre::FrameEvent& evt)
{
    return true;
}
