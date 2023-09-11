
@insertpiece( ParticleWorldBinarySearch )
@insertpiece( ParticleWorldStructures )

RWStructuredBuffer<Particle> particleDataList : register(u0);
StructuredBuffer<EmitterCoreData> emitterCoreData : register(t0);
StructuredBuffer<EmitterInstanceData> emitterInstanceData : register(t1);
StructuredBuffer<BucketGroup> bucketGroupData : register(t2);
StructuredBuffer<ParticleWorld> particleWorld : register(t3);
@property(useDepthTexture)
Texture2D<float> depthTexture	: register(t4);
SamplerState depthSamplerState  : register(s4);
@end

uniform uint BucketSize;
uniform uint MaxParticles;

#define world particleWorld[0]

// Maps [-1, 1] -> [0, 1], but y is reversed
float2 cameraOrthoRectToUnit(float2 pos2d) {
    return float2((pos2d.x / 2.0f) + 0.5, -(pos2d.y / 2.0f) + 0.5);
}

// Maps [0, 1] -> [-1, 1], but y is reversed
float2 unitToCameraOrthoRect(float2 uv) {
    return float2(uv.x * 2.0 - 1.0, (1.0 - uv.y) * 2.0 - 1);
}

@property(useDepthTexture)
float getLinearDepth(float2 uv) {
    float fDepth = depthTexture.SampleLevel( depthSamplerState, uv, 0 ).x;
    float linearDepth = world.cameraProjectionParamsAB.y / (fDepth - world.cameraProjectionParamsAB.x);
    return linearDepth;
}
@end

// (-1.0, -1.0) - bottom left screen corner
// ( 1.0,  1.0) - top right screen corner
// Normalizes xyz, but contains w as well.
float4 posToScreenWithW(float4 pos) {
    float4 pos2d = mul(pos, world.prevCameraVP);
    pos2d.xyz /= pos2d.w;
    return pos2d;
}

float4 screenToPos(float4 pos2d) {
    float4 p = mul(pos2d, world.prevCameraInvVP);
    p.xyz /= p.w;
    return p;
}

// param n - should be normalized
float3 getReflectionVec(float3 dir, float3 n)
{
    return dir - 2.0 * dot(dir, n) * n;
}

@insertpiece( custom_ComputeParticleWorldUpdate_declarations)

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
    
    // Go from younger to older. There is maximum one bucket change.
    uint currentBucketCount = entry.lastParticleIndex % BucketSize;
    uint particleIndex = localId <= currentBucketCount
       ? entry.lastParticleIndex - localId
       : entry.nextBucketParticleIndex + currentBucketCount - localId;
    
    // uint particleIndex = entry.bucketId * BucketSize + localId;
    #define particle particleDataList[particleIndex]
    #define emitterCore emitterCoreData[entry.emitterCoreId]
    #define emitterInstance emitterInstanceData[entry.emitterInstanceId]

    float elapsedTime = world.elapsedTime;

@property(initLocationInUpdate)
    if(particle.lifetime < 0.0f ) {
        particle.lifetime = 0.0f;
        
        // particle.pos after creation will be in local coordinates
        float4 localPos = float4(particle.pos, 1.0);
        particle.pos = mul(localPos, emitterInstance.emitterLocation).xyz;
        
        float3x3 emitterRotMatrix = (float3x3)emitterInstance.emitterLocation;
        particle.dir = mul(particle.dir, emitterRotMatrix);
        
        @insertpiece( custom_ComputeParticleWorldUpdate_initLocationInUpdate )
    }
@end

	// float fDepth = depthTexture.SampleLevel( depthSamplerState, float2(0.0, 0.0), 0 ).x;
	// float linearDepth = world.cameraProjectionParamsAB.y / (fDepth - world.cameraProjectionParamsAB.x);
	// 
    // // float fDepth = depthTexture.Load(int3( 0, 0, 0 )).x * 4.0f;
    // if(linearDepth > 1.0f) {
    //     linearDepth = 1.0f;
    // }
    // if(linearDepth < 1.0f) {
    //     linearDepth = 0.0f;
    // }
    
@property(useDepthTexture && affector_depth_collision)
    if(emitterCore.affectorDepthCollisionEnabled)
    {  
        float4 pos2d = posToScreenWithW(float4(particle.pos, 1.0));
        if(pos2d.x > -1.0 && pos2d.x < 1.0 && pos2d.y > -1.0 && pos2d.y < 1.0) {
            float2 uv = cameraOrthoRectToUnit(pos2d.xy);
	        float linearDepth = getLinearDepth(uv);
            
            float layerSize = 1.5f;
            float particleS = max(particle.size.x, particle.size.y);
            
            if((pos2d.w + particleS > linearDepth) && (pos2d.w - particleS < linearDepth + layerSize)) {
                // R - right, T - top
                float2 uvR = uv + float2(0.01,  0.00);
                float2 uvT = uv + float2(0.00, -0.01);
                
                float linearDepthR = getLinearDepth(uvR);
                float linearDepthT = getLinearDepth(uvT);
                
                float3 p0 = screenToPos(float4(unitToCameraOrthoRect(uv ), linearDepth , 1.0)).xyz;
                float3 pR = screenToPos(float4(unitToCameraOrthoRect(uvR), linearDepthR, 1.0)).xyz;
                float3 pT = screenToPos(float4(unitToCameraOrthoRect(uvT), linearDepthT, 1.0)).xyz;
                
                float3 n = normalize(cross(pT-p0, pR-p0));
                if(dot(particle.dir, n) < 0) {
                    particle.dir = getReflectionVec(particle.dir, n);
                    particle.dirVelocity *= 0.975;
                }
            }

          // // debug code checking if particle goes behind obstacle
          // if(pos2d.w > linearDepth) {
          //     particle.colour.x = 1.0;
          //     particle.colour.y = 0.0;
          //     particle.colour.z = 0.0;
          // }
        }
    }
@end

    particle.lifetime += elapsedTime;
    
    if(particle.lifetime > particle.maxLifetime) {
        return;
    }
    
    // particle.colour = float4(0.0, 0.0, 1.0, 1.0);
    // particle.pos += float3(0.0, 10.0, 0.0)*elapsedTime;
    particle.pos += particle.dir*particle.dirVelocity*elapsedTime;
    
    float3 dir = particle.dir * particle.dirVelocity;
@property(affector_global_gravity)
    dir += emitterCore.affectorGlobalGravity * elapsedTime;
@end
    particle.dirVelocity = length(dir);
    if(particle.dirVelocity != 0.0) {
        particle.dir = normalize(dir);
    }
    
    if(emitterCore.useSpriteTrack) {
        particle.spriteNumber = binarySearch8(particle.lifetime, emitterCore.spriteTrackTimes);
    }
    
@property(affector_set_colour_track)
    if(emitterCore.affectorSetColourTrackEnabled) {
        particle.colour.xyz = getFromTrack3(particle.lifetime, emitterCore.affectorSetColourTrackTimes, emitterCore.affectorSetColourTrackValuesR, emitterCore.affectorSetColourTrackValuesG, emitterCore.affectorSetColourTrackValuesB);
    }
@end
@property(affector_set_alpha_track)
    if(emitterCore.affectorSetAlphaTrackEnabled) {
        particle.colour.w = getFromTrack1(particle.lifetime, emitterCore.affectorSetAlphaTrackTimes, emitterCore.affectorSetAlphaTrackValues);
    }
@end
@property(affector_set_size_track)
    if(emitterCore.affectorSetSizeTrackEnabled) {
        particle.size = getFromTrack2(particle.lifetime, emitterCore.affectorSetSizeTrackTimes, emitterCore.affectorSetSizeTrackValuesX, emitterCore.affectorSetSizeTrackValuesY);
        if(emitterCore.uniformSize) {
            particle.size.y = particle.size.x;
        }
    }
@end
@property(affector_set_velocity_track)
    if(emitterCore.affectorSetVelocityTrackEnabled) {
        particle.dirVelocity = getFromTrack1(particle.lifetime, emitterCore.affectorSetVelocityTrackTimes, emitterCore.affectorSetVelocityTrackValues);
    }
@end

    @insertpiece( custom_ComputeParticleWorldUpdate_end )
}
