#define POINTERHOLDER_TRANSITION 3
#include "face.hpp"

#include "font.hpp"
#include "pdf.hpp"

#include <qpdf/QUtil.hh>

#include <iostream>
#include <utility>
using namespace PDFLib;
Face::Face(FaceHolder face, double scale, std::string handle)
    : glyphSet{subsetInput},
      charSet{},
      face{std::move(face)},
      scale{scale},
      handle{std::move(handle)} {};

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
double Face::getLineGap() {
    hb_font_extents_t extents;
    hb_font_get_h_extents(face, &extents);
    return static_cast<double>(extents.line_gap) * scale;
}

float Face::getItalicAngle() {
    return hb_style_get_value(face, HB_STYLE_TAG_SLANT_ANGLE);
}

int Face::getCapHeight() {
    int32_t out;
    hb_ot_metrics_get_position(face, HB_OT_METRICS_TAG_CAP_HEIGHT, &out);
    return (out * scale > 1 ? out * scale : getAscender());
}

std::pair<float, std::vector<std::pair<std::vector<uint32_t>, int32_t>>> Face::shape(std::string_view text,
                                                                                     float points, iso_script_tag script,
                                                                                     hb_direction_t direction,
                                                                                     std::string_view language) {
    BufferHolder buffer;
    hb_buffer_add_utf8(buffer, text.begin(), -1, 0, -1);
    hb_buffer_set_direction(buffer, direction);
    hb_buffer_set_script(buffer, static_cast<hb_script_t>(Util::as_tag(script.iso_tag)));
    hb_buffer_set_language(buffer, hb_language_from_string(language.begin(), -1));
    hb_shape(face, buffer, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    for (unsigned int i = 0; i < glyph_count; i++) {
        if (glyph_info[i].codepoint == 0)
            throw std::runtime_error("Required glyph not defined");
    }
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
    float width = 0;
    std::vector<std::pair<std::vector<uint32_t>, int32_t>> runs;
    runs.emplace_back();
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid = glyph_info[i].codepoint;
        hb_position_t x_advance = glyph_pos[i].x_advance;
        hb_position_t error = hb_font_get_glyph_h_advance(face, glyphid) - x_advance;
        hb_set_add(glyphSet, glyphid);
        width += x_advance;
        runs.back().first.push_back(glyphid);
        if (error) {
            runs.back().second = ((int32_t)(error * scale));
            runs.emplace_back();
        }
    }
    return std::make_pair(width * scale * points / 1000.0, runs);
};
