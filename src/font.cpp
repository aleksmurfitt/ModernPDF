#include "font.hpp"
#include "fonts.hpp"
#include "pdf.hpp"

PDFLib::Font::Font(HbFontT *fontHandle, FontManager &manager)
    : font{fontHandle},
      manager{manager},
      headTable{font},
      nameTable{font},
      os2Table{font} {};

PDFLib::Face &PDFLib::Font::makeFace() {
    HbFaceT *pFace = hb_font_create(font);
    return faces.emplace_back(pFace, *this);
}

PDFLib::FontManager::FontManager(Document &document)
    : document{document},
      dictionary{document.pdf.makeIndirectObject(QPDFObjectHandle::newDictionary())} {};