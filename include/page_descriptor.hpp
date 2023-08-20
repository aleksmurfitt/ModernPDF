//
// Created by Aleksandur Murfitt on 11/08/2023.
//

#ifndef PDF_GENERATOR_PAGE_DESCRIPTOR_HPP
#define PDF_GENERATOR_PAGE_DESCRIPTOR_HPP
#include "charconv"
#include "fmt/format.h"
#include "graphics.hpp"
namespace pdf_lib {
class page_descriptor {
    graphics::color::rgb color{"#ffffff"};
};
}
#endif // PDF_GENERATOR_PAGE_DESCRIPTOR_HPP
