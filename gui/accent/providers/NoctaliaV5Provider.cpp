#include "NoctaliaV5Provider.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>

AccentColorResult NoctaliaV5Provider::getAccentColor() const {
    QString configHome = qgetenv("NOCTALIA_CONFIG_HOME");
    if (configHome.isEmpty()) {
        configHome = qgetenv("XDG_CONFIG_HOME");
        if (configHome.isEmpty()) {
            configHome = QDir::homePath() + "/.config";
        }
    }
    
    QString colorsPath = configHome + "/noctalia/colors.json";
    QFile file(colorsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return AccentColorResult{false, "", "Noctalia v5", ""};
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return AccentColorResult{false, "", "Noctalia v5", ""};
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("mPrimary")) {
        QString color = obj.value("mPrimary").toString();
        return AccentColorResult{true, color, "Noctalia v5", colorsPath};
    }
    
    return AccentColorResult{false, "", "Noctalia v5", ""};
}
