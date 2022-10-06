#if !defined(PDFLIB_PRIVATE_FONT_TABLES_PARSER_H)
#    define PDFLIB_PRIVATE_FONT_TABLES_PARSER_H
#    include "private/harfbuzz-helpers.hpp"

#    include <stddef.h>
#    include <stdint.h>

namespace PDFLib {
// A stateful helper class
class FontTableParser {
    size_t offset = 0;

    const char *getTableRef(HbFontT *font, uint32_t tag) {
        uint32_t _;
        return hb_blob_get_data(hb_face_reference_table(font, tag), &_);
    };

  public:
    const char *startPtr;

    template <typename T = uint16_t> T getNext() {
        const T u = *reinterpret_cast<const T *>(startPtr + offset);
        offset += sizeof(T);

        union {
            T u;
            unsigned char u8[sizeof(T)];
        } source, dest;

        source.u = u;
        for (size_t k = 0; k < sizeof(T); k++) dest.u8[k] = source.u8[sizeof(T) - k - 1];
        return dest.u;
    }

    FontTableParser(HbFontT *ptr, uint32_t tag) : startPtr{getTableRef(ptr, tag)} {};
};
} // namespace PDFLib

#endif // PDFLIB_PRIVATE_FONT_TABLES_PARSER_H
