#define POINTERHOLDER_TRANSITION 3
#include "fontManager.hpp"
#include "pdf.hpp"

PDFLib::FontManager::FontManager(Document &document)
    : document{document},
      dictionary{document.pdf.makeIndirectObject(QPDFObjectHandle::newDictionary())} {};

void PDFLib::FontManager::embedFonts() {
    for (auto &&[key, font] : fonts) {
        auto &faces = font.getFaces();
        for (auto &&face : faces) {
            BlobHolder blob(hb_face_reference_blob(font.getHbObj()));
            face.embed(blob);
            document.pdf.makeIndirectObject(face.getDictionary());
            document.pdf.makeIndirectObject(face.getDescriptor());
            dictionary.replaceKey(face.getHandle(), face.getDictionary());
        }
    }
}