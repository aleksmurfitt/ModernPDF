#if !defined(PDFLIB_UTIL_H)
    #define PDFLIB_UTIL_H
    #include <memory>

namespace PDFLib {
namespace Util {
template <typename T, auto deleter, auto initialiser = nullptr, bool owning = true> class PtrHolder {
    using SafeT = std::unique_ptr<T, decltype([](T *ptr) {
                                      if constexpr (owning)
                                          deleter(ptr);
                                  })>;
    SafeT object;

  public:
    // Basically a std::make_unique for C functions
    // Enable if initialiser is set
    template <auto W = initialiser,
              typename std::enable_if<!std::is_same_v<decltype(W), std::nullptr_t>, int>::type = 0>
    PtrHolder(auto &&...args) : object{initialiser(std::forward<decltype(args)>(args)...)} {};

    PtrHolder(T *object) : object{object} {};

    operator T *() {
        return object.get();
    }
};
} // namespace Util
} // namespace PDFLib

#endif // PDFLIB_UTIL_H
