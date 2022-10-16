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
            auto fontDescriptor = document.newIndirectObject(("<<"
                                                              " /Type /FontDescriptor"
                                                              ">>"_qpdf));
            auto fontDictionary = document.newIndirectObject(("<<"
                                                              " /Type /Font"
                                                              " /Subtype /TrueType"
                                                              " /Encoding /WinAnsiEncoding"
                                                              ">>"_qpdf));

            fontDescriptor.replaceKey("/FontBBox", font.getBoundingBox());
            fontDescriptor.replaceKey("/FontName",
                                      QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
            fontDescriptor.replaceKey("/ItalicAngle", QPDFObjectHandle::newReal(face.getSlantAngle(), 1));
            fontDescriptor.replaceKey("/Ascent", QPDFObjectHandle::newInteger(face.getAscender()));
            fontDescriptor.replaceKey("/Flags", QPDFObjectHandle::newInteger(1 << 5));
            fontDescriptor.replaceKey("/Descent", QPDFObjectHandle::newInteger(face.getDescender()));
            fontDescriptor.replaceKey("/CapHeight", QPDFObjectHandle::newInteger(face.getCapHeight()));
            fontDescriptor.replaceKey("/StemV", QPDFObjectHandle::newInteger(10 + (face.getWeight() - 50) * 220 / 900));

            fontDictionary.replaceKey("/BaseFont",
                                      QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
            fontDictionary.replaceKey("/FontDescriptor", fontDescriptor);

            dictionary.replaceKey(face.getHandle(), fontDictionary);

            fontDictionary.replaceKey("/FirstChar", QPDFObjectHandle::newInteger(hb_set_get_min(face.getUsedGlyphs())));
            fontDictionary.replaceKey("/LastChar", QPDFObjectHandle::newInteger(hb_set_get_max(face.getUsedGlyphs())));
            std::vector<QPDFObjectHandle> glyphAdvances;

            for (size_t i = hb_set_get_min(face.getUsedGlyphs()); i <= hb_set_get_max(face.getUsedGlyphs()); ++i) {
                hb_codepoint_t codepoint = 0;
                if (hb_set_has(face.getUsedGlyphs(), i))
                    codepoint = i;
                glyphAdvances.push_back(QPDFObjectHandle::newInteger(face.getGlyphAdvance(face.getGlyph(codepoint))));
            }
            fontDictionary.replaceKey("/Widths", QPDFObjectHandle::newArray(glyphAdvances));

            auto stream = document.newStream();
            auto dict = stream.getDict();
            stream.replaceStreamData(
                font.makeSubsetFunction(face, dict), QPDFObjectHandle(), QPDFObjectHandle::newNull());
            fontDescriptor.replaceKey("/FontFile2", stream);
        }
    }
}

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
