#ifndef AV_IO_HPP
#define AV_IO_HPP

#include <iostream>
#include <string>
#include <cstring>

namespace av {
    /** @brief Encapsulates an abstract output stream. */
    struct writes {
        /** @brief The output stream reference. */
        std::ostream &out;

        /** @brief Constructs an output stream writer from an output stream reference. */
        writes(std::ostream &out): out(out) {}

        /**
         * @brief Writes an arbitrary value. `string` and C-string types are further specialized.
         *
         * @tparam T     The implicit value type.
         * @param  value The value reference.
         * @return This writer, for chaining.
         */
        template<typename T>
        inline writes &write(const T &value) {
            out.write(reinterpret_cast<const char *>(&value), sizeof(T));
            return *this;
        }
    };

    template<>
    inline writes &writes::write<std::string>(const std::string &value) {
        unsigned int len = value.length();

        write(len);
        out.write(value.c_str(), len);
        return *this;
    }

    template<>
    inline writes &writes::write<const char *>(const char *const &value) {
        unsigned int len = strlen(value);

        write(len);
        out.write(value, len);
        return *this;
    }

    /** @brief Encapsulates an abstract input stream. */
    struct reads {
        /** @brief The input stream reference. */
        std::istream &in;

        /** @brief Constructs an input stream writer from an output stream reference. */
        reads(std::istream &in): in(in) {}

        /**
         * @brief Convenience function to read a value given its type.
         *
         * @tparam T The value type.
         * @return The read value.
         */
        template<typename T>
        inline T read() const {
            T value{};
            read(value);

            return value;
        }

        /**
         * @brief Reads an arbitrary value. `string` and C-string types are further specialized.
         *
         * @tparam T     The implicit value type.
         * @param  value The value reference to be set.
         * @return This writer, for chaining.
         */
        template<typename T>
        inline reads &read(T &value) {
            in.read(reinterpret_cast<char *>(&value), sizeof(T));
            return *this;
        }

        /**
         * @brief Reads an arbitrary value. `string` and C-string types are further specialized.
         *
         * @tparam T     The implicit value type.
         * @param  value The value reference to be set.
         * @return This writer, for chaining.
         */
        template<typename T>
        inline const reads &read(T &value) const {
            in.read(reinterpret_cast<char *>(&value), sizeof(T));
            return *this;
        }
    };

    template<>
    inline reads &reads::read<std::string>(std::string &value) {
        unsigned int len = read<unsigned int>();

        char str[len + 1];
        in.read(str, len);
        str[len] = '\0';

        value = str;
        return *this;
    }

    template<>
    inline const reads &reads::read<std::string>(std::string &value) const {
        unsigned int len = read<unsigned int>();

        char str[len + 1];
        in.read(str, len);
        str[len] = '\0';

        value = str;
        return *this;
    }
}

#endif // !AV_IO_HPP
