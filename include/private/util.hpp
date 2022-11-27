#if !defined(PDFLIB_UTIL_H)
    #define PDFLIB_UTIL_H
    #include <memory>

namespace PDFLib {
namespace Util {
template <size_t N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    constexpr char operator[](size_t i) const {
        return value[i];
    }
    char value[N];
    size_t size = N;
};

template <StringLiteral c>
constexpr unsigned int tag = (static_cast<unsigned int>(c[0]) << 24) | (static_cast<unsigned int>(c[1]) << 16) |
                             (static_cast<unsigned int>(c[2]) << 8) | c[3];
} // namespace Util
} // namespace PDFLib

#endif // PDFLIB_UTIL_H
