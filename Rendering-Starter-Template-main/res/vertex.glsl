#version 410

layout(location = 0) in vec3 in_position_os; // Position en Object Space
layout(location = 1) in vec2 in_uv;          // UVs
layout(location = 2) in vec3 in_normal_os;   // Normale en Object Space

out vec2 frag_uv;
out vec3 frag_position_ws; // Position en World Space
out vec3 frag_normal_ws;   // Normale en World Space

uniform mat4 model_matrix;
uniform mat4 normal_matrix; // Inverse de la transposée de la model_matrix
uniform mat4 model_view_projection_matrix;

vec3 apply_matrix_to_position(mat4 matrix, vec3 point)
{
    vec4 tmp = matrix * vec4(point, 1.0);
    return tmp.xyz / tmp.w;
}

vec3 apply_matrix_to_direction(mat4 matrix, vec3 direction)
{
    vec4 tmp = matrix * vec4(direction, 0.0);
    return normalize(tmp.xyz);
}

void main()
{
    frag_uv = in_uv;

    // Appliquer la Model Matrix aux positions
    frag_position_ws = apply_matrix_to_position(model_matrix, in_position_os);

    // Appliquer la Normal Matrix aux normales
    frag_normal_ws = apply_matrix_to_direction(normal_matrix, in_normal_os);

    // Projection finale
    gl_Position = model_view_projection_matrix * vec4(in_position_os, 1.0);
}
