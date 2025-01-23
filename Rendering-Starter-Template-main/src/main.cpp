#include "opengl-framework/opengl-framework.hpp" // Inclue la librairie de rendu
#include "glm/ext/matrix_transform.hpp"         // Pour glm::rotate et glm::translate
#include "glm/ext/matrix_clip_space.hpp"        // Pour glm::infinitePerspective

int main()
{
    // Initialisation
    gl::init("TPs de Rendering"); // On crée une fenêtre
    gl::maximize_window();        // Maximiser la fenêtre si souhaité
    auto camera = gl::Camera{};
    gl::set_events_callbacks({camera.events_callbacks()}); // Connecter la caméra aux événements

    // À mettre dans l'initialisation
    glEnable(GL_DEPTH_TEST);

    // Activer la transparence (blending)
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    // Définition du maillage (rectangle)
    auto const rectangle_mesh = gl::Mesh{{
        .vertex_buffers = { {
            .layout = {gl::VertexAttribute::Position3D{0}},
            .data   = {
                -0.5f, -0.5f, -0.5f, // Position3D du 1er coin
                +0.5f, -0.5f, -0.5f, // Position3D du 2ème coin
                +0.5f, +0.5f, -0.5f, // Position3D du 3ème coin
                -0.5f, +0.5f, -0.5f, // Position3D du 4ème coin
                -0.5f, -0.5f, +0.5f, // Position3D du 5er coin
                +0.5f, -0.5f, +0.5f, // Position3D du 6ème coin
                +0.5f, +0.5f, +0.5f, // Position3D du 7ème coin
                -0.5f, +0.5f, +0.5f // Position3D du 8ème coin
            },
        }},
        .index_buffer   = {
            0, 1, 2,
            2, 3, 0,
            4, 5, 6,
            6, 7, 4,
            3, 2, 6,
            6, 7, 3,
            4, 5, 1,
            1, 0, 4,
            4, 0, 3,
            3, 7, 4,
            1, 5, 6,
            6, 2, 1
        },
    }};

    // Charger les shaders
    auto const shader = gl::Shader{{
        .vertex   = gl::ShaderSource::File{"res/vertex.glsl"},
        .fragment = gl::ShaderSource::File{"res/fragment.glsl"},
    }};

    while (gl::window_is_open())
    {
        glClearColor(0.78, 0.2, 0.2, 1.0); // Couleur de fond
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();

        // Récupérer les matrices View et Projection
        glm::mat4 const view_matrix = camera.view_matrix();
        glm::mat4 const projection_matrix = glm::infinitePerspective(
            glm::radians(45.f),         // Field of View (FOV)
            gl::framebuffer_aspect_ratio(), // Aspect ratio (largeur/hauteur)
            0.001f                      // Near plane
        );

        glm::mat4 const view_projection_matrix = projection_matrix * view_matrix;

        // Créer une rotation statique autour de l'axe Z
        glm::mat4 const rotation = glm::rotate(
            glm::mat4{1.f},            // Matrice identité
            glm::radians(30.f),        // Angle fixe en radians (30° ici)
            glm::vec3{0.f, 0.f, 1.f}   // Rotation autour de l'axe Z
        );

        // Créer une translation statique le long de l'axe Y
        glm::mat4 const translation = glm::translate(
            glm::mat4{1.f},            // Matrice identité
            glm::vec3{0.f, 1.f, 0.f}   // Déplacement vers le haut
        );

        // Combinaison : rotation suivie de translation
        glm::mat4 const model_matrix = translation * rotation;

        // Calcul final : Model-View-Projection Matrix
        glm::mat4 const model_view_projection_matrix = view_projection_matrix * model_matrix;

        // Envoyer la MVP au shader
        shader.set_uniform("model_view_projection_matrix", model_view_projection_matrix);

        // (Optionnel) Récupérer le temps pour l'effet de fade
        float time_in_seconds = gl::time_in_seconds();
        shader.set_uniform("u_time", time_in_seconds);

        // Dessiner le rectangle
        rectangle_mesh.draw();

        gl::framebuffer_aspect_ratio(); // Gérer le redimensionnement
    }
}
