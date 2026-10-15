#pragma once
#include "ScreenColors.h"
#include "legionaura.h"
#include <QObject>
#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QVideoSink>
#include <QTimer>
#include <QVideoFrame>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>

class ScreenSync : public QObject {
    Q_OBJECT
public:
    explicit ScreenSync(LegionAura &keyboard, QObject *parent = nullptr);
    ~ScreenSync() override;
    void start(int brightness);
    void stop(); // Joins any in-flight USB write before returning ownership to the GUI.
    void setBrightness(int brightness) { m_brightness = brightness; }
    ScreenColors colors() const { return m_current; }
signals:
    void colorsChanged();
    void failed(const QString &message);
private:
    void tick();
    void processFrame(quint64 generation);
    LegionAura &m_keyboard;
    QMediaCaptureSession m_session;
    QScreenCapture m_capture;
    QVideoSink m_sink;
    QTimer m_timer;
    ScreenColors m_current{}, m_target{};
    std::mutex m_frameMutex;
    QVideoFrame m_pendingFrame;
    bool m_framePosted = false, m_acceptFrames = false;
    quint64 m_frameGeneration = 0;
    bool m_running = false, m_haveFrame = false;
    int m_brightness = 1;
    quint64 m_generation = 0;
    std::thread m_writer;
    std::mutex m_mutex;
    std::condition_variable m_ready;
    bool m_stopping = true;
    std::optional<LAParams> m_pending;
};
