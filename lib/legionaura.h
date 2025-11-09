#pragma once
#include <cstdint>
#include <array>
#include <optional>
#include <string>
#include <vector>
#include <libusb-1.0/libusb.h>

struct LAColor { uint8_t r, g, b; };

enum class LAEffect : uint8_t {
    None   = 0x00,  // brightness-only mode
    Static = 0x01,
    Breath = 0x03,
    Wave   = 0x04,
    Hue    = 0x06
};

enum class LAWaveDir : uint8_t {
    None=0, LTR, RTL
};

struct LAParams {
    LAEffect effect;
    uint8_t speed;
    uint8_t brightness;
    std::array<LAColor,4> zones;
    LAWaveDir waveDir;
};

class LegionAura {
public:
    LegionAura(uint16_t vid = 0x048D, uint16_t pid = 0xC993);
    ~LegionAura();

    bool open();
    bool autoDetect();   // loads VID/PIDs from devices/devices.json and opens first match
    void close();

    bool apply(const LAParams& p);
    bool off();

    bool setBrightnessOnly(uint8_t level); // change only brightness, keep current mode/colors
    bool readState(LAParams& out);         // read current device state (effect/speed/brightness/colors)

    static std::optional<LAColor> parseHexRGB(const std::string& hex); // "RRGGBB"
    static std::vector<std::pair<uint16_t,uint16_t>>
    loadSupportedDevices(const std::string& path);

    // ----------------------------------------------------
    // NEW: Safe accessors for GUI
    // ----------------------------------------------------
    uint16_t getVid() const { return vid_; }
    uint16_t getPid() const { return pid_; }

private:
    std::vector<uint8_t> buildPayload(const LAParams& p);
    bool ctrlSendCC(const std::vector<uint8_t>& data);

    uint16_t vid_, pid_;
    libusb_context* ctx_ = nullptr;
    libusb_device_handle* dev_ = nullptr;
    int iface_ = 0;
};
