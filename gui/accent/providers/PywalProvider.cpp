#include "PywalProvider.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

AccentColorResult PywalProvider::getAccentColor() const {
    QString cacheHome = qgetenv("XDG_CACHE_HOME");
    if (cacheHome.isEmpty()) {
        cacheHome = QDir::homePath() + "/.cache";
    }
    
    QString colorsPath = cacheHome + "/wal/colors.json";
    QFile file(colorsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return AccentColorResult{false, "", "Pywal", ""};
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return AccentColorResult{false, "", "Pywal", ""};
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("colors")) {
        QJsonObject colors = obj.value("colors").toObject();
        // Pywal maps color1 or color2 as primary accents usually.
        if (colors.contains("color2")) {
            QString color = colors.value("color2").toString();
            return AccentColorResult{true, color, "Pywal", colorsPath};
        }
    }
    
    return AccentColorResult{false, "", "Pywal", ""};
}
