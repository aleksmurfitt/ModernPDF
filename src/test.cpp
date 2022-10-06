#include "pdf.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

using namespace PDFLib;
constexpr auto timer = [](auto &&func, auto &&...params) -> std::chrono::duration<double, std::milli> {
    // get time before function invocation
    const auto &start = std::chrono::high_resolution_clock::now();
    // function invocation using perfect forwarding
    std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
    // get time after function invocation
    const auto &stop = std::chrono::high_resolution_clock::now();
    return stop - start;
};

inline void test() {
    Document d{};
    d.makeFontTest();
    d.makePageTest();
    d.write("/Users/aleks/workbench/C++/pdf-generator v2/test.pdf");
}

int main() {
    std::vector<double> times;
    for (size_t i = 0; i < 1; i++) {
        /* code */
        times.push_back(timer(test).count());
    }
    std::cout << *std::min_element(std::begin(times), std::end(times)) << std::endl;
    std::cout << std::reduce(std::begin(times), std::end(times)) / times.size() << std::endl;
    return 0;
};