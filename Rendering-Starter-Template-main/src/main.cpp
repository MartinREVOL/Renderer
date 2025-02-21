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

    auto render_target = gl::RenderTarget{gl::RenderTarget_Descriptor{
        .width          = gl::framebuffer_width_in_pixels(),
        .height         = gl::framebuffer_height_in_pixels(),
        .color_textures = {
            gl::ColorAttachment_Descriptor{
                .format  = gl::InternalFormat_Color::RGBA8,
                .options = {
                    .minification_filter  = gl::Filter::NearestNeighbour, // On va toujours afficher la texture à la taille de l'écran,
                    .magnification_filter = gl::Filter::NearestNeighbour, // donc les filtres n'auront pas d'effet. Tant qu'à faire on choisit le moins coûteux.
                    .wrap_x               = gl::Wrap::ClampToEdge,
                    .wrap_y               = gl::Wrap::ClampToEdge,
                },
            },
        },
        .depth_stencil_texture = gl::DepthStencilAttachment_Descriptor{
            .format  = gl::InternalFormat_DepthStencil::Depth32F,
            .options = {
                .minification_filter  = gl::Filter::NearestNeighbour,
                .magnification_filter = gl::Filter::NearestNeighbour,
                .wrap_x               = gl::Wrap::ClampToEdge,
                .wrap_y               = gl::Wrap::ClampToEdge,
            },
        },
    }};

    gl::set_events_callbacks({
        camera.events_callbacks(),
        {.on_framebuffer_resized = [&](gl::FramebufferResizedEvent const& e) {
            if(e.width_in_pixels != 0 && e.height_in_pixels != 0) // OpenGL crash si on tente de faire une render target avec une taille de 0
                render_target.resize(e.width_in_pixels, e.height_in_pixels);
        }},
    });

    // À mettre dans l'initialisation
    glEnable(GL_DEPTH_TEST);

    // Activer la transparence (blending)
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    // Définition du maillage (rectangle)
    auto const rectangle_mesh = gl::Mesh{{
        .vertex_buffers = { {
            .layout = {gl::VertexAttribute::Position3D{0}, gl::VertexAttribute::UV{1}},
            .data   = {
                // Positions 3D       // UVs
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Face avant
                +0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
                +0.5f, +0.5f, -0.5f,  1.0f, 1.0f,
                -0.5f, +0.5f, -0.5f,  0.0f, 1.0f,

                -0.5f, -0.5f, +0.5f,  0.0f, 0.0f, // Face arrière
                +0.5f, -0.5f, +0.5f,  1.0f, 0.0f,
                +0.5f, +0.5f, +0.5f,  1.0f, 1.0f,
                -0.5f, +0.5f, +0.5f,  0.0f, 1.0f
            },
        }},
        .index_buffer   = {
            0, 1, 2,  2, 3, 0, // Face avant
            4, 5, 6,  6, 7, 4, // Face arrière
            3, 2, 6,  6, 7, 3, // Face haut
            0, 1, 5,  5, 4, 0, // Face bas
            0, 3, 7,  7, 4, 0, // Face gauche
            1, 2, 6,  6, 5, 1  // Face droite
        },
    }};

    auto const screen_quad = gl::Mesh{{
        .vertex_buffers = {{
            .layout = {
                gl::VertexAttribute::Position2D{0}, // Positions en 2D
                gl::VertexAttribute::UV{1}          // UVs
            },
            .data   = {
                // x, y   // u, v
                -1.0f, -1.0f,  0.0f, 0.0f, // Bas gauche
                +1.0f, -1.0f,  1.0f, 0.0f, // Bas droit
                +1.0f, +1.0f,  1.0f, 1.0f, // Haut droit
                -1.0f, +1.0f,  0.0f, 1.0f  // Haut gauche
            },
        }},
        .index_buffer   = {
            0, 1, 2,  2, 3, 0  // Deux triangles formant le quad
        },
    }};
    

    // Charger les shaders
    auto const shader = gl::Shader{{
        .vertex   = gl::ShaderSource::File{"res/vertex.glsl"},
        .fragment = gl::ShaderSource::File{"res/fragment.glsl"},
    }};

    auto const screen_shader = gl::Shader{{
        .vertex   = gl::ShaderSource::File{"res/screen_vertex.glsl"},
        .fragment = gl::ShaderSource::File{"res/screen_fragment.glsl"},
    }};
    

    auto const texture = gl::Texture{
        gl::TextureSource::File{ // Peut être un fichier, ou directement un tableau de pixels
            .path           = "res/texture.png",
            .flip_y         = true, // Il n'y a pas de convention universelle sur la direction de l'axe Y. Les fichiers (.png, .jpeg) utilisent souvent une direction différente de celle attendue par OpenGL. Ce booléen flip_y est là pour inverser la texture si jamais elle n'apparaît pas dans le bon sens.
            .texture_format = gl::InternalFormat::RGBA8, // Format dans lequel la texture sera stockée. On pourrait par exemple utiliser RGBA16 si on voulait 16 bits par canal de couleur au lieu de 8. (Mais ça ne sert à rien dans notre cas car notre fichier ne contient que 8 bits par canal, donc on ne gagnerait pas de précision). On pourrait aussi stocker en RGB8 si on ne voulait pas de canal alpha. On utilise aussi parfois des textures avec un seul canal (R8) pour des usages spécifiques.
        },
        gl::TextureOptions{
            .minification_filter  = gl::Filter::Linear, // Comment on va moyenner les pixels quand on voit l'image de loin ?
            .magnification_filter = gl::Filter::Linear, // Comment on va interpoler entre les pixels quand on zoom dans l'image ?
            .wrap_x               = gl::Wrap::Repeat,   // Quelle couleur va-t-on lire si jamais on essaye de lire en dehors de la texture ?
            .wrap_y               = gl::Wrap::Repeat,   // Idem, mais sur l'axe Y. En général on met le même wrap mode sur les deux axes.
        }
    };

    while (gl::window_is_open())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_target.render([&]() {
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

            shader.set_uniform("texture_sampler", texture);

            // Dessiner le rectangle
            rectangle_mesh.draw();

            gl::framebuffer_aspect_ratio(); // Gérer le redimensionnement
        });
        screen_shader.bind();
        screen_shader.set_uniform("screen_texture", render_target.color_texture(0));
        screen_quad.draw();
    }
}
