#if !defined(PDFLIB_FONT_H)
    #define PDFLIB_FONT_H
    #include "face.hpp"
    #include "private/font-tables/parser.hpp"
    #include "private/harfbuzz-helpers.hpp"
    #include "private/types/font.hpp"
    #include "private/util.hpp"

    #include <hb-ot.h>
    #include <hb.h>

    #include <array>
    #include <climits>
    #include <filesystem>
    #include <functional>
    #include <vector>

namespace PDFLib {
class FontManager;
class Document;

class Font {
    FontHolder font;
    std::vector<Face> faces;
    const size_t index;
    std::string fontFamily;
    std::string fontSubFamily;
    std::string fontName;
    std::string fontPostscriptName;
    std::string fontCFFName;
    bool CFF = false;

    class CFFTable {
        FontTableParser parser;
        static constexpr uint32_t Tag = Util::tag<"CFF ">;
        /**
         * @brief Skips the index table
         */
        void skipIndex() {
            std::vector<size_t> offsets{};
            size_t count = parser.getNext<uint16_t>();
            if (count == 0)
                return;

            size_t offsetSize = parser.getNext<uint8_t>();
            for (size_t i = 0; i <= count; ++i)
                offsets.push_back(parser.getNext(offsetSize));

            for (size_t i = 0; i < count; ++i)
                parser.getBytes<uint8_t>((offsets[i + 1] - offsets[i]));

            return;
        };

        std::vector<std::span<const uint8_t>> parseIndex() {
            std::vector<size_t> offsets{};
            std::vector<std::span<const uint8_t>> objects{};
            size_t count = parser.getNext<uint16_t>();
            if (count == 0)
                return objects;

            size_t offsetSize = parser.getNext<uint8_t>();
            for (size_t i = 0; i <= count; ++i)
                offsets.push_back(parser.getNext(offsetSize));

            for (size_t i = 0; i < count; ++i)
                objects.push_back(parser.getBytes<uint8_t>((offsets[i + 1] - offsets[i])));

            return objects;
        };

        void parseFloatOperand(Parser &parser) {
            const uint8_t eof = 15;
            while (true) {
                std::integral auto b = parser.getNext<uint8_t>();
                const uint8_t n1 = b >> 4;
                const uint8_t n2 = b & 15;
                if (n1 == eof || n2 == eof) {
                    break;
                }
            }
        }

        int parseOperand(Parser &parser, uint8_t b0) {
            if (b0 == 28)
                return static_cast<int>(parser.getNext<uint16_t>());

            if (b0 == 29) {
                return static_cast<int>(parser.getNext<uint32_t>());
            }

            if (b0 == 30) {
                parseFloatOperand(parser);
                return 0;
            }

            if (b0 >= 32 && b0 <= 246) {
                return b0 - 139;
            }

            if (b0 >= 247 && b0 <= 250) {
                return (b0 - 247) * 256 + parser.getNext<uint8_t>() + 108;
            }

            if (b0 >= 251 && b0 <= 254) {
                return -(b0 - 251) * 256 - parser.getNext<uint8_t>() - 108;
            }

            throw std::runtime_error("Invalid b0 " + std::to_string(b0));
        }

        template <typename T> std::map<uint16_t, std::vector<int>> parseDict(std::span<T> data) {
            Parser parser{data.data()};
            std::vector<int> operands;
            std::map<uint16_t, std::vector<int>> records;
            while (parser.offset < data.size()) {
                uint16_t op = parser.getNext<uint8_t>();

                if (op <= 21) {
                    if (op == 12) {
                        op = 1200 + parser.getNext<uint8_t>();
                    }

                    records.emplace(op, operands);
                    operands.clear();
                } else
                    operands.push_back(parseOperand(parser, op));
            }
            return records;
        }
        std::vector<uint16_t> parseCharset(Parser parser, size_t nGlyphs) {
            std::vector<uint16_t> charset{0};
            nGlyphs--;
            const uint8_t format = parser.getNext<uint8_t>();
            uint16_t sid;
            uint16_t count;
            if (format == 0) {
                for (size_t i = 0; i < nGlyphs; ++i)
                    charset.push_back(parser.getNext<uint16_t>());
            } else if (format == 1) {
                while (charset.size() <= nGlyphs) {
                    sid = parser.getNext<uint16_t>();
                    count = parser.getNext<uint8_t>();
                    for (size_t i = 0; i <= count; ++i) {
                        charset.push_back(sid);
                        ++sid;
                    }
                }
            } else if (format == 2) {
                while (charset.size() <= nGlyphs) {
                    sid = parser.getNext<uint16_t>();
                    count = parser.getNext<uint16_t>();
                    for (size_t i = 0; i <= count; ++i) {
                        charset.push_back(sid);
                        ++sid;
                    }
                }
            } else
                throw std::runtime_error("Unknown charset format");
            return charset;
        }

        // This is intentionally mostly unimplemented — we only care to see if the
        // CID to GID mapping is 1-to-1, and if not to get the mapping.
      public:
        bool isCID = false;
        std::vector<uint16_t> charset;

        CFFTable(HbFontT *font) : parser{font, Tag} {

            // Skip header
            parser.moveTo(4);
            // Skip names
            skipIndex();
            auto topDictIndex = parseIndex();
            if (topDictIndex.size() == 0)
                return;
            auto topDict = parseDict(topDictIndex[0]);
            if (topDict.contains(1230) && (topDict[1230][0] != 0xFF || topDict[1230][1] != 0xFF))
                isCID = true;
            if (isCID)
                charset = parseCharset(Parser(parser.startPtr + topDict[15][0]), hb_face_get_glyph_count(font));
        };
    };
    class HeadTable {
        static constexpr uint32_t Tag = Util::tag<"head">;
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
        static constexpr uint32_t Tag = Util::tag<"OS/2">;
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
        static constexpr uint32_t Tag = Util::tag<"post">;
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

    std::optional<CFFTable> cffTable = std::nullopt;

  public:
    const double scale;
    BlobHolder blob;

    Font(HbFontT *fontHandle, size_t index);

    HbFontT *getHbObj() {
        return font;
    }

    std::string &getFamily() {
        if (fontFamily.empty()) {
            fontFamily.resize(50);
            unsigned int length = 50;
            if (hb_ot_name_get_utf8(
                    font, HB_OT_NAME_ID_TYPOGRAPHIC_FAMILY, HB_LANGUAGE_INVALID, &length, &fontFamily[0]) == 0) {
                length = 50;
                hb_ot_name_get_utf8(font, HB_OT_NAME_ID_FONT_FAMILY, HB_LANGUAGE_INVALID, &length, &fontFamily[0]);
            }

            fontFamily.resize(length);
        }
        return fontFamily;
    }

    std::string &getName() {
        if (fontName.empty()) {
            fontName.resize(50);
            unsigned int length = 50;
            if (hb_ot_name_get_utf8(font, HB_OT_NAME_ID_MAC_FULL_NAME, HB_LANGUAGE_INVALID, &length, &fontName[0]) ==
                0) {
                length = 50;
                hb_ot_name_get_utf8(font, HB_OT_NAME_ID_FULL_NAME, HB_LANGUAGE_INVALID, &length, &fontName[0]);
            }

            fontName.resize(length);
        }
        return fontName;
    }

    std::string &getSubFamily() {
        if (fontSubFamily.empty()) {
            fontSubFamily.resize(50);
            unsigned int length = 50;
            if (hb_ot_name_get_utf8(
                    font, HB_OT_NAME_ID_TYPOGRAPHIC_SUBFAMILY, HB_LANGUAGE_INVALID, &length, &fontSubFamily[0]) == 0) {
                length = 50;
                hb_ot_name_get_utf8(
                    font, HB_OT_NAME_ID_FONT_SUBFAMILY, HB_LANGUAGE_INVALID, &length, &fontSubFamily[0]);
            }
            fontSubFamily.resize(length);
        }
        return fontSubFamily;
    }

    std::string &getPostScriptName() {
        if (fontPostscriptName.empty()) {
            fontPostscriptName.resize(50);
            unsigned int length = 50;
            hb_ot_name_get_utf8(
                font, HB_OT_NAME_ID_POSTSCRIPT_NAME, HB_LANGUAGE_INVALID, &length, &fontPostscriptName[0]);
            fontPostscriptName.resize(length);
        }
        return fontPostscriptName;
    }

    std::string runToString(std::vector<std::pair<std::vector<uint32_t>, int32_t>> run) {
        auto toHex = [](uint32_t w, size_t hex_len = 4) -> std::string {
            static const char *digits = "0123456789ABCDEF";
            std::string rc(hex_len, '0');
            for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
                rc[i] = digits[(w >> j) & 0x0f];
            return rc;
        };
        std::string out{"["};
        for (auto &pair : run) {
            out += '<';
            for (auto &gid : pair.first)
                out += toHex(CFF && cffTable->isCID ? cffTable->charset[gid] : gid);
            out += '>';
            out += std::to_string(pair.second);
        }
        out += ']';
        return out;
    }

    bool isCFF() {
        return CFF;
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

    std::function<void(Pipeline *)> makeSubsetFunction(Face &face, bool subset = true);
    Face &makeFace();

    decltype(faces) &getFaces() {
        return faces;
    }
};
} // namespace PDFLib

#endif // PDFLIB_FONT_H
