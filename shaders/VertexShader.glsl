#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

uniform vec3 object_color;
uniform vec3 object_decolor;
uniform int deformation_factor;

// Output value to fragment shader
out vec3 color;
out vec3 def_color;

void main()
{
	// Compute world space vectors
	vec3 world_position = (Model * vec4(v_position, 1)).xyz;
	vec3 world_normal	= normalize(mat3(Model) * normalize(v_normal));

	vec3 V = normalize(eye_position - world_position);
	vec3 L = normalize(light_position - world_position);
	vec3 H = normalize(L + V);
	vec3 R = normalize(reflect(L, world_normal));

	// Define ambient light component
	float ambient_light = 0.5;

	// Compute diffuse light component
	float diffuse_light = material_kd * max(dot(world_normal, L), 0);

	// Compute specular light component
	float specular_light = 0;

	if (diffuse_light > 0)
	{
		// specular_light = material_ks * pow(max(dot(world_normal, H), 0), material_shininess);
		specular_light = material_ks * pow(max(dot(V, R), 0), material_shininess);
	}

	// Compute light
	float d						= distance(light_position, v_position);
	float attenuation_factor	= 1 / (1 + 0.14 * d + 0.07 * d * d);
	float light					= ambient_light + attenuation_factor * (diffuse_light + specular_light);

	// Send color light output to fragment shader
	color = object_color * light;
	def_color = object_decolor * light;
	vec3 v_position_deformed = v_position;
	if (deformation_factor != 0) {
		v_position_deformed.y += sin(v_position_deformed.x + v_position_deformed.z)/15 * deformation_factor;
	}
	gl_Position = Projection * View * Model * vec4(v_position_deformed, 1.0);
}