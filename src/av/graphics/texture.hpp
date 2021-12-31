#ifndef AV_GRAPHICS_TEXTURE_HPP
#define AV_GRAPHICS_TEXTURE_HPP

#include "../glad.h"

namespace av {
    /**
     * @brief Wraps an OpenGL texture object.
     * @tparam T_type The texture type, either `GL_TEXTURE_1D`, `GL_TEXTURE_2D`, or `GL_TEXTURE_3D`.
     */
    template<int T_type>
    class texture {
        static_assert(T_type == GL_TEXTURE_1D || T_type == GL_TEXTURE_2D || T_type == GL_TEXTURE_3D, "Invalid texture type.");

        protected:
        /** @brief The handle to the generated OpenGL texture object. */
        unsigned int handle;

        public:
        /** @brief Default copy-constructor, copies the other texture's pixels. */
        texture(const texture<T_type> &from): handle([&]() -> unsigned int {
            unsigned int handle;
            glGenTextures(1, &handle);

            from.bind();

            unsigned char pixels[from.buffer_size()];
            glGetTexImage(T_type, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(pixels));

            load(from.get_width(), from.get_height(), from.get_depth(), pixels);
            return handle;
        }()) {}
        /** @brief Default move-constructor, invalidates the other texture. */
        texture(texture<T_type> &&from): handle(std::move(from.handle)) {
            from.handle = 0;
        }
        /** @brief Generates an OpenGL texture object. */
        texture(): handle([]() -> unsigned int {
            unsigned int handle;
            glGenTextures(1, &handle);

            return handle;
        }()) {}
        /** @brief Deletes the OpenGL texture object this instance holds. */
        ~texture() {
            glDeleteTextures(1, &handle);
        }

        /** @brief Binds this texture for further usage. */
        inline void bind() const {
            glBindTexture(T_type, handle);
        }
        /**
         * @brief Activates this texture for usage in shader texture sampler uniforms.
         * @param unit The texture unit, ranging from [0..32].
         * @return The texture unit itself, for convenience.
         */
        inline int active(int unit) const {
            glActiveTexture(GL_TEXTURE0 + unit);
            bind();

            return unit;
        }

        /** @return Pixel buffer size for allocating memory that fits this texture's pixels. */
        virtual int buffer_size() const = 0;
        /** @return The texture width, implemented in derived classes. */
        virtual int get_width() const = 0;
        /** @return The texture height, implemented in derived classes. */
        virtual int get_height() const = 0;
        /** @return The texture depth, implemented in derived classes. */
        virtual int get_depth() const = 0;

        /**
         * @brief Loads the texture with given dimensions and pixels, implemented in derived classes.
         * @param width       The texture width.
         * @param height      The texture height.
         * @param depth       The texture depth.
         * @param data        The texture pixels in RGBA format.
         * @param should_bind Whether a call to `bind()` should be invoked. Defaults to `true`.
         */
        virtual void load(int width, int height, int depth, const unsigned char *data, bool should_bind = true) = 0;
    };

    /** @brief Specification for 2D textures. */
    class texture_2D: public texture<GL_TEXTURE_2D> {
        /** @brief The texture width. */
        int width;
        /** @brief The texture height. */
        int height;

        public:
        /** @brief Default constructor, must be loaded later on. */
        texture_2D(): width(0), height(0) {}
        /**
         * @brief Loads the texture with given dimensions and pixels.
         * @param width  The texture width.
         * @param height The texture height.
         * @param data   The texture pixels in RGBA format.
         */
        texture_2D(int width, int height, const unsigned char *data) {
            load(width, height, data);
        }

        /**
         * @brief Loads the texture with given dimensions and pixels.
         * @param width       The texture width.
         * @param height      The texture height.
         * @param data        The texture pixels in RGBA format.
         * @param should_bind Whether a call to `bind()` should be invoked. Defaults to `true`.
         */
        inline void load(int width, int height, const unsigned char *data, bool should_bind = true) {
            if(should_bind) bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            this->width = width;
            this->height = height;
        }

        inline int buffer_size() const override { return width * height * 4; }
        inline int get_width() const override { return width; }
        inline int get_height() const override { return height; }
        inline int get_depth() const override { return 0; }

        inline void load(int width, int height, [[maybe_unused]] int depth, const unsigned char *data, bool should_bind = true) override {
            load(width, height, data, should_bind);
        }
    };
}

#endif // !AV_GRAPHICS_TEXTURE_HPP
