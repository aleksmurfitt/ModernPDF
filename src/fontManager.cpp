#define POINTERHOLDER_TRANSITION 3
#include "fontManager.hpp"

#include "pdf.hpp"
using namespace PDFLib;

FontManager::FontManager(Document &document)
    : document{document},
      dictionary{document.newIndirectObject(QPDFObjectHandle::newDictionary())} {};

void FontManager::embedFonts() {
    for (auto &&[key, font] : fonts) {
        auto &faces = font.getFaces();
        for (auto &&face : faces) {
            auto fontDescriptor = document.newIndirectObject(QPDFObjectHandle::newDictionary());
            auto fontDictionary = document.newIndirectObject(QPDFObjectHandle::newDictionary());
            auto descendantFontDictionary = document.newIndirectObject(QPDFObjectHandle::newDictionary());

            fontDescriptor.replaceKey("/Type", QPDFObjectHandle::newName("/FontDescriptor"));
            fontDescriptor.replaceKey("/FontBBox", font.getBoundingBox());
            fontDescriptor.replaceKey("/FontName",
                                      QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
            fontDescriptor.replaceKey("/ItalicAngle", QPDFObjectHandle::newReal(face.getSlantAngle(), 1));
            fontDescriptor.replaceKey("/Ascent", QPDFObjectHandle::newInteger(face.getAscender()));
            fontDescriptor.replaceKey("/Flags", QPDFObjectHandle::newInteger(font.getFlags()));
            fontDescriptor.replaceKey("/Descent", QPDFObjectHandle::newInteger(face.getDescender()));
            fontDescriptor.replaceKey("/CapHeight",
                                      QPDFObjectHandle::newInteger(face.getCapHeight() || face.getAscender()));
            fontDescriptor.replaceKey("/StemV", QPDFObjectHandle::newInteger(10 + (face.getWeight() - 50) * 220 / 900));

            descendantFontDictionary.replaceKey("/Type", QPDFObjectHandle::newName("/Font"));
            descendantFontDictionary.replaceKey("/Subtype", QPDFObjectHandle::newName("/CIDFontType2"));
            descendantFontDictionary.replaceKey("/CIDToGIDMap", QPDFObjectHandle::newName("/Identity"));
            descendantFontDictionary.replaceKey("/BaseFont",
                                                QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
            descendantFontDictionary.replaceKey("/CIDSystemInfo",
                                                "<< /Registry (Adobe) /Ordering (Identity) /Supplement 0 >>"_qpdf);
            descendantFontDictionary.replaceKey("/FontDescriptor", fontDescriptor);

            std::vector<QPDFObjectHandle> glyphAdvances{};
            for (size_t i = 0; i <= hb_set_get_max(face.getUsedGlyphs()); ++i)
                glyphAdvances.push_back(QPDFObjectHandle::newInteger(face.getGlyphAdvance(i)));

            descendantFontDictionary.replaceKey(
                "/W",
                document.newIndirectObject(QPDFObjectHandle::newArray(
                    {QPDFObjectHandle::newInteger(0), QPDFObjectHandle::newArray(glyphAdvances)})));

            fontDictionary.replaceKey("/Type", QPDFObjectHandle::newName("/Font"));
            fontDictionary.replaceKey("/Subtype", QPDFObjectHandle::newName("/Type0"));
            fontDictionary.replaceKey("/BaseFont",
                                      QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
            fontDictionary.replaceKey("/Encoding", QPDFObjectHandle::newName("/Identity-H"));

            fontDictionary.replaceKey("/DescendantFonts", QPDFObjectHandle::newArray({descendantFontDictionary}));

            dictionary.replaceKey(face.getHandle(), fontDictionary);
            auto stream = document.newStream();
            auto dict = stream.getDict();
            stream.replaceStreamData(
                font.makeSubsetFunction(face, dict), QPDFObjectHandle(), QPDFObjectHandle::newNull());
            fontDescriptor.replaceKey("/FontFile2", stream);
        }
    }
}

// TODO: Add support for vertical writing
std::vector<std::string> FontManager::loadFontFile(std::filesystem::path file) {
    BlobHolder blob{file};
    std::vector<std::string> out;
    auto fontCount = hb_face_count(blob);
    out.reserve(fontCount);
    for (size_t i = 0; i < fontCount; ++i) {
        auto font = Font(hb_face_create(blob, i), *this, fonts.size());
        auto name = font.getName();
        fonts.emplace(std::make_pair(name, std::move(font)));
        out.emplace_back(name);
    }
    return out;
}
