#if !defined(PDFLIB_PRIVATE_FONT_TABLES_PARSER_H)
    #define PDFLIB_PRIVATE_FONT_TABLES_PARSER_H
    #include "private/harfbuzz-helpers.hpp"

    #include <stddef.h>
    #include <stdint.h>

    #include <span>

namespace PDFLib {
// A stateful helper class
template <typename T> struct BasicParser {
    size_t offset = 0;
    template <typename Type = uint16_t> Type getNext() {
        Type out = *(static_cast<const T *>(this)->startPtr + offset++);
        if constexpr (sizeof(Type) > 1)
            for (size_t i = 1; i < sizeof(Type); i++) {
                out <<= 8;
                out |= *(static_cast<const T *>(this)->startPtr + offset++);
            }
        return out;
    }

    size_t getNext(size_t size) {
        size_t out = *(static_cast<const T *>(this)->startPtr + offset++);
        for (size_t i = 1; i < size; i++) {
            out <<= 8;
            out |= *(static_cast<const T *>(this)->startPtr + offset++);
        }
        return out;
    }
    template <typename Type = uint8_t> std::span<const Type> getBytes(size_t count) {
        std::span<const Type> out{reinterpret_cast<const Type *>(static_cast<const T *>(this)->startPtr + offset),
                                  count};
        offset += count;
        return out;
    }

    void moveTo(size_t pos) {
        offset = pos;
    }
};

class Parser : public BasicParser<Parser> {
  public:
    const uint8_t *startPtr;
    Parser(const uint8_t *ptr) : startPtr{ptr} {};
};

class FontTableParser : public BasicParser<FontTableParser> {
    PtrHolder<HbBlobT, hb_blob_destroy, hb_face_reference_table> tableBlob;
    const uint8_t *getTableRef() {
        uint32_t _;
        return reinterpret_cast<const uint8_t *>(hb_blob_get_data(tableBlob, &_));
    };

  public:
    const uint8_t *startPtr;
    FontTableParser(HbFontT *ptr, uint32_t tag) : tableBlob{ptr, tag}, startPtr{getTableRef()} {};
};
} // namespace PDFLib

#endif // PDFLIB_PRIVATE_FONT_TABLES_PARSER_H
