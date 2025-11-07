#include "legionaura.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <regex>

static uint8_t clampByte(int v){ return (uint8_t)std::max(0,std::min(255,v)); }


LegionAura::LegionAura(uint16_t vid, uint16_t pid) : vid_(vid), pid_(pid) {}
LegionAura::~LegionAura(){ close(); }

bool LegionAura::open() {
    if (ctx_) return true;
    if (libusb_init(&ctx_) != 0) return false;

    dev_ = libusb_open_device_with_vid_pid(ctx_, vid_, pid_);
    if (!dev_) {
        libusb_exit(ctx_);
        ctx_ = nullptr;
        return false;
    }

    if (libusb_kernel_driver_active(dev_, iface_) == 1)
        libusb_detach_kernel_driver(dev_, iface_);

    if (libusb_claim_interface(dev_, iface_) != 0) {
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
    if (libusb_init(&ctx_) != 0)
        return false;

    auto devices = loadSupportedDevices("devices/devices.json");
    if (devices.empty())
        return false;

    for (auto& vp : devices)
    {
        uint16_t vid = vp.first;
        uint16_t pid = vp.second;

        libusb_device_handle* handle =
            libusb_open_device_with_vid_pid(ctx_, vid, pid);

        if (handle)
        {
            vid_ = vid;
            pid_ = pid;
            dev_ = handle;

            if (libusb_kernel_driver_active(dev_, iface_) == 1)
                libusb_detach_kernel_driver(dev_, iface_);

            if (libusb_claim_interface(dev_, iface_) == 0)
                return true;

            libusb_close(dev_);
            dev_ = nullptr;
        }
    }

    libusb_exit(ctx_);
    ctx_ = nullptr;
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

    // Read current state
    if (!readState(cur)) {
        return false;
    }

    // Modify only brightness
    cur.brightness = std::max<uint8_t>(1, std::min<uint8_t>(2, level));

    // Re-apply full packet
    return apply(cur);
}



std::vector<uint8_t> LegionAura::buildPayload(const LAParams& p) {
    std::vector<uint8_t> d;
    d.reserve(32);

    d.push_back(0xCC);
    d.push_back(0x16);
    d.push_back(static_cast<uint8_t>(p.effect));
    d.push_back(p.speed);
    d.push_back(p.brightness);

    bool needsColors = (p.effect == LAEffect::Static || p.effect == LAEffect::Breath);

    if (needsColors) {
        for (auto& z : p.zones) {
            d.push_back(z.r);
            d.push_back(z.g);
            d.push_back(z.b);
        }
    } else {
        d.insert(d.end(), 12, 0x00);
    }

    d.push_back(0x00);
    uint8_t rtl=0, ltr=0;
    if (p.effect == LAEffect::Wave) {
        if (p.waveDir == LAWaveDir::RTL) rtl = 1;
        if (p.waveDir == LAWaveDir::LTR) ltr = 1;
    }
    d.push_back(rtl);
    d.push_back(ltr);

    if (d.size() < 32)
        d.insert(d.end(), 32 - d.size(), 0x00);

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

    // 64-byte buffer to read the feature report
    std::vector<uint8_t> buf(64);

    // libusb HID GET_REPORT
    // bmRequestType: 0xA1 (device→host, class, interface)
    // bRequest: 0x01 = GET_REPORT
    // wValue: 0x03CC = (FEATURE << 8) | ReportID(0xCC)
    int r = libusb_control_transfer(
        dev_,
        0xA1,
        0x01,
        0x03CC,
        0x0000,
        buf.data(),
        buf.size(),
        1000
    );

    if (r < 5) return false;

    // Parse
    out.effect     = static_cast<LAEffect>(buf[2]);
    out.speed      = buf[3];
    out.brightness = buf[4];

    // Colors (only for static/breath)
    if (out.effect == LAEffect::Static || out.effect == LAEffect::Breath) {
        for (int i = 0; i < 4; i++) {
            out.zones[i].r = buf[5 + i*3 + 0];
            out.zones[i].g = buf[5 + i*3 + 1];
            out.zones[i].b = buf[5 + i*3 + 2];
        }
    } else {
        for (auto& z : out.zones) z = LAColor{0,0,0};
    }

    // Wave direction (for wave)
    if (out.effect == LAEffect::Wave) {
        uint8_t rtl = buf[18];
        uint8_t ltr = buf[19];
        if (rtl) out.waveDir = LAWaveDir::RTL;
        else if (ltr) out.waveDir = LAWaveDir::LTR;
        else out.waveDir = LAWaveDir::None;
    } else {
        out.waveDir = LAWaveDir::None;
    }

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
