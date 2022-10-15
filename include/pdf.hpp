#if !defined(PDFLIB_DOCUMENT_H)
    #define PDFLIB_DOCUMENT_H
    #include "config.hpp"
    #include "fontManager.hpp"

    #include <qpdf/QPDF.hh>
    #include <qpdf/QPDFPageDocumentHelper.hh>
    #include <qpdf/QPDFWriter.hh>

    #include <chrono>
    #include <filesystem>
    #include <format>
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
        std::filesystem::path fontDir(Config::fonts);

        for (auto &&file : std::filesystem::directory_iterator(fontDir)) {
            if (std::set<std::string>({".ttf", ".ttc", ".otf"}).contains(file.path().extension()))
                fontManager.loadFontFile(file);
        }

        auto line = [](float xMin, float yMin, float xMax, float yMax) -> std::string {
            return (" " + std::to_string(xMin) + " " + std::to_string(yMin) + " m " + std::to_string(xMax) + " " +
                    std::to_string(yMax) + " l ");
        };

        float points = 20;
        float lineHeight = 1.14;
        size_t index = 0;
        auto &fonts = fontManager.getFonts();
        float ytotal = 0;
        bool left = true;
        float max = 0;
        for (auto &&[name, font] : fonts) {
            auto &face = font.makeFace();
            float above = static_cast<float>(face.getAscender()) * points / 1000.0;
            float below = static_cast<float>(face.getDescender()) * points / -1000.0;
            if (left)
                max = (above + below);
            else
                ytotal += (max < (above + below) ? (above + below) : max) * lineHeight;
            left = !left;
        }
        float ypos = (842 - ytotal) / 2;
        left = true;
        max = 0;
        for (auto &&[name, font] : fonts) {
            auto &face = font.getFaces()[0];
            auto &&[width, _, text] = face.shape(name, points);
            float above = static_cast<float>(face.getAscender()) * points / 1000.0;
            float below = static_cast<float>(face.getDescender()) * points / -1000.0;
            float height = above + below;
            float startx = left ? 25 : 595 - 25 - width;
            contents += "BT " + face.getHandle() + " " + std::to_string(points) + " Tf 0.9867 0.9867 0.9867 rg 0 Tr " +
                        std::to_string(startx) + " " + std::to_string(842 - above - ypos) + " Td " + text +
                        " TJ ET q 0.5 w [1 2] 0 d 0.5867 0.5867 0.5867 RG" +
                        line(startx, 842 - ypos - height, startx, 842 - ypos) +
                        line(startx + width, 842 - ypos, startx + width, 842 - height - ypos) +
                        line(startx, 842 - ypos - height, startx + width, 842 - ypos - height) +
                        line(startx, 842 - ypos, startx + width, 842 - ypos) + " S Q ";
            index++;
            if (left)
                max = height;
            else
                ypos += (max < height ? height : max) * lineHeight;
            left = !left;
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
        QPDFWriter w(pdf, path.c_str());
        w.setCompressStreams(false);
        w.setPreserveUnreferencedObjects(true);
        w.write();
    }

    QPDFObjectHandle makeIndirectObject(const QPDFObjectHandle &obj) {
        return pdf.makeIndirectObject(obj);
    }
};
} // namespace PDFLib
#endif // PDFLIB_DOCUMENT_H
