#define POINTERHOLDER_TRANSITION 3
#include "font.hpp"

#include "fontManager.hpp"
#include "pdf.hpp"

#include <fstream>

PDFLib::Font::Font(HbFontT *fontHandle, size_t index)
    : font{fontHandle},
      blob{hb_face_reference_blob(font)},
      headTable{font},
      index{index},
      os2Table{font},
      postTable{font},
      scale{1000.0 / static_cast<double>(headTable.unitsPerEM)} {
    {
        std::array<uint32_t, 10> tags;
        uint32_t length = 10;
        uint32_t total = 0;
        while (length > 0) {
            hb_face_get_table_tags(font, total, &length, tags.data());
            total += length;
            if (length > 0)
                for (auto tag : tags) {
                    if (tag == Util::tag<"CFF ">)
                        CFF = true;
                }
        }
    }
    if (CFF)
        cffTable.emplace(font);
};

PDFLib::Face &PDFLib::Font::makeFace() {
    return faces.emplace_back(
        hb_font_create(font), scale, "/F" + std::to_string(index) + "v" + std::to_string(faces.size()));
}
std::function<void(Pipeline *)> PDFLib::Font::makeSubsetFunction(Face &face, bool subset) {
    return [&face, this, subset](Pipeline *pipeline) mutable {
        unsigned int length;
        if (subset) {
            hb_subset_input_set_flags(face.getSubsetInput(), HB_SUBSET_FLAGS_RETAIN_GIDS);
            FontHolder subsetFont = hb_subset_or_fail(font, face.getSubsetInput());
            BlobHolder subsetBlob = hb_face_reference_blob(subsetFont);
            pipeline->write(hb_blob_get_data(subsetBlob, &length), length);
        } else
            pipeline->write(hb_blob_get_data(blob, &length), length);

        pipeline->finish();
    };
}
