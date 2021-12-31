#ifndef AV_GRAPHICS_SHADER_HPP
#define AV_GRAPHICS_SHADER_HPP

#include "../glad.h"

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

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

        /** @brief The vertex shader source. */
        const char *vertex_source;
        /** @brief The fragment shader source. */
        const char *fragment_source;

        /** @brief The output datas of this program's fragment shader. */
        std::vector<std::string> fragment_outs;
        /** @brief The handle to the compiled OpenGL vertex shader. */
        unsigned int vertex_shader;
        /** @brief The handle to the compiled OpenGL fragment shader. */
        unsigned int fragment_shader;
        /** @brief The handle to the linked OpenGL shader program. */
        unsigned int program;

        public:
        /** @brief Default copy-constructor, creates a new shader with the same source. */
        shader(const shader &from):
            vertex_source(from.vertex_source),
            fragment_source(from.fragment_source),
            fragment_outs(from.fragment_outs),

            vertex_shader(create_shader<GL_VERTEX_SHADER>(from.vertex_source)),
            fragment_shader(create_shader<GL_FRAGMENT_SHADER>(from.fragment_source)),
            program(create_program(vertex_shader, fragment_shader, from.fragment_outs)) {
            if(!vertex_shader || !fragment_shader || !program) throw std::runtime_error("Couldn't create shader.");
            query_fields<true>(uniforms);
            query_fields<false>(attributes);
        }
        /** @brief Default move-constructor, invalidates the other shader. */
        shader(shader &&from):
            uniforms(std::move(from.uniforms)),
            attributes(std::move(from.attributes)),

            vertex_source(std::move(from.vertex_source)),
            fragment_source(std::move(from.fragment_source)),
            fragment_outs(std::move(from.fragment_outs)),

            vertex_shader(std::move(from.vertex_shader)),
            fragment_shader(std::move(from.fragment_shader)),
            program(std::move(from.program)) {
            from.program = 0;
            from.vertex_shader = 0;
            from.fragment_shader = 0;
        }
        /**
         * Compiles and links a shader program given the shader sources and specified fragment shader color outputs.
         * 
         * @param  vertex_source   The vertex shader source.
         * @param  fragment_source The fragment shader source.
         * @param  frag_datas      The outputs of the fragment shader, defaults to `{"out_color"}`.
         * @tparam T_list          Typically `initializer_list<string>` or `vector<string>`.
         */
        template<typename T_list = std::initializer_list<std::string>>
        shader(const char *vertex_source, const char *fragment_source, T_list frag_datas = {"out_color"}):
            vertex_source(vertex_source),
            fragment_source(fragment_source),
            fragment_outs(frag_datas),

            vertex_shader(create_shader<GL_VERTEX_SHADER>(vertex_source)),
            fragment_shader(create_shader<GL_FRAGMENT_SHADER>(fragment_source)),
            program(create_program(vertex_shader, fragment_shader, fragment_outs)) {
            if(!vertex_shader || !fragment_shader || !program) throw std::runtime_error("Couldn't create shader.");
            query_fields<true>(uniforms);
            query_fields<false>(attributes);
        }
        /** Destroys this shader program, freeing the OpenGL resources it holds. */
        ~shader() {
            glDeleteProgram(program);
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
        }

        /**
         * @brief Binds the shader program to be used. There may be only one used program at a time, so invocations to
         * this method on another instance will cause this instance to be unused.
         */
        inline void bind() const {
            glUseProgram(program);
        }

        /** @return The vertex shader attachment source. */
        inline const char *get_vertex_source() const {
            return vertex_source;
        }
        /** @return The fragment shader attachment source. */
        inline const char *get_fragment_source() const {
            return fragment_source;
        }
        /** @return The output names of the fragment shader attachment. */
        inline const std::vector<std::string> &get_fragment_outs() const {
            return fragment_outs;
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
        template<bool T_uniform, template<typename, typename, typename...> typename T_map, typename T_key, typename T_value, typename... T_args>
        void query_fields(T_map<T_key, T_value, T_args...> &map) const {
            static constexpr int length_type = T_uniform ? GL_ACTIVE_UNIFORM_MAX_LENGTH : GL_ACTIVE_ATTRIBUTE_MAX_LENGTH;
            static constexpr int fields_type = T_uniform ? GL_ACTIVE_UNIFORMS : GL_ACTIVE_ATTRIBUTES;

            int max_length;
            glGetProgramiv(program, length_type, &max_length);

            char name[max_length + 1];
            int length;

            int count;
            glGetProgramiv(program, fields_type, &count);

            for(int i = 0; i < count; i++) {
                if constexpr(T_uniform) {
                    glGetActiveUniform(program, i, 16, &length, nullptr, nullptr, name);
                } else {
                    glGetActiveAttrib(program, i, 16, &length, nullptr, nullptr, name);
                }

                name[length] = '\0';
                if constexpr(T_uniform) {
                    map[static_cast<T_key>(name)] = static_cast<T_value>(glGetUniformLocation(program, name));
                } else {
                    map[static_cast<T_key>(name)] = static_cast<T_value>(glGetAttribLocation(program, name));
                }
            }
        }

        private:
        /** @return The handle to the linked OpenGL shader program, or `0` if fails. */
        unsigned int create_program(unsigned int vertex_shader, unsigned int fragment_shader, const std::vector<std::string> &frag_datas) {
            if(!vertex_shader || !glIsShader(vertex_shader)) return 0;
            if(!fragment_shader || !glIsShader(fragment_shader)) return 0;

            unsigned int program = glCreateProgram();
            if(!program) return 0;

            glAttachShader(program, vertex_shader);
            glAttachShader(program, fragment_shader);
            for(int i = 0; i < fragment_outs.size(); i++) glBindFragDataLocation(program, i, fragment_outs[i].c_str());

            glLinkProgram(program);
            log_program();

            int success;
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if(!success) {
                glDeleteProgram(program);
                return 0;
            }

            return program;
        }

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
        void log_shader(unsigned int shader) const {
            if(!shader || !glIsShader(shader)) return;

            int len = 0;
            int maxLen = len;

            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLen);
            char log[maxLen + 1];

            glGetShaderInfoLog(shader, maxLen, &len, log);
            if(len > 0) {
                log[len] = '\0';
                log::msg<log_level::warn>("Shader attachment logs not empty: \n%s", static_cast<const char *>(log));
            }
        }
        /** @brief Logs the linked shader program. */
        void log_program() const {
            if(!program || !glIsProgram(program)) return;

            int len = 0;
            int maxLen = len;

            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
            char log[maxLen + 1];

            glGetProgramInfoLog(program, maxLen, &len, log);
            if(len > 0) {
                log[len] = '\0';
                log::msg<log_level::warn>("Shader program logs not empty: \n%s", static_cast<const char *>(log));
            }
        }
    };
}

#endif // !AV_GRAPHICS_SHADER_HPP
