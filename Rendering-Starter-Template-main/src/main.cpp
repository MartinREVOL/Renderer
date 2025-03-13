#include "opengl-framework/opengl-framework.hpp" // Inclure la librairie OpenGL
#include "glm/ext/matrix_transform.hpp"         // Pour glm::rotate et glm::translate
#include "glm/ext/matrix_clip_space.hpp"        // Pour glm::infinitePerspective
#include <iostream>

auto load_mesh(std::filesystem::path const& path) -> gl::Mesh
{
    // Lecture du fichier .obj avec tinyobjloader
    auto reader = tinyobj::ObjReader{};
    reader.ParseFromFile(gl::make_absolute_path(path).string(), {});

    if (!reader.Error().empty())
        throw std::runtime_error("Failed to read 3D model:\n" + reader.Error());
    if (!reader.Warning().empty())
        std::cout << "Warning while reading 3D model:\n" + reader.Warning();

    // Tableaux de stockage des données
    auto vertices = std::vector<float>{};  // Positions, UVs et normales
    auto indices = std::vector<uint32_t>{}; // Indices des faces

    for (auto const& shape : reader.GetShapes())
    {
        for (auto const& idx : shape.mesh.indices)
        {
            // Position 3D
            vertices.push_back(reader.GetAttrib().vertices[3 * idx.vertex_index + 0]);
            vertices.push_back(reader.GetAttrib().vertices[3 * idx.vertex_index + 1]);
            vertices.push_back(reader.GetAttrib().vertices[3 * idx.vertex_index + 2]);

            // UVs (peut être absent)
            if (!reader.GetAttrib().texcoords.empty())
            {
                vertices.push_back(reader.GetAttrib().texcoords[2 * idx.texcoord_index + 0]);
                vertices.push_back(reader.GetAttrib().texcoords[2 * idx.texcoord_index + 1]);
            }
            else
            {
                vertices.push_back(0.0f); // UV.x par défaut
                vertices.push_back(0.0f); // UV.y par défaut
            }

            // Normales (peut être absent)
            if (!reader.GetAttrib().normals.empty())
            {
                vertices.push_back(reader.GetAttrib().normals[3 * idx.normal_index + 0]);
                vertices.push_back(reader.GetAttrib().normals[3 * idx.normal_index + 1]);
                vertices.push_back(reader.GetAttrib().normals[3 * idx.normal_index + 2]);
            }
            else
            {
                vertices.push_back(0.0f); // Normale.x par défaut
                vertices.push_back(0.0f); // Normale.y par défaut
                vertices.push_back(1.0f); // Normale.z par défaut (face vers l'avant)
            }

            // Ajout des indices
            indices.push_back(static_cast<uint32_t>(indices.size()));
        }
    }

    // Création du mesh OpenGL
    return gl::Mesh{{
        .vertex_buffers = {{
            .layout = {
                gl::VertexAttribute::Position3D{0}, // Positions 3D
                gl::VertexAttribute::UV{1},         // UVs
                gl::VertexAttribute::Vec3{2}      // Normales
            },
            .data   = vertices,
        }},
        .index_buffer = indices
    }};
}

int main()
{
    // Initialisation OpenGL
    gl::init("TPs de Rendering");
    gl::maximize_window();
    auto camera = gl::Camera{};
    gl::set_events_callbacks({camera.events_callbacks()});

    // Activer le test de profondeur
    glEnable(GL_DEPTH_TEST);

    // Activer la transparence (blending)
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    // Charger le modèle 3D
    auto const model_mesh = load_mesh("res/fourareen.obj");

    // Charger les shaders
    auto const shader = gl::Shader{{
        .vertex   = gl::ShaderSource::File{"res/vertex.glsl"},
        .fragment = gl::ShaderSource::File{"res/fragment.glsl"},
    }};

    // Charger la texture
    auto const texture = gl::Texture{
        gl::TextureSource::File{
            .path           = "res/texture2.jpg",
            .flip_y         = true,
            .texture_format = gl::InternalFormat::RGBA8,
        },
        gl::TextureOptions{
            .minification_filter  = gl::Filter::Linear,
            .magnification_filter = gl::Filter::Linear,
            .wrap_x               = gl::Wrap::Repeat,
            .wrap_y               = gl::Wrap::Repeat,
        }
    };

    while (gl::window_is_open())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shader.bind();

        // Récupérer le temps écoulé
        float time_in_seconds = gl::time_in_seconds();

        // Matrices de vue et de projection
        glm::mat4 const view_matrix = camera.view_matrix();
        glm::mat4 const projection_matrix = glm::infinitePerspective(
            glm::radians(45.f), 
            gl::framebuffer_aspect_ratio(),
            0.001f
        );

        glm::mat4 const view_projection_matrix = projection_matrix * view_matrix;

        // **Appliquer une rotation dynamique autour de l'axe Y**
        glm::mat4 const rotation = glm::rotate(
            glm::mat4{1.f}, 
            glm::radians(time_in_seconds * 45.f),  // Rotation de 45° par seconde
            glm::vec3{1.f, 0.f, 0.f} 
        );

        glm::mat4 const translation = glm::translate(
            glm::mat4{1.f}, 
            glm::vec3{1.f, 0.f, 0.f}  // Déplacement vers le haut
        );

        // Appliquer la transformation (Translation * Rotation)
        glm::mat4 const model_matrix = translation * rotation;
        glm::mat4 const normal_matrix = glm::transpose(glm::inverse(model_matrix));

        glm::mat4 const model_view_projection_matrix = view_projection_matrix * model_matrix;

        // Définition des lumières en World Space
        glm::vec3 light_direction_ws = glm::normalize(glm::vec3(0.2f, 0.3f, -1.0f));
        glm::vec3 light_position_ws = glm::vec3(2.0f, 2.0f, 2.0f);

        glm::vec3 light_color = glm::vec3(0.2f, 0.2f, 0.2f); // Lumière orange

        // Envoyer les uniformes aux shaders
        shader.set_uniform("model_view_projection_matrix", model_view_projection_matrix);
        shader.set_uniform("model_matrix", model_matrix);
        shader.set_uniform("normal_matrix", normal_matrix);
        shader.set_uniform("light_direction_ws", light_direction_ws);
        shader.set_uniform("light_position_ws", light_position_ws);
        shader.set_uniform("texture_sampler", texture);
        shader.set_uniform("light_color", light_color);

        // Dessiner le modèle avec rotation
        model_mesh.draw();

        gl::framebuffer_aspect_ratio(); // Gérer le redimensionnement
    }


}
