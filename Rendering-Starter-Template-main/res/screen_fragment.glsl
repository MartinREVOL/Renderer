#version 410

out vec4 out_color;

in vec2 frag_uv;
uniform sampler2D screen_texture; // Texture de sortie du render target

void main()
{
    vec4 variabledecon = texture(screen_texture, frag_uv);
    out_color = vec4(variabledecon.r, variabledecon.g, variabledecon.b, 1.0); // Appliquer la texture
}
