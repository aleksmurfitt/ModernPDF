#define POINTERHOLDER_TRANSITION 3
#include "config.hpp"
#include "pdf.hpp"

#include <algorithm>
#include <array>
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
    for (size_t i = 0; i < 1; i++)
        d.makePageTest();
    d.write(Config::output / "test.pdf");
}

int main() {
    using namespace std::chrono_literals;
    std::array<double, 100> times;
    for (size_t i = 0; i < 100; i++) {
        times[i] = timer(test).count();
    }
    std::cout << "Min: " << *std::min_element(std::begin(times), std::end(times)) << "ms" << std::endl;
    std::cout << "Avg: " << std::reduce(std::begin(times), std::end(times)) / times.size() << "ms" << std::endl;
    std::cout << times.size() << " runs" << std::endl;
    return 0;
};
