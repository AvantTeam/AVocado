#ifndef AV_GRAPHICS_MESH_HPP
#define AV_GRAPHICS_MESH_HPP

#include "shader.hpp"

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
        /** @brief 2 `float` components; U and V. */
        static const vert_attribute tex_coords;

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
        inline int count_size() const {
            switch(type) {
                case GL_BYTE:
                case GL_UNSIGNED_BYTE: return sizeof(char) * components;
                case GL_SHORT:
                case GL_UNSIGNED_SHORT: return sizeof(short) * components;
                case GL_INT:
                case GL_UNSIGNED_INT: return sizeof(int) * components;
                case GL_FLOAT: return sizeof(float) * components;
                default: throw std::runtime_error("Invalid vertex attribute type.");
            }
        }

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

    const vert_attribute vert_attribute::pos_2D = vert_attribute::create<2, GL_FLOAT>("a_position");
    const vert_attribute vert_attribute::color = vert_attribute::create<4, GL_FLOAT>("a_color");
    const vert_attribute vert_attribute::color_packed = vert_attribute::create<4, GL_UNSIGNED_BYTE, true>("a_color");
    const vert_attribute vert_attribute::tex_coords = vert_attribute::create<2, GL_FLOAT>("a_tex_coords_0");

    /**
     * @brief A mesh is a class holding a state of a vertex buffer object and an element buffer object in order to draw
     * objects on an OpenGL surface.
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
        /** Default copy-constructor, creates new buffers and fills them with the same copied data. */
        mesh(const mesh &from):
            vertex_size(from.vertex_size),
            attributes(from.attributes),
            has_elements(from.has_elements),
            vertex_buffer(create_buffer()),
            element_buffer(create_buffer()) {
            float vertices[from.max_vertices * vertex_size / sizeof(float)];
            glBindBuffer(GL_ARRAY_BUFFER, from.vertex_buffer);
            glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            set_vertices(vertices, 0, sizeof(vertices));

            if(has_elements) {
                unsigned short elements[from.max_elements];
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, from.element_buffer);
                glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(elements), elements);
                set_elements(elements, 0, sizeof(elements));
            }
        }
        /** Default move-constructor, invalidates the other mesh. */
        mesh(mesh &&from):
            vertex_size(std::move(from.vertex_size)),
            attributes(std::move(from.attributes)),
            max_vertices(std::move(from.max_vertices)),
            max_elements(std::move(from.max_elements)),
            has_elements(std::move(from.has_elements)),
            vertex_buffer(std::move(from.vertex_buffer)),
            element_buffer(std::move(from.element_buffer)) {
            from.vertex_buffer = 0;
            from.element_buffer = 0;
        }
        /**
         * @brief Constructs an empty mesh with given vertex attributes. These attributes are identifiers to each
         * vertices' data, e.g. position and color. Calls to `set_vertices(float *, size_t, size_t)` and (optionally)
         * `set_elements(unsigned short *, size_t, size_t)` must be invoked in order to initialize the mesh data to be
         * rendered.
         * 
         * @param attributes The vertex attributes.
         */
        mesh(std::initializer_list<vert_attribute> attributes):
            vertex_size([&]() -> size_t {
            size_t size = 0;
            for(const vert_attribute &attribute : attributes) size += attribute.size;
            return size;
        }()),

            attributes(attributes),
            max_vertices(0),
            max_elements(0),
            has_elements(false),
            vertex_buffer(create_buffer()),
            element_buffer(create_buffer()) {}
        /** Destroys this mesh, freeing the OpenGL resources it holds. */
        ~mesh() {
            glDeleteBuffers(1, &vertex_buffer);
            glDeleteBuffers(1, &element_buffer);
        }

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
            glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), vertices + offset, T_usage);

            max_vertices = length / (vertex_size / sizeof(float));
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
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, length * sizeof(unsigned short), elements + offset, T_usage);

            max_elements = length;
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
         * @param length         Specifies the length of vertex (or element, if any) buffer to be rendered.
         * @param auto_bind      Whether to automatically bind and enable/disable vertex attributes data.
         */
        void render(const shader &program, int primitive_type, size_t offset, size_t length, bool auto_bind = true) const {
            if(auto_bind) bind(program);

            if(has_elements) {
                glDrawElements(primitive_type, length, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(offset));
            } else {
                glDrawArrays(primitive_type, offset, length);
            }

            if(auto_bind) unbind(program);
        }
        /**
         * @brief Binds and enables this mesh's vertices data to the given shader.
         * 
         * @param program The shader program. The attributes supported by this shader must fulfill this mesh's own vertex
         *        attributes, otherwise an exception is thrown.
         */
        void bind(const shader &program) const {
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

            size_t off = 0;
            for(const vert_attribute &attr : attributes) {
                unsigned int loc = program.attribute_loc(attr.name);

                glEnableVertexAttribArray(loc);
                glVertexAttribPointer(loc, attr.components, attr.type, attr.normalized, vertex_size, reinterpret_cast<void *>(off));

                off += attr.size;
            }

            if(has_elements) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
        }
        /**
         * @brief Resets this mesh's vertices data to the given shader.
         * 
         * @param program The shader program. The attributes supported by this shader must fulfill this mesh's own vertex
         *        attributes, otherwise an exception is thrown.
         */
        void unbind(const shader &program) const {
            for(const vert_attribute &attr : attributes) glDisableVertexAttribArray(program.attribute_loc(attr.name));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            if(has_elements) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        private:
        /** @return The generated OpenGL buffer object. */
        inline unsigned int create_buffer() {
            unsigned int buffer;
            glGenBuffers(1, &buffer);

            return buffer;
        }
    };
}

#endif // !AV_GRAPHICS_MESH_HPP
