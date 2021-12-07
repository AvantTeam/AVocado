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

    /**
     * @brief A mesh is a non copy-constructible class holding a state of a vertex buffer object and an element buffer
     * object in order to draw objects on an OpenGL surface.
     * 
     * Optionally holding a non-empty element buffer, a mesh can be rendered by invoking
     * `render(const shader &, int, size_t, size_t, bool)`, which, of course, requires the mesh's vertices to be set
     * first. The element buffer can be used to reduce the amount of memory required for the vertices, preventing the
     * same vertices to be defined twice.
     */
    class mesh {
        /** @brief How many bytes each vertex take. Determined by given vertex attributes. */
        size_t vertex_size;
        /** @brief Lists an attribute each vertex has. */
        std::vector<vert_attribute> attributes;

        /** @brief How many vertices this mesh currently holds. */
        size_t max_vertices;
        /** @brief How many elements this mesh currently holds. */
        size_t max_elements;
        /** @brief Whether the element buffer is not empty. */
        bool has_elements;

        /** @brief The handle to the generated OpenGL vertex buffer object. */
        unsigned int vertex_buffer;
        /** @brief The handle to the generated OpenGL element buffer object. */
        unsigned int element_buffer;

        public:
        mesh(const mesh &) = delete;
        /**
         * @brief Constructs an empty mesh with given vertex attributes. These attributes are identifiers to each
         * vertices' data, e.g. position and color. Calls to `set_vertices(float *, size_t, size_t)` and (optionally)
         * `set_elements(unsigned short *, size_t, size_t)` must be invoked in order to initialize the mesh data to be
         * rendered.
         * 
         * @param attributes The vertex attributes.
         */
        mesh(std::initializer_list<vert_attribute> attributes);
        /** Destroys this mesh, freeing the OpenGL resources it holds. */
        ~mesh();

        /** @return How many bytes each vertex take. */
        inline size_t get_vertex_size() const {
            return vertex_size;
        }
        /** @return How many vertices this mesh currently holds. */
        inline size_t get_max_vertices() const {
            return max_vertices;
        }
        /** @return How many elements this mesh currently holds. */
        inline size_t get_max_elements() const {
            return max_elements;
        }

        /**
         * @brief Sets the vertices of this mesh.
         * 
         * @param vertices The vertices array to be used. Each vertex must be in the same signature as this mesh's
         *                 vertex attributes.
         * @param offset   Specifies the offset of the vertices pointer to be uploaded to the buffer.
         * @param length   Specifies the amount of the vertices to be uploaded to the buffer.
         * @tparam T_usage Buffer data usage, must be either `GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`, or `GL_STREAM_DRAW`.
         */
        template<int T_usage = GL_STATIC_DRAW>
        inline void set_vertices(float *vertices, size_t offset, size_t length) {
            static_assert(T_usage == GL_STATIC_DRAW || T_usage == GL_DYNAMIC_DRAW || T_usage == GL_STREAM_DRAW, "Invalid vertex data usage.");

            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, length, vertices + offset, T_usage);

            max_vertices = length / vertex_size;
        }

        /**
         * @brief Sets the elements of this mesh.
         * 
         * @param elements The elements array to be used.
         * @param offset   Specifies the offset of the elements pointer to be uploaded to the buffer.
         * @param length   Specifies the amount of the elements to be uploaded to the buffer.
         * @tparam T_usage Buffer data usage, must be either `GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`, or `GL_STREAM_DRAW`.
         */
        template<int T_usage = GL_STATIC_DRAW>
        inline void set_elements(unsigned short *elements, size_t offset, size_t length) {
            static_assert(T_usage == GL_STATIC_DRAW || T_usage == GL_DYNAMIC_DRAW || T_usage == GL_STREAM_DRAW, "Invalid index data usage.");

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, elements + offset, T_usage);

            max_elements = length / sizeof(unsigned short);
            has_elements = max_elements > 0;
        }
        
        /**
         * @brief Renders this mesh to the default or the currently bound frame buffer.
         * 
         * @param program        The shader program. The attributes supported by this shader must fulfill this mesh's
                                 own vertex attributes, otherwise an exception is thrown.
         * @param primitive_type OpenGL rendered object primitive types. Must be one of `GL_POINTS`, `GL_LINES`,
         *                       `GL_LINE_LOOP`, `GL_LINE_STRIP`, `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, or `GL_TRIANGLE_FAN`.
         * @param offset         Specifies the offset of vertex (or element, if any) buffer to be rendered.
         * @param count          Specifies the length of vertex (or element, if any) buffer to be rendered.
         * @param auto_bind      Whether to automatically bind and enable/disable vertex attributes data.
         */
        void render(const shader &program, int primitive_type, size_t offset, size_t count, bool auto_bind = true) const;
        /**
         * @brief Binds and enables this mesh's vertices data to the given shader.
         * 
         * @param program The shader program. The attributes supported by this shader must fulfill this mesh's own vertex
         *        attributes, otherwise an exception is thrown.
         */
        void bind(const shader &program) const;
        /**
         * @brief Resets this mesh's vertices data to the given shader.
         * 
         * @param program The shader program. The attributes supported by this shader must fulfill this mesh's own vertex
         *        attributes, otherwise an exception is thrown.
         */
        void unbind(const shader &program) const;
    };
}

#endif // !AV_CORE_GRAPHICS_MESH_HPP
