#if !defined(PDFLIB_FONT_H)
    #define PDFLIB_FONT_H
    #include "face.hpp"
    #include "private/font-tables/parser.hpp"
    #include "private/harfbuzz-helpers.hpp"
    #include "private/types/font.hpp"

    #include <hb-ot.h>
    #include <hb.h>
    #include <qpdf/Buffer.hh>
    #include <qpdf/QPDFObjectHandle.hh>

    #include <climits>
    #include <filesystem>
    #include <functional>
    #include <string_view>
    #include <vector>

namespace PDFLib {
class FontManager;
class Document;

class Font {
    FontHolder font;
    std::vector<Face> faces;
    FontManager &manager;
    const size_t index;

    class HeadTable {
        static constexpr uint32_t Tag = 1751474532;
        FontTableParser parser;

      public:
        uint16_t majorVersion{parser.getNext()};
        uint16_t minorVersion{parser.getNext()};
        uint32_t fontRevision{parser.getNext<uint32_t>()};
        uint32_t checksum{parser.getNext<uint32_t>()};
        uint32_t magicNumber{parser.getNext<uint32_t>()};
        uint16_t flags{parser.getNext()};
        uint16_t unitsPerEM{parser.getNext()};
        uint64_t created{parser.getNext<uint64_t>()};
        uint64_t modified{parser.getNext<uint64_t>()};
        int16_t xMin{parser.getNext<int16_t>()};
        int16_t yMin{parser.getNext<int16_t>()};
        int16_t xMax{parser.getNext<int16_t>()};
        int16_t yMax{parser.getNext<int16_t>()};
        uint16_t macStyle{parser.getNext()};
        uint16_t lowestRecPPEM{parser.getNext()};
        int16_t fontDirectionHint{parser.getNext<int16_t>()};
        int16_t indexToLocFormat{parser.getNext<int16_t>()};
        int16_t glyphDataFormat{parser.getNext<int16_t>()};

        HeadTable(HbFontT *font) : parser{font, Tag} {};

    } headTable;

    class OS2Table {
        static constexpr uint32_t Tag = 1330851634;
        FontTableParser parser;

      public:
        uint16_t version{parser.getNext()};
        int16_t xAvgCharWidth{parser.getNext<int16_t>()};
        uint16_t usWeightClass{parser.getNext()};
        uint16_t usWidthClass{parser.getNext()};
        uint32_t fsType{parser.getNext()};
        int16_t ySubscriptXSize{parser.getNext<int16_t>()};
        int16_t ySubscriptYSize{parser.getNext<int16_t>()};
        int16_t ySubscriptXOffset{parser.getNext<int16_t>()};
        int16_t ySubscriptYOffset{parser.getNext<int16_t>()};
        int16_t ySuperscriptXSize{parser.getNext<int16_t>()};
        int16_t ySuperscriptYSize{parser.getNext<int16_t>()};
        int16_t ySuperscriptXOffset{parser.getNext<int16_t>()};
        int16_t ySuperscriptYOffset{parser.getNext<int16_t>()};
        int16_t yStrikeoutSize{parser.getNext<int16_t>()};
        int16_t yStrikeoutPosition{parser.getNext<int16_t>()};
        int16_t sFamilyClass{parser.getNext<int16_t>()};
        uint8_t panose[10]{parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>(),
                           parser.getNext<uint8_t>()};
        uint32_t ulUnicodeRange1{parser.getNext<uint32_t>()};
        uint32_t ulUnicodeRange2{parser.getNext<uint32_t>()};
        uint32_t ulUnicodeRange3{parser.getNext<uint32_t>()};
        uint32_t ulUnicodeRange4{parser.getNext<uint32_t>()};
        uint32_t achVendID{parser.getNext<uint32_t>()};
        uint16_t fsSelection{parser.getNext()};
        uint16_t usFirstCharIndex{parser.getNext()};
        uint16_t usLastCharIndex{parser.getNext()};
        int16_t sTypoAscender{parser.getNext<int16_t>()};
        int16_t sTypoDescender{parser.getNext<int16_t>()};
        int16_t sTypoLineGap{parser.getNext<int16_t>()};
        uint16_t usWinAscent{parser.getNext()};
        uint16_t usWinDescent{parser.getNext()};
        uint16_t usCodePageRange1{static_cast<uint16_t>(version > 0 ? parser.getNext() : 0)};
        uint16_t usCodePageRange2{static_cast<uint16_t>(version > 0 ? parser.getNext() : 0)};
        int16_t sxHeight{static_cast<int16_t>(version > 1 ? parser.getNext<int16_t>() : 0)};
        int16_t sxCapHeight{static_cast<int16_t>(version > 1 ? parser.getNext<int16_t>() : 0)};
        uint16_t usDefaultChar{static_cast<uint16_t>(version > 1 ? parser.getNext() : 0)};
        uint16_t usBreakChar{static_cast<uint16_t>(version > 1 ? parser.getNext() : 0)};
        uint16_t usMaxContext{static_cast<uint16_t>(version > 1 ? parser.getNext() : 0)};
        uint16_t usLowerOpticalPointSize{static_cast<uint16_t>(version > 4 ? parser.getNext() : 0)};
        uint16_t usUpperOpticalPointSize{static_cast<uint16_t>(version > 4 ? parser.getNext() : 0)};

        OS2Table(HbFontT *font) : parser{font, Tag} {};

    } os2Table;
    class PostTable {
        static constexpr uint32_t Tag = 1886352244;
        FontTableParser parser;

      public:
        uint32_t version{parser.getNext<uint32_t>()};
        int32_t italicAngle{parser.getNext<int32_t>()};
        int16_t underlinePosition{parser.getNext<int16_t>()};
        int16_t underlineThickness{parser.getNext<int16_t>()};
        uint32_t isFixedPitch{parser.getNext<uint32_t>()};
        // We don't care about the rest
        PostTable(HbFontT *font) : parser{font, Tag} {};

    } postTable;

  public:
    const double scale;
    BlobHolder blob;

    Font(HbFontT *fontHandle, FontManager &manager, size_t index);

    HbFontT *getHbObj() {
        return font;
    }

    std::string getFamily() {
        std::string out;
        out.resize(50);
        unsigned int length = 50;
        if (hb_ot_name_get_utf8(font, HB_OT_NAME_ID_TYPOGRAPHIC_FAMILY, HB_LANGUAGE_INVALID, &length, &out[0]) == 0) {
            length = 50;
            hb_ot_name_get_utf8(font, HB_OT_NAME_ID_FONT_FAMILY, HB_LANGUAGE_INVALID, &length, &out[0]);
        }

        out.resize(length);
        return out;
    }

    std::string getName() {
        std::string out;
        out.resize(50);
        unsigned int length = 50;
        if (hb_ot_name_get_utf8(font, HB_OT_NAME_ID_MAC_FULL_NAME, HB_LANGUAGE_INVALID, &length, &out[0]) == 0) {
            length = 50;
            hb_ot_name_get_utf8(font, HB_OT_NAME_ID_FULL_NAME, HB_LANGUAGE_INVALID, &length, &out[0]);
        }

        out.resize(length);
        return out;
    }

    std::string getSubFamily() {
        std::string out;
        out.resize(50);
        unsigned int length = 50;
        if (hb_ot_name_get_utf8(font, HB_OT_NAME_ID_TYPOGRAPHIC_SUBFAMILY, HB_LANGUAGE_INVALID, &length, &out[0]) ==
            0) {
            length = 50;
            hb_ot_name_get_utf8(font, HB_OT_NAME_ID_FONT_SUBFAMILY, HB_LANGUAGE_INVALID, &length, &out[0]);
        }
        out.resize(length);
        return out;
    }

    std::string getPostScriptName() {
        std::string out;
        out.resize(50);
        unsigned int length = 50;
        hb_ot_name_get_utf8(font, HB_OT_NAME_ID_POSTSCRIPT_NAME, HB_LANGUAGE_INVALID, &length, &out[0]);
        out.resize(length);
        return out;
    }

    BBox getBoundingBox() {
        BBox out{headTable.xMin, headTable.yMin, headTable.xMax, headTable.yMax};
        out *= scale;
        return out;
    }

    int getVariations() {
        return hb_ot_var_get_axis_count(font);
    }

    uint8_t getFlags() {
        uint8_t flags = 0;
        uint8_t familyClass = (os2Table.sFamilyClass) >> 8;
        if (postTable.isFixedPitch)
            flags |= 1;
        if (familyClass > 0 && familyClass < 8)
            flags |= 1 << 1;
        else if (familyClass == 10)
            flags |= 1 << 3;
        flags |= 1 << 2;
        if (headTable.macStyle & 2)
            flags |= 1 << 6;
        return flags;
    }

    std::function<void(Pipeline *)> makeSubsetFunction(Face &face, QPDFObjectHandle dict);
    Face &makeFace();

    decltype(faces) &getFaces() {
        return faces;
    }

    FontManager &getManager() {
        return manager;
    }
};
} // namespace PDFLib

#endif // PDFLIB_FONT_H
