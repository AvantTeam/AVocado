#ifndef AV_GRAPHICS_2D_TEXTUREATLAS_HPP
#define AV_GRAPHICS_2D_TEXTUREATLAS_HPP

#include "pixmap.hpp"
#include "../texture.hpp"
#include "../../io.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace av {
    /** @brief A region that takes place in a 2D texture. */
    struct texture_region {
        /** @brief The texture this region takes place in. May be null. */
        const texture_2D *texture;

        /** @brief The X offset of this region, originating from top-left of the texture. */
        int x;
        /** @brief The Y offset of this region, originating from top-left of the texture. */
        int y;
        /** @brief The width of this region. */
        int width;
        /** @brief The height of this region. */
        int height;

        /** @brief The U coordinate of this region; practically the X position scaled with the texture width. */
        float u;
        /** @brief The V coordinate of this region; practically the Y position scaled with the texture height. */
        float v;
        /** @brief The end U coordinate of this region; practically the region width relative to X scaled with the texture width. */
        float u2;
        /** @brief The end V coordinate of this region; practically the region height relative to Y scaled with the texture height. */
        float v2;

        public:
        /** @brief Default constructor, sets the UV mapping to [(0.0, 0.0), (1.0, 1.0)]. */
        texture_region():
            texture(nullptr),
            x(0), y(0), width(0), height(0),
            u(0.0f), v(0.0f), u2(1.0f), v2(1.0f) {}
        /** @brief Default copy constructor. Doesn't copy the texture, only the reference. */
        texture_region(const texture_region &from):
            texture(from.texture),
            x(from.x), y(from.y), width(from.width), height(from.height),
            u(from.u), v(from.v), u2(from.u2), v2(from.v2) {}
        /** @brief Default move constructor. */
        texture_region(texture_region &&from):
            texture(std::move(from.texture)),
            x(std::move(from.x)), y(std::move(from.y)), width(std::move(from.width)), height(std::move(from.height)),
            u(std::move(from.u)), v(std::move(from.v)), u2(std::move(from.u2)), v2(std::move(from.v2)) {}

        /**
         * @brief Constructs a region from a texture.
         *
         * @param texture The texture. UV mapping will be set to [(0.0, 0.0), (1.0, 1.0)].
         */
        texture_region(const texture_2D &texture):
            texture(&texture),
            x(0), y(0), width(texture.get_width()), height(texture.get_height()),
            u(0.0f), v(0.0f), u2(1.0f), v2(1.0f) {}
        /**
         * @brief Constructs a region from given texture, dimension, and offset. UV mapping will be further calculated.
         *
         * @param texture The texture.
         * @param x       The X offset of this region.
         * @param y       The Y offset of this region.
         * @param width   The region width.
         * @param height  The region height.
         */
        texture_region(const texture_2D &texture, int x, int y, int width, int height):
            texture(&texture),
            x(x), y(y), width(width), height(height),
            u(static_cast<float>(x) / texture.get_width()), v(static_cast<float>(y) / texture.get_height()),
            u2(static_cast<float>(x + width) / texture.get_width()), v2(static_cast<float>(y + height) / texture.get_height()) {}

        /**
         * @brief Sets the region to the given texture, dimension, and offset. UV mapping will be further calculated.
         *
         * @param texture The texture.
         * @param x       The X offset of this region.
         * @param y       The Y offset of this region.
         * @param width   The region width.
         * @param height  The region height.
         */
        void set(const texture_2D &texture, int x, int y, int width, int height) {
            this->texture = &texture;
            this->x = x;
            this->y = y;
            this->width = width;
            this->height = height;
            count_coords();
        }
        /** @brief Calculates this region's UV mapping. */
        void count_coords() {
            if(!texture) return;

            const texture_2D &tex = *texture;
            u = static_cast<float>(x) / tex.get_width();
            v = static_cast<float>(y) / tex.get_height();
            u2 = static_cast<float>(x + width) / tex.get_width();
            v2 = static_cast<float>(y + height) / tex.get_height();
        }
    };

    /** @brief Holds one or more texture pages along with their own texture region mappings. */
    class texture_atlas {
        /** @brief All the pages that this atlas contains. */
        std::vector<texture_2D> textures;
        /** @brief All the regions this atlas contains, mapped with their names. */
        std::unordered_map<std::string, texture_region> regions;

        public:
        /** @brief Returned in `find(const string &)` if the region with the specified name isn't found. */
        texture_region not_found;

        /** @brief Default constructor. */
        texture_atlas() = default;
        /** @brief Default copy-constructor, copies the textures. */
        texture_atlas(const texture_atlas &from):
            textures(from.textures),
            regions(from.regions),
            not_found(from.not_found) {
            const std::vector<texture_2D> &from_tex = from.textures;
            for(auto &[name, region] : regions) {
                const texture_2D *tex = region.texture;

                for(int i = 0; i < from_tex.size(); i++) {
                    if(tex == &from_tex[i]) {
                        region.texture = &textures[i];
                        break;
                    }
                }
            }
        }
        /** @brief Default move-constructor. */
        texture_atlas(texture_atlas &&from):
            textures(std::move(from.textures)),
            regions(std::move(from.regions)),
            not_found(std::move(from.not_found)) {
            const std::vector<texture_2D> &from_tex = from.textures;
            for(auto &[name, region] : regions) {
                const texture_2D *tex = region.texture;

                for(int i = 0; i < from_tex.size(); i++) {
                    if(tex == &from_tex[i]) {
                        region.texture = &textures[i];
                        break;
                    }
                }
            }
        }
        /**
         * @brief Constructs a texture atlas with specified binary input stream.
         *
         * @param read The input stream wrapper. The stream's contents must match the format of texture atlas datas.
         */
        texture_atlas(const reads &read) {
            load(read);
        }

        /**
         * @brief (Re-)loads this texture atlas from a binary input stream.
         *
         * @param read The input stream.
         */
        void load(const reads &read) {
            regions.clear();
            textures.clear();

            unsigned char version = read.read<unsigned char>(); // Read version.
            if(version != 1) throw std::runtime_error(std::string("Unsupported texture atlas version: ").append(std::to_string(version)).c_str());

            unsigned char page_size = read.read<unsigned char>(); // Read page amount, up to 256.
            for(char i = 0; i < page_size; i++) {
                std::string page_name(std::move(read.read<std::string>())); // Read page texture name.

                pixmap pix(page_name.c_str());
                texture_2D &page = textures.emplace_back(pix.get_width(), pix.get_height(), pix.buf());

                unsigned short region_size = read.read<unsigned short>(); // Read regions amount, up to 65536.
                for(short j = 0; j < region_size; j++) {
                    std::string name(std::move(read.read<std::string>())); // Read region name.
                    unsigned short
                        x = read.read<unsigned short>(), // Read region X position, up to 65536.
                        y = read.read<unsigned short>(), // Read region Y position, up to 65536.
                        w = read.read<unsigned short>(), // Read region width, up to 65536.
                        h = read.read<unsigned short>(); // Read region height, up to 65536.

                    regions.emplace(name, texture_region(page, x, y, w, h));
                }
            }
        }

        /**
         * @brief Inserts a texture region by its name.
         * @param name   The region name.
         * @param region The texture region. This will replace existing regions.
         */
        inline texture_region &put(const std::string &name, const texture_region &region) {
            regions.erase(name);
            return regions.emplace(name, region).first->second;
        }

        /**
         * @param name The region name.
         *
         * @return The region with the specified name, or `not_found` if there is none.
         */
        inline texture_region &find(const std::string &name) {
            auto it = regions.find(name);
            if(it == regions.end()) return not_found;

            return it->second;
        }
        /**
         * @param name The region name.
         *
         * @return The region with the specified name, or `not_found` if there is none.
         */
        inline const texture_region &find(const std::string &name) const {
            auto it = regions.find(name);
            if(it == regions.end()) return not_found;

            return it->second;
        }
    };
}

#endif // !AV_GRAPHICS_2D_TEXTUREATLAS_HPP
