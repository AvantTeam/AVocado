#ifndef AV_CORE_GRAPHICS_SHADER_HPP
#define AV_CORE_GRAPHICS_SHADER_HPP

#include <glad/glad.h>

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace av {
    class shader {
        std::unordered_map<std::string, unsigned int> uniforms;
        std::unordered_map<std::string, unsigned int> attributes;

        unsigned int vertex_shader;
        unsigned int fragment_shader;
        unsigned int program;

        int color_attachments;

        public:
        shader(const shader &) = delete;
        shader() = default;
        shader(const char *vertex_source, const char *fragment_source, std::initializer_list<std::string> frag_datas = {"out_color"});
        ~shader();

        inline void use() const {
            glUseProgram(program);
        }

        inline int get_color_attachments() const {
            return color_attachments;
        }

        inline unsigned int uniform_loc(const std::string &uniform) const {
            const auto &it = uniforms.find(uniform);
            if(it == uniforms.end()) throw std::runtime_error(std::string("No such uniform: '").append(uniform).append("'").c_str());
            return it->second;
        }
        inline unsigned int attribute_loc(const std::string &attribute) const {
            const auto &it = attributes.find(attribute);
            if(it == attributes.end()) throw std::runtime_error(std::string("No such vertex attribute: '").append(attribute).append("'").c_str());
            return it->second;
        }

        template<bool T_uniform>
        void query_fields(std::unordered_map<std::string, unsigned int> &map) const {
            static constexpr int length_type = T_uniform ? GL_ACTIVE_UNIFORM_MAX_LENGTH : GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
            static constexpr int fields_type = T_uniform ? GL_ACTIVE_UNIFORMS : GL_ACTIVE_ATTRIBUTES;

            int max_length;
            glGetProgramiv(program, length_type, &max_length);

            char name[max_length + 1];
            int length;

            int count;
            glGetProgramiv(program, fields_type, &count);

            map.clear();
            for(unsigned int i = 0; i < count; i++) {
                if constexpr(T_uniform) {
                    glGetActiveUniform(program, i, 16, &length, nullptr, nullptr, name);
                } else {
                    glGetActiveAttrib(program, i, 16, &length, nullptr, nullptr, name);
                }

                name[length] = '\0';
                map.emplace(static_cast<const char *>(name), i);
            }
        }

        private:
        unsigned int create_shader(int type, const char *source) const;

        void log_shader(unsigned int shader) const;
        void log_program() const;
    };
}

#endif // !AV_CORE_GRAPHICS_SHADER_HPP
