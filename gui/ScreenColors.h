#pragma once
#include <QImage>
#include <QColor>
#include <array>
#include <algorithm>

using ScreenColors = std::array<QColor, 4>;

// Sample each quarter independently so downsampling cannot mix zone boundaries.
inline ScreenColors sampleScreenColors(const QImage &image)
{
    ScreenColors colors;
    colors.fill(Qt::black);
    if (image.isNull()) return colors;
    for (int zone = 0; zone < 4; ++zone) {
        int left = zone * image.width() / 4;
        int right = (zone + 1) * image.width() / 4;
        if (right <= left) right = left + 1;
        auto small = image.copy(left, 0, right - left, image.height())
                         .scaled(24, 24, Qt::IgnoreAspectRatio, Qt::FastTransformation)
                         .convertToFormat(QImage::Format_RGB32);
        int r = 0, g = 0, b = 0;
        for (int y = 0; y < small.height(); ++y) {
            auto row = reinterpret_cast<const QRgb *>(small.constScanLine(y));
            for (int x = 0; x < small.width(); ++x) {
                r += qRed(row[x]); g += qGreen(row[x]); b += qBlue(row[x]);
            }
        }
        const int count = small.width() * small.height();
        colors[zone] = QColor(r / count, g / count, b / count);
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
