#if !defined(PDFLIB_DOCUMENT_H)
    #define PDFLIB_DOCUMENT_H
    #include "fontManager.hpp"
    #include "page_descriptor.hpp"
    #include "resource_manager.hpp"

    #include <qpdf/QPDF.hh>
    #include <qpdf/QPDFPageDocumentHelper.hh>
    #include <qpdf/QPDFWriter.hh>

    #include <chrono>
    #include <filesystem>
    #include <iostream>
    #include <set>
    #include <thread>
    #include "page_descriptor.hpp"
namespace pdf_lib {
class Document {
    QPDF pdf;
    page_descriptor default_config;
    std::vector<resource_manager*> resource_managers;
    QPDFPageDocumentHelper pages;
    class Page {
        std::string contents;
        QPDFObjectHandle pageDict;
        Document &doc;
        page_descriptor config;

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

    QPDFObjectHandle resources;
    Document() : pages{(pdf.emptyPDF(), pdf)}, resources{QPDFObjectHandle::newDictionary()}{};
    template <std::derived_from<resource_manager> T>
    auto create_resource_manager(auto&&...args){

    }
    void register_resource_manager(resource_manager& resourceManager){
        resource_managers.push_back(&resourceManager);
    }
    void write(std::filesystem::path path) {
        for(auto* res : resource_managers)
            res->embed(*this);
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

    Page createPage() {
        return Page(*this);
    };
};
} // namespace pdf_lib
#endif // PDFLIB_DOCUMENT_H
