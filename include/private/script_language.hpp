//
// Created by Aleksandur Murfitt on 11/08/2023.
//

#ifndef PDF_GENERATOR_SCRIPT_LANGUAGE_HPP
#define PDF_GENERATOR_SCRIPT_LANGUAGE_HPP
#include <string_view>
#include <ranges>

namespace pdf_lib {
class iso_script_tag {
    static inline constexpr auto data = [] () constexpr{
      namespace sr = std::ranges;
      namespace sv = std::views;
      // Filthy hack to read the file at compile time - will be replaced with std::embed soon
      constexpr std::string_view data {
#include "iso15924.txt"
      };
      std::array<std::pair<std::string_view, std::string_view>, sr::count(data, '\n')> pairs;
      auto count = 0;
      for(auto line: data | sv::split('\n')){
          if(line.empty() || line.front() == '#') continue;
          auto chunks = sv::split(line, ';');
          auto first = std::string_view {chunks.front().begin(), chunks.front().end()};
          auto third = chunks | sv::drop(2);
          auto second = std::string_view {third.front().begin(), third.front().end() };
          pairs[count++] = std::pair{second, first};
      }
      sr::sort(pairs, [](auto& p1, auto& p2){return p1.first < p2.first;});
      return pairs;
    }();
    static constexpr char lower (char c){
        if(c >= 'A' && c <= 'Z')
            return 'a' + (c - 'A');
        return c;
    };
    static constexpr bool case_insensitive_compare(std::string_view l, std::string_view r){
        return std::ranges::lexicographical_compare(l, r, [&](char f, char s){
            return lower(f) < lower(s);
        });
    }
  public:
    std::string_view iso_tag;
    // This is implicit by design - the conversion should be invisible to the user
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    consteval iso_script_tag(std::string_view script_name){
        namespace sv = std::views;
        namespace sr = std::ranges;
        auto result = sr::lower_bound(data, script_name, [](auto l, auto r){
            return case_insensitive_compare(l, r);
        }, [](auto&p){return p.first;});
        if(result == data.end() || sr::mismatch(result->first, script_name, {}, lower, lower).in2 != script_name.end())
            throw std::invalid_argument("Invalid script");
        iso_tag = result->second;
    }
    consteval iso_script_tag(const char * script_name) : iso_script_tag(std::string_view{script_name}){};
#pragma clang diagnostic pop
};
}
#endif // PDF_GENERATOR_SCRIPT_LANGUAGE_HPP
