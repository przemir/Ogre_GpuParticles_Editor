
@insertpiece( ParticleWorldRandom )

@insertpiece( ParticleWorldStructures )

static const float PI = 3.14159265;

float3x3 getXRotationMatrix(float angle)
{
    float s, c;
    sincos(angle, s, c);
    return float3x3(
        1, 0, 0,
        0, c,-s,
        0, s, c
    );
}

float3x3 getYRotationMatrix(float angle)
{
    float s, c;
    sincos(angle, s, c);
    return float3x3(
        c, 0, s,
        0, 1, 0,
       -s, 0, c
    );
}

RWStructuredBuffer<Particle> particleDataList : register(u0);
StructuredBuffer<EmitterCoreData> emitterCoreData : register(t0);
// InstanceData should come as Const buffer: emitter location matrix - those values changes per turn, but won't be modified here.
StructuredBuffer<EmitterInstanceData> emitterInstanceData : register(t1);
StructuredBuffer<BucketGroup> bucketGroupData : register(t2);
StructuredBuffer<ParticleWorld> particleWorld : register(t3);

// Max particles may be const (once per shader compilation) 
uniform uint BucketSize;
uniform uint MaxParticles;

[numthreads(@value( threads_per_group_x ), @value( threads_per_group_y ), @value( threads_per_group_z ))]
void main
(
	uint3 gl_GlobalInvocationID		: SV_DispatchThreadId,
	uint3 gl_WorkGroupID			: SV_GroupID,
	uint gl_LocalInvocationIndex	: SV_GroupIndex
)
{
    uint bucketGroupId = gl_GlobalInvocationID.x / BucketSize;
    uint localId = gl_GlobalInvocationID.x % BucketSize;
    BucketGroup entry = bucketGroupData[bucketGroupId];
    
    if(localId >= entry.particleCount) {
        return;
    }
    
    // uint particleIndex = entry.bucketId * BucketSize + localId;
    
    // Go from younger to older. There is maximum one bucket change.
    uint currentBucketCount = entry.lastParticleIndex % BucketSize;
    uint particleIndex = localId <= currentBucketCount
       ? entry.lastParticleIndex - localId
       : entry.nextBucketParticleIndex + currentBucketCount - localId;
    
    #define particle particleDataList[particleIndex]
    #define emitterCore emitterCoreData[entry.emitterCoreId]
    #define emitterInstance emitterInstanceData[entry.emitterInstanceId]

    NumberGenerator random;
    random.seed = float2(particleIndex, particleWorld[0].randomIteration);
    random.seedAddon = MaxParticles+1;
    
@property(!initLocationInUpdate)
    particle.lifetime = 0.0;
@else
    particle.lifetime = -1.0;
@end
    particle.maxLifetime = lerp(emitterCore.lifetime.x, emitterCore.lifetime.y, random.generate());


    particle.pos = float3(0.0, 0.0, 0.0);
    if(emitterCore.spawnShape == 1) {
        // box shape
        
        float3 vec = float3( lerp(0.0, emitterCore.spawnShapeDimensions.x, random.generate()),
                             lerp(0.0, emitterCore.spawnShapeDimensions.y, random.generate()),
                             lerp(0.0, emitterCore.spawnShapeDimensions.z, random.generate()) );
        vec -= emitterCore.spawnShapeDimensions / 2.0;
        
        particle.pos = vec;
    }
    else if(emitterCore.spawnShape == 2) {
        // sphere shape
        
        float angleH = lerp(-PI, PI, random.generate());
        float angleV = lerp(-PI/2.0, PI/2.0, random.generate());
        float r = lerp(0.0, emitterCore.spawnShapeDimensions.x, random.generate());
        
        float3x3 sphereVMatrix = getXRotationMatrix(angleV);
        float3x3 sphereHMatrix = getYRotationMatrix(angleH);
        
        float3 vec = float3(0.0, 0.0, r);
        vec = mul(vec, sphereVMatrix);
        vec = mul(vec, sphereHMatrix);
        
        particle.pos = vec;
    }
    else if(emitterCore.spawnShape == 3) {
        // disc shape
        
        float angle = lerp(-PI, PI, random.generate());
        float r = lerp(emitterCore.spawnShapeDimensions.x, emitterCore.spawnShapeDimensions.y, random.generate());

        float3x3 sphereHMatrix = getYRotationMatrix(angle);
        float3 vec = float3(0.0, 0.0, r);
        vec = mul(vec, sphereHMatrix);
    
        particle.pos = vec;
    }
    
@property(!initLocationInUpdate)
    float4 localPos = float4(particle.pos, 1.0);
    particle.pos = mul(localPos, emitterInstance.emitterLocation).xyz;
@end
    
    particle.rot = 0.0;
    float sizeX = lerp(emitterCore.sizeX.x, emitterCore.sizeX.y, random.generate());
    float sizeY = sizeX;
    if(!emitterCore.uniformSize) {
        sizeY = lerp(emitterCore.sizeY.x, emitterCore.sizeY.y, random.generate());
    }
    particle.size = float2(sizeX, sizeY);
    particle.colour = lerp(emitterCore.colourA, emitterCore.colourB, random.generate());
//    particle.colour = float4(1.0, (float)localId / (float)BucketSize, 0.0, 1.0);
    particle.spriteNumber = 0.0;
    
    float spotAngle = lerp(emitterCore.spotAngle.x, emitterCore.spotAngle.y, random.generate());
    float spotAngle2 = lerp(-PI, PI, random.generate());
    
    float3x3 xRot = getXRotationMatrix(spotAngle);
    float3x3 yRot = getYRotationMatrix(spotAngle2);

    float3 dir = mul(float3(0.0, 1.0, 0.0), xRot);
    dir = mul(dir, yRot);
    
@property(!initLocationInUpdate)
    float3x3 emitterRotMatrix = (float3x3)emitterInstance.emitterLocation;
    particle.dir = mul(dir, emitterRotMatrix);
@else
    particle.dir = dir;
@end

    particle.dirVelocity = lerp(emitterCore.directionVelocity.x, emitterCore.directionVelocity.y, random.generate());
    
    if(emitterCore.spriteRange != 0) {
        // particle.spriteNumber = emitterCore.spriteRange-1;
        particle.spriteNumber = random.generate() * (float)emitterCore.spriteRange;
    }
    
    @insertpiece( custom_ComputeParticleWorldCreate_end )
}
