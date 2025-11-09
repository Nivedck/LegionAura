// /LegionAura/lib/legionaura.cpp

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <regex>
#include <string>

#include "legionaura.h"
#include <iostream>

static uint8_t clampByte(int v){ return (uint8_t)std::max(0,std::min(255,v)); }


LegionAura::LegionAura(uint16_t vid, uint16_t pid) : vid_(vid), pid_(pid) {}
LegionAura::~LegionAura(){ close(); }

bool LegionAura::open() {
    if (ctx_) return true;

    if (libusb_init(&ctx_) != 0) {
        std::cerr << "libusb initialization failed.\n";
        return false;
    }

    dev_ = libusb_open_device_with_vid_pid(ctx_, vid_, pid_);
    if (!dev_) {

        // Permission hint:
        std::cerr <<
            "Device open failed.\n"
            "Permission denied. The keyboard device cannot be accessed.\n\n"
            "Solutions:\n"
            "  1) Run the command with sudo:\n"
            "       sudo legionaura <command>\n\n"
            "  2) Or install the udev rules for non-root access:\n"
            "       sudo cp udev/10-legionaura.rules /etc/udev/rules.d/\n"
            "       sudo udevadm control --reload-rules\n"
            "       sudo udevadm trigger\n\n"
            "After installing the rules, unplug and reconnect the keyboard.\n";

        libusb_exit(ctx_);
        ctx_ = nullptr;
        return false;
    }

    // Permission OK, now try interface
    if (libusb_kernel_driver_active(dev_, iface_) == 1)
        libusb_detach_kernel_driver(dev_, iface_);

    int claim = libusb_claim_interface(dev_, iface_);
    if (claim != 0) {
        std::cerr <<
            "Failed to claim USB interface. This usually means:\n"
            "  • Another program is using the device\n"
            "  • You need sudo\n"
            "  • Or udev rules are not applied\n";

        libusb_close(dev_);
        dev_ = nullptr;
        libusb_exit(ctx_);
        ctx_ = nullptr;
        return false;
    }

    return true;
}


void LegionAura::close() {
    if (!ctx_) return;

    if (dev_) {
        libusb_release_interface(dev_, iface_);
        libusb_close(dev_);
    }
    dev_ = nullptr;

    libusb_exit(ctx_);
    ctx_ = nullptr;
}

// AUTODETECT -------------------------------------------------------
std::vector<std::pair<uint16_t,uint16_t>>
LegionAura::loadSupportedDevices(const std::string& path)
{
    std::vector<std::pair<uint16_t,uint16_t>> list;

    std::ifstream f(path);
    if (!f.is_open()) return list;

    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();

    // raw-string with safe delimiter
    std::regex pidRegex(R"REGEX("pid"\s*:\s*"0x([0-9A-Fa-f]+)")REGEX");

    auto begin = std::sregex_iterator(content.begin(), content.end(), pidRegex);
    auto end   = std::sregex_iterator();

    const uint16_t VID = 0x048D;

    for (auto it = begin; it != end; ++it) {
        std::string hex = (*it)[1];
        uint16_t pid = (uint16_t)std::stoi(hex, nullptr, 16);
        list.emplace_back(VID, pid);
    }

    return list;
}

bool LegionAura::autoDetect() {
    bool createdCtx = false;
    if (!ctx_) {
        if (libusb_init(&ctx_) != 0) return false;
        createdCtx = true;
    }

    auto devices = loadSupportedDevices(std::string(PROJECT_SOURCE_DIR) + "/devices/devices.json");

    if (devices.empty()) {
        if (createdCtx) { libusb_exit(ctx_); ctx_ = nullptr; }
        return false;
    }

    for (auto& vp : devices) {
        uint16_t vid = vp.first, pid = vp.second;
        libusb_device_handle* h = libusb_open_device_with_vid_pid(ctx_, vid, pid);
        if (!h) continue;

        // Try to prepare the interface
        if (libusb_kernel_driver_active(h, iface_) == 1)
            libusb_detach_kernel_driver(h, iface_);

        if (libusb_claim_interface(h, iface_) == 0) {
            // success: adopt this handle
            vid_ = vid; pid_ = pid; dev_ = h;
            return true;
        }

        libusb_close(h);
    }

    if (createdCtx) { libusb_exit(ctx_); ctx_ = nullptr; }
    return false;
}


// --------------------------------------------------------------

bool LegionAura::apply(const LAParams& p) {
    auto payload = buildPayload(p);
    return ctrlSendCC(payload);
}

bool LegionAura::off() {
    LAParams p{LAEffect::Static, 1, 1, {}, LAWaveDir::None};
    p.zones = {LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}};
    return apply(p);
}

bool LegionAura::setBrightnessOnly(uint8_t level)
{
    LAParams cur;

    // // Read current state
    // if (!readState(cur)) {
    //     return false;
    // }

    // // Modify only brightness
    // cur.brightness = std::max<uint8_t>(1, std::min<uint8_t>(2, level));

    cur.effect = LAEffect::Static;
    cur.speed = 1;
    cur.brightness = std::max<uint8_t>(1, std::min<uint8_t>(2, level));
    cur.zones = {
        LAColor{255,255,255},
        LAColor{255,255,255},
        LAColor{255,255,255},
        LAColor{255,255,255}
    };
    cur.waveDir = LAWaveDir::None;


    // Re-apply full packet
    return apply(cur);
}



std::vector<uint8_t> LegionAura::buildPayload(const LAParams& p) {
    LAEffect eff = p.effect;
    // Only valid effects should be sent. If LAEffect::None leaks in,
    // reuse Static as a safe default (we never call buildPayload with None from CLI flow).
    if (!(eff == LAEffect::Static || eff == LAEffect::Breath ||
          eff == LAEffect::Wave   || eff == LAEffect::Hue)) {
        eff = LAEffect::Static;
    }

    uint8_t speed = std::max<uint8_t>(1, std::min<uint8_t>(4, p.speed));
    uint8_t bright = std::max<uint8_t>(1, std::min<uint8_t>(2, p.brightness));

    std::vector<uint8_t> d;
    d.reserve(32);
    d.push_back(0xCC);
    d.push_back(0x16);
    d.push_back(static_cast<uint8_t>(eff));
    d.push_back(speed);
    d.push_back(bright);

    bool needsColors = (eff == LAEffect::Static || eff == LAEffect::Breath);
    if (needsColors) {
        for (auto& z : p.zones) { d.push_back(z.r); d.push_back(z.g); d.push_back(z.b); }
    } else {
        d.insert(d.end(), 12, 0x00);
    }

    d.push_back(0x00); // unused

    uint8_t rtl=0, ltr=0;
    if (eff == LAEffect::Wave) {
        if (p.waveDir == LAWaveDir::RTL) rtl = 1;
        else if (p.waveDir == LAWaveDir::LTR) ltr = 1;
    }
    d.push_back(rtl);
    d.push_back(ltr);

    if (d.size() < 32) d.insert(d.end(), 32 - d.size(), 0x00);
    return d;
}


bool LegionAura::ctrlSendCC(const std::vector<uint8_t>& data) {
    if (!dev_) return false;

    int r = libusb_control_transfer(
        dev_,
        0x21,
        0x09,
        0x03CC,
        0x00,
        const_cast<unsigned char*>(data.data()),
        (uint16_t)data.size(),
        1000
    );

    return r == (int)data.size();
}

bool LegionAura::readState(LAParams& out)
{
    if (!dev_) return false;

    std::vector<uint8_t> buf(64);
    int r = libusb_control_transfer(
        dev_, 0xA1, 0x01, 0x03CC, 0x0000,
        buf.data(), (uint16_t)buf.size(), 1000
    );
    if (r < 20) return false; // need at least up to direction bytes

    out.effect     = static_cast<LAEffect>(buf[2]);
    out.speed      = buf[3];
    out.brightness = buf[4];

    if (out.effect == LAEffect::Static || out.effect == LAEffect::Breath) {
        // need 5..(5+12-1) available
        if (r < 17) return false;
        for (int i = 0; i < 4; i++) {
            out.zones[i].r = buf[5  + i*3 + 0];
            out.zones[i].g = buf[5  + i*3 + 1];
            out.zones[i].b = buf[5  + i*3 + 2];
        }
    } else {
        for (auto& z : out.zones) z = LAColor{0,0,0};
    }

    if (out.effect == LAEffect::Wave) {
        uint8_t rtl = buf[18], ltr = buf[19];
        out.waveDir = rtl ? LAWaveDir::RTL : (ltr ? LAWaveDir::LTR : LAWaveDir::None);
    } else {
        out.waveDir = LAWaveDir::None;
    }

    // Clamp
    if (out.speed < 1 || out.speed > 4) out.speed = 1;
    if (out.brightness < 1 || out.brightness > 2) out.brightness = 1;

    return true;
}



std::optional<LAColor> LegionAura::parseHexRGB(const std::string& s) {
    if (s.size()!=6) return std::nullopt;

    auto hex2 = [&](char a, char b)->std::optional<uint8_t>{
        auto hv=[&](char c)->int{
            if (std::isdigit((unsigned char)c)) return c - '0';
            if (std::isxdigit((unsigned char)c)) return 10 + (std::tolower(c) - 'a');
            return -1;
        };
        int hi=hv(a), lo=hv(b);
        if (hi<0 || lo<0) return std::nullopt;
        return (uint8_t)((hi<<4)|lo);
    };

    auto r=hex2(s[0],s[1]);
    auto g=hex2(s[2],s[3]);
    auto b=hex2(s[4],s[5]);

    if (!r||!g||!b) return std::nullopt;
    return LAColor{*r,*g,*b};
}