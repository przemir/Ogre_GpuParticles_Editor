#include <GpuParticles/GpuParticleSystem.h>

GpuParticleSystem::GpuParticleSystem()
{

}

GpuParticleSystem::~GpuParticleSystem()
{
    for(size_t i = 0; i < mEmitters.size(); ++i) {
        OGRE_DELETE mEmitters[i];
    }
}

void GpuParticleSystem::addEmitter(GpuParticleEmitter* particleEmitter)
{
    mEmitters.push_back(particleEmitter);
}

const std::vector<GpuParticleEmitter*>& GpuParticleSystem::getEmitters() const
{
    return mEmitters;
}

GpuParticleEmitter* GpuParticleSystem::_takeEmitter(int index)
{
    assert(index >= 0 && index < (int)mEmitters.size());

    GpuParticleEmitter* result = mEmitters[index];
    mEmitters.erase(mEmitters.begin()+index);
    return result;
}

std::vector<GpuParticleEmitter*> GpuParticleSystem::_takeEmitters()
{
    std::vector<GpuParticleEmitter*> result = mEmitters;
    mEmitters.clear();
    return result;
}

void GpuParticleSystem::_insertEmitter(int index, GpuParticleEmitter* emitter)
{
    assert(index >= 0 && index <= (int)mEmitters.size());

    mEmitters.insert(mEmitters.begin()+index, emitter);
}
