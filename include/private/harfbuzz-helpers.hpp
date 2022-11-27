#ifndef PDFLIB_HARFBUZZ_HELPERS_H
#define PDFLIB_HARFBUZZ_HELPERS_H
#include <hb-subset.h>
#include <hb.h>

#include <filesystem>
#include <iostream>
#include <memory>
namespace PDFLib {
using HbFontT = hb_face_t;
using HbFaceT = hb_font_t;
using HbBlobT = hb_blob_t;
using HbBufferT = hb_buffer_t;
using HbSetT = hb_set_t;
using HbSubsetInputT = hb_subset_input_t;
struct nullarg_t { };
inline constexpr nullarg_t nullarg;
template <auto arg> inline constexpr bool is_nullarg_v = std::is_same_v<std::remove_cv_t<decltype(arg)>, nullarg_t>;
struct HarfbuzzError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
} // namespace PDFLib
namespace detail {
using ::PDFLib::HarfbuzzError;
inline constexpr auto bufferInit = [](std::filesystem::path fontFile) -> auto{
    if (auto blob = hb_blob_create_from_file_or_fail(fontFile.c_str()); blob != NULL) {
        return blob;
    }
    throw HarfbuzzError(std::string("Failed to load font file: ") + fontFile.stem().c_str());
};
} // namespace detail
namespace PDFLib {
template <typename T, auto deleter = nullarg, auto initialiser = nullarg> class PtrHolder {
    struct PtrDeleter {
        void operator()(T *ptr) {
            if constexpr (!is_nullarg_v<deleter>)
                deleter(ptr);
        }
    };

    using SafeT = std::unique_ptr<T, PtrDeleter>;
    SafeT object;

  public:
    // Basically a std::make_unique for C functions
    // Enable if initialiser is set
    template <auto W = initialiser, typename std::enable_if<!is_nullarg_v<W>, int>::type = 0>
    PtrHolder(auto &&...args) : object{initialiser(std::forward<decltype(args)>(args)...)} {};

    PtrHolder(T *object) : object{object} {};

    operator T *() {
        return object.get();
    }
};

using FaceHolder = PtrHolder<HbFaceT, hb_font_destroy>;
using FontHolder = PtrHolder<HbFontT, hb_face_destroy>;
using BlobHolder = PtrHolder<HbBlobT, hb_blob_destroy, detail::bufferInit>;
using BufferHolder = PtrHolder<HbBufferT, hb_buffer_destroy, hb_buffer_create>;

using GlyphSetHolder = PtrHolder<HbSetT, nullarg, hb_subset_input_glyph_set>;
using UnicodeSetHolder = PtrHolder<HbSetT, nullarg, hb_subset_input_unicode_set>;
using SubsetSetHolder = PtrHolder<HbSetT, nullarg, hb_subset_input_set>;

using SetHolder = PtrHolder<HbSetT, hb_set_destroy, hb_set_create>;

using SubsetInputHolder = PtrHolder<HbSubsetInputT, hb_subset_input_destroy, hb_subset_input_create_or_fail>;

} // namespace PDFLib

#endif
