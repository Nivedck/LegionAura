// /LegionAura/gui/MainWindow.h
#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QColor>
#include <array>
#include <optional>
#include "legionaura.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Top bar
    void onDetectClicked();

    // Zone pickers
    void onPickZ1();
    void onPickZ2();
    void onPickZ3();
    void onPickZ4();

    void autoDetectOnStartup();  //For auto detecting the device


    // Apply/Off
    void onApplyClicked();
    void onOffClicked();

    // Effect changes (enable/disable controls as needed)
    void onEffectChanged(int index);

private:
    std::optional<QString> pickHexColor(const QString &initialHex);
    static QString rgbToHex(const QColor &c);
    static std::optional<QColor> hexToRgb(const QString &hex);

    void setBtnSwatch(QPushButton* btn, const QString& hex);
    void setStatusOk(const QString& msg);
    void setStatusErr(const QString& msg);

    // Build LAParams from current UI state (with auto-fill if checked)
    std.optional<LAParams> buildParamsFromUi() const;

    // Normalize 1..3 colors to 4 (same rule as CLI)
    static std::array<QString,4> normalize4(const std::vector<QString>& in);

private:
    Ui::MainWindow *ui;
    LegionAura kb_;
    bool deviceReady_ = false;
};