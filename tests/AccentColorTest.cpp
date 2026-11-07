#include "accent/AccentManager.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <cassert>

int main()
{
    QTemporaryDir directory;
    assert(directory.isValid());

    const QString imagePath = directory.filePath("wallpaper.png");
    QImage image(32, 32, QImage::Format_RGB32);
    image.fill(QColor("#d14f62"));
    assert(image.save(imagePath));

    const QString statePath = directory.filePath("last_wallpaper.txt");
    QFile stateFile(statePath);
    assert(stateFile.open(QIODevice::WriteOnly | QIODevice::Text));
    stateFile.write(imagePath.toUtf8());
    stateFile.write("\n");
    stateFile.close();

    qputenv("LEGIONAURA_WALLPAPER_STATE", statePath.toUtf8());
    qputenv("LEGIONAURA_WALLPAPER", imagePath.toUtf8());

    const AccentColorResult result = AccentManager().detectAccentColor();
    assert(result.found);
    assert(result.colorHex == "#d14f62");
    assert(result.sourcePath == imagePath);
    return 0;
}
