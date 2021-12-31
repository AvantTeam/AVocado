#ifndef AV_GRAPHICS_2D_SPRITEBATCH_HPP
#define AV_GRAPHICS_2D_SPRITEBATCH_HPP

#include "texture_atlas.hpp"
#include "../color.hpp"
#include "../mesh.hpp"
#include "../../math.hpp"

#include <stdexcept>

namespace av {
    #define SPRITE_BATCH_ATTRIBUTES {vert_attribute::pos_2D, vert_attribute::color_packed, vert_attribute::tex_coords}

    /**
     * @brief General implementation of a sprite batch. A sprite batch effectively collects and buffers sprite vertices
     * and renders them later, to avoid OpenGL render calls to an extent.
     * 
     * This implementation uses the `pos_2D`, `color_packed`, and `tex_coords` vertex attributes. Default and custom
     * shaders must comply with this requirement.
     */
    class sprite_batch {
        /** @brief The max vertices this sprite batch can buffer. */
        int max_vertices;
        /** @brief Each vertices' size, in bytes. */
        int sprite_size;
        /** @brief The current buffer offset. */
        int index;
        /** @brief The vertices buffer. */
        float *vertices;

        /** @brief The mesh, supplied with `pos_2D`, `color_packed`, and `tex_coords` vertex attributes. */
        mesh batch;
        /** @brief The sprite batch's default shader. */
        shader batch_shader;
        /** @brief The sprite batch's current custom shader, or null if currently using the default one. */
        shader *custom_shader;

        /** @brief Whether this sprite batch is buffering sprites. */
        bool batching;
        /** @brief The current bound texture, typically an atlas page. */
        const texture_2D *texture;

        public:
        /** @brief The following sprite's color multiplier. */
        color col;
        /** @brief The sprite batch's projection matrix, should be multiplied with the camera transform matrix. */
        glm::mat4 projection;

        /** @brief Default copy-constructor, copies the vertices and state. */
        sprite_batch(const sprite_batch &from):
            max_vertices(from.max_vertices),
            sprite_size(from.sprite_size),
            index(from.index),
            vertices([&]() -> float * {
            int len = max_vertices * sprite_size;
            float *vertices = new float[len];

            memcpy(vertices, from.vertices, len * sizeof(float));
            return vertices;
        }()),

            batch(from.batch),
            batch_shader(from.batch_shader),
            custom_shader(from.custom_shader),

            batching(from.batching),
            texture(from.texture),
            projection(from.projection) {
            set_elements();
        }
        /** @brief Default move-constructor, invalidates the other sprite batch. */
        sprite_batch(sprite_batch &&from):
            max_vertices(std::move(from.max_vertices)),
            sprite_size(std::move(from.sprite_size)),
            index(std::move(from.index)),
            vertices(std::move(from.vertices)),

            batch(std::move(from.batch)),
            batch_shader(std::move(from.batch_shader)),
            custom_shader(std::move(from.custom_shader)),

            batching(std::move(from.batching)),
            texture(std::move(from.texture)),
            projection(std::move(from.projection)) {
            from.vertices = nullptr;
        }
        /**
         * @brief Constructs a sprite batch with given max vertices and a shader.
         * 
         * @param max_vertices The max vertices, at most 16384.
         * @param batch_shader The user-defined shader, leave untouched to use the default shader.
         */
        sprite_batch(int max_vertices = 4096, shader &&batch_shader = std::move(default_shader())):
            max_vertices(max_vertices),
            sprite_size([&]() -> int {
            if(max_vertices % 4 != 0) throw std::runtime_error("Max vertices must be a multiple of 4.");
            if(max_vertices > 16384) throw std::runtime_error("Max vertices can't be more than 16384 (16384 * 4 == 65536).");

            int size = 0;
            for(const vert_attribute &attribute : SPRITE_BATCH_ATTRIBUTES) size += attribute.size;
            return size / sizeof(float);
        }()),
            index(0),
            vertices(new float[max_vertices * sprite_size]),

            batch(SPRITE_BATCH_ATTRIBUTES),
            batch_shader(batch_shader),
            custom_shader(nullptr),

            batching(false),
            texture(nullptr),
            col(1.0f, 1.0f, 1.0f, 1.0f),
            projection(glm::identity<glm::mat4>()) {
            set_elements();
        }
        /** @brief Default destructor, destroys all the resources this sprite batch holds. */
        ~sprite_batch() {
            if(vertices) delete[] vertices;
        }

        /** @brief Begins the sprite batch buffering. Switches to default shader and turns off depth-masking. */
        void begin() {
            if(batching) throw std::runtime_error("Don't `begin()` twice.");
            batching = true;

            glDepthMask(false);
            switch_shader();
        }
        /** @brief Ends the sprite batch buffering. Flushes the batch and turns depth-masking back on. */
        void end() {
            if(!batching) throw std::runtime_error("Don't `end()` twice.");
            batching = false;

            flush();
            glDepthMask(true);
        }
        /** @brief Renders the mesh with the buffered vertices, limited by `index`. */
        void flush() {
            if(!index || !vertices || !texture) return;

            shader &program = get_current_shader();
            glUniformMatrix4fv(program.uniform_loc("u_projection"), 1, false, glm::value_ptr(projection));
            glUniform1i(program.uniform_loc("u_texture"), texture->active(0));

            batch.set_vertices(vertices, 0, index);
            batch.render(program, GL_TRIANGLES, 0, static_cast<size_t>(index / sprite_size / 4 * 6));
            index = 0;
        }

        /**
         * @brief Switches this sprite batch's shader and flushes it.
         * 
         * @param other The shader, leave untouched to switch to the default shader.
         */
        inline void switch_shader(shader *other = nullptr) {
            if(other) {
                if(custom_shader != other) {
                    flush();
                    custom_shader = other;
                    custom_shader->bind();
                }
            } else {
                flush();
                custom_shader = nullptr;
                batch_shader.bind();
            }
        }

        /** @return The default shader. */
        inline shader &get_shader() {
            return batch_shader;
        }
        /** @return The (read-only) default shader. */
        inline const shader &get_shader() const {
            return batch_shader;
        }
        /** @return The currently used shader. */
        inline shader &get_current_shader() {
            return custom_shader ? *custom_shader : batch_shader;
        }
        /** @return The (read-only) currently used shader. */
        inline const shader &get_current_shader() const {
            return custom_shader ? *custom_shader : batch_shader;
        }

        /**
         * @brief Draws a texture region with the specified transforms.
         * 
         * @param region   The sprite region.
         * @param center_x The sprite's center X position.
         * @param center_y The sprite's center Y position.
         * @param rotation The sprite's rotation, in radians.
         */
        inline void draw(const texture_region &region, float center_x, float center_y, float rotation = 0.0f) {
            draw(region, center_x, center_y, center_x - region.width / 2.0f, center_y - region.height / 2.0f, region.width, region.height, rotation);
        }
        /**
         * @brief Draws a texture region with the specified transforms.
         * 
         * @param region   The sprite region.
         * @param center_x The sprite's center X position.
         * @param center_y The sprite's center Y position.
         * @param width    The sprite's width.
         * @param height   The sprite's height.
         * @param rotation The sprite's rotation, in radians.
         */
        inline void draw(const texture_region &region, float center_x, float center_y, float width, float height, float rotation = 0.0f) {
            draw(region, center_x, center_y, center_x - width / 2.0f, center_y - height / 2.0f, width, height, rotation);
        }
        /**
         * @brief Draws a texture region with the specified transforms.
         * 
         * @param region   The sprite region.
         * @param center_x The sprite's center X position, used for rotation pivot.
         * @param center_y The sprite's center Y position, used for rotation pivot.
         * @param origin_x The sprite's bottom-left X position.
         * @param origin_y The sprite's bottom-left Y position.
         * @param width    The sprite's width.
         * @param height   The sprite's height.
         * @param rotation The sprite's rotation, in radians.
         */
        void draw(const texture_region &region,
            float center_x, float center_y,
            float origin_x, float origin_y,
            float width, float height,
            float rotation = 0.0f
        ) {
            switch_texture(region.texture);

            float
                color = col.float_bits(),
                u = region.u, v = region.v,
                u2 = region.u2, v2 = region.v2;

            int len = sprite_size * 4;
            float vertices[len];
            if(within(rotation, 0.0f)) {
                float xw = origin_x + width, yh = origin_y + height;

                vertices[0] = origin_x;
                vertices[1] = origin_y;
                vertices[2] = color;
                vertices[3] = u;
                vertices[4] = v;

                vertices[5] = xw;
                vertices[6] = origin_y;
                vertices[7] = color;
                vertices[8] = u2;
                vertices[9] = v;

                vertices[10] = xw;
                vertices[11] = yh;
                vertices[12] = color;
                vertices[13] = u2;
                vertices[14] = v2;

                vertices[15] = origin_x;
                vertices[16] = yh;
                vertices[17] = color;
                vertices[18] = u;
                vertices[19] = v2;
            } else {
                float
                    cos = glm::cos(rotation), sin = glm::sin(rotation),

                    rel_x = origin_x - center_x, rel_y = origin_y - center_y,
                    rel_xw = rel_x + width, rel_yh = rel_y + height,

                    x1 = (cos * rel_x - sin * rel_y) + center_x, y1 = (sin * rel_x + cos * rel_y) + center_y,
                    x2 = (cos * rel_xw - sin * rel_y) + center_x, y2 = (sin * rel_xw + cos * rel_y) + center_y,
                    x3 = (cos * rel_xw - sin * rel_yh) + center_x, y3 = (sin * rel_xw + cos * rel_yh) + center_y,
                    x4 = (cos * rel_x - sin * rel_yh) + center_x, y4 = (sin * rel_x + cos * rel_yh) + center_y;

                vertices[0] = x1;
                vertices[1] = y1;
                vertices[2] = color;
                vertices[3] = region.u;
                vertices[4] = region.v;

                vertices[5] = x2;
                vertices[6] = y2;
                vertices[7] = color;
                vertices[8] = region.u2;
                vertices[9] = region.v;

                vertices[10] = x3;
                vertices[11] = y3;
                vertices[12] = color;
                vertices[13] = region.u2;
                vertices[14] = region.v2;

                vertices[15] = x4;
                vertices[16] = y4;
                vertices[17] = color;
                vertices[18] = region.u;
                vertices[19] = region.v2;
            }

            draw(region.texture, vertices, 0, len);
        }
        /**
         * @brief Draws a texture with the specified vertices.
         * 
         * @param texture  The texture.
         * @param vertices The sprite vertices.
         * @param offset   The vertices array offset.
         * @param length   The vertices array length.
         */
        void draw(const texture_2D *texture, float *vertices, size_t offset, size_t length) {
            switch_texture(texture);

            int array_len = max_vertices * sprite_size;
            while(length) {
                int copy_len = min<int>(array_len - index, length);
                if(!copy_len) {
                    flush();
                    continue;
                }

                memcpy(this->vertices + index, vertices + offset, copy_len * sizeof(float));
                index += copy_len;
                offset += copy_len;
                length -= copy_len;
            }
        }

        private:
        /**
         * @brief Switches this sprite batch's bound texture and flushes it.
         *
         * @param other The texture.
         */
        inline void switch_texture(const texture_2D *other) {
            if(!other) throw std::runtime_error("Texture-switch target can't be null.");
            if(!texture || texture != other) {
                flush();
                texture = other;
            }
        }

        /** @brief Sets up the mesh's element buffer. */
        void set_elements() {
            int max_elements = max_vertices * 6;

            unsigned short elements[max_elements];
            for(int i = 0, j = 0; i < max_elements; i += 6, j += 4) {
                elements[i] = j;
                elements[i + 1] = j + 1;
                elements[i + 2] = j + 2;
                elements[i + 3] = j + 2;
                elements[i + 4] = j + 3;
                elements[i + 5] = j;
            }

            batch.set_elements(elements, 0, max_elements);
        }

        /** @return The sprite batch's default shader. */
        static shader default_shader() {
            return shader(R"(
#version 150 core
in vec2 a_position;
in vec4 a_color;
in vec2 a_tex_coords_0;

out vec4 v_color;
out vec2 v_tex_coords;

uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
    v_color = a_color;
    v_tex_coords = a_tex_coords_0;
})", R"(
#version 150 core
out vec4 out_color;

in vec4 v_color;
in vec2 v_tex_coords;

uniform sampler2D u_texture;

void main() {
    out_color = texture2D(u_texture, v_tex_coords) * v_color;
})");
        }
    };

    #undef SPRITE_BATCH_ATTRIBUTES
}

#endif // !AV_GRAPHICS_2D_SPRITEBATCH_HPP
