
@piece( custom_vs_uniformDeclaration )

@property( syntax == glsl )
	#define ogre_R2 binding = 2
	#define ogre_R3 binding = 3
	#define ogre_R4 binding = 4
	#define ogre_R5 binding = 5
@end

@property(particleWorldEnabled)
	@insertpiece(ParticleWorldStructures)

	layout(std430, ogre_R5) readonly restrict buffer particleDataListBuf
	{
		Particle particleDataList[];
	};
	layout(std430, ogre_R2) readonly restrict buffer emitterCoreDataBuf
	{
		EmitterCoreData emitterCoreData[];
	};
	layout(std430, ogre_R3) readonly restrict buffer bucketGroupDataBuf
	{
		BucketGroup bucketGroupData[];
	};

	const float2 QuadCorners[] =
	{
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
