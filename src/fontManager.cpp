#define POINTERHOLDER_TRANSITION 3
#include "fontManager.hpp"
#include "pdf.hpp"

#include <limits>
#include <random>
using namespace PDFLib;

FontManager::FontManager(Document &document)
    : document{document},
      dictionary{document.newIndirectObject(QPDFObjectHandle::newDictionary())} {};

std::vector<QPDFObjectHandle> glyphAdvances(Face &face, Font &font, bool subset) {
    std::vector<QPDFObjectHandle> glyphAdvances{};
    auto &glyphs = face.getUsedGlyphs();
    size_t max = (subset ? hb_set_get_max(glyphs) : hb_face_get_glyph_count(font.getHbObj()));
    size_t min = (subset ? hb_set_get_min(glyphs) : 0);
    size_t pop = (subset ? hb_set_get_population(glyphs) : hb_face_get_glyph_count(font.getHbObj()));
    if (subset)
        for (uint32_t i = HB_SET_VALUE_INVALID; hb_set_next(glyphs, &i);) {
            uint32_t adv = face.getGlyphAdvance(i);
            if (adv == 1000)
                continue;
            glyphAdvances.push_back(QPDFObjectHandle::newInteger(i));
            std::vector<QPDFObjectHandle> advances{};
            while (i <= max) {
                advances.push_back(QPDFObjectHandle::newInteger(adv));
                if (!hb_set_has(glyphs, ++i))
                    break;
                adv = face.getGlyphAdvance(i);
                if (adv == 1000)
                    break;
            }
            glyphAdvances.push_back(QPDFObjectHandle::newArray(advances));
        }
    else
        for (uint32_t i = min; i < max; i++) {
            uint32_t adv = face.getGlyphAdvance(i);
            if (adv == 1000)
                continue;
            glyphAdvances.push_back(QPDFObjectHandle::newInteger(i));
            std::vector<QPDFObjectHandle> advances{};
            while (i <= max) {
                advances.push_back(QPDFObjectHandle::newInteger(adv));
                adv = face.getGlyphAdvance(++i);
                if (adv == 1000)
                    break;
            }
            glyphAdvances.push_back(QPDFObjectHandle::newArray(advances));
        }
    return glyphAdvances;
};
// We embed fonts at the end
void FontManager::embedFonts(bool subset) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution distrib(static_cast<short>('A'), static_cast<short>('Z'));

    for (auto &&[key, font] : fonts) {
        auto &faces = font.getFaces();
        for (auto &&face : faces) {
            if (hb_set_get_population(face.getUsedGlyphs()) == 0)
                continue;

            auto fontDescriptor = document.newIndirectObject(QPDFObjectHandle::newDictionary());
            auto fontDictionary = document.newIndirectObject(QPDFObjectHandle::newDictionary());
            auto descendantFontDictionary = document.newIndirectObject(QPDFObjectHandle::newDictionary());

            std::string name = (subset ? "/AAAAAA+" : "/") + font.getPostScriptName();
            if (subset) {
                for (size_t i = 1; i <= 6; ++i)
                    name[i] = static_cast<char>(distrib(gen));
                name += "Identity-H";
            }

            fontDescriptor.replaceKey("/Type", QPDFObjectHandle::newName("/FontDescriptor"));
            fontDescriptor.replaceKey("/FontBBox", font.getBoundingBox());
            fontDescriptor.replaceKey("/FontName", QPDFObjectHandle::newName(name));
            fontDescriptor.replaceKey("/ItalicAngle", QPDFObjectHandle::newReal(face.getSlantAngle(), 1));
            fontDescriptor.replaceKey("/Ascent", QPDFObjectHandle::newInteger(face.getAscender()));
            fontDescriptor.replaceKey("/Flags", QPDFObjectHandle::newInteger(font.getFlags()));
            fontDescriptor.replaceKey("/Descent", QPDFObjectHandle::newInteger(face.getDescender()));
            fontDescriptor.replaceKey("/CapHeight", QPDFObjectHandle::newInteger(face.getCapHeight()));
            fontDescriptor.replaceKey("/StemV", QPDFObjectHandle::newInteger(10 + (face.getWeight() - 50) * 220 / 900));

            auto CIDSystemInfo = QPDFObjectHandle::newDictionary();
            CIDSystemInfo.replaceKey("/Registry", QPDFObjectHandle::newString("Adobe"));
            CIDSystemInfo.replaceKey("/Ordering", QPDFObjectHandle::newString("Identity"));
            CIDSystemInfo.replaceKey("/Supplement", QPDFObjectHandle::newInteger(0));

            descendantFontDictionary.replaceKey("/Type", QPDFObjectHandle::newName("/Font"));
            descendantFontDictionary.replaceKey(
                "/Subtype", QPDFObjectHandle::newName(font.isCFF() ? "/CIDFontType0" : "/CIDFontType2"));

            if (!font.isCFF())
                descendantFontDictionary.replaceKey("/CIDToGIDMap", QPDFObjectHandle::newName("/Identity"));
            descendantFontDictionary.replaceKey("/BaseFont", QPDFObjectHandle::newName(name));
            descendantFontDictionary.replaceKey("/CIDSystemInfo", CIDSystemInfo);
            descendantFontDictionary.replaceKey("/FontDescriptor", fontDescriptor);

            descendantFontDictionary.replaceKey(
                "/W", document.newIndirectObject(QPDFObjectHandle::newArray(glyphAdvances(face, font, subset))));

            fontDictionary.replaceKey("/Type", QPDFObjectHandle::newName("/Font"));
            fontDictionary.replaceKey("/Subtype", QPDFObjectHandle::newName("/Type0"));
            fontDictionary.replaceKey("/BaseFont", QPDFObjectHandle::newName(name));
            fontDictionary.replaceKey("/Encoding", QPDFObjectHandle::newName("/Identity-H"));

            fontDictionary.replaceKey("/DescendantFonts", QPDFObjectHandle::newArray({descendantFontDictionary}));

            dictionary.replaceKey(face.getHandle(), fontDictionary);
            auto stream = document.newStream();
            auto dict = stream.getDict();
            dict.replaceKey("/Subtype", QPDFObjectHandle::newName("/OpenType"));

            stream.replaceStreamData(
                font.makeSubsetFunction(face, subset), QPDFObjectHandle(), QPDFObjectHandle::newNull());

            fontDescriptor.replaceKey("/FontFile3", stream);
        }
    }
}

// TODO: Add support for vertical writing
std::vector<std::string> FontManager::loadFontFile(std::filesystem::path file, uint32_t startIndex,
                                                   std::optional<uint32_t> maxFonts) {
    BlobHolder blob{file};
    std::vector<std::string> out;
    auto fontCount =
        std::min(hb_face_count(blob), startIndex + maxFonts.value_or(std::numeric_limits<uint16_t>::max()));

    out.reserve(fontCount);
    for (size_t i = startIndex; i < fontCount; ++i) {
        Font font{hb_face_create(blob, i), fonts.size()};
        auto name = font.getName();
        fonts.emplace(std::make_pair(name, std::move(font)));
        out.emplace_back(name);
    }
    return out;
}
