#define POINTERHOLDER_TRANSITION 3
#include "face.hpp"

#include "font.hpp"
#include "fontManager.hpp"
#include "pdf.hpp"

#include <iostream>
using namespace PDFLib;
Face::Face(FaceHolder face, double scale, std::string handle)
    : glyphSet{subsetInput},
      face{std::move(face)},
      scale{scale},
      handle{handle} {
    hb_subset_input_set_flags(subsetInput, HB_SUBSET_FLAGS_GLYPH_NAMES);
};

double Face::getAscender() {
    hb_font_extents_t extents;
    hb_font_get_h_extents(face, &extents);
    return static_cast<double>(extents.ascender) * scale;
}

double Face::getDescender() {
    hb_font_extents_t extents;
    hb_font_get_h_extents(face, &extents);
    return static_cast<double>(extents.descender) * scale;
}

float Face::getItalicAngle() {
    return hb_style_get_value(face, HB_STYLE_TAG_SLANT_ANGLE);
}

int Face::getCapHeight() {
    int32_t out;
    hb_ot_metrics_get_position(face, HB_OT_METRICS_TAG_CAP_HEIGHT, &out);
    return out * scale;
}

std::pair<float, std::string> Face::shape(std::string text, float points) {
    BufferHolder buffer;
    hb_buffer_add_utf8(buffer, text.c_str(), -1, 0, -1);
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buffer, hb_language_from_string("en", -1));
    hb_shape(face, buffer, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
    float width = 0;
    float height = getAscender() - getDescender();
    std::string out;
    out.reserve(text.length() * 2);
    out.push_back('[');
    out.push_back('(');
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid = glyph_info[i].codepoint;
        hb_position_t x_advance = glyph_pos[i].x_advance;
        hb_position_t error = hb_font_get_glyph_h_advance(face, glyphid) - x_advance;
        hb_set_add(glyphSet, text[i]);
        width += x_advance;
        out.push_back(text[i]);
        if (error) {
            out.push_back(')');
            out += std::to_string((int32_t)(error * scale));
            out.push_back('(');
        }
    }
    out += ")]";
    return std::make_tuple(width * scale * points / 1000.0, out);
};
