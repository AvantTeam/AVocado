#include <av/core/graphics/mesh.hpp>
#include <av/util/log.hpp>
#include <stdexcept>

namespace av {
    const vert_attribute vert_attribute::pos_2D = vert_attribute::create<2, GL_FLOAT>("a_pos");
    const vert_attribute vert_attribute::color = vert_attribute::create<4, GL_FLOAT>("a_col");
    const vert_attribute vert_attribute::color_packed = vert_attribute::create<4, GL_UNSIGNED_BYTE, true>("a_col");

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

    mesh::mesh(std::initializer_list<vert_attribute> attributes):
        vertex_size([&]() -> size_t {
        size_t size = 0;
        for(const vert_attribute &attribute : attributes) size += attribute.size;
        return size;
    }()),

        attributes(attributes),
        max_vertices(0),
        max_elements(0),
        has_elements(false),

        vertex_buffer([&]() -> unsigned int {
        unsigned int vertex_buffer;
        glGenBuffers(1, &vertex_buffer);

        return vertex_buffer;
    }()),

        element_buffer([&]() -> unsigned int {
        unsigned int index_buffer;
        glGenBuffers(1, &index_buffer);

        return index_buffer;
    }()) {}

    mesh::~mesh() {
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteBuffers(1, &element_buffer);
    }

    void mesh::render(const shader &program, int primitive_type, size_t offset, size_t count, bool auto_bind) const {
        if(auto_bind) bind(program);

        if(has_elements) {
            glDrawElements(primitive_type, count, GL_UNSIGNED_SHORT, (void *)offset);
        } else {
            glDrawArrays(primitive_type, offset, count);
        }

        if(auto_bind) unbind(program);
    }

    void mesh::bind(const shader &program) const {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        size_t off = 0;
        for(const vert_attribute &attr : attributes) {
            unsigned int loc = program.attribute_loc(attr.name);

            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, attr.components, attr.type, attr.normalized, vertex_size, (void *)off);

            off += attr.size;
        }
        
        if(has_elements) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    }

    void mesh::unbind(const shader &program) const {
        for(const vert_attribute &attr : attributes) glDisableVertexAttribArray(program.attribute_loc(attr.name));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        if(has_elements) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
