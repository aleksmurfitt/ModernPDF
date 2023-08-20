#define POINTERHOLDER_TRANSITION 3
#include "config.hpp"
#include "pdf.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <set>
#include <thread>
#include <vector>

using namespace pdf_lib;
template <typename Unit = std::milli>
auto timer(auto &&func, auto &&...params) -> std::chrono::duration<double, Unit> {
    // get time before function invocation
    const auto &start = std::chrono::high_resolution_clock::now();
    // function invocation using perfect forwarding
    std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
    // get time after function invocation
    const auto &stop = std::chrono::high_resolution_clock::now();
    return stop - start;
};

inline auto Line(double xMin, double yMin, double xMax, double yMax) -> std::string {
    return (" " + std::to_string(xMin) + " " + std::to_string(yMin) + " m " + std::to_string(xMax) + " " +
            std::to_string(yMax) + " l ");
};
inline auto Text(double x, double y, Face &face, std::string text, double points) -> std::string {
    return (" BT " + face.getHandle() + " " + std::to_string(points) + " Tf 0.9867 0.9867 0.9867 rg 0 Tr " +
            std::to_string(x) + " " + std::to_string(y) + " Td " + text + " TJ ET ");
};
auto draw = [](auto &face, auto &font, auto &str, auto width, auto ypos, auto size, char dir, bool showMetrics, auto& contents) -> double {
    double above = face.getAscender() * size / 1000.0;
    double below = face.getDescender() * size / -1000.0;
    double height = above + below;
    double startx;
    switch(dir) {
        case 'l':
            startx = 25;
            break;
        case 'r':
            startx = 595 - 25 - width;
            break;
        case 'c':
            startx = (595 - width)/2;
            break;
    }
    if (showMetrics)
        contents += "q 0.5 w [1 2] 0 d 0.6867 0.6867 0.6867 RG" +
                    Line(startx, 842 - ypos - height, startx, 842 - ypos) +                  // left border
                    Line(startx + width, 842 - ypos, startx + width, 842 - height - ypos) +  // right border
                    Line(startx, 842 - ypos, startx + width, 842 - ypos) +                   // top border
                    Line(startx, 842 - ypos - height, startx + width, 842 - ypos - height) + // bottom border
                    Line(startx, 842 - ypos - above, startx + width, 842 - ypos - above) +   // baseline
                    " S Q";
    contents += Text(startx, 842 - above - ypos, face, font.runs_to_string(str), size);
    return height;
};
inline std::string latinTest(FontManager &fontManager, double points, double lineSpacing, bool showMetrics) {
    // Paint the background;
    std::string contents = "q 0.18824 0.18824 0.18824 rg 0 0 600 850 re F Q ";

    // Draw the names and metrics (if relevant)
    auto &fonts = fontManager.getFonts();

    double ytotal = 0;
    bool left = true;
    double max = 0;
    for (auto &&[name, font] : fonts) {
        auto &face = font.getFaces()[0];
        if (name.find("Noto ") != std::string::npos || name.find("Apple") != std::string::npos ||
            name.find("PingFang") != std::string::npos)
            continue;
        double height = (face.getAscender() - face.getDescender()) * points / 1000.0;
        if (left)
            max = height;
        else {
            ytotal += (max < height ? height : max) * lineSpacing;
        }
        left = !left;
    }

    double ypos = (842 - ytotal) / 2;
    left = true;
    max = 0;
    double prevWidth = 0;
    double subpoints = points / 2.0;
    for (auto &&[name, font] : fonts) {
        if (name.find("Noto ") != std::string::npos || name.find("Apple") != std::string::npos ||
            name.find("PingFang") != std::string::npos)
            continue;
        auto &face = font.getFaces()[0];
        try {
            auto &&[width, title] = face.shape(name, points);
            auto &&[subwidth, subtitle] = face.shape("And this is a subtitle.", subpoints);

            if (!left && prevWidth + width > (540)) {
                left = true;
                ypos += max * lineSpacing;
            }
            auto height = draw(face, font, title, width, ypos, points, left ? 'l' : 'r', showMetrics, contents);
            draw(face, font, subtitle, subwidth, ypos + height + 4, subpoints, left ? 'l' : 'r', showMetrics, contents);
            if (left) {
                max = height;
                prevWidth = width;
            } else {
                ypos += (max < height ? height : max) * lineSpacing;
            }
            left = !left;

        } catch (...) {
            continue;
        }
    }

    return contents;
}
inline double ChineseTest(FontManager &fontManager, std::string &contents) {
    int points = 15;
    double pos = 40;
    bool ping = false;
    for (auto &&[name, font] : fontManager.getFonts()) {

        if (name.find("Ping") != std::string::npos){
            if(!ping)
                ping = true;
            else
                continue;
        }
        auto &face = font.getFaces()[0];
        const std::vector<std::string_view> lines{
            std::string_view("人皆生而自由；在尊嚴及權利上均各平等！人各賦有理性良知，誠應和睦相處，情"),
            std::string_view("同手足鑑於對人類家庭所有成員的固有尊嚴及其平等的和不移的權利的承認，乃是"),
            std::string_view("世界自由、正鑑於各聯合國國家的人民已在聯合國憲章中重申他們對基本人權、人"),
            std::string_view("格尊嚴和價值以及男大會，發布這一世界人權宣言，作為所有人民和所有國家努。"),
        };
        try {
            double above = face.getAscender() * points / 1000.0;
            double below = face.getDescender() * points / -1000.0;
            double height = above + below;
            for (const auto &line : lines) {
                auto &&[width, text] = face.shape(line, points, "Han (Hanzi, Kanji, Hanja)", HB_DIRECTION_LTR, "CN");
                draw(face, font, text, width, pos, points, 'c', false, contents);
                pos += height + face.getLineGap();
            }
        } catch (std::runtime_error& e) {
            continue;
        }
    }
    return pos;
}

inline void ArabicTest(FontManager &fontManager, double &pos, std::string &contents) {
    int points = 15;
    for (auto &&[name, font] : fontManager.getFonts()) {
        auto &face = font.getFaces()[0];
        const std::vector<std::string_view> lines{
            std::string_view(
                "کو تبدیل کرنے اور کُھلے عام یا نجی طور پر، تنہا یا دوسروں کے ساتھ مل جل کر عقیدے کی تبلیغ،"),
            std::string_view("ہر انسان کو آزادیٔ فکر، آزادیٔ ضمیر، اور آزادیٔ مذہب کا پورا حق۔"),
        };
        try {
            double above = face.getAscender() * points / 1000.0;
            double below = face.getDescender() * points / -1000.0;
            double height = above + below;
            for (const auto &line : lines) {
                auto &&[width, text] = face.shape(line, points, "Arabic", HB_DIRECTION_RTL, "AR");
                draw(face, font, text, width, pos, points, 'r', false, contents);
                pos += height + face.getLineGap();
            }
        } catch (std::runtime_error& e) {
            continue;
        }
    }
}
inline void DevanagariTest(FontManager &fontManager, double &pos, std::string &contents) {
    int points = 15;
    for (auto &&[name, font] : fontManager.getFonts()) {
        auto &face = font.getFaces()[0];
        const std::vector<std::string_view> lines{
            std::string_view(
                "सबिन मानमि कोअ हियातिङ अन्डो एकतियर को रेय मामले रे जोनोमेए तेगे दनामुल अन्डोः बराबरि रेयः नमा कना"),
            std::string_view(
                "इनिकु बुद्दि अन्डो जिबोन बितर रेयः एनेम नमा कना अन्डो अकोअको रे हगेयाबोहया (भाईचारे) रेयः जिबोन उङुः"),
            std::string_view("तेको जानागर तेयः दोरकर। सबिन मानमि कोअ हियातिङ अन्डोः एकतियर को रेयः मामले रे जोनोमेए तेगेदनामुल"),
            std::string_view("अन्डोः बराबरि रेयः नमा कना। इनिकु बुद्दि अन्डोः जिबोन बितर रेयः एनेम"),
        };
        try {
            double above = face.getAscender() * points / 1000.0;
            double below = face.getDescender() * points / -1000.0;
            double height = above + below;
            for (const auto &line : lines) {
                auto &&[width, text] = face.shape(line, points, "Devanagari", HB_DIRECTION_LTR, "HI");
                draw(face, font, text, width, pos, points, 'l', false, contents);
                pos += height + face.getLineGap();
            }
        } catch (std::runtime_error& e) {
            continue;
        }
    }
}
inline void EthiopicTest(FontManager &fontManager, double &pos, std::string &contents) {
    int points = 15;
    for (auto &&[name, font] : fontManager.getFonts()) {
        auto &face = font.getFaces()[0];
        const std::vector<std::string_view> lines{
            std::string_view("ተወልዱ፡ኵሉ፡ሰብእ፡ግዑዛን፡ወዕሩያን፡በማዕረግ፡ወብሕግ።ቦሙ፡ኅሊና፡ወዐቅል፡ወይትጌበሩ፡አሐዱ፡ ምስለ፡አ"),
            std::string_view("ሀዱ፡በመንፈሰ፡እኍና። ተወልዱ፡ኵሉ፡ሰብእ፡ግዑዛን፡ወዕሩያን፡በማዕረግ፡ወብሕግ።ቦሙ፡ኅሊና፡ወዐቅል፡ወይት"),
            std::string_view("በሩ፡አሐዱ፡ ምስለ፡አሀዱ፡በመንፈሰ፡እበማዕረግ፡ወብሕግ።ቦሙ፡ኅግዑዛሕን፡ወብሕግዕሊና፡ወዐቅል፡ወኍና።"),
            std::string_view("ጌበሩወዕሩያን፡በማዕረግ፡ወብሕግ።ቦሙ፡ኅሊና፡ወዐቅል፡ወይ"),
        };
        try {
            double above = face.getAscender() * points / 1000.0;
            double below = face.getDescender() * points / -1000.0;
            double height = above + below;
            for (const auto &line : lines) {
                auto &&[width, text] = face.shape(line, points, "Ethiopic", HB_DIRECTION_LTR, "AM");
                draw(face, font, text, width, pos, points, 'l', false, contents);
                pos += height + face.getLineGap();
            }
        } catch (std::runtime_error& e) {
            continue;
        }
    }
}
inline void EmojiTest(FontManager &fontManager, double &pos, std::string &contents) {
    int points = 15;
    for (auto &&[name, font] : fontManager.getFonts()) {
        auto &face = font.getFaces()[0];
        const auto line = std::string_view("☺︎☻♨︎♋︎♥︎");
        try {
            double above = face.getAscender() * points / 1000.0;
            double below = face.getDescender() * points / -1000.0;
            double height = above + below;
            auto &&[width, text] = face.shape(line, points);
            draw(face, font, text, width, pos, points, 'c', false, contents);
            pos += height + face.getLineGap();
        } catch (std::runtime_error& e) {
            continue;
        }
    }
}

// takes font point size (double), line-spacing (double), and show-metrics (bool)
int main(int argc, char *argv[]) {
    using namespace std::chrono_literals;
    Document doc;
    FontManager fontManager{};

    double points;
    double lineSpacing;
    bool showMetrics;

    if (argc == 4) {
        points = std::stol(argv[1]);
        lineSpacing = std::stof(argv[2]);
        showMetrics = (std::string(argv[3]) == "true");
    } else {
        points = 20;
        lineSpacing = 2.6;
        showMetrics = true;
    }


    for (auto &&file : std::filesystem::directory_iterator(Config::fonts)) {
        if (std::set<std::string>({".ttf", ".ttc", ".otf"}).contains(file.path().extension())) {
            auto out = fontManager.loadFontFile(file);
            for (auto &f : out)
                fontManager.get_from_name(f).makeFace();
        }
    }

    doc.createPage().setContents(latinTest(fontManager, points, lineSpacing, showMetrics));

    std::string multiLingual;
    multiLingual.reserve(2000);
    multiLingual.append("q 0.18824 0.18824 0.18824 rg 0 0 600 850 re F Q ");
    double pos = ChineseTest(fontManager, multiLingual) + 20;
    ArabicTest(fontManager, pos, multiLingual);
    pos += 20;
    DevanagariTest(fontManager, pos, multiLingual);
    pos += 20;
    EthiopicTest(fontManager, pos, multiLingual);
    pos += 20;
    EmojiTest(fontManager, pos, multiLingual);

    doc.createPage().setContents(multiLingual);
    doc.register_resource_manager(fontManager);
    doc.write(Config::output / "test.pdf");
    return 0;
};
