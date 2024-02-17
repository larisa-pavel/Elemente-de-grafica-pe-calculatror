#version 330

layout(location = 0) out vec4 out_color;

// Get color value from vertex shader
in vec3 color;
in vec3 def_color;

void main()
{
	// Write pixel out color
	out_color = vec4(color - def_color, 0);
}