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
    std::map<std::string, Font> fonts;
    Document &document;
    QPDFObjectHandle dictionary;

  public:
    FontManager(Document &document);

    decltype(fonts) &getFonts() {
        return fonts;
    }

    Font &getFromName(std::string fullName) {
        return fonts.at(fullName);
    }

    Document &getDocument() {
        return document;
    }

    QPDFObjectHandle &getDictionary() {
        return dictionary;
    }

    std::vector<std::string> loadFontFile(std::filesystem::path file, uint32_t startIndex = 0,
                                          std::optional<uint32_t> maxFonts = std::nullopt);
    void embedFonts(bool subset = true);
};
}; // namespace PDFLib
#endif
