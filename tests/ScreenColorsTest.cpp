#include "ScreenColors.h"
#include <iostream>

static int failures = 0;
static void check(bool condition, const char *message)
{
    if (!condition) { std::cerr << message << '\n'; ++failures; }
}

int main()
{
    const ScreenColors expected{QColor(Qt::red), QColor(Qt::green), QColor(Qt::blue), QColor(Qt::white)};
    // Include tiny, odd-width, and portrait images, plus a non-RGB source format.
    for (const auto size : {QSize(4, 1), QSize(101, 55), QSize(9, 101)}) {
        QImage image(size, QImage::Format_RGBA8888);
        for (int zone = 0; zone < 4; ++zone)
            for (int y = 0; y < image.height(); ++y)
                for (int x = zone * image.width() / 4; x < (zone + 1) * image.width() / 4; ++x)
                    image.setPixelColor(x, y, expected[zone]);
        check(sampleScreenColors(image) == expected, "Quarter boundaries or channel mapping incorrect");
    }
    ScreenColors black; black.fill(Qt::black);
    check(sampleScreenColors(QImage()) == black, "Null image must produce black");
    QImage tiny(1, 1, QImage::Format_RGB32); tiny.fill(Qt::red);
    ScreenColors red; red.fill(Qt::red);
    check(sampleScreenColors(tiny) == red, "Single-pixel frame must fill all zones");
    QImage dark(7, 3, QImage::Format_RGB32); dark.fill(Qt::black);
    check(sampleScreenColors(dark) == black, "Black frame must stay black");
    QImage mixed(8, 2, QImage::Format_RGB32); mixed.fill(Qt::black);
    for (int x = 0; x < mixed.width(); ++x) mixed.setPixelColor(x, 0, Qt::white);
    ScreenColors gray; gray.fill(QColor(127, 127, 127));
    check(sampleScreenColors(mixed) == gray, "Sampling must average each region");
    QImage washed(8, 2, QImage::Format_RGB32); washed.fill(Qt::white);
    for (int x = 0; x < washed.width(); ++x) washed.setPixelColor(x, 0, Qt::red);
    check(sampleScreenColors(washed) == red, "White pixels must not turn saturated red into pink");
    QImage neutral(8, 8, QImage::Format_RGB32); neutral.fill(QColor(70, 70, 70));
    check(sampleScreenColors(neutral)[0] == QColor(70, 70, 70), "Gray must not gain a color cast");
    auto smoothed = smoothScreenColors(black, expected);
    check(smoothed[0].red() > 0 && smoothed[0].red() < 255, "Transitions must be gradual");
    for (int i = 0; i < 30; ++i) smoothed = smoothScreenColors(smoothed, expected);
    check(smoothed == expected, "Smoothing must converge exactly");
    for (int i = 0; i < 30; ++i) smoothed = smoothScreenColors(smoothed, black);
    check(smoothed == black, "Smoothing must converge downward to black");
    check(smoothScreenColors(expected, expected) == expected, "Unchanged colors must remain stable");
    return failures ? 1 : 0;
}
