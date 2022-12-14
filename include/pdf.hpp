#if !defined(PDFLIB_DOCUMENT_H)
    #define PDFLIB_DOCUMENT_H
    #include "config.hpp"
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
    QPDF pdf;
    QPDFPageDocumentHelper pages;
    QPDFObjectHandle resources;
    FontManager fontManager;

    class Page {
        std::string contents = "";
        QPDFObjectHandle pageDict;
        Document &doc;

      public:
        Page(Document &doc) : doc{doc}, pageDict{doc.newIndirectObject(QPDFObjectHandle::newDictionary())} {
            pageDict.replaceKey("/Type", QPDFObjectHandle::newName("/Page"));
            pageDict.replaceKey("/MediaBox", QPDFObjectHandle::newArray(QPDFObjectHandle::Rectangle(0, 0, 600, 850)));
            pageDict.replaceKey("/CropBox", QPDFObjectHandle::newArray(QPDFObjectHandle::Rectangle(2.5, 4, 595, 842)));
            pageDict.replaceKey("/Resources", doc.resources);
            doc.pages.addPage(pageDict, false);
        }
        void setContents(const std::string &contents) {
            pageDict.replaceKey("/Contents", QPDFObjectHandle::newStream(&doc.pdf, contents));
        }
    };

  public:
    Document() : pages{(pdf.emptyPDF(), pdf)}, resources{QPDFObjectHandle::newDictionary()}, fontManager{*this} {};

    void finish() {
        fontManager.embedFonts(true);
        resources.replaceKey("/Font", fontManager.getDictionary());
    }

    void write(std::filesystem::path path) {
        QPDFWriter w(pdf, path.c_str());
        w.setCompressStreams(false);
        w.setPreserveUnreferencedObjects(false);
        w.setLinearization(false);
        w.setMinimumPDFVersion(PDFVersion(1, 6));
        w.write();
    }

    QPDFObjectHandle newIndirectObject(const QPDFObjectHandle &obj) {
        return pdf.makeIndirectObject(obj);
    }

    QPDFObjectHandle newStream() {
        return QPDFObjectHandle::newStream(&pdf);
    }

    FontManager &getFontManager() {
        return fontManager;
    }

    Page createPage() {
        return Page(*this);
    };
};
} // namespace PDFLib
#endif // PDFLIB_DOCUMENT_H
