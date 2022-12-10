# Ogre_GpuParticles_Editor

Particle editor for gpu particles for OgreNext. Currently only DirectX11 is working (shaders are written only in HLSL).
Editor uses Ogre and Qt as dependencies.

![screen](screens/OgreGpuParticlesEditor_screen.jpg 'Screen')

# Instalation

OgreGpuParticlesEditor.pro is project file (Uses QMake). Inside OgreGpuParticlesEditorPaths.pri paths OGRE_ROOT, OGRE_BUILD and OGRE_DEPENDENCIES should be filled.

Set ./bin/resources.cfg with paths to Ogre. Generally all paths starting with ../Data/OgreCommon should be changed to match resources from Ogre.
Alternatively create this folder and copy those files from Ogre.

Release build executable should be inside "./bin/Release/" folder, debug inside "./bin/Debug/" folder.
All needed dll should be placed there (except plugins which will go inside "./bin/Release/Plugins" or "./bin/Debug/Plugins" folder).
Currently only RenderSystem_Direct3D11.dll (RenderSystem_Direct3D11_d.dll) is supported.
