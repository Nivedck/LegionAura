#include "ScreenSync.h"
#include <QGuiApplication>
#include <QVideoFrame>

ScreenSync::ScreenSync(LegionAura &keyboard, QObject *parent)
    : QObject(parent), m_keyboard(keyboard)
{
    m_current.fill(Qt::black);
    m_target = m_current;
    m_session.setScreenCapture(&m_capture);
    m_session.setVideoSink(&m_sink);
    // Continue fades on static screens; incoming frames are processed immediately.
    m_timer.setInterval(16);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &ScreenSync::tick);
    // Receive on the producer thread and post at most one GUI task. Never queue
    // full-resolution frames faster than the GUI can convert/sample them.
    connect(&m_sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (!m_acceptFrames || !frame.isValid()) return;
        m_pendingFrame = frame;
        if (m_framePosted) return;
        m_framePosted = true;
        const auto generation = m_frameGeneration;
        QMetaObject::invokeMethod(this, [this, generation] { processFrame(generation); }, Qt::QueuedConnection);
    }, Qt::DirectConnection);
    connect(&m_capture, &QScreenCapture::errorOccurred, this,
            [this](QScreenCapture::Error, const QString &message) {
        if (!m_running) return;
        stop();
        emit failed("Screen capture failed: " + message);
    });
    connect(&m_capture, &QScreenCapture::activeChanged, this, [this](bool active) {
        if (!active && m_running) {
            stop();
            emit failed("Screen sharing stopped or was cancelled. Press Apply to try again.");
        }
    });
}

ScreenSync::~ScreenSync()
{
    disconnect(&m_sink, nullptr, this, nullptr);
    stop();
}

void ScreenSync::start(int brightness)
{
    stop();
    const auto generation = ++m_generation;
    m_brightness = brightness;
    m_current.fill(Qt::black);
    m_target = m_current;
    m_haveFrame = false;
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_acceptFrames = true;
        m_frameGeneration = generation;
    }
    m_stopping = false;
    m_running = true;
    m_writer = std::thread([this, generation] {
        std::optional<LAParams> previous;
        while (true) {
            LAParams params;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_ready.wait(lock, [this] { return m_stopping || m_pending.has_value(); });
                if (m_stopping) return;
                params = *m_pending;
                m_pending.reset();
            }
            bool same = previous && previous->brightness == params.brightness;
            for (int i = 0; same && i < 4; ++i)
                same = previous->zones[i].r == params.zones[i].r &&
                       previous->zones[i].g == params.zones[i].g &&
                       previous->zones[i].b == params.zones[i].b;
            if (same) continue;
            if (!m_keyboard.apply(params)) {
                QMetaObject::invokeMethod(this, [this, generation] {
                    if (!m_running || generation != m_generation) return;
                    stop();
                    emit failed("Screen Sync stopped: keyboard write failed. Reconnect and press Apply.");
                }, Qt::QueuedConnection);
                return;
            }
            previous = params;
            // Send the latest pending colors immediately; slow transfers never build a queue.
        }
    });
    if (QGuiApplication::platformName() != "wayland")
        m_capture.setScreen(QGuiApplication::primaryScreen());
    m_timer.start();
    m_capture.start();
}

void ScreenSync::stop()
{
    m_running = false;
    ++m_generation;
    m_timer.stop();
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_acceptFrames = false;
        m_pendingFrame = {};
        m_framePosted = false;
    }
    m_capture.stop();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopping = true;
        m_pending.reset();
    }
    m_ready.notify_all();
    if (m_writer.joinable()) m_writer.join();
    m_haveFrame = false;
}

void ScreenSync::processFrame(quint64 generation)
{
    QVideoFrame frame;
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (generation != m_frameGeneration || !m_acceptFrames) return;
        frame = std::move(m_pendingFrame);
        m_pendingFrame = {};
        m_framePosted = false;
    }
    if (!m_running || generation != m_generation || !frame.isValid()) return;
    const auto image = frame.toImage();
    if (image.isNull()) return;
    m_target = sampleScreenColors(image);
    m_haveFrame = true;
    tick();
}

void ScreenSync::tick()
{
    if (!m_running) return;
    if (!m_haveFrame) return;
    // Some backends emit only on screen changes; keep converging to the last target.
    m_current = smoothScreenColors(m_current, m_target);
    LAParams params{LAEffect::Static, 1, static_cast<uint8_t>(m_brightness), {}, LAWaveDir::None};
    for (int i = 0; i < 4; ++i)
        params.zones[i] = {static_cast<uint8_t>(m_current[i].red()),
                           static_cast<uint8_t>(m_current[i].green()),
                           static_cast<uint8_t>(m_current[i].blue())};
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending = params;
    }
    m_ready.notify_one();
    emit colorsChanged();
}
