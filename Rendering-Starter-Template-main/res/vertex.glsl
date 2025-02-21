#version 410

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv; // Attribut UV

out vec2 frag_uv; // Passer les UVs au fragment shader

uniform mat4 model_view_projection_matrix;

void main()
{
    frag_uv = in_uv; // Passer les UVs au fragment shader
    gl_Position = model_view_projection_matrix * vec4(in_position, 1.0);
}
