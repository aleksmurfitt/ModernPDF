#ifndef PDFLIB_FONTS_H
#define PDFLIB_FONTS_H
#include "font.hpp"
#include "private/font-tables/parser.hpp"
#include "private/harfbuzz-helpers.hpp"
#include "private/types/font.hpp"

#include <hb-ot.h>
#include <hb.h>
#include <qpdf/Buffer.hh>
#include <qpdf/QPDFObjectHandle.hh>

#include <climits>
#include <filesystem>
#include <string_view>
#include <vector>

namespace PDFLib {
class Document;

// The face <-> font swap is intentional to better reflect what the types represent

// TODO Add variations on faces
class FontManager {
    // Should provide a getFontsDict method to get the
    std::vector<BlobHolder> blobs;
    std::map<std::string, Font> fonts;
    Document &document;
    QPDFObjectHandle dictionary;
    size_t dictionaryEntries = 0;

  public:
    FontManager(Document &document);

    decltype(fonts) &getFonts() {
        return fonts;
    }

    Font &getFromFullName(std::string fullName) {
        return fonts.at(fullName);
    }

    std::vector<std::string> loadFontFile(std::filesystem::path file) {
        auto &blob = blobs.emplace_back(file);
        auto fontCount = hb_face_count(blob);
        std::vector<std::string> out;
        out.reserve(fontCount);
        for (size_t i = 0; i < fontCount; ++i) {
            auto font = Font(hb_face_create(blob, i), *this, fonts.size());
            auto name = font.getName();
            fonts.emplace(std::make_pair(name, std::move(font)));
            out.emplace_back(name);
        }
        return out;
    }

    Document &getDocument() {
        return document;
    }

    void embedFonts();

    QPDFObjectHandle &getDictionary() {
        return dictionary;
    }
};
}; // namespace PDFLib
#endif
