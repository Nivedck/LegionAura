// /LegionAura/gui/MainWindow.cpp
//Nivedck
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
#include <QFrame>
#include <unordered_map>

// ------------------------------------------------------------------
// Device name resolver
// ------------------------------------------------------------------
static QString resolveDeviceName(uint16_t pid)
{
    static const std::unordered_map<uint16_t, QString> names = {
        // From devices/devices.json
        {0xC995, "Lenovo Legion Pro"},
        {0xC994, "Lenovo Legion Regular/Slim"},
        {0xC993, "Lenovo LOQ"},
        {0xC985, "Lenovo Legion Pro"},
        {0xC984, "Lenovo Legion Slim"},
        {0xC983, "Lenovo LOQ"},
        {0xC975, "Lenovo Legion Pro/Regular"},
        {0xC973, "Lenovo IdeaPad Gaming"},
        {0xC965, "Lenovo Legion Pro/Regular"},
        {0xC963, "Lenovo IdeaPad Gaming"},
        {0xC955, "Lenovo Legion Pro/Regular"},

        // Legacy/extra (not currently in devices.json)
        {0xC996, "Lenovo Legion"},
    };

    auto it = names.find(pid);
    if (it != names.end())
        return it->second;

    return "Lenovo (Unknown Model)";
}

static void setDeviceStatusText(Ui::MainWindow* ui, const QString& deviceName, bool connected)
{
    const QString nameRaw = deviceName.trimmed();
    const QString name = nameRaw;

    if (!connected) {
        ui->lblDeviceLeft->setText("Not connected");
        ui->lblDeviceName->setText(QString());
        ui->lblDeviceStatusIcon->setStyleSheet("color: #555555;"); // Grey dot
        if (ui->lblDeviceName) ui->lblDeviceName->hide();
        return;
    }

    ui->lblDeviceLeft->setText(name.isEmpty() ? QString("Connected") : QString("%1").arg(name));
    ui->lblDeviceName->setText(QString());
    ui->lblDeviceStatusIcon->setStyleSheet("color: #4CAF50;"); // Green dot
    if (ui->lblDeviceName) ui->lblDeviceName->hide();
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

    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette dark;
    dark.setColor(QPalette::Window, QColor(24,24,27));
    dark.setColor(QPalette::WindowText, Qt::white);
    dark.setColor(QPalette::Base, QColor(18,18,20));
    dark.setColor(QPalette::AlternateBase, QColor(30,30,34));
    dark.setColor(QPalette::Text, Qt::white);
    dark.setColor(QPalette::Button, QColor(36,36,40));
    dark.setColor(QPalette::ButtonText, Qt::white);
    dark.setColor(QPalette::Highlight, QColor(0,122,204));
    dark.setColor(QPalette::HighlightedText, Qt::white);
    qApp->setPalette(dark);

    qApp->setStyleSheet(
        "QLabel#lblAppTitle { font-size: 24px; font-weight: bold; color: #ffffff; }"
        "QLabel#lblDeviceLeft { font-size: 14px; font-weight: 500; color: #a0a0a5; }"
        "QLabel#lblDeviceStatusIcon { font-size: 18px; }"
        "QGroupBox { border: 1px solid #303036; border-radius: 8px; margin-top: 24px; font-weight: 600; font-size: 14px; color: #d0d0d5; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 16px; padding: 0 4px; }"
        "QLineEdit, QComboBox { background: #121214; border: 1px solid #303036; border-radius: 6px; padding: 6px 10px; color: white; }"
        "QComboBox::drop-down { border: 0px; }"
        "QPushButton { background: #242428; border: 1px solid #3a3a40; border-radius: 6px; padding: 8px 12px; font-weight: 500; color: white; }"
        "QPushButton:hover { background: #2d2d33; }"
        "QPushButton:pressed { background: #1c1c1f; }"
        "QPushButton:checked { background: #007acc; border-color: #0098ff; color: white; font-weight: bold; }"
        "QPushButton#btnApply { background: #007acc; border-color: #007acc; color: white; font-weight: bold; font-size: 16px; }"
        "QPushButton#btnApply:hover { background: #0098ff; }"
        "QPushButton#btnOff { background: #3d2222; border-color: #552b2b; color: #ffcccc; font-weight: bold; font-size: 16px; }"
        "QPushButton#btnOff:hover { background: #552b2b; }"
        "QPushButton:disabled { color: #666; background: #1a1a1c; border-color: #2a2a2c; }"
        "QSlider::groove:horizontal { border: 1px solid #2a2a2c; height: 6px; background: #121214; margin: 2px 0; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #007acc; border: 1px solid #0098ff; width: 14px; margin: -5px 0; border-radius: 7px; }"
        "QFrame#cardZ1, QFrame#cardZ2, QFrame#cardZ3, QFrame#cardZ4 { background: #1c1c1e; border: 1px solid #2a2a2c; border-radius: 8px; padding: 4px; }"
        "QPushButton#btnColor1, QPushButton#btnColor2, QPushButton#btnColor3, QPushButton#btnColor4 { border-radius: 4px; border: 1px solid #3a3a40; }"
    );

    if (ui->statusbar) {
        auto *github = new QLabel("<a href=\"https://github.com/nivedck\">github.com/nivedck</a>");
        github->setOpenExternalLinks(true);
        github->setTextInteractionFlags(Qt::TextBrowserInteraction);
        github->setStyleSheet("QLabel { color: #777; font-size: 11px; } QLabel:hover { text-decoration: underline; }");
        ui->statusbar->addPermanentWidget(github);
    }

    connect(ui->btnDetect, &QPushButton::clicked, this, &MainWindow::onDetectClicked);
    connect(ui->btnApply,  &QPushButton::clicked, this, &MainWindow::onApplyClicked);
    connect(ui->btnOff,    &QPushButton::clicked, this, &MainWindow::onOffClicked);

    connect(ui->btnColor1, &QPushButton::clicked, this, &MainWindow::onPickZ1);
    connect(ui->btnColor2, &QPushButton::clicked, this, &MainWindow::onPickZ2);
    connect(ui->btnColor3, &QPushButton::clicked, this, &MainWindow::onPickZ3);
    connect(ui->btnColor4, &QPushButton::clicked, this, &MainWindow::onPickZ4);

    m_effectGroup = new QButtonGroup(this);
    m_effectGroup->addButton(ui->btnEffectStatic, 0);
    m_effectGroup->addButton(ui->btnEffectBreath, 1);
    m_effectGroup->addButton(ui->btnEffectWave, 2);
    m_effectGroup->addButton(ui->btnEffectHue, 3);
    m_effectGroup->addButton(ui->btnEffectOff, 4);

    connect(m_effectGroup, &QButtonGroup::idToggled, this, [this](int id, bool checked){
        if(checked) onEffectChanged(id);
    });

    connect(ui->sliderSpeed, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
    connect(ui->sliderBrightness, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);

    connect(ui->editZ1, &QLineEdit::textChanged, this, &MainWindow::updatePreview);
    connect(ui->editZ2, &QLineEdit::textChanged, this, &MainWindow::updatePreview);
    connect(ui->editZ3, &QLineEdit::textChanged, this, &MainWindow::updatePreview);
    connect(ui->editZ4, &QLineEdit::textChanged, this, &MainWindow::updatePreview);

    ui->btnEffectStatic->setChecked(true);
    onEffectChanged(0);
    onSliderChanged();

    setDeviceStatusText(ui, QString(), false);
    ui->lblDeviceName->hide();
    updatePreview();
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
        setDeviceStatusText(ui, resolveDeviceName(kb_.getPid()), true);
        setStatusOk("Device connected");
        return;
    }

    if (kb_.open()) {
        deviceReady_ = true;
        setDeviceStatusText(ui, resolveDeviceName(kb_.getPid()), true);
        setStatusOk("Device connected (default)");
    } else {
        deviceReady_ = false;
        setDeviceStatusText(ui, QString(), false);
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
        setDeviceStatusText(ui, resolveDeviceName(kb_.getPid()), true);
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
        QString("background-color: #%1; border: 1px solid #555; border-radius: 4px;").arg(hex)
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
// Sliders
// ------------------------------------------------------------------
void MainWindow::onSliderChanged()
{
    ui->lblSpeedVal->setText(QString::number(ui->sliderSpeed->value()));
    ui->lblBrightVal->setText(QString::number(ui->sliderBrightness->value()));
}

// ------------------------------------------------------------------
// Effect change handler
// ------------------------------------------------------------------
void MainWindow::onEffectChanged(int idx)
{
    QString mode;
    switch (idx) {
        case 0: mode = "static"; break;
        case 1: mode = "breath"; break;
        case 2: mode = "wave"; break;
        case 3: mode = "hue"; break;
        case 4: mode = "off"; break;
        default: mode = "static"; break;
    }

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

    ui->keyboardPreviewWidget->setMode(mode);

    updatePreview();
}

void MainWindow::updatePreview()
{
    int idx = m_effectGroup ? m_effectGroup->checkedId() : 0;
    QString mode;
    switch (idx) {
        case 0: mode = "static"; break;
        case 1: mode = "breath"; break;
        case 2: mode = "wave"; break;
        case 3: mode = "hue"; break;
        case 4: mode = "off"; break;
        default: mode = "static"; break;
    }
    bool needsColors = (mode == "static" || mode == "breath");

    auto normalizeHex = [&](QLineEdit* edit) -> QString {
        QString hex = edit->text().trimmed().toLower();
        if (hex.startsWith('#')) hex = hex.mid(1);
        return hex;
    };

    auto updateOne = [&](QLineEdit* edit, QPushButton* btn, int zoneIndex) {
        const QString hex = normalizeHex(edit);

        if (!needsColors) {
            btn->setStyleSheet(QString());
            ui->keyboardPreviewWidget->setZoneColor(zoneIndex, QColor(0,0,0));
            return;
        }

        if (hex.isEmpty()) {
            btn->setStyleSheet(QString());
            ui->keyboardPreviewWidget->setZoneColor(zoneIndex, QColor(0,0,0));
            return;
        }

        auto rgb = hexToRgb(hex);
        if (!rgb) {
            btn->setStyleSheet(QString());
            ui->keyboardPreviewWidget->setZoneColor(zoneIndex, QColor(0,0,0));
            return;
        }

        ui->keyboardPreviewWidget->setZoneColor(zoneIndex, *rgb);
        setBtnSwatch(btn, hex);
    };

    updateOne(ui->editZ1, ui->btnColor1, 0);
    updateOne(ui->editZ2, ui->btnColor2, 1);
    updateOne(ui->editZ3, ui->btnColor3, 2);
    updateOne(ui->editZ4, ui->btnColor4, 3);
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

// ------------------------------------------------------------------
// Build params from UI
// ------------------------------------------------------------------
std::optional<LAParams> MainWindow::buildParamsFromUi() const
{
    int idx = m_effectGroup ? m_effectGroup->checkedId() : 0;
    QString mode;
    switch (idx) {
        case 0: mode = "static"; break;
        case 1: mode = "breath"; break;
        case 2: mode = "wave"; break;
        case 3: mode = "hue"; break;
        case 4: mode = "off"; break;
        default: mode = "static"; break;
    }

    LAParams p;
    p.speed      = ui->sliderSpeed->value();
    p.brightness = ui->sliderBrightness->value();
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

        std::array<QString,4> normalized;
        if (ui->chkAutofill->isChecked()) {
            normalized = normalize4(cols);
        } else {
            // Without autofill, require all 4 zones to be provided to avoid out-of-range access.
            if (cols.size() != 4) return std::nullopt;
            normalized = {cols[0], cols[1], cols[2], cols[3]};
        }

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

    int idx = m_effectGroup ? m_effectGroup->checkedId() : 0;
    if (idx == 4) {
        onOffClicked();
        return;
    }

    auto params = buildParamsFromUi();
    if (!params) {
        setStatusErr("Invalid color values.");
        return;
    }

    bool ok = kb_.apply(*params);
    if (ok) {
        LegionAura::saveUserConfig(*params);
        setStatusOk("Lighting updated.");
    } else {
        setStatusErr("Failed to send command.");
    }
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

    if (kb_.off()) {
        LAParams saved{LAEffect::Static, 1, 1, {}, LAWaveDir::None};
        saved.zones = {LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}};
        LegionAura::saveUserConfig(saved);
        setStatusOk("Keyboard turned off.");
    } else {
        setStatusErr("Failed to send off command.");
    }
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