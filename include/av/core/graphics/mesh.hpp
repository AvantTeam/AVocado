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
        /** @brief 4 `unsigned char` components; alpha, blue, green, and red. Can be packed into a single float value. */
        static const vert_attribute color;

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
            normalized(normalized),
            name(std::move(name)),
            size(count_size()) {}
        /**
         * @brief Constructs a new vertex attribute with given constructor arguments. It is recommended not to use this
         * constructor directly, but to use `create()` instead.
         */
        vert_attribute(int components, int type, int size, const std::string &name, bool normalized = false):
            components(components),
            type(type),
            normalized(normalized),
            name(std::move(name)),
            size(size) {}

        /**
         * Counts what should be used in `size`.
         * @return The memory chunk size of this attribute per vertex.
         */
        int count_size() const;

        /**
         * @return A new vertex attribute with given template arguments. The `size` of this attribute will be automatically
         * accounted for at compile-time.
         */
        template<int T_components, int T_type>
        static vert_attribute create(const std::string &name, bool normalized = false) {
            static constexpr int type_size = T_components * (
                T_type == GL_BYTE || T_type == GL_UNSIGNED_BYTE ? sizeof(char) :
                T_type == GL_SHORT || T_type == GL_UNSIGNED_SHORT ? sizeof(short) :
                T_type == GL_INT || T_type == GL_UNSIGNED_INT ? sizeof(int) :
                T_type == GL_FLOAT ? sizeof(float) : -1
            );

            static_assert(type_size != -1, "Invalid vertex attribute type.");
            return vert_attribute(T_components, T_type, type_size, name, normalized);
        }
    };

    class mesh {
        size_t max_vertices;
        size_t max_indices;
        size_t vertex_size;
        std::vector<vert_attribute> attributes;

        unsigned int vertex_buffer;
        unsigned int index_buffer;

        public:
        mesh(const mesh &) = delete;
        mesh(size_t max_vertices, size_t max_indices, std::initializer_list<vert_attribute> attributes);
        ~mesh();

        inline size_t get_max_vertices() const {
            return max_vertices;
        }
        inline size_t get_max_indices() const {
            return max_indices;
        }
        inline size_t get_vertex_size() const {
            return vertex_size;
        }
    };
}

#endif // !AV_CORE_GRAPHICS_MESH_HPP
