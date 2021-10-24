#type vertex
#version 430 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


out vec2 v_TexCoords;

void main()
{
    v_TexCoords = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 430

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoords;

layout(binding = 0) uniform sampler2D u_Texture[2];

const float c_Gamma = 2.2;

// Based on http://www.oscars.org/science-technology/sci-tech-projects/aces
vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

vec3 GammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}

void main()
{    
    vec3 color = texture(u_Texture[0], v_TexCoords).rgb;
	vec3 bloomColor = texture(u_Texture[1], v_TexCoords).rgb;
	bloomColor = ACESTonemap(bloomColor);
	bloomColor = GammaCorrect(bloomColor, c_Gamma);
	o_Color = vec4(color + bloomColor, 1.0);
}