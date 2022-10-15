#define POINTERHOLDER_TRANSITION 3
#include "font.hpp"

#include "fontManager.hpp"
#include "pdf.hpp"

PDFLib::Font::Font(HbFontT *fontHandle, FontManager &manager, size_t index)
    : font{fontHandle},
      blob{hb_face_reference_blob(font)},
      manager{manager},
      headTable{font},
      nameTable{font},
      index{index},
      os2Table{font},
      scale{1000.0 / static_cast<double>(headTable.unitsPerEM)} {};

PDFLib::Face &PDFLib::Font::makeFace() {
    return faces.emplace_back(
        hb_font_create(font), scale, "/F" + std::to_string(index) + "v" + std::to_string(faces.size()));
}
PDFLib::FontHolder PDFLib::Font::makeSubset(Face &face) {
    return hb_subset_or_fail(font, face.getSubsetInput());
}
