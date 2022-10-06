#include "face.hpp"
#include "font.hpp"
#include "fonts.hpp"
#include "pdf.hpp"

#include <iostream>

PDFLib::Face::Face(HbFaceT *faceHandle, Font &font)
    : glyphSet{subsetInput},
      face{faceHandle},
      font{font},
      dictionary{font.getManager().getDocument().makeIndirectObject("<<"
                                                                    " /Type /Font"
                                                                    " /Subtype /TrueType"
                                                                    " /Encoding /WinAnsiEncoding"
                                                                    ">>"_qpdf)},
      descriptor{font.getManager().getDocument().makeIndirectObject("<<"
                                                                    " /Type /FontDescriptor"
                                                                    ">>"_qpdf)} {
    descriptor.replaceKey("/FontBBox", font.getBoundingBox());
    descriptor.replaceKey("/FontName", QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
    descriptor.replaceKey("/ItalicAngle", QPDFObjectHandle::newReal(getSlantAngle(), 1));
    descriptor.replaceKey("/Ascent", QPDFObjectHandle::newInteger(getAscender()));
    descriptor.replaceKey("/Flags", QPDFObjectHandle::newInteger(1 << 5));
    descriptor.replaceKey("/Descent", QPDFObjectHandle::newInteger(getDescender()));
    descriptor.replaceKey("/CapHeight", QPDFObjectHandle::newInteger(getCapHeight()));
    descriptor.replaceKey("/StemV", QPDFObjectHandle::newInteger(10 + (getWeight() - 50) * 220 / 900));

    dictionary.replaceKey("/BaseFont", QPDFObjectHandle::newName("/" + std::string(font.getPostScriptName())));
    dictionary.replaceKey("/FirstChar", QPDFObjectHandle::newInteger(0));
    dictionary.replaceKey("/LastChar", QPDFObjectHandle::newInteger(font.getLastCharIndex()));
    std::vector<QPDFObjectHandle> glyphAdvances;
    for (size_t i = 0; i <= font.getLastCharIndex(); ++i) {
        hb_codepoint_t glyph;
        hb_font_get_glyph(face, i, 0, &glyph);
        glyphAdvances.push_back(
            QPDFObjectHandle::newInteger(hb_font_get_glyph_h_advance(face, glyph) * font.getScale()));
    }
    dictionary.replaceKey("/Widths", QPDFObjectHandle::newArray(glyphAdvances));
    dictionary.replaceKey("/FontDescriptor", descriptor);
    font.getManager().addDictionaryEntry(dictionary);
};

void PDFLib::Face::embed(HbBlobT *blob) {
    hb_subset_input_set_flags(subsetInput, HB_SUBSET_FLAGS_GLYPH_NAMES);

    FontHolder newFont{hb_subset_or_fail(font.getHbObj(), subsetInput)};

    unsigned int length;
    auto data =
        std::make_shared<Buffer>((unsigned char *)(hb_blob_get_data(hb_face_reference_blob(newFont), &length)), length);
    auto stream = QPDFObjectHandle::newStream(&font.getManager().getDocument().pdf);
    stream.replaceStreamData(data, QPDFObjectHandle(), QPDFObjectHandle::newNull());
    auto dict = stream.getDict();
    dict.replaceKey("/Length1", QPDFObjectHandle::newInteger(length));
    descriptor.replaceKey("/FontFile2", stream);
}

int PDFLib::Face::getAscender() {
    hb_font_extents_t extents;
    hb_font_get_h_extents(face, &extents);
    return extents.ascender * font.getScale();
}

int PDFLib::Face::getDescender() {
    hb_font_extents_t extents;
    hb_font_get_h_extents(face, &extents);
    return extents.descender * font.getScale();
}

float PDFLib::Face::getItalicAngle() {
    return hb_style_get_value(face, HB_STYLE_TAG_SLANT_ANGLE) * font.getScale();
}

int PDFLib::Face::getCapHeight() {
    int32_t out;
    hb_ot_metrics_get_position(face, HB_OT_METRICS_TAG_CAP_HEIGHT, &out);
    return out * font.getScale();
}

std::pair<int, std::string> PDFLib::Face::shape(std::string text, float points) {
    BufferHolder buffer;
    hb_buffer_add_utf8(buffer, text.c_str(), -1, 0, -1);
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buffer, hb_language_from_string("en", -1));
    hb_shape(face, buffer, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
    int width = 0;
    std::string out;
    out.reserve(text.length() * 2);
    out.push_back('[');
    out.push_back('(');
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid = glyph_info[i].codepoint;
        hb_position_t x_advance = glyph_pos[i].x_advance;
        hb_position_t error = hb_font_get_glyph_h_advance(face, glyphid) - x_advance;
        hb_set_add(glyphSet, glyphid);
        width += x_advance;
        out.push_back(text[i]);
        if (error) {
            out.push_back(')');
            out += std::to_string(error);
            out.push_back('(');
        }
    }
    out += ")]";
    return std::make_tuple(width * points, out);
};
