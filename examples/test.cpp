#define POINTERHOLDER_TRANSITION 3
#include "config.hpp"
#include "pdf.hpp"
#include "private/harfbuzz-helpers.hpp"

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

inline void test(bool write, float points, float lineSpacing, bool showMetrics) {
    Document d{};
    FontManager &fontManager = d.getFontManager();
    // Paint the background;
    std::string contents = "q 0.18824 0.18824 0.18824 rg 0 0 600 850 re F Q ";

    // Load the fonts
    std::filesystem::path fontDir(Config::fonts);

    for (auto &&file : std::filesystem::directory_iterator(fontDir)) {
        if (std::set<std::string>({".ttf", ".ttc", ".otf"}).contains(file.path().extension()))
            fontManager.loadFontFile(file);
    }

    auto line = [](float xMin, float yMin, float xMax, float yMax) -> std::string {
        return (" " + std::to_string(xMin) + " " + std::to_string(yMin) + " m " + std::to_string(xMax) + " " +
                std::to_string(yMax) + " l ");
    };

    // Draw the names and metrics (if relevant)
    size_t index = 0;
    auto &fonts = fontManager.getFonts();
    float ytotal = 0;
    bool left = true;
    float max = 0;
    for (auto &&[name, font] : fonts) {
        auto &face = font.makeFace();
        float above = static_cast<float>(face.getAscender()) * points / 1000.0;
        float below = static_cast<float>(face.getDescender()) * points / -1000.0;
        if (left)
            max = (above + below);
        else
            ytotal += (max < (above + below) ? (above + below) : max) * lineSpacing;
        left = !left;
    }
    float ypos = (842 - ytotal) / 2;
    left = true;
    max = 0;
    float prevWidth = 0;
    for (auto &&[name, font] : fonts) {
        auto &face = font.getFaces()[0];
        auto &&[width, text] = face.shape(name, points);

        if (!left && prevWidth + width > (540)) {
            left = true;
            ypos += max * lineSpacing;
        }

        float above = static_cast<float>(face.getAscender()) * points / 1000.0;
        float below = static_cast<float>(face.getDescender()) * points / -1000.0;
        float height = above + below;
        float startx = left ? 25 : 595 - 25 - width;
        if (showMetrics)
            contents += "q 0.5 w [1 2] 0 d 0.6867 0.6867 0.6867 RG" +
                        line(startx, 842 - ypos - height, startx, 842 - ypos) +                  // left border
                        line(startx + width, 842 - ypos, startx + width, 842 - height - ypos) +  // right border
                        line(startx, 842 - ypos, startx + width, 842 - ypos) +                   // top border
                        line(startx, 842 - ypos - height, startx + width, 842 - ypos - height) + // bottom border
                        line(startx, 842 - ypos - above, startx + width, 842 - ypos - above) +   // baseline
                        " S Q";
        contents += " BT " + face.getHandle() + " " + std::to_string(points) + " Tf 0.9867 0.9867 0.9867 rg 0 Tr " +
                    std::to_string(startx) + " " + std::to_string(842 - above - ypos) + " Td " + text + " TJ ET ";
        index++;
        if (left) {
            max = height;
            prevWidth = width;
        } else
            ypos += (max < height ? height : max) * lineSpacing;
        left = !left;
    }

    // Create the page
    for (size_t i = 0; i < 1; ++i)
        d.createPage().setContents(contents);
    if (write)
        d.write(Config::output / "test.pdf");
}

// takes font point size (float), line-spacing (float), and show-metrics (bool)
int main(int argc, char *argv[]) {
    using namespace std::chrono_literals;

    float points;
    float lineSpacing;
    bool showMetrics;

    if (argc == 4) {
        points = std::stol(argv[1]);
        lineSpacing = std::stof(argv[2]);
        showMetrics = std::string(argv[3]) == "true";
    } else {
        points = 20;
        lineSpacing = 1.14;
        showMetrics = true;
    }

    test(true, points, lineSpacing, showMetrics);

    std::array<double, 100> times;
    for (size_t i = 0; i < 100; i++) {
        times[i] = timer(test, false, points, lineSpacing, showMetrics).count();
    }

    std::cout << "Min: " << *std::min_element(std::begin(times), std::end(times)) << "ms" << std::endl;
    std::cout << "Avg: " << std::reduce(std::begin(times), std::end(times)) / times.size() << "ms" << std::endl;
    std::cout << times.size() << " runs" << std::endl;
    return 0;
};
