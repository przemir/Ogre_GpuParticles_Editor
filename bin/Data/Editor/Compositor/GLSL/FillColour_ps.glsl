#version ogre_glsl_ver_330

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform vec4 colour;
vulkan( }; )

vulkan_layout( location = 0 )
out vec4 fragColour;

void main()
{
  fragColour = colour;
}
