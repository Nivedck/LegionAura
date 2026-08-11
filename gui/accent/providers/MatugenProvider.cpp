#include "MatugenProvider.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

AccentColorResult MatugenProvider::getAccentColor() const {
    QString cacheHome = qgetenv("XDG_CACHE_HOME");
    if (cacheHome.isEmpty()) {
        cacheHome = QDir::homePath() + "/.cache";
    }
    
    QString colorsPath = cacheHome + "/matugen/colors.json";
    QFile file(colorsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return AccentColorResult{false, "", "Matugen", ""};
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return AccentColorResult{false, "", "Matugen", ""};
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("colors")) {
        QJsonObject colors = obj.value("colors").toObject();
        if (colors.contains("primary")) {
            QString color = colors.value("primary").toString();
            return AccentColorResult{true, color, "Matugen", colorsPath};
        }
    }
    
    return AccentColorResult{false, "", "Matugen", ""};
}
