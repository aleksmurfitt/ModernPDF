#if !defined(PDFLIB_DOCUMENT_H)
    #define PDFLIB_DOCUMENT_H
    #include "fontManager.hpp"

    #include <qpdf/QPDF.hh>
    #include <qpdf/QPDFPageDocumentHelper.hh>
    #include <qpdf/QPDFWriter.hh>

    #include <chrono>
    #include <filesystem>
    #include <iostream>
    #include <set>
    #include <thread>

namespace PDFLib {
class Document {
  public:
    QPDF pdf;

  private:
    QPDFPageDocumentHelper pages;
    QPDFObjectHandle resources;
    FontManager fontManager;
    std::string contents;

  public:
    Document()
        : pages{(pdf.emptyPDF(), pdf)},
          resources{QPDFObjectHandle::newDictionary()},
          fontManager{*this},
          contents{"q 0.18824 0.18824 0.18824 rg 0 0 600 850 re F Q "} {};

    void makeFontTest() {
        int points = 25;
        std::filesystem::path fontDir("/Users/aleks/workbench/C++/pdf-generator v2/fonts");

        for (auto &&file : std::filesystem::directory_iterator(fontDir)) {
            if (file.path().stem().generic_string().find("Gilroy") == std::string::npos)
                fontManager.loadFontFile(file);
        }

        size_t index = 0;
        float ypos = 30;
        for (auto &&[name, font] : fontManager.getFonts()) {
            auto &face = font.makeFace();
            auto &&[width, text] = face.shape(name, points);
            ypos += (static_cast<float>(842 - 74) / fontManager.getFonts().size());
            contents += "BT " + face.getHandle() + " " + std::to_string(points) +
                        " Tf 0.9867 0.9867 0.9867 rg 0 Tr 25 " + std::to_string(842 - ypos) + " Td " + text + " TJ ET ";
            index++;
        }
        fontManager.embedFonts();
        resources.replaceKey("/Font", fontManager.getDictionary());
    }
    void makePageTest() {
        QPDFObjectHandle pageDict = QPDFObjectHandle::newDictionary();
        pageDict.replaceKey("/Type", QPDFObjectHandle::newName("/Page"));
        pageDict.replaceKey("/MediaBox", QPDFObjectHandle::newArray(QPDFObjectHandle::Rectangle(0, 0, 600, 850)));
        pageDict.replaceKey("/CropBox", QPDFObjectHandle::newArray(QPDFObjectHandle::Rectangle(2.5, 3.5, 595, 842)));
        pageDict.replaceKey("/Resources", resources);
        pageDict.replaceKey("/Contents", QPDFObjectHandle::newStream(&pdf, contents));
        QPDFObjectHandle page = pdf.makeIndirectObject(pageDict);
        pages.addPage(page, false);
    }

    void write(std::filesystem::path path) {
        // std::cout << contents << std::endl;
        QPDFWriter w(pdf, path.c_str());
        w.setCompressStreams(true);
        w.setPreserveUnreferencedObjects(true);
        w.write();
    }

    QPDFObjectHandle makeIndirectObject(const QPDFObjectHandle &obj) {
        return pdf.makeIndirectObject(obj);
    }
};
} // namespace PDFLib
#endif // PDFLIB_DOCUMENT_H
