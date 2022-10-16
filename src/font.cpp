#define POINTERHOLDER_TRANSITION 3
#include "font.hpp"

#include "fontManager.hpp"
#include "pdf.hpp"

#include <qpdf/Pipeline.hh>

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
std::function<void(Pipeline *)> PDFLib::Font::makeSubsetFunction(Face &face, QPDFObjectHandle dict) {
    return [&face, this, dict](Pipeline *pipeline) mutable {
        FontHolder subsetFont = hb_subset_or_fail(font, face.getSubsetInput());
        BlobHolder subsetBlob = hb_face_reference_blob(subsetFont);

        unsigned int length;
        pipeline->write(hb_blob_get_data(subsetBlob, &length), length);
        pipeline->finish();
        dict.replaceKey("/Length1", QPDFObjectHandle::newInteger(length));
    };
}
