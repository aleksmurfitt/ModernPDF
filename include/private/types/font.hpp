#if !defined(PDFLIB_PRIVATE_TYPES_FONT)
#    define PDFLIB_PRIVATE_TYPES_FONT
#    include <qpdf/QPDFObjectHandle.hh>
#    include <stdint.h>

#    include <string>
#    include <type_traits>

namespace PDFLib {
struct BBox {
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;

    operator QPDFObjectHandle() {
        return QPDFObjectHandle::newArray(QPDFObjectHandle::Rectangle(xMin, yMin, xMax, yMax));
    };

    template <typename T>

    requires(std::is_integral_v<T> || std::is_floating_point_v<T>) BBox &operator*=(T i) {
        xMin *= i;
        yMin *= i;
        xMax *= i;
        yMax *= i;
        return *this;
    }

    template <typename T>

    requires(std::is_integral_v<T> || std::is_floating_point_v<T>) BBox operator*(std::integral auto i) {
        BBox out = *this;
        out *= i;
        return out;
    };
};
} // namespace PDFLib
#endif // PDFLIB_PRIVATE_TYPES_FONT
