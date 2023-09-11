
@piece( custom_vs_uniformDeclaration )

@property(particleWorldEnabled)
@insertpiece(ParticleWorldStructures)

StructuredBuffer<Particle> particleDataList : register(t5);
StructuredBuffer<EmitterCoreData> emitterCoreData : register(t2);
StructuredBuffer<BucketGroup> bucketGroupData : register(t3);

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
