#ifndef AV_CORE_GRAPHICS_SHADER_HPP
#define AV_CORE_GRAPHICS_SHADER_HPP

#include <glad/glad.h>
#include <av/util/log.hpp>

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace av {
    /**
     * @brief Holds the state of a runtime-compiled OpenGL shader program, attached with vertex and fragment shaders, each
     * being used to project vertices positions and to color rasterized texels, respectively. Typically used with `mesh`.
     */
    class shader {
        /** @brief Cached uniform locations, mapped by their names. */
        std::unordered_map<std::string, int> uniforms;
        /** @brief Caches vertex attribute locations, mapped by their names. */
        std::unordered_map<std::string, int> attributes;

        /** @brief The handle to the compiled OpenGL vertex shader. */
        unsigned int vertex_shader;
        /** @brief The handle to the compiled OpenGL fragment shader. */
        unsigned int fragment_shader;
        /** @brief The handle to the linked OpenGL shader program. */
        unsigned int program;
        /** @brief The amount of supported color attachments this shader can output. */
        int color_attachments;

        public:
        shader(const shader &) = delete;
        /**
         * Compiles and links a shader program given the shader sources and specified fragment shader color outputs.
         * 
         * @param vertex_source   The vertex shader source.
         * @param fragment_source The fragment shader source.
         * @param frag_datas      The outputs of the fragment shader, defaults to `{"out_color"}`.
         */
        shader(const char *vertex_source, const char *fragment_source, std::initializer_list<std::string> frag_datas = {"out_color"});
        /** Destroys this shader program, freeing the OpenGL resources it holds. */
        ~shader();

        /**
         * @brief Binds the shader program to be used. There may be only one used program at a time, so invocations to
         * this method on another instance will cause this instance to be unused.
         */
        inline void bind() const {
            glUseProgram(program);
        }
        /** @return The amount of supported color attachments this shader can output. */
        inline int get_color_attachments() const {
            return color_attachments;
        }

        /**
         * @brief Retrieves a uniform location in the shader program by its name. If not found, then an exception will be
         * thrown.
         * 
         * @param uniform The uniform name.
         * @return The uniform location.
         */
        inline int uniform_loc(const std::string &uniform) const {
            const auto &it = uniforms.find(uniform);
            if(it == uniforms.end()) throw std::runtime_error(std::string("No such uniform: '").append(uniform).append("'").c_str());
            return it->second;
        }
        /**
         * @brief Retrieves a vertex attribute location in the shader program by its name. If not found, then an exception
         * will be thrown.
         *
         * @param attribute The vertex attribute name.
         * @return The vertex attribute location.
         */
        inline int attribute_loc(const std::string &attribute) const {
            const auto &it = attributes.find(attribute);
            if(it == attributes.end()) throw std::runtime_error(std::string("No such vertex attribute: '").append(attribute).append("'").c_str());
            return it->second;
        }

        /**
         * @brief Queries either uniforms or vertex attributes this shader contains and stores them to a map containing pairs
         * of names and locations.
         * 
         * @tparam T_uniform If `true`, uniforms will be queried, vertex attributes otherwise.
         * @tparam T_map     The map type. It is your responsibility to make sure you supplied a map that can store the
         *                   queried pairs.
         * @tparam T_key     The key type of the map.
         * @tparam T_value   The value type of the map.
         * @param  map       The map that is used to pair up field names with their locations.
         */
        template<bool T_uniform, typename T_map, typename T_key = typename T_map::key_type, typename T_value = typename T_map::mapped_type>
        void query_fields(T_map &map) const {
            static constexpr int length_type = T_uniform ? GL_ACTIVE_UNIFORM_MAX_LENGTH : GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
            static constexpr int fields_type = T_uniform ? GL_ACTIVE_UNIFORMS : GL_ACTIVE_ATTRIBUTES;

            int max_length;
            glGetProgramiv(program, length_type, &max_length);

            char name[max_length + 1];
            int length;

            int count;
            glGetProgramiv(program, fields_type, &count);

            for(unsigned int i = 0; i < count; i++) {
                if constexpr(T_uniform) {
                    glGetActiveUniform(program, i, 16, &length, nullptr, nullptr, name);
                } else {
                    glGetActiveAttrib(program, i, 16, &length, nullptr, nullptr, name);
                }

                name[length] = '\0';
                if constexpr(T_uniform) { // Apparently I can't just use `i` for this...
                    map[static_cast<T_key>(name)] = static_cast<T_value>(glGetUniformLocation(program, name));
                } else {
                    map[static_cast<T_key>(name)] = static_cast<T_value>(glGetAttribLocation(program, name));
                }
            }
        }

        private:
        /**
         * @brief Compiles a shader with the given source.
         * 
         * @tparam T_type The shader type, either `GL_VERTEX_SHADER` or `GL_FRAGMENT_SHADER`.
         * @param  source The shader source.
         * @return The handle to the compiled shader attachment, or `0` if fails.
         */
        template<int T_type>
        unsigned int create_shader(const char *source) const {
            static_assert(T_type == GL_VERTEX_SHADER || T_type == GL_FRAGMENT_SHADER, "Invalid shader type.");

            unsigned int shader = glCreateShader(T_type);
            if(!shader) return 0;

            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            int compiled;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

            log_shader(shader);
            if(!compiled) {
                glDeleteShader(shader);
                return 0;
            } else {
                return shader;
            }
        }

        /**
         * @brief Logs a shader attachment.
         * @param shader The handle to the OpenGL shader attachment to be logged.
         */
        void log_shader(unsigned int shader) const;
        /** @brief Logs the linked shader program. */
        void log_program() const;
    };
}

#endif // !AV_CORE_GRAPHICS_SHADER_HPP
