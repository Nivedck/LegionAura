#pragma once
#include <QImage>
#include <QColor>
#include <array>
#include <algorithm>

using ScreenColors = std::array<QColor, 4>;

// Sample a bounded grid directly: no full-resolution quadrant copies, even at 4K.
// Chroma weighting prevents neutral pixels from washing vivid regions toward white.
inline ScreenColors sampleScreenColors(const QImage &image)
{
    ScreenColors colors;
    colors.fill(Qt::black);
    if (image.isNull()) return colors;
    for (int zone = 0; zone < 4; ++zone) {
        const int left = zone * image.width() / 4;
        const int width = std::max(1, (zone + 1) * image.width() / 4 - left);
        const int nx = std::min(24, width), ny = std::min(24, image.height());
        double r = 0, g = 0, b = 0, wr = 0, wg = 0, wb = 0, weight = 0;
        for (int y = 0; y < ny; ++y) {
            for (int x = 0; x < nx; ++x) {
                const QColor c = image.pixelColor(left + x * width / nx, y * image.height() / ny);
                r += c.red(); g += c.green(); b += c.blue();
                const double chroma = (std::max({c.red(), c.green(), c.blue()})
                                     - std::min({c.red(), c.green(), c.blue()})) / 255.0;
                const double w = chroma * chroma;
                wr += c.red() * w; wg += c.green() * w; wb += c.blue() * w;
                weight += w;
            }
        }
        const int count = nx * ny;
        // Fade the weighting out when colorful pixels cover only a tiny area.
        const double mix = std::min(1.0, 4.0 * weight / count);
        auto channel = [&](double sum, double weighted) {
            return std::clamp(int((1.0 - mix) * sum / count
                                 + (weight > 0 ? mix * weighted / weight : 0)), 0, 255);
        };
        colors[zone] = QColor(channel(r, wr), channel(g, wg), channel(b, wb));
    }
    return colors;
}

inline ScreenColors smoothScreenColors(const ScreenColors &current, const ScreenColors &target)
{
    ScreenColors result;
    auto blend = [](int a, int b) {
        return a == b ? a : a + (b > a ? 1 : -1) * std::max(1, qRound(std::abs(b - a) * 0.35));
    };
    for (int i = 0; i < 4; ++i)
        result[i] = QColor(blend(current[i].red(), target[i].red()),
                           blend(current[i].green(), target[i].green()),
                           blend(current[i].blue(), target[i].blue()));
    return result;
}
