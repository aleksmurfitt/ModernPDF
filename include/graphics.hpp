//
// Created by Aleksandur Murfitt on 16/08/2023.
//

#ifndef PDF_GENERATOR_GRAPHICS_HPP
#define PDF_GENERATOR_GRAPHICS_HPP
namespace pdf_lib::graphics{
namespace detail {
template <typename F, typename I>
concept member_draw = requires(F f, I i){
    f.draw(i);
};
template <typename F, typename I>
concept non_member_draw = requires(F f, I i){
    draw(f, i);
};
template <typename F, typename I>
concept drawable = member_draw<F, I> || non_member_draw<F, I>;
}
struct draw_fn{
    template <typename F, typename I>
    requires detail::non_member_draw<F,I>
    constexpr void operator ()(F&& f, I&& i){
        draw(f, i);
    }
    template <typename F, typename I>
    requires detail::member_draw<F,I>
    constexpr void operator()(F&& f, I&& i){
        f.draw(i);
    }
};
inline constexpr auto draw = draw_fn{};

namespace color {
    struct generic_color {
        constexpr virtual void operator+(std::string& contents) = 0;
    };
    struct rgb : public generic_color {
        double r, g, b;
        constexpr rgb(double r, double g, double b): r{r}, g{g}, b{b}{}
        explicit constexpr rgb(std::string_view hex){
            if((hex.length() != 4 && hex.length() != 7) || hex[0] != '#')
                throw std::invalid_argument("Invalid hex code");

            const auto get_seg = [&](size_t segment){
                const char * start = hex.data() + 1;
                size_t multiplier = hex.length() == 7 ? 2 : 1;
                uint8_t temp = 0;
                auto [ptr, err] = std::from_chars(
                    start + segment * multiplier,
                    start + (segment + 1) * multiplier,
                    temp,
                    16);
                if(err == std::errc::invalid_argument)
                    throw std::invalid_argument("Invalid hex string");
                return static_cast<double>(temp) / 0xFF;
            };
            r = get_seg(0);
            g = get_seg(1);
            b = get_seg(2);
        }
        constexpr void operator+(std::string& contents) override{
            fmt::format_to(std::back_inserter(contents), "{} {} {} rg", r, g, b);
        }
    };
}
struct fill_fn{

};
}


#endif // PDF_GENERATOR_GRAPHICS_HPP
