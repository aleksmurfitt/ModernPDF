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
#include "resource_manager.hpp"
namespace pdf_lib {
class Document;

// The face <-> font swap is intentional to better reflect what the types represent

// TODO Add variations on faces
class FontManager : public resource_manager{
    // Should provide a getFontsDict method to get the
    std::map<std::string, Font> fonts;
    QPDFObjectHandle dictionary;
    bool subset;
  public:
    explicit FontManager(bool subset_fonts = true);

    decltype(fonts) &getFonts() {
        return fonts;
    }

    Font &get_from_name(const std::string& full_name) {
        return fonts.at(full_name);
    }

    std::vector<std::string> loadFontFile(std::filesystem::path file, uint32_t startIndex = 0,
                                          std::optional<uint32_t> maxFonts = std::nullopt);
    void embed(Document& document);
};
}; // namespace pdf_lib
#endif
