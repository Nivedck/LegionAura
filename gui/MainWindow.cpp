#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QColorDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QStatusBar>
#include <QSettings>
#include <QPalette>
#include <QStyleFactory>
#include <QTimer>
#include <map>

// ------------------------------------------------------------------
// Device name resolver
// ------------------------------------------------------------------
static QString resolveDeviceName(uint16_t pid)
{
    static const std::map<uint16_t, QString> names = {
        {0xC993, "Lenovo LOQ"},
        {0xC996, "Lenovo Legion"},
        {0xC963, "Lenovo IdeaPad Gaming"}
    };

    auto it = names.find(pid);
    if (it != names.end())
        return it->second;

    return "Lenovo (Unknown Model)";
}

// ------------------------------------------------------------------
// MainWindow
// ------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Auto-detect device shortly after app start
    QTimer::singleShot(100, this, &MainWindow::autoDetectOnStartup);

    // Dark UI theme
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette dark;
    dark.setColor(QPalette::Window, QColor(30,30,30));
    dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base, QColor(22,22,22));
    dark.setColor(QPalette::AlternateBase, QColor(36,36,36));
    dark.setColor(QPalette::Text, Qt::white);
    dark.setColor(QPalette::Button, QColor(45,45,45));
    dark.setColor(QPalette::ButtonText, Qt::white);
    dark.setColor(QPalette::Highlight, QColor(100,150,255));
    dark.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(dark);

    // UI Signals
    connect(ui->btnDetect, &QPushButton::clicked, this, &MainWindow::onDetectClicked);
    connect(ui->btnApply,  &QPushButton::clicked, this, &MainWindow::onApplyClicked);
    connect(ui->btnOff,    &QPushButton::clicked, this, &MainWindow::onOffClicked);

    connect(ui->btnColor1, &QPushButton::clicked, this, &MainWindow::onPickZ1);
    connect(ui->btnColor2, &QPushButton::clicked, this, &MainWindow::onPickZ2);
    connect(ui->btnColor3, &QPushButton::clicked, this, &MainWindow::onPickZ3);
    connect(ui->btnColor4, &QPushButton::clicked, this, &MainWindow::onPickZ4);

    connect(ui->comboEffect, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onEffectChanged);

    // Initial UI state
    onEffectChanged(ui->comboEffect->currentIndex());
    ui->lblDeviceLeft->setText("Device: (not connected)");
    ui->lblDeviceName->setText("");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------
// Manual detect
// ------------------------------------------------------------------
void MainWindow::onDetectClicked()
{
    if (kb_.autoDetect()) {
        deviceReady_ = true;

        QString name = resolveDeviceName(kb_.getPid());
        ui->lblDeviceLeft->setText("Device: connected");
        ui->lblDeviceName->setText(name);

        setStatusOk("Device connected");
        return;
    }

    if (kb_.open()) {
        deviceReady_ = true;

        QString name = resolveDeviceName(kb_.getPid());
        ui->lblDeviceLeft->setText("Device: connected");
        ui->lblDeviceName->setText(name);

        setStatusOk("Device connected (default)");
    } else {
        deviceReady_ = false;

        ui->lblDeviceLeft->setText("Device: (not connected)");
        ui->lblDeviceName->setText("");

        setStatusErr("Failed to open device. Try installing udev rules.");
    }
}

// ------------------------------------------------------------------
// Auto detect on startup
// ------------------------------------------------------------------
void MainWindow::autoDetectOnStartup()
{
    if (kb_.autoDetect()) {
        deviceReady_ = true;

        ui->lblDeviceLeft->setText("Device: connected");
        ui->lblDeviceName->setText(resolveDeviceName(kb_.getPid()));

        setStatusOk("Device auto-detected");
    }
}

// ------------------------------------------------------------------
// Color picker helpers
// ------------------------------------------------------------------
std::optional<QString> MainWindow::pickHexColor(const QString &initialHex)
{
    QColor initial = hexToRgb(initialHex).value_or(QColor(255,0,0));
    QColor c = QColorDialog::getColor(initial, this, "Pick Color");

    if (!c.isValid())
        return std::nullopt;

    return rgbToHex(c);
}

QString MainWindow::rgbToHex(const QColor &c)
{
    return QString("%1%2%3")
        .arg(c.red(),   2, 16, QLatin1Char('0'))
        .arg(c.green(), 2, 16, QLatin1Char('0'))
        .arg(c.blue(),  2, 16, QLatin1Char('0'))
        .toLower();
}

std::optional<QColor> MainWindow::hexToRgb(const QString &hex)
{
    if (hex.size() != 6)
        return std::nullopt;

    bool ok;
    int r = hex.mid(0,2).toInt(&ok,16); if (!ok) return std::nullopt;
    int g = hex.mid(2,2).toInt(&ok,16); if (!ok) return std::nullopt;
    int b = hex.mid(4,2).toInt(&ok,16); if (!ok) return std::nullopt;

    return QColor(r,g,b);
}

void MainWindow::setBtnSwatch(QPushButton* btn, const QString& hex)
{
    btn->setStyleSheet(
        QString("background-color: #%1; border: 1px solid #555;").arg(hex)
    );
}

// ------------------------------------------------------------------
// Zone pickers
// ------------------------------------------------------------------
void MainWindow::onPickZ1()
{
    auto val = pickHexColor(ui->editZ1->text());
    if (!val) return;
    ui->editZ1->setText(*val);
    setBtnSwatch(ui->btnColor1, *val);
}

void MainWindow::onPickZ2()
{
    auto val = pickHexColor(ui->editZ2->text());
    if (!val) return;
    ui->editZ2->setText(*val);
    setBtnSwatch(ui->btnColor2, *val);
}

void MainWindow::onPickZ3()
{
    auto val = pickHexColor(ui->editZ3->text());
    if (!val) return;
    ui->editZ3->setText(*val);
    setBtnSwatch(ui->btnColor3, *val);
}

void MainWindow::onPickZ4()
{
    auto val = pickHexColor(ui->editZ4->text());
    if (!val) return;
    ui->editZ4->setText(*val);
    setBtnSwatch(ui->btnColor4, *val);
}

// ------------------------------------------------------------------
// Effect change handler
// ------------------------------------------------------------------
void MainWindow::onEffectChanged(int idx)
{
    QString mode = ui->comboEffect->itemText(idx).toLower();

    bool needsColors = (mode == "static" || mode == "breath");
    bool needsDir    = (mode == "wave");

    ui->editZ1->setEnabled(needsColors);
    ui->editZ2->setEnabled(needsColors);
    ui->editZ3->setEnabled(needsColors);
    ui->editZ4->setEnabled(needsColors);

    ui->btnColor1->setEnabled(needsColors);
    ui->btnColor2->setEnabled(needsColors);
    ui->btnColor3->setEnabled(needsColors);
    ui->btnColor4->setEnabled(needsColors);

    ui->chkAutofill->setEnabled(needsColors);

    ui->comboDirection->setEnabled(needsDir);
}

// ------------------------------------------------------------------
// Auto-fill colors
// ------------------------------------------------------------------
std::array<QString,4> MainWindow::normalize4(const std::vector<QString> &in)
{
    if (in.empty()) return {"ffffff","ffffff","ffffff","ffffff"};
    if (in.size() == 1) return {in[0], in[0], in[0], in[0]};
    if (in.size() == 2) return {in[0], in[1], in[1], in[1]};
    if (in.size() == 3) return {in[0], in[1], in[2], in[2]};
    return {in[0], in[1], in[2], in[3]};
}

// ----------------
// ------------------------------------------------------------------
// Build params from UI
// ------------------------------------------------------------------
std::optional<LAParams> MainWindow::buildParamsFromUi() const
{
    QString mode = ui->comboEffect->currentText().toLower();

    LAParams p;
    p.speed      = ui->comboSpeed->currentText().toUInt();
    p.brightness = ui->comboBrightness->currentText().toUInt();
    p.waveDir    = LAWaveDir::None;

    if (mode == "static")      p.effect = LAEffect::Static;
    else if (mode == "breath") p.effect = LAEffect::Breath;
    else if (mode == "wave")   p.effect = LAEffect::Wave;
    else if (mode == "hue")    p.effect = LAEffect::Hue;
    else return std::nullopt;

    if (p.effect == LAEffect::Wave) {
        QString d = ui->comboDirection->currentText().toLower();
        if (d == "ltr") p.waveDir = LAWaveDir::LTR;
        else if (d == "rtl") p.waveDir = LAWaveDir::RTL;
    }

    if (p.effect == LAEffect::Static || p.effect == LAEffect::Breath) {
        std::vector<QString> cols;

        if (!ui->editZ1->text().isEmpty()) cols.push_back(ui->editZ1->text());
        if (!ui->editZ2->text().isEmpty()) cols.push_back(ui->editZ2->text());
        if (!ui->editZ3->text().isEmpty()) cols.push_back(ui->editZ3->text());
        if (!ui->editZ4->text().isEmpty()) cols.push_back(ui->editZ4->text());

        if (cols.empty()) return std::nullopt;

        auto normalized = ui->chkAutofill->isChecked()
                        ? normalize4(cols)
                        : std::array<QString,4>{cols[0], cols[1], cols[2], cols[3]};

        for (int i = 0; i < 4; i++) {
            auto c = hexToRgb(normalized[i]);
            if (!c) return std::nullopt;
            p.zones[i] = LAColor{
                (uint8_t)c->red(),
                (uint8_t)c->green(),
                (uint8_t)c->blue()
            };
        }
    }

    return p;
}

// ------------------------------------------------------------------
// APPLY
// ------------------------------------------------------------------
void MainWindow::onApplyClicked()
{
    if (!deviceReady_) {
        setStatusErr("Device not connected.");
        return;
    }

    auto params = buildParamsFromUi();
    if (!params) {
        setStatusErr("Invalid color values.");
        return;
    }

    bool ok = kb_.apply(*params);
    if (ok) setStatusOk("Lighting updated.");
    else    setStatusErr("Failed to send command.");
}

// ------------------------------------------------------------------
// TURN OFF
// ------------------------------------------------------------------
void MainWindow::onOffClicked()
{
    if (!deviceReady_) {
        setStatusErr("Device not connected.");
        return;
    }

    if (kb_.off())
        setStatusOk("Keyboard turned off.");
    else
        setStatusErr("Failed to send off command.");
}

// ------------------------------------------------------------------
// STATUS BAR HELPERS
// ------------------------------------------------------------------
void MainWindow::setStatusOk(const QString& msg)
{
    ui->statusbar->showMessage(msg, 3000);
}

void MainWindow::setStatusErr(const QString& msg)
{
    ui->statusbar->showMessage("Error: " + msg, 5000);
}
