#ifndef AV_GRAPHICS_2D_PIXMAP_HPP
#define AV_GRAPHICS_2D_PIXMAP_HPP

#include "../../math.hpp"
#include "../../stb_image.h"
#include "../../stb_image_write.h"

#include <stdexcept>
#include <string>

namespace av {
    /** @brief A 2-dimensional pixel map in RGBA format. */
    class pixmap {
        /** @brief The width of the pixel map. */
        int width;
        /** @brief The height of the pixel map. */
        int height;
        /** @brief The row pointers of this pixel map, in an order of `pixels[y * width * 4 + x * 4]` for each pixel. */
        unsigned char *pixels;

        public:
        /** @brief Default copy-constructor, copies the pixels. */
        pixmap(const pixmap &from):
            width(from.width), height(from.height),
            pixels(copy(from.width, from.height, from.pixels)) {}
        /** @brief Default move-constructor, invalidates the other pixel map. */
        pixmap(pixmap &&from):
            width(std::move(from.width)), height(std::move(from.height)),
            pixels(std::move(from.pixels)) {
            from.pixels = nullptr;
        }
        /** @brief Default constructor, allocates no pixels and sets dimensions to (0, 0). */
        pixmap():
            width(0), height(0),
            pixels(static_cast<unsigned char *>(calloc(0, sizeof(unsigned char)))) {}
        /**
         * @brief Constructs a pixel map with given dimensions and row pointers.
         *
         * @param width  The pixel map width.
         * @param height The pixel map height.
         * @param pixels The row pointers, may be null. Note that the memory is copied if supplied with any.
         */
        pixmap(size_t width, size_t height, unsigned char *data = nullptr):
            width(width), height(height),
            pixels(data ? copy(width, height, data) : static_cast<unsigned char *>(calloc(width * height * 4, sizeof(unsigned char)))) {}
        /**
         * @brief Constructs a pixel map with given file.
         *
         * @param filename The file name.
         */
        pixmap(const char *filename):
            pixels([&]() -> unsigned char * {
            unsigned char *pixels = stbi_load(filename, &width, &height, nullptr, STBI_rgb_alpha);
            if(!pixels) throw std::runtime_error(std::string("Couldn't load '").append(filename).append("': ").append(stbi_failure_reason()).c_str());

            return pixels;
        }()) {}
        /** @brief Frees all resources this pixel map holds. */
        ~pixmap() {
            if(pixels) stbi_image_free(pixels);
        }

        /**
         * @brief Blends 2 colors.
         *
         * @param source The incoming color.
         * @param dest   The existing color.
         * @return The blended color.
         */
        static int blend(int source, int dest) {
            //TODO very buggy
            int s_r = source & 0x000000ff,
                s_g = (source >> 8) & 0x0000ff00,
                s_b = (source >> 16) & 0x00ff0000,
                s_a = (source >> 24) & 0xff000000;
            if(s_a == 0xff) return source;

            int
                d_r = dest & 0x000000ff,
                d_g = (dest >> 8) & 0x0000ff00,
                d_b = (dest >> 16) & 0x00ff0000,
                d_a = (dest >> 24) & 0xff000000;

            float blend_s = s_a / 255.0f, blend_d = 1.0f - blend_s;
            return
                (min(s_a + d_a, 0xff) << 24) |
                (static_cast<int>(s_b * blend_s + d_b * blend_d) << 16) |
                (static_cast<int>(s_g * blend_s + d_g * blend_d) << 8) |
                static_cast<int>(s_r * blend_s + d_r * blend_d);
        }

        /**
         * @brief (Re-)loads this pixmap from a file.
         *
         * @param filename The file name.
         */
        void load(const char *filename) {
            if(pixels) stbi_image_free(pixels);

            pixels = stbi_load(filename, &width, &height, nullptr, STBI_rgb_alpha);
            if(!pixels) throw std::runtime_error(std::string("Couldn't load '").append(filename).append("': ").append(stbi_failure_reason()).c_str());
        }

        /** @return The width of the pixel map. */
        int get_width() const {
            return width;
        }
        /** @return The height of the pixel map. */
        int get_height() const {
            return height;
        }

        /** @return The pixel data of this pixel map. Each column contains 4 bytes; red, green, blue, and alpha. */
        inline unsigned char *buf() {
            return pixels;
        }
        /** @return The (read-only) pixel data of this pixel map. Each column contains 4 bytes; red, green, blue, and alpha. */
        inline const unsigned char *buf() const {
            return pixels;
        }

        /**
         * @brief Writes this pixel map to a file in PNG format.
         *
         * @param filename The file name to be written to.
         */
        void write_to(const char *filename) const {
            bool succeed = stbi_write_png(filename, width, height, STBI_rgb_alpha, pixels, 0);
            if(!succeed) throw std::runtime_error(std::string("Couldn't write to '").append(filename).append("'.").c_str());
        }

        /**
         * @brief Draws a pixel to this pixel map.
         *
         * @param x     The pixel X position.
         * @param y     The pixel Y position.
         * @param color The pixel color bits.
         * @param blend Whether to blend the color to the existing one.
         */
        void draw(int x, int y, int color, bool blend = true) {
            int *dest = reinterpret_cast<int *>(pixels + (y * width * 4 + x * 4));
            color = blend ? pixmap::blend(color, *dest) : color;

            *dest = color;
        }
        /**
         * @brief Draws a colored rectangle to this pixel map.
         *
         * @param x      The rectangle top left X position.
         * @param y      The rectangle top left Y position.
         * @param width  The rectangle width.
         * @param height The rectangle height.
         * @param color  The rectangle color bits.
         * @param blend  Whether to blend the color to the existing one.
         */
        void draw(int x, int y, int width, int height, int color, bool blend = false) {
            int tw = this->width, th = this->height;
            width = x + min(width, tw - x);
            height = y + min(height, th - y);

            for(int ty = y; ty < height; ty++) {
                int pos = ty * tw * 4;
                for(int tx = x; tx < width; tx++) {
                    int *dest = reinterpret_cast<int *>(pixels + (pos + tx * 4));

                    color = blend ? pixmap::blend(color, *dest) : color;
                    *dest = color;
                }
            }
        }
        /**
         * @brief Draws another pixel map to this pixel map.
         *
         * @param x      The image top left X position.
         * @param y      The image top left Y position.
         * @param blend  Whether to blend the color to the existing one.
         * @param flip_x Whether to flip the image horizontally.
         * @param flip_y Whether to flip the image vertically.
         */
        void draw_image(const pixmap &image, int x, int y, bool blend = false, bool flip_x = false, bool flip_y = false) {
            int tw = this->width, th = this->height,
                width = x + min(image.width, tw - x),
                height = y + min(image.height, th - y);

            for(int ty = max(y, 0); ty < height; ty++) {
                int pos_d = ty * tw * 4;
                int pos_s = (!flip_y ? (ty - y) : (image.height - (ty - y))) * image.width * 4;
                for(int tx = max(x, 0); tx < width; tx++) {
                    int *dest = reinterpret_cast<int *>(pixels + (pos_d + tx * 4));
                    int color = *reinterpret_cast<int *>(image.pixels + (pos_s + ((!flip_x ? tx : (image.width - tx)) - x) * 4));

                    color = blend ? pixmap::blend(color, *dest) : color;
                    *dest = color;
                }
            }
        }

        /** @brief Horizontally flips this pixel map. */
        void flip_x() {
            for(int y = 0; y < height; y++) {
                int *begin = reinterpret_cast<int *>(pixels + y * width * 4);
                int *end = begin + width;

                std::reverse(begin, end);
            }
        }
        /** @brief Vertically flips this pixel map. */
        void flip_y() {
            for(int x = 0; x < width; x++) {
                int *begin = reinterpret_cast<int *>(pixels + x * 4);
                int *end = reinterpret_cast<int *>(pixels + height * width * 4 + x * 4);

                while(begin != end && begin != (end -= width)) {
                    std::iter_swap(begin, end);
                    begin += width;
                }
            }
        }

        /**
         * @brief Copies a row pointer memory.
         *
         * @param width  The pixel map width.
         * @param height The pixel map height.
         * @param source The row pointers to be copied.
         */
        static unsigned char *copy(size_t width, size_t height, unsigned char *source) {
            unsigned char *result = static_cast<unsigned char *>(calloc(width * height * 4, sizeof(unsigned char)));
            memcpy(result, source, height * width * 4);

            return result;
        }
    };
}

#endif // !AV_GRAPHICS_2D_PIXMAP_HPP
