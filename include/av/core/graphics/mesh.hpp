#ifndef AV_CORE_GRAPHICS_MESH_HPP
#define AV_CORE_GRAPHICS_MESH_HPP

#include <glad/glad.h>
#include <av/core/graphics/shader.hpp>

#include <string>
#include <vector>

namespace av {
    /** @brief A vertex attribute. That is, the stride or region that take place in a vertex buffer object. */
    struct vert_attribute {
        /** @brief 2 `float` components; X and Y. */
        static const vert_attribute pos_2D;
        /** @brief 4 `float` components; alpha, blue, green, and red. */
        static const vert_attribute color;
        /** @brief 4 `unsigned char` components; alpha, blue, green, and red. Can be packed into a single float value. */
        static const vert_attribute color_packed;

        /** @brief How many components this attribute has. Affects `size`. */
        int components;
        /**
         * @brief The type of this vertex attribute. Must be one of `GL_BYTE`, `GL_UNSIGNED_BYTE`, `GL_SHORT`,
         * `GL_UNSIGNED_SHORT`, `GL_INT`, `GL_UNSIGNED_INT`, or `GL_FLOAT`.
         */
        int type;
        /** @brief Total data that this attribute take per vertex in the vertex buffer. */
        int size;
        /** @brief Whether the value is normalized. */
        bool normalized;
        /** @brief The name of this vertex attribute, to be used in shaders. */
        std::string name;

        /** @brief Default constructor, does nothing. Values should be set later. */
        vert_attribute() = default;
        /**
         * @brief Constructs a new vertex attribute with given constructor arguments. The `size` of this attribute will
         * be automatically accounted for.
         */
        vert_attribute(int components, int type, const std::string &name, bool normalized = false):
            components(components),
            type(type),
            size(count_size()),
            normalized(normalized),
            name(name) {}
        /**
         * @brief Constructs a new vertex attribute with given constructor arguments. It is recommended not to use this
         * constructor directly, but to use `create()` instead.
         */
        vert_attribute(int components, int type, int size, const std::string &name, bool normalized = false):
            components(components),
            type(type),
            size(size),
            normalized(normalized),
            name(name) {}

        /**
         * Counts what should be used in `size`.
         * @return The memory chunk size of this attribute per vertex.
         */
        int count_size() const;

        /**
         * @return A new vertex attribute with given template arguments. The `size` of this attribute will be automatically
         * accounted for at compile-time.
         */
        template<int T_components, int T_type, bool T_normalized = false>
        static vert_attribute create(const std::string &name) {
            static constexpr int type_size = T_components * (
                T_type == GL_BYTE || T_type == GL_UNSIGNED_BYTE ? sizeof(char) :
                T_type == GL_SHORT || T_type == GL_UNSIGNED_SHORT ? sizeof(short) :
                T_type == GL_INT || T_type == GL_UNSIGNED_INT ? sizeof(int) :
                T_type == GL_FLOAT ? sizeof(float) : -1);

            static_assert(type_size != -1, "Invalid vertex attribute type.");
            return vert_attribute(T_components, T_type, type_size, name, T_normalized);
        }
    };

    class mesh {
        size_t vertex_size;
        std::vector<vert_attribute> attributes;

        size_t max_vertices;
        size_t max_indices;
        bool has_indices;

        unsigned int vertex_buffer;
        unsigned int index_buffer;

        public:
        mesh(const mesh &) = delete;
        mesh(std::initializer_list<vert_attribute> attributes);
        ~mesh();

        inline size_t get_vertex_size() const {
            return vertex_size;
        }
        inline size_t get_max_vertices() const {
            return max_vertices;
        }
        inline size_t get_max_indices() const {
            return max_indices;
        }

        template<int T_usage = GL_STATIC_DRAW>
        inline void set_vertices(float *vertices, size_t offset, size_t length) {
            static_assert(T_usage == GL_STATIC_DRAW || T_usage == GL_DYNAMIC_DRAW || T_usage == GL_STREAM_DRAW, "Invalid vertex data usage.");

            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, length, vertices + offset, T_usage);

            max_vertices = length / sizeof(float);
        }

        template<int T_usage = GL_STATIC_DRAW>
        inline void set_indices(unsigned short *indices, size_t offset, size_t length) {
            static_assert(T_usage == GL_STATIC_DRAW || T_usage == GL_DYNAMIC_DRAW || T_usage == GL_STREAM_DRAW, "Invalid index data usage.");

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, indices + offset, T_usage);

            max_indices = length / sizeof(unsigned short);
            has_indices = max_indices > 0;
        }

        void render(const shader &program, int primitive_type, size_t offset, size_t count, bool auto_bind = true) const;
        void bind(const shader &program) const;
        void unbind(const shader &program) const;
    };
}

#endif // !AV_CORE_GRAPHICS_MESH_HPP
