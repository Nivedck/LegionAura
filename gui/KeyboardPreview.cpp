#include "KeyboardPreview.h"
#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>
#include <QBrush>
#include <QPen>

KeyboardPreview::KeyboardPreview(QWidget *parent)
    : QWidget(parent)
{
    m_mode = "static";
    m_zoneColors.fill(QColor(0, 0, 0));
    setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void KeyboardPreview::setZoneColor(int zoneIndex, const QColor &color)
{
    if (zoneIndex >= 0 && zoneIndex < 4) {
        m_zoneColors[zoneIndex] = color;
        update();
    }
}

void KeyboardPreview::setMode(const QString &mode)
{
    m_mode = mode.toLower();
    update();
}

void KeyboardPreview::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();

    // Draw laptop chassis (simplified)
    QRect chassisRect = r.adjusted(10, 10, -10, -10);
    painter.setBrush(QColor(30, 30, 34)); // Dark grey chassis
    painter.setPen(QPen(QColor(50, 50, 55), 2));
    painter.drawRoundedRect(chassisRect, 8, 8);

    // Keyboard area
    QRect kbRect = chassisRect.adjusted(10, 20, -10, -20);
    int zoneWidth = kbRect.width() / 4;

    for (int i = 0; i < 4; ++i) {
        QRect zoneRect(kbRect.x() + i * zoneWidth, kbRect.y(), zoneWidth, kbRect.height());

        QColor fill = QColor(20, 20, 20); // default dark key color

        if (m_mode == "static" || m_mode == "breath") {
            if (m_zoneColors[i].isValid() && m_zoneColors[i] != QColor(0, 0, 0)) {
                fill = m_zoneColors[i];
                // Make it look a bit like glowing keys by dimming the background
                painter.setBrush(fill.darker(150));
            } else {
                painter.setBrush(QColor(20, 20, 20));
            }
        } else if (m_mode == "wave" || m_mode == "hue") {
            // Gradient to simulate RGB wave/hue
            QLinearGradient grad(zoneRect.topLeft(), zoneRect.bottomRight());
            grad.setColorAt(0, QColor::fromHsv((i * 60 + 0) % 360, 200, 200));
            grad.setColorAt(1, QColor::fromHsv((i * 60 + 60) % 360, 200, 200));
            painter.setBrush(grad);
        } else {
            painter.setBrush(QColor(20, 20, 20)); // off
        }

        painter.setPen(Qt::NoPen);
        painter.drawRect(zoneRect);

        // Draw some "keys" in this zone
        painter.setPen(QPen(QColor(10, 10, 10, 100), 1));
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 5; ++col) {
                int keyW = zoneRect.width() / 5;
                int keyH = zoneRect.height() / 6;
                QRect keyR(zoneRect.x() + col * keyW + 2, zoneRect.y() + row * keyH + 2, keyW - 4, keyH - 4);
                painter.drawRoundedRect(keyR, 3, 3);
            }
        }
    }

    // Outline around the whole keyboard grid
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(40, 40, 45), 2));
    painter.drawRect(kbRect);
}
