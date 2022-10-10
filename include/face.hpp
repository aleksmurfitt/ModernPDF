#if !defined(PDFLIB_FACE_H)
    #define PDFLIB_FACE_H
    #include "private/font-tables/parser.hpp"
    #include "private/harfbuzz-helpers.hpp"
    #include "private/types/font.hpp"

    #include <hb-ot.h>
    #include <hb-subset.h>
    #include <hb.h>
    #include <qpdf/Buffer.hh>
    #include <qpdf/QPDFObjectHandle.hh>

    #include <climits>
    #include <filesystem>
    #include <string_view>
    #include <vector>

namespace PDFLib {
class Font;
class Document;

class Face {
    SubsetInputHolder subsetInput;
    std::optional<BlobHolder> subsetBlob;
    std::optional<FontHolder> subsetFont;
    UnicodeSetHolder glyphSet;
    FaceHolder face;
    Font &font;
    QPDFObjectHandle descriptor;
    QPDFObjectHandle dictionary;
    const std::string handle;

  public:
    Face(HbFaceT *faceHandle, Font &font, std::string handle);

    int getAscender();
    int getDescender();
    int getCapHeight();
    float getItalicAngle();
    std::pair<int, std::string> shape(std::string text, float points);
    void embed(HbBlobT *blob);

    int getWeight() {
        return hb_style_get_value(face, HB_STYLE_TAG_WEIGHT);
    }

    const std::string &getHandle() {
        return handle;
    }

    float getSlantAngle() {
        return hb_style_get_value(face, HB_STYLE_TAG_SLANT_ANGLE);
    }

    QPDFObjectHandle &getDictionary() {
        return dictionary;
    }
    QPDFObjectHandle &getDescriptor() {
        return descriptor;
    }
};
} // namespace PDFLib
#endif // PDFLIB_FACE_H
