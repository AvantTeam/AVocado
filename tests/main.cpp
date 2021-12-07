#include <av/core/app.hpp>
#include <av/core/graphics/mesh.hpp>
#include <av/util/graphics/color.hpp>
#include <glm/vec2.hpp>

#include <stdexcept>

using namespace av; // I don't care.

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    log::set_level<log_level::error>();
    log::msg<log_level::debug>("Can't see me.");
    log::msg<log_level::error>("Notice me!");

    log::set_level<log_level::debug>();
    log::msg<log_level::debug>("Size of input_manager: %d", sizeof(input_manager));
    log::msg<log_level::debug>("Size of input_value  : %d", sizeof(input_value));
    log::msg<log_level::debug>("Size of key_bind     : %d", sizeof(key_bind));

    log::msg<log_level::warn>("%d, %s, %d, %s, %s",
        vert_attribute::pos_2D.components,
        vert_attribute::pos_2D.type == GL_FLOAT ? "true" : "false",
        vert_attribute::pos_2D.size,
        vert_attribute::pos_2D.normalized ? "true" : "false",
        vert_attribute::pos_2D.name.c_str()
    );

    log::msg<log_level::warn>("%d, %s, %d, %s, %s",
        vert_attribute::color_packed.components,
        vert_attribute::color_packed.type == GL_UNSIGNED_BYTE ? "true" : "false",
        vert_attribute::color_packed.size,
        vert_attribute::color_packed.normalized ? "true" : "false",
        vert_attribute::color_packed.name.c_str()
    );

    app process;
    if(!process.has_initialized()) return 1;

    class listener: public app_listener {
        mesh model;
        shader model_shader;

        public:
        listener():
            model({vert_attribute::pos_2D, vert_attribute::color_packed}),
            model_shader(R"(
#version 150 core
in vec2 a_pos;
in vec4 a_col;

out vec4 v_col;

void main() {
    gl_Position = vec4(a_pos, 0.0, 1.0);
    v_col = a_col;
})", R"(
#version 150 core
out vec4 out_color;
in vec4 v_col;

void main() {
    out_color = v_col;
})") {}

        protected:
        void init(app &) override {
            float vertices[] = {
                -1.0f, -1.0f, color(1.0f, 0.0f, 0.0f).float_bits(),
                1.0f, -1.0f, color(0.0f, 1.0f, 0.0f).float_bits(),
                1.0f, 1.0f, color(0.0f, 0.0f, 1.0f).float_bits(),
                -1.0f, 1.0f, color(1.0f, 1.0f, 1.0f).float_bits()
            };

            unsigned short indices[] = {0, 1, 2, 2, 3, 0};

            model.set_vertices(vertices, 0, sizeof(vertices));
            model.set_indices(indices, 0, sizeof(indices));

            log::msg("Start test.");
            log::msg<log_level::warn>("Vertex size : %d", model.get_vertex_size());
            log::msg<log_level::warn>("Max vertices: %d", model.get_max_vertices());
            log::msg<log_level::warn>("Max indices : %d", model.get_max_indices());
        }

        void update(app &process) override {
            model_shader.use();
            model.render(model_shader, GL_TRIANGLES, 0, model.get_max_indices());

            SDL_SetWindowTitle(process.get_window(), std::string("Delta time: ").append(std::to_string(process.get_time().delta())).c_str());
        }

        void dispose(app &) override {
            log::msg("End test.");
        }
    } listener;

    process.add_listener(&listener);

    key_bind key;
    key.keyboard.set_keys<SDLK_w, SDLK_s, SDLK_a, SDLK_d>();
    key.callback = [](const input_value &value) {
        [[maybe_unused]] const glm::vec2 &axis = value.read<glm::vec2>();
        // Do something with axis...
    };

    process.get_input().bind<key_type::keyboard>("WASD", key);
    return process.loop();
}
