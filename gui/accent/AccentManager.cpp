#include "AccentManager.h"
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QProcess>
#include <QRegularExpression>
#include <QUrl>
#include <array>

namespace {

struct WallpaperCandidate {
    QString path;
    QString source;
};

QString validPath(QString path)
{
    path = path.trimmed();
    if (path.startsWith("file://"))
        path = QUrl(path).toLocalFile();
    if (path.startsWith("~/"))
        path = QDir::homePath() + path.mid(1);
    return QFileInfo::exists(path) ? path : QString{};
}

WallpaperCandidate commandWallpaper(const QString& program, const QStringList& arguments,
                                    const QString& source)
{
    QProcess process;
    process.start(program, arguments, QIODevice::ReadOnly);
    if (!process.waitForStarted(500) || !process.waitForFinished(1500))
        return {};

    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    const QRegularExpression pathPattern(
        R"((file:///[^\s"'<>]+|/[^\s"'<>]+\.(?:png|jpe?g|webp|bmp|gif|avif|jxl)))",
        QRegularExpression::CaseInsensitiveOption);
    const auto match = pathPattern.match(output);
    if (!match.hasMatch())
        return {};

    const QString path = validPath(match.captured(1));
    return path.isEmpty() ? WallpaperCandidate{} : WallpaperCandidate{path, source};
}

QString wallpaperFromStateFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    return validPath(QString::fromUtf8(file.readLine()));
}

QString wallpaperFromWaypaper()
{
    const QString path = QDir::homePath() + "/.config/waypaper/config.ini";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.startsWith("wallpaper") || !line.contains('='))
            continue;

        const QString wallpaper = line.section('=', 1).trimmed();
        const QString expanded = validPath(wallpaper);
        if (!expanded.isEmpty())
            return expanded;
    }
    return {};
}

WallpaperCandidate findWallpaper()
{
    const QString override = qEnvironmentVariable("LEGIONAURA_WALLPAPER").trimmed();
    if (!override.isEmpty()) {
        const QString path = validPath(override);
        if (!path.isEmpty())
            return {path, "LEGIONAURA_WALLPAPER"};
    }

    const WallpaperCandidate awww = commandWallpaper(
        "awww", {"query"}, "awww");
    if (!awww.path.isEmpty())
        return awww;

    const WallpaperCandidate swww = commandWallpaper(
        "swww", {"query"}, "swww");
    if (!swww.path.isEmpty())
        return swww;

    const WallpaperCandidate hyprpaper = commandWallpaper(
        "hyprctl", {"hyprpaper", "listactive"}, "hyprpaper");
    if (!hyprpaper.path.isEmpty())
        return hyprpaper;

    const WallpaperCandidate gnome = commandWallpaper(
        "gsettings", {"get", "org.gnome.desktop.background", "picture-uri"},
        "GNOME");
    if (!gnome.path.isEmpty())
        return gnome;

    const WallpaperCandidate gnomeDark = commandWallpaper(
        "gsettings", {"get", "org.gnome.desktop.background", "picture-uri-dark"},
        "GNOME");
    if (!gnomeDark.path.isEmpty())
        return gnomeDark;

    const QString waypaper = wallpaperFromWaypaper();
    if (!waypaper.isEmpty())
        return {waypaper, "Waypaper"};

    const QString statePath = qEnvironmentVariable("LEGIONAURA_WALLPAPER_STATE").trimmed();
    const QStringList stateFiles = {
        statePath,
        QDir::homePath() + "/.cache/last_wallpaper.txt"
    };
    for (const QString& path : stateFiles) {
        if (!path.isEmpty()) {
            const QString wallpaper = wallpaperFromStateFile(path);
            if (!wallpaper.isEmpty())
                return {wallpaper, "state file"};
        }
    }

    return {};
}

QColor extractAccent(const QImage& source)
{
    const QImage image = source.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                             .convertToFormat(QImage::Format_RGB32);
    struct Bucket {
        int count = 0;
        int red = 0;
        int green = 0;
        int blue = 0;
    };
    std::array<Bucket, 4096> buckets{};

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor pixel = image.pixelColor(x, y);
            const int value = pixel.value();
            const int saturation = pixel.saturation();
            if (value < 30 || value > 250 || saturation < 35)
                continue;

            const int index = (pixel.red() / 16) * 256
                            + (pixel.green() / 16) * 16
                            + pixel.blue() / 16;
            auto& bucket = buckets[static_cast<size_t>(index)];
            ++bucket.count;
            bucket.red += pixel.red();
            bucket.green += pixel.green();
            bucket.blue += pixel.blue();
        }
    }

    const Bucket* best = nullptr;
    double bestScore = 0.0;
    for (const Bucket& bucket : buckets) {
        if (bucket.count == 0)
            continue;
        const QColor color(bucket.red / bucket.count, bucket.green / bucket.count,
                           bucket.blue / bucket.count);
        const double score = bucket.count * (0.5 + color.saturationF());
        if (score > bestScore) {
            bestScore = score;
            best = &bucket;
        }
    }

    if (!best)
        return {};
    return QColor(best->red / best->count, best->green / best->count,
                  best->blue / best->count);
}

}

AccentColorResult AccentManager::detectAccentColor() const
{
    const WallpaperCandidate wallpaper = findWallpaper();
    if (wallpaper.path.isEmpty())
        return AccentColorResult{false, "", "Wallpaper", ""};

    return detectAccentColor(wallpaper.path, wallpaper.source);
}

AccentColorResult AccentManager::detectAccentColor(const QString& wallpaperPath,
                                                    const QString& source) const
{
    const QString path = wallpaperPath.trimmed();
    if (path.isEmpty())
        return AccentColorResult{false, "", source, ""};

    QImage image(path);
    if (image.isNull())
        return AccentColorResult{false, "", source, path};

    const QColor accent = extractAccent(image);
    if (!accent.isValid())
        return AccentColorResult{false, "", source, path};

    return AccentColorResult{
        true,
        accent.name(QColor::HexRgb),
        source,
        path
    };
}
