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
    #include <optional>
    #include <string_view>
    #include <vector>

namespace PDFLib {
class Font;
class Document;

class Face {
    SubsetInputHolder subsetInput;
    UnicodeSetHolder glyphSet;
    FaceHolder face;
    double scale;
    const std::string handle;

  public:
    Face(FaceHolder face, double scale, std::string handle);

    double getAscender();
    double getDescender();
    int getCapHeight();
    float getItalicAngle();
    std::pair<float, std::string> shape(std::string text, float points);
    float getHeight(std::string text);
    int getWeight() {
        return hb_style_get_value(face, HB_STYLE_TAG_WEIGHT);
    }
    const std::string &getHandle() {
        return handle;
    }
    float getSlantAngle() {
        return hb_style_get_value(face, HB_STYLE_TAG_SLANT_ANGLE);
    }
    UnicodeSetHolder &getUsedGlyphs() {
        return glyphSet;
    }
    hb_codepoint_t getGlyph(hb_codepoint_t unicodeCodepoint) {
        hb_codepoint_t glyph;
        hb_font_get_glyph(face, unicodeCodepoint, 0, &glyph);
        return glyph;
    }
    hb_position_t getGlyphAdvance(hb_codepoint_t glyph, bool LTR = true) {
        if (LTR)
            return hb_font_get_glyph_h_advance(face, glyph) * scale;
        throw std::logic_error("Not implemented");
    }
    SubsetInputHolder &getSubsetInput() {
        return subsetInput;
    }
};
} // namespace PDFLib
#endif // PDFLIB_FACE_H
