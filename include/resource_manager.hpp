//
// Created by Aleksandur Murfitt on 12/08/2023.
//
#include "pdf.hpp"
#ifndef PDF_GENERATOR_RESOURCE_MANAGER_HPP
#define PDF_GENERATOR_RESOURCE_MANAGER_HPP
namespace pdf_lib {
class resource_manager {
  public:
    virtual void embed(Document&) = 0;
};
}
#endif // PDF_GENERATOR_RESOURCE_MANAGER_HPP
