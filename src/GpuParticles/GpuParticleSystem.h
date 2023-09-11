#ifndef GPUPARTICLESYSTEM_H
#define GPUPARTICLESYSTEM_H

#include <GpuParticles/GpuParticleEmitter.h>

#include <OgreVector3.h>

class GpuParticleSystem
{
public:

    GpuParticleSystem();
    ~GpuParticleSystem();

    void addEmitter(GpuParticleEmitter* particleEmitter);
    const std::vector<GpuParticleEmitter*>& getEmitters() const;

    /// Takes emitter from this particle systems (with ownership).
    GpuParticleEmitter* _takeEmitter(int index);

    /// Takes all emitters from this particle systems (with ownership).
    std::vector<GpuParticleEmitter*> _takeEmitters();

    void _insertEmitter(int index, GpuParticleEmitter* emitter);

private:
    std::vector<GpuParticleEmitter*> mEmitters;
};

#endif
