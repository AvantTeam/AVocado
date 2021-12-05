#include <av/core/graphics/shader.hpp>
#include <av/util/log.hpp>

namespace av {
    shader::shader(const char *vertex_source, const char *fragment_source, std::initializer_list<const char *> frag_datas):
        vertex_shader(create_shader(GL_VERTEX_SHADER, vertex_source)),
        fragment_shader(create_shader(GL_FRAGMENT_SHADER, fragment_source)),

        color_attachments([&]() -> int {
        int size = frag_datas.size();
        if(size > 32) throw std::runtime_error("Fragment shaders only support up to 32 out color attachments.");
        return size;
    }()),

        program([&]() -> unsigned int {
        if(!vertex_shader) throw std::runtime_error("Couldn't create vertex shader!");
        if(!fragment_shader) throw std::runtime_error("Couldn't create fragment shader!");

        unsigned int program = glCreateProgram();
        if(!program) throw std::runtime_error("Couldn't create shader program!");

        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        
        size_t index = 0;
        for(const char *const &data : frag_datas) glBindFragDataLocation(program, index++, data);

        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success) throw std::runtime_error("Couldn't link GL program!");

        return program;
    }()) {
        query_fields<true>(uniforms);
        query_fields<false>(attributes);
    }

    shader::~shader() {
        glDeleteProgram(program);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    unsigned int shader::create_shader(int type, const char *source) const {
        unsigned int shader = glCreateShader(type);
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

    void shader::log_shader(unsigned int shader) const {
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

    void shader::log_program() const {
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
}
