#include "../../../glad/glad.h"

#include <vector>

namespace av {
    /** @brief A vertex attribute. That is, the stride or region that take place in a vertex buffer object. */
    struct vert_attribute {
        static const vert_attribute pos_2D;

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
        const char *name;

        /** @brief Default constructor, does nothing. Values should be set later. */
        vert_attribute() = default;
        /**
         * @brief Constructs a new vertex attribute with given template arguments. The `size` of this attribute will be
         * automatically accounted for.
         */
        template<int T_components, int T_type, bool T_normalized, const char *T_name>
        vert_attribute():
            components(T_components),
            type(T_type),
            normalized(T_normalized),
            name(T_name)
        {
            static constexpr int type_size = T_components * (
                T_type == GL_BYTE || T_type == GL_UNSIGNED_BYTE ? sizeof(char) :
                T_type == GL_SHORT || T_type == GL_UNSIGNED_SHORT ? sizeof(short) :
                T_type == GL_INT || T_type == GL_UNSIGNED_INT ? sizeof(int) :
                T_type == GL_FLOAT ? sizeof(float) : -1
            );

            static_assert(type_size != -1, "Invalid vertex attribute type.");
            size = type_size;
        }
        /**
         * @brief Constructs a new vertex attribute with given constructor arguments. The `size` of this attribute will
         * be automatically accounted for.
         */
        vert_attribute(int components, int type, bool normalized, const char *name):
            components(components),
            type(type),
            normalized(normalized),
            name(name),
            size(count_size())
        {}

        /**
         * Counts what should be used in `size`.
         * @return The memory chunk size of this attribute per vertex.
         */
        int count_size() const;
    };

    class mesh {
        size_t max_vertices;
        size_t max_indices;
        std::vector<vert_attribute> attributes;

        public:
        mesh(const mesh &) = delete;
        mesh(size_t max_vertices, size_t max_indices, std::initializer_list<vert_attribute> attributes);
        ~mesh();
    };
}
