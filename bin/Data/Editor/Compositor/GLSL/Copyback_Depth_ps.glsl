#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D depthTexture;

void main()
{
	gl_FragDepth = texelFetch( depthTexture, ivec2( gl_FragCoord.xy ), 0 ).x;
}
