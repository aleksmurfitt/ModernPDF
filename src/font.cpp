#include "font.hpp"
#include "fontManager.hpp"
#include "pdf.hpp"

PDFLib::Font::Font(HbFontT *fontHandle, FontManager &manager, size_t index)
    : font{fontHandle},
      manager{manager},
      headTable{font},
      nameTable{font},
      index{index},
      os2Table{font} {};

PDFLib::Face &PDFLib::Font::makeFace() {
    HbFaceT *pFace = hb_font_create(font);
    return faces.emplace_back(pFace, *this, "/F" + std::to_string(index) + "v" + std::to_string(faces.size()));
}