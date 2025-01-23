#version 410

// À mettre avant le main
out vec3 vertex_position;

layout(location = 0) in vec3 in_position;

uniform mat4 model_view_projection_matrix; // Matrice combinée MVP



void main()
{
    vertex_position = in_position;
    // Appliquer la matrice MVP à la position des sommets
    gl_Position = model_view_projection_matrix * vec4(in_position, 1.0);
}
