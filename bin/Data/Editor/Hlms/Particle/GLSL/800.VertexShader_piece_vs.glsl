
@undefpiece( ExamplePiece )

@piece( custom_vs_posExecution )

@property(particleWorldEnabled)
    outVs.drawId = finalDrawId;

	@property( !hlms_identity_world )
		float4x4 worldViewProj = UNPACK_MAT4( worldMatBuf, finalDrawId );
	@end
    
    uint bufferStart = worldMaterialIdx[finalDrawId].w;

	uint quadIndex = inVs_vertexId / 4u;
	uint vertexInQuad = inVs_vertexId % 4u;
    
    uint bucketGroupId = quadIndex / @value(BucketSize);
    uint localId = quadIndex % @value(BucketSize);
	BucketGroup entry = bucketGroupData[bufferStart + bucketGroupId];
    
    // Go from younger to older. There is maximum one bucket change.
	uint currentBucketCount = entry.lastParticleIndex % @value(BucketSize)u;
    uint particleIndex = localId <= currentBucketCount
       ? entry.lastParticleIndex - localId
       : entry.nextBucketParticleIndex + currentBucketCount - localId;
    
    // uint particleIndex = entry.bucketId * @value(BucketSize) + localId;
    
    #define emitterCore emitterCoreData[entry.emitterCoreId]
    #define particle particleDataList[particleIndex]

    float2 texCornerPos = QuadCorners[vertexInQuad];
    
	float4 spriteTexCoords = emitterCore.spriteCoordRange[int(particle.spriteNumber)];
    
    float2 cornerPos = QuadCorners[vertexInQuad] - float2(0.5, 0.5);
    cornerPos.x *= particle.size.x;
    cornerPos.y *= particle.size.y;
    
    // float sin_rot = sin(particle.rot);
    // float cos_rot = cos(particle.rot);
    // 
	// // rotate the billboard:
	// float2x2 rot = float2x2(
	// 	cos_rot, -sin_rot,
	// 	sin_rot, cos_rot
	// 	);
	// cornerPos = mul(cornerPos, rot);
    
    float4x4 pCam = transpose(passBuf.particleCameraViewMatrix);
    float3x3 pCam3x3 = toFloat3x3(pCam);
    
	float3 cameraRight = mul( float3( 1, 0, 0 ), pCam3x3 );
	float3 cameraUp = mul( float3( 0, 1, 0 ), pCam3x3 );
	float3 cameraFront = mul( float3( 0, 0, -1 ), pCam3x3 );
    
    // like in OgreBillboardSet Ogre::BBT_POINT not accurate facing
    float3 pX = cameraRight;
    float3 pY = cameraUp;
    
    if(emitterCore.billboardType == BillboardType_OrientedSelf) {
        // like in OgreBillboardSet Ogre::BBT_ORIENTED_SELF
        pX = normalize(cross(cameraFront, particle.dir));
        pY = particle.dir;
    }
    
	// outVs_Position = float4(particle.pos + cameraRight * cornerPos.x + cameraUp * cornerPos.y, 1.0);
	outVs_Position = float4(particle.pos + pX * cornerPos.x + pY * cornerPos.y, 1.0);
	@property( !hlms_identity_world )
		outVs_Position = mul( outVs_Position, worldViewProj );
	@end
    outVs.colour = particle.colour;
    // outVs.uv0.xy = texCornerPos;
    outVs.uv0.xy = float2(spriteTexCoords.x + spriteTexCoords.z * texCornerPos.x, spriteTexCoords.y + spriteTexCoords.w * (1.0f - texCornerPos.y));

    if(emitterCore.useFader != 0) {
        float percent = 1.0f;
        if(emitterCore.faderStartPhaseTime != 0.0 && particle.lifetime < emitterCore.faderStartPhaseTime) {
            percent = particle.lifetime / emitterCore.faderStartPhaseTime;
        }
        float endPhase = particle.maxLifetime - emitterCore.faderEndPhaseTime;
        if(emitterCore.faderEndPhaseTime != 0.0 && particle.lifetime > endPhase) {
            percent = 1.0f - ((particle.lifetime - endPhase) / emitterCore.faderEndPhaseTime);
        }
        
        if(emitterCore.useFader == 1) {
            outVs.colour *= percent;
        }
        else {
            outVs.colour.w *= percent;
        }
    }

    if(particle.lifetime < 0.0 || particle.lifetime > particle.maxLifetime || localId >= entry.particleCount) {
        outVs.colour = float4(0.0, 0.0, 0.0, 0.0);
    }
    
@end

@property( hlms_shadowcaster )

	@property( !hlms_instanced_stereo )
		@property( !hlms_identity_world )
			float4x4 worldViewProj = UNPACK_MAT4( worldMatBuf, finalDrawId );
		@end

		@property( !hlms_dual_paraboloid_mapping )
			outVs_Position = mul( float4(0, 0, 0, 0), masterMatrix );
		@else
			//Dual Paraboloid Mapping
			outVs_Position.w	= 1.0f;
			outVs_Position.xyz	= mul( vertex, masterMatrix ).xyz;
			float L = length( outVs_Position.xyz );
			outVs_Position.z	+= 1.0f;
			outVs_Position.xy	/= outVs_Position.z;
			outVs_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);
		@end
	@else
		// Instanced stereo must index the viewProj matrix dynamically, thus considering
		// hlms_identity_viewproj_dynamic to save performance by hardcoding the index is pointless
		float4x4 worldMat = UNPACK_MAT4( worldMatBuf, finalDrawId );
		@property( !hlms_identity_world )
			float4 worldPos = float4( mul( float4(0, 0, 0, 0), worldMat ).xyz, 1.0f );
		@else
			float4 worldPos = float4( float4(0, 0, 0, 0).xyz, 1.0f );
		@end
		uint vpmatIdx = ( (inVs_stereoDrawId & 0x01u) << 1u ) + worldMaterialIdx[finalDrawId].z;
		outVs_Position = mul( worldPos, passBuf.viewProj[vpmatIdx] );
	@end

	@property( (hlms_global_clip_planes || (hlms_shadowcaster && (exponential_shadow_maps || hlms_shadowcaster_point))) && !hlms_instanced_stereo )
		float3 worldPos = mul( outVs_Position, passBuf.invViewProj ).xyz;
	@end
	@insertpiece( DoShadowCasterVS )

@end

@property( hlms_shadowcaster2 )
    // copied non-caster code for shader to compile
			float4x4 worldViewProj = UNPACK_MAT4( worldMatBuf, finalDrawId );
		

		
		//	outVs_Position = mul( inVs_vertex, masterMatrix );
			outVs_Position = mul( float4(0, 0, 0, 0), masterMatrix );
		
	

	

	
		float3 worldPos = mul( outVs_Position, passBuf.invViewProj ).xyz;
	
	
		
			float shadowConstantBias = uintBitsToFloat( worldMaterialIdx[inVs_drawId].y ) * passBuf.depthRange.y;
		

		

		
			outVs.toCameraWS	= worldPos.xyz - passBuf.cameraPosWS.xyz;
			
				outVs.constBias = shadowConstantBias;
			
		

		
			//We can't make the depth buffer linear without Z out in the fragment shader;
			//however we can use a cheap approximation ("pseudo linear depth")
			//see http://www.yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
			
				outVs_Position.z = outVs_Position.z + shadowConstantBias;
@end

@end
