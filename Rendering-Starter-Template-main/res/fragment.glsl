#version 410

out vec4 out_color;

in vec2 frag_uv;
in vec3 frag_position_ws; // Position du fragment en World Space
in vec3 frag_normal_ws;   // Normale en World Space

uniform sampler2D texture_sampler;
uniform vec3 light_direction_ws; // Lumière directionnelle
uniform vec3 light_position_ws;  // Lumière ponctuelle
uniform vec3 light_color;        // 🌈 Nouvelle couleur de la lumière

const float ambient_intensity = 0.3;

void main()
{
    vec3 normal = normalize(frag_normal_ws);
    vec3 albedo = texture(texture_sampler, frag_uv).rgb;

    // Lumière directionnelle
    float light_intensity = max(dot(normal, -light_direction_ws), 0.0);

    // Lumière ponctuelle
    vec3 light_dir = normalize(light_position_ws - frag_position_ws);
    float distance = length(light_position_ws - frag_position_ws);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    float point_light_intensity = max(dot(normal, light_dir), 0.0) * attenuation;

    // Lumière finale (ajout de la couleur de la lumière)
    float final_intensity = max(light_intensity + point_light_intensity + ambient_intensity, ambient_intensity);
    vec3 shaded_color = albedo * light_color * final_intensity;

    out_color = vec4(shaded_color, 1.0);
}
