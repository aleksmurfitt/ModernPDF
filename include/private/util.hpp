#if !defined(PDFLIB_UTIL_H)
    #define PDFLIB_UTIL_H
    #include <memory>

namespace pdf_lib::util {

template <size_t N> struct sized_string_literal {
    consteval sized_string_literal(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    consteval char operator[](size_t i) const {
        return value[i];
    }
    char value[N];
    size_t size = N;
};

constexpr unsigned int as_tag(std::string_view c){
    return (static_cast<unsigned int>(c[0]) << 24) | (static_cast<unsigned int>(c[1]) << 16) |
           (static_cast<unsigned int>(c[2]) << 8) | c[3];
}

template <sized_string_literal c>
constexpr unsigned int tag = as_tag(c.value);
} // namespace pdf_lib::util

#endif // PDFLIB_UTIL_H
