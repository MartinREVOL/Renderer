#version 410

out vec4 out_color;

in vec2 frag_uv; // UVs passés depuis le vertex shader

uniform sampler2D texture_sampler; // La texture

void main()
{
    out_color = texture(texture_sampler, frag_uv); // Appliquer la texture
}
