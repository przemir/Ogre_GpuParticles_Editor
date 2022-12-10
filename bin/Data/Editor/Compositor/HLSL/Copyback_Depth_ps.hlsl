struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

Texture2D<float> depthTexture	: register(t0);

float main
(
	PS_INPUT inPs,
	float4 gl_FragCoord : SV_Position
) : SV_Depth
{
//    return 1.0;
	return depthTexture.Load( int3( int2(gl_FragCoord.xy), 0 ) ).x;
}
