
@piece( custom_vs_uniformDeclaration )

@property(particleWorldEnabled)
@insertpiece(ParticleWorldStructures)

StructuredBuffer<Particle> particleDataList : register(t14);
StructuredBuffer<EmitterCoreData> emitterCoreData : register(t11);
StructuredBuffer<BucketGroup> bucketGroupData : register(t12);

static const float2 QuadCorners[] = {
	float2(0, 0),
	float2(1, 0),
	float2(1, 1),
	float2(0, 1),
};
@end

@property(particle_enabled)
struct Particle {
float3 pos;
float rot;
float4 colour;
float2 size;
float spriteNumber;
// float anything;
};

StructuredBuffer<Particle> particleDataList : register(t14);

struct EmitterInstanceData {
    float4x4 emitterLocation;
    float elapsedTime;
    
    uint particleArrayStartIndex;
    
    /// Particles count without new particles
    uint particlesCount;
    
    uint newParticles;
    
    uint MaxParticles;
    uint randomIteration;
};

StructuredBuffer<EmitterInstanceData> emitterInstanceData : register(t13);

static const float2 QuadCorners[] = {
	float2(0, 0),
	float2(1, 0),
	float2(1, 1),
	float2(0, 1),
};

@end
@end


@piece( custom_passBuffer )

    float4x4 particleInvViewProj;
    float4x4 particleCameraViewMatrix;
    
    // float3 particleCameraPos;
    // float3 particleCameraRight;
    // float3 particleCameraUp;
    // float3 particleCameraForward;
    
@end
