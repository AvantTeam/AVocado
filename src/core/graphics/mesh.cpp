#include <av/core/graphics/mesh.hpp>
#include <stdexcept>

namespace av {
    const vert_attribute vert_attribute::pos_2D = vert_attribute::create<2, GL_FLOAT>("a_pos");
    const vert_attribute vert_attribute::color = vert_attribute::create<4, GL_UNSIGNED_BYTE>("u_color", true);

    int vert_attribute::count_size() const {
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

    mesh::mesh(size_t max_vertices, size_t max_indices, std::initializer_list<vert_attribute> attributes):
        max_vertices(max_vertices),
        max_indices(max_indices),
        has_indices(max_indices > 0),
        attributes(attributes),

        vertex_size([&]() -> size_t {
        size_t size = 0;
        for(const vert_attribute &attribute : attributes) size += attribute.size;
        return size;
    }()),

        vertex_buffer([&]() -> unsigned int {
        unsigned int vertex_buffer;
        glGenBuffers(1, &vertex_buffer);

        return vertex_buffer;
    }()),

        index_buffer([&]() -> unsigned int {
        if(!has_indices) return 0;

        unsigned int index_buffer;
        glGenBuffers(1, &index_buffer);

        return index_buffer;
    }()) {}

    mesh::~mesh() {
        glDeleteBuffers(1, &vertex_buffer);
        if(has_indices) glDeleteBuffers(1, &index_buffer);
    }

    void mesh::bind(const shader &program) {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        size_t off = 0;
        for(const vert_attribute &attr : attributes) {
            unsigned int loc = program.attribute_loc(attr.name);

            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, attr.components, attr.type, attr.normalized, vertex_size, (void *)off);

            off += attr.size;
        }

        if(has_indices) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    }

    void mesh::unbind(const shader &program) {
        for(const vert_attribute &attr : attributes) glDisableVertexAttribArray(program.attribute_loc(attr.name));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
