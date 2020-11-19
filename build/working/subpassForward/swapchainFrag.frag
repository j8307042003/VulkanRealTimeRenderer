#version 450
  
layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;

layout (binding = 2) uniform UBO {
	vec4 color;
} ubo;

layout (location = 0) out vec4 outColor;



void main()
{

	outColor.rgb = (subpassLoad(inputColor).rgb +  0.2 * (1 - subpassLoad(inputDepth).r)) * vec3(0.5, 0.5, 1.0);
	outColor = subpassLoad(inputColor);
	// outColor.rgba = vec4(subpassLoad(inputDepth).r);

}