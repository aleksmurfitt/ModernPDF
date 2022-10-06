#ifndef PDFLIB_HARFBUZZ_HELPERS_H
#define PDFLIB_HARFBUZZ_HELPERS_H
#include <hb-subset.h>
#include <hb.h>

#include <filesystem>
#include <memory>

namespace PDFLib {
using HbFontT = hb_face_t;
using HbFaceT = hb_font_t;
using HbBlobT = hb_blob_t;
using HbBufferT = hb_buffer_t;
using HbSetT = hb_set_t;
using HbSubsetInputT = hb_subset_input_t;

template <typename T, auto deleter, typename U = std::nullptr_t, auto initialiser = nullptr, bool owning = true>
class HbHolder {
    static constexpr auto HbDestroyer = [](T *ptr) {
        if constexpr (owning)
            deleter(ptr);
    };
    using SafeT = std::unique_ptr<T, decltype(HbDestroyer)>;
    SafeT object;

    static constexpr auto Init = [](U object) -> auto{
        if constexpr (std::is_same_v<decltype(initialiser), std::nullptr_t>)
            return object;
        else if constexpr (!std::is_same_v<U, std::nullptr_t>)
            return initialiser(object);
    };

  public:
    template <typename W = U, typename std::enable_if<!std::is_same_v<W, std::nullptr_t>, int>::type = 0>
    HbHolder(U object) : object{Init(object)} {};

    template <typename W = U, typename std::enable_if<std::is_same_v<W, std::nullptr_t>, int>::type = 0>
    HbHolder() : object{initialiser()} {};

    operator T *() {
        return object.get();
    }
};

using FaceHolder = HbHolder<HbFaceT, hb_font_destroy, HbFaceT *>;
using FontHolder = HbHolder<HbFontT, hb_face_destroy, HbFontT *>;
using BlobHolder = HbHolder<HbBlobT, hb_blob_destroy, std::filesystem::path,
                            ([](auto fontFile) -> auto{ return hb_blob_create_from_file(fontFile.c_str()); })>;
using BufferHolder = HbHolder<HbBufferT, hb_buffer_destroy, std::nullptr_t, hb_buffer_create>;
template <bool owning = true>
using GlyphSetHolder = HbHolder<HbSetT, hb_set_destroy, HbSubsetInputT *, hb_subset_input_glyph_set, owning>;
template <bool owning = true>
using UnicodeSetHolder = HbHolder<HbSetT, hb_set_destroy, HbSubsetInputT *, hb_subset_input_unicode_set, owning>;

using SubsetInputHolder =
    HbHolder<HbSubsetInputT, hb_subset_input_destroy, std::nullptr_t, hb_subset_input_create_or_fail>;

} // namespace PDFLib

#endif