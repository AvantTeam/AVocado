#include <av/core/app.hpp>
#include <av/core/graphics/mesh.hpp>
#include <glm/vec2.hpp>

#include <stdexcept>

using namespace av; // I don't care.

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    log::msg<log_level::info>("Start test.");

    log::set_level<log_level::error>();
    log::msg<log_level::debug>("Can't see me.");
    log::msg<log_level::error>("Notice me!");

    log::set_level<log_level::debug>();
    log::msg<log_level::debug>("Size of input_manager: %d", sizeof(input_manager));
    log::msg<log_level::debug>("Size of input_value  : %d", sizeof(input_value));
    log::msg<log_level::debug>("Size of key_bind     : %d", sizeof(key_bind));

    app process;
    if(!process.has_initialized()) return 1;

    class listener: public app_listener {
        mesh model;
        shader model_shader;

        public:
        listener():
            model(8, 6, {vert_attribute::pos_2D}),
            model_shader(R"(
#version 150 core
in vec2 a_pos;
out vec2 v_col;

void main() {
    gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);
    v_col = (a_pos + vec2(1.0, 1.0)) / 2.0;
})", R"(
#version 150 core
out vec4 out_color;
in vec2 v_col;

void main() {
    out_color = vec4(v_col.x, 0.0, v_col.y, 1.0);
})") {}

        void init(app &) override {
            float vertices[] = {
                -1.0f, -1.0f,
                 1.0f, -1.0f,
                 1.0f,  1.0f,
                -1.0f,  1.0f
            };

            unsigned short indices[] = {0, 1, 2, 2, 3, 0};

            model.set_vertices(vertices, 0, sizeof(vertices));
            model.set_indices(indices, 0, sizeof(indices));
        }

        void update(app &) override {
            model_shader.use();
            model.render(model_shader, GL_TRIANGLES, 0, model.get_max_indices());
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
    if(!process.init_listeners()) return 1;

    bool success = process.loop();

    log::msg<log_level::info>("End test.");
    return success;
}
