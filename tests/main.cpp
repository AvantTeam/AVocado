#include <av_sdl/app.hpp>
#include <av/graphics/2d/sprite_batch.hpp>
#include <av/graphics/2d/texture_atlas.hpp>

using namespace av;

class player_t {
    public:
    texture_atlas *atlas;
    sprite_batch *batch;

    glm::vec2 position;

    void on_move(const input_value &input) {
        position += input.read<glm::vec2>();
    }

    void update() {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        batch->projection = glm::ortho(-400.0f, 400.0f, -300.0f, 300.0f);

        batch->begin();
        batch->draw(atlas->find("player"), position.x, position.y);
        batch->end();
    }
};

int main(int argc, char *argv[]) {
    try {
        sdl_app([](sdl_app &app) {
            log::level = log_level::debug;

            texture_atlas *atlas = new texture_atlas();
            sprite_batch *batch = new sprite_batch();

            pixmap pix(32, 32);
            for(int y = 0; y < 32; y++) {
                for(int x = 0; x < 32; x++) {
                    pix.draw(x, y, color((x + 1) / 32.0f, (y + 1) / 32.0f, 1.0f, 1.0f).int_bits(), false);
                }
            }
            atlas->put("player", texture_region(*new texture_2D(32, 32, pix.buf())));

            player_t *player = new player_t();
            player->atlas = atlas;
            player->batch = batch;

            key_bind bind;
            bind.data.type = key_type::keyboard;
            bind.data.callback.connect<player_t::on_move>(player);
            bind.keyboard.set_keys<SDLK_w, SDLK_s, SDLK_a, SDLK_d>();

            app.get_input().bind("Movement", bind);
            app.on_update<player_t::update>(player);
        }, {"AVocado"});

        return 0;
    } catch(std::exception &e) {
        log::msg<log_level::error>(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Runtime error encountered!", e.what(), nullptr);

        return 1;
    }
}
