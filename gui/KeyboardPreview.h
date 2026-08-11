#pragma once

#include <QWidget>
#include <QColor>
#include <QString>
#include <array>

class KeyboardPreview : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardPreview(QWidget *parent = nullptr);

    void setZoneColor(int zoneIndex, const QColor &color);
    void setMode(const QString &mode); // "static", "breath", "wave", "hue", "off"

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::array<QColor, 4> m_zoneColors;
    QString m_mode;
};
