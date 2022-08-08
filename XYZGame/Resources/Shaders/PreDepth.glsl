// Shadow Map shader

#type vertex
#version 450 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

layout(location = 2) in vec4  a_TransformRow0;
layout(location = 3) in vec4  a_TransformRow1;
layout(location = 4) in vec4  a_TransformRow2;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

layout(location = 0) out float v_LinearDepth;

void main()
{
	mat4 transform = mat4(
		vec4(a_TransformRow0.x, a_TransformRow1.x, a_TransformRow2.x, 0.0),
		vec4(a_TransformRow0.y, a_TransformRow1.y, a_TransformRow2.y, 0.0),
		vec4(a_TransformRow0.z, a_TransformRow1.z, a_TransformRow2.z, 0.0),
		vec4(a_TransformRow0.w, a_TransformRow1.w, a_TransformRow2.w, 1.0)
	);

	vec4 worldPosition = transform * vec4(a_Position, 1.0);

	v_LinearDepth = -(u_ViewMatrix * worldPosition).z;

	gl_Position = u_ViewProjectionMatrix * worldPosition;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_LinearDepth;

layout(location = 0) in float v_LinearDepth;

void main()
{
	// TODO: Check for alpha in texture
	o_LinearDepth = vec4(v_LinearDepth, 0.0, 0.0, 1.0);
}