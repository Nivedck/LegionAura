#pragma once
#include <cstdint>
#include <array>
#include <optional>
#include <string>
#include <vector>

struct LAColor { uint8_t r, g, b; };

enum class LAEffect : uint8_t {
    Static  = 0x01,
    Breath  = 0x03,
    Wave    = 0x04,
    Hue     = 0x06
};

enum class LAWaveDir : uint8_t {
    None=0, LTR, RTL
};

struct LAParams {
    LAEffect effect;
    uint8_t speed;       // 1..4 (animated effects)
    uint8_t brightness;  // 1..2
    std::array<LAColor,4> zones; // ignored for non-static/breath
    LAWaveDir waveDir;   // only for wave
};

class LegionAura {
public:
    LegionAura(uint16_t vid = 0x048D, uint16_t pid = 0xC993);
    ~LegionAura();

    bool open();
    void close();
    bool apply(const LAParams& p);
    bool off();

    static std::optional<LAColor> parseHexRGB(const std::string& hex); // "RRGGBB"

private:
    std::vector<uint8_t> buildPayload(const LAParams& p);
    bool ctrlSendCC(const std::vector<uint8_t>& data);

    uint16_t vid_, pid_;
    struct libusb_context* ctx_ = nullptr;
    struct libusb_device_handle* dev_ = nullptr;
    int iface_ = 0; // interface 0 is typical for these ITE devices
};