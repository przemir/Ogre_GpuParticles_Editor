@insertpiece( SetCrossPlatformSettings )

@insertpiece( ParticleWorldRandom )

@insertpiece( ParticleWorldStructures )

const float PI = 3.14159265;

float3x3 getXRotationMatrix(float angle)
{
@property( syntax == hlsl )
    float s, c;
    sincos(angle, s, c);
@end
@property( syntax == glsl || syntax == glslvk )
    float s = sin( angle );
    float c = cos( angle );
@end
@property( syntax == metal )
    float s, c;
    s = sincos(angle, c);
@end
    return float3x3(
        1, 0, 0,
        0, c,-s,
        0, s, c
    );
}

float3x3 getYRotationMatrix(float angle)
{
@property( syntax == hlsl )
    float s, c;
    sincos(angle, s, c);
@end
@property( syntax == glsl || syntax == glslvk )
    float s = sin( angle );
    float c = cos( angle );
@end
@property( syntax == metal )
    float s, c;
    s = sincos(angle, c);
@end
    return float3x3(
        c, 0, s,
        0, 1, 0,
       -s, 0, c
    );
}

@property( syntax == glsl )
    #define ogre_U0 binding = 0
    #define ogre_R0 binding = 1
    #define ogre_R1 binding = 2
    #define ogre_R2 binding = 3
    #define ogre_R3 binding = 4
@end

layout(std430, ogre_U0) /*writeonly ?*/ restrict buffer particleDataListBuf
{
    Particle particleDataList[];
};

layout(std430, ogre_R0) readonly restrict buffer emitterCoreDataBuf
{
    EmitterCoreData emitterCoreData[];
};
// InstanceData should come as Const buffer: emitter location matrix -
// those values changes per turn, but won't be modified here.
layout(std430, ogre_R1) readonly restrict buffer emitterInstanceDataBuf
{
    EmitterInstanceData emitterInstanceData[];
};
layout(std430, ogre_R2) readonly restrict buffer bucketGroupDataBuf
{
    BucketGroup bucketGroupData[];
};
layout(std430, ogre_R3) readonly restrict buffer particleWorldBuf
{
    ParticleWorld particleWorld[];
};

layout( local_size_x = @value( threads_per_group_x ),
        local_size_y = @value( threads_per_group_y ),
        local_size_z = @value( threads_per_group_z ) ) in;

vulkan( layout( ogre_P0 ) uniform Params { )
    // Max particles may be const (once per shader compilation)
    uniform uint BucketSize;
    uniform uint MaxParticles;
vulkan( }; )

@insertpiece( custom_ComputeParticleWorldCreate_declarations)

void main()
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
    random.seedAddon = int( MaxParticles + 1u );
    
@property(!initLocationInUpdate)
    particle.lifetime = 0.0;
@else
    particle.lifetime = -1.0;
@end
    particle.maxLifetime = lerp(emitterCore.lifetime.x, emitterCore.lifetime.y, NumberGenerator_generate( random ));


    particle.pos = float3(0.0, 0.0, 0.0);
    if(emitterCore.spawnShape == 1) {
        // box shape
        
        float3 vec = float3( lerp(0.0, emitterCore.spawnShapeDimensions.x, NumberGenerator_generate( random )),
                             lerp(0.0, emitterCore.spawnShapeDimensions.y, NumberGenerator_generate( random )),
                             lerp(0.0, emitterCore.spawnShapeDimensions.z, NumberGenerator_generate( random )) );
        vec -= emitterCore.spawnShapeDimensions / 2.0;
        
        particle.pos = vec;
    }
    else if(emitterCore.spawnShape == 2) {
        // sphere shape
        
        float angleH = lerp(-PI, PI, NumberGenerator_generate( random ));
        float angleV = lerp(-PI/2.0, PI/2.0, NumberGenerator_generate( random ));
        float r = lerp(0.0, emitterCore.spawnShapeDimensions.x, NumberGenerator_generate( random ));
        
        float3x3 sphereVMatrix = getXRotationMatrix(angleV);
        float3x3 sphereHMatrix = getYRotationMatrix(angleH);
        
        float3 vec = float3(0.0, 0.0, r);
        vec = mul(vec, sphereVMatrix);
        vec = mul(vec, sphereHMatrix);
        
        particle.pos = vec;
    }
    else if(emitterCore.spawnShape == 3) {
        // disc shape
        
        float angle = lerp(-PI, PI, NumberGenerator_generate( random ));
        float r = lerp(emitterCore.spawnShapeDimensions.x,
                       emitterCore.spawnShapeDimensions.y,
                       NumberGenerator_generate( random ));

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
    float sizeX = lerp(emitterCore.sizeX.x, emitterCore.sizeX.y, NumberGenerator_generate( random ));
    float sizeY = sizeX;
    if(emitterCore.uniformSize == 0u) {
        sizeY = lerp(emitterCore.sizeY.x, emitterCore.sizeY.y, NumberGenerator_generate( random ));
    }
    particle.size = float2(sizeX, sizeY);
    particle.colour = lerp(emitterCore.colourA, emitterCore.colourB, NumberGenerator_generate( random ));
//    particle.colour = float4(1.0, (float)localId / (float)BucketSize, 0.0, 1.0);
    particle.spriteNumber = 0.0;
    
    float spotAngle = lerp(emitterCore.spotAngle.x, emitterCore.spotAngle.y, NumberGenerator_generate( random ));
    float spotAngle2 = lerp(-PI, PI, NumberGenerator_generate( random ));
    
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

    particle.dirVelocity = lerp(emitterCore.directionVelocity.x,
                                emitterCore.directionVelocity.y,
                                NumberGenerator_generate( random ));
    
    if(emitterCore.spriteRange != 0u) {
        // particle.spriteNumber = emitterCore.spriteRange-1;
        particle.spriteNumber = NumberGenerator_generate( random ) * float(emitterCore.spriteRange);
    }
    
    @insertpiece( custom_ComputeParticleWorldCreate_end )
}
