// /LegionAura/lib/legionaura.cpp

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <regex>
#include <string>

#include "legionaura.h"
#include <iostream>

static uint8_t clampByte(int v){ return (uint8_t)std::max(0,std::min(255,v)); }

namespace {
constexpr uint16_t kLenovoIteVid = 0x048D;

std::string toLowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return (char)std::tolower(c);
    });
    return s;
}

std::string effectToString(LAEffect e)
{
    switch (e) {
        case LAEffect::Static: return "static";
        case LAEffect::Breath: return "breath";
        case LAEffect::Wave:   return "wave";
        case LAEffect::Hue:    return "hue";
        case LAEffect::None:   return "none";
        default:               return "static";
    }
}

std::optional<LAEffect> effectFromString(const std::string& s)
{
    std::string v = toLowerCopy(s);
    if (v == "static") return LAEffect::Static;
    if (v == "breath") return LAEffect::Breath;
    if (v == "wave")   return LAEffect::Wave;
    if (v == "hue")    return LAEffect::Hue;
    if (v == "none")   return LAEffect::None;
    if (v == "off")    return LAEffect::Static; // off is represented by static + black zones
    return std::nullopt;
}

std::string waveDirToString(LAWaveDir d)
{
    switch (d) {
        case LAWaveDir::LTR:  return "ltr";
        case LAWaveDir::RTL:  return "rtl";
        case LAWaveDir::None: return "none";
        default:              return "none";
    }
}

LAWaveDir waveDirFromString(const std::string& s)
{
    std::string v = toLowerCopy(s);
    if (v == "ltr") return LAWaveDir::LTR;
    if (v == "rtl") return LAWaveDir::RTL;
    return LAWaveDir::None;
}

std::string rgbToHex(const LAColor& c)
{
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(6);
    out[0] = hex[(c.r >> 4) & 0xF];
    out[1] = hex[(c.r >> 0) & 0xF];
    out[2] = hex[(c.g >> 4) & 0xF];
    out[3] = hex[(c.g >> 0) & 0xF];
    out[4] = hex[(c.b >> 4) & 0xF];
    out[5] = hex[(c.b >> 0) & 0xF];
    return out;
}

std::vector<std::string> normalizeHexColors(std::vector<std::string> in)
{
    if (in.empty()) return in;
    for (auto& s : in) s = toLowerCopy(s);

    if (in.size() == 1) return {in[0], in[0], in[0], in[0]};
    if (in.size() == 2) return {in[0], in[1], in[1], in[1]};
    if (in.size() == 3) return {in[0], in[1], in[2], in[2]};
    if (in.size() > 4)  in.resize(4);
    return in;
}

bool devicePresentOnBus(libusb_context* ctx, uint16_t vid, uint16_t pid)
{
    libusb_device** list = nullptr;
    ssize_t n = libusb_get_device_list(ctx, &list);
    if (n < 0 || !list) return false;

    bool found = false;
    for (ssize_t i = 0; i < n; ++i) {
        libusb_device_descriptor desc{};
        if (libusb_get_device_descriptor(list[i], &desc) != 0) continue;
        if (desc.idVendor == vid && desc.idProduct == pid) {
            found = true;
            break;
        }
    }

    libusb_free_device_list(list, 1);
    return found;
}

std::vector<std::pair<uint16_t, uint16_t>> builtInSupportedDevices()
{
    // This project targets Lenovo laptops with the ITE 8295 RGB controller.
    // The set of known PIDs is small and hardware-dependent; keep a built-in
    // fallback so packaged installs still work if devices.json isn't found.
    static const uint16_t pids[] = {
        0xC995, 0xC994, 0xC993,
        0xC985, 0xC984, 0xC983,
        0xC975, 0xC973,
        0xC965, 0xC963,
        0xC955,
    };

    std::vector<std::pair<uint16_t, uint16_t>> list;
    list.reserve(sizeof(pids) / sizeof(pids[0]));
    for (auto pid : pids) list.emplace_back(kLenovoIteVid, pid);
    return list;
}

std::vector<std::string> splitColonList(const char* s)
{
    std::vector<std::string> parts;
    if (!s || !*s) return parts;

    std::string cur;
    for (const char* p = s; ; ++p) {
        if (*p == ':' || *p == '\0') {
            if (!cur.empty()) parts.push_back(cur);
            cur.clear();
            if (*p == '\0') break;
        } else {
            cur.push_back(*p);
        }
    }
    return parts;
}

std::optional<std::string> resolveDevicesJsonPath()
{
    namespace fs = std::filesystem;

    auto existsFile = [&](const fs::path& p) -> bool {
        std::error_code ec;
        return fs::exists(p, ec) && fs::is_regular_file(p, ec);
    };

    // 1) Explicit override
    if (const char* env = std::getenv("LEGIONAURA_DEVICES_JSON")) {
        fs::path p(env);
        if (existsFile(p)) return p.string();
    }

    // 2) Compiled-in install location
#ifdef LEGIONAURA_DEVICES_JSON_PATH
    {
        fs::path p(LEGIONAURA_DEVICES_JSON_PATH);
        if (existsFile(p)) return p.string();
    }
#endif

    // 3) XDG data locations
    {
        if (const char* xdgHome = std::getenv("XDG_DATA_HOME")) {
            fs::path p = fs::path(xdgHome) / "legionaura" / "devices.json";
            if (existsFile(p)) return p.string();
        } else if (const char* home = std::getenv("HOME")) {
            fs::path p = fs::path(home) / ".local" / "share" / "legionaura" / "devices.json";
            if (existsFile(p)) return p.string();
        }

        const char* xdgDirs = std::getenv("XDG_DATA_DIRS");
        if (!xdgDirs || !*xdgDirs) xdgDirs = "/usr/local/share:/usr/share";
        for (const auto& dir : splitColonList(xdgDirs)) {
            fs::path p = fs::path(dir) / "legionaura" / "devices.json";
            if (existsFile(p)) return p.string();
        }
    }

    // 4) Dev-tree fallback
#ifdef PROJECT_SOURCE_DIR
    {
        fs::path p = fs::path(PROJECT_SOURCE_DIR) / "devices" / "devices.json";
        if (existsFile(p)) return p.string();
    }
#endif

    return std::nullopt;
}

std::string defaultUserConfigPathInternal()
{
    namespace fs = std::filesystem;

    if (const char* env = std::getenv("LEGIONAURA_CONFIG")) {
        return fs::path(env).string();
    }

    if (const char* xdg = std::getenv("XDG_CONFIG_HOME")) {
        return (fs::path(xdg) / "legionaura" / "config.json").string();
    }

    if (const char* home = std::getenv("HOME")) {
        return (fs::path(home) / ".config" / "legionaura" / "config.json").string();
    }

    // Last resort: relative path
    return "./legionaura-config.json";
}
} // namespace


LegionAura::LegionAura(uint16_t vid, uint16_t pid) : vid_(vid), pid_(pid) {}
LegionAura::~LegionAura(){ close(); }

std::string LegionAura::defaultUserConfigPath()
{
    return defaultUserConfigPathInternal();
}

bool LegionAura::saveUserConfig(const LAParams& p, std::string path)
{
    namespace fs = std::filesystem;

    if (path.empty()) path = defaultUserConfigPathInternal();
    fs::path outPath(path);

    std::error_code ec;
    fs::create_directories(outPath.parent_path(), ec);

    std::ofstream f(outPath);
    if (!f.is_open()) return false;

    // Represent "off" as static + black zones.
    bool isOff = (p.effect == LAEffect::Static);
    for (const auto& z : p.zones) {
        if (z.r != 0 || z.g != 0 || z.b != 0) { isOff = false; break; }
    }

    f << "{\n";
    f << "  \"version\": 1,\n";
    f << "  \"effect\": \"" << (isOff ? "off" : effectToString(p.effect)) << "\",\n";
    f << "  \"speed\": " << (unsigned)p.speed << ",\n";
    f << "  \"brightness\": " << (unsigned)p.brightness << ",\n";
    f << "  \"waveDir\": \"" << waveDirToString(p.waveDir) << "\",\n";
    f << "  \"zones\": [\n";
    for (int i = 0; i < 4; ++i) {
        f << "    \"" << rgbToHex(p.zones[i]) << "\"";
        f << (i == 3 ? "\n" : ",\n");
    }
    f << "  ]\n";
    f << "}\n";

    return true;
}

std::optional<LAParams> LegionAura::loadUserConfig(std::string path)
{
    if (path.empty()) path = defaultUserConfigPathInternal();

    std::ifstream f(path);
    if (!f.is_open()) return std::nullopt;

    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();

    auto findString = [&](const char* key) -> std::optional<std::string> {
        std::regex re(std::string("\\\"") + key + "\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"");
        std::smatch m;
        if (!std::regex_search(content, m, re)) return std::nullopt;
        return m[1].str();
    };
    auto findInt = [&](const char* key) -> std::optional<int> {
        std::regex re(std::string("\\\"") + key + "\\\"\\s*:\\s*(\\d+)");
        std::smatch m;
        if (!std::regex_search(content, m, re)) return std::nullopt;
        try { return std::stoi(m[1].str()); } catch (...) { return std::nullopt; }
    };

    auto effectStr = findString("effect");
    auto speedVal = findInt("speed");
    auto brightVal = findInt("brightness");
    auto waveStr = findString("waveDir");

    if (!effectStr || !speedVal || !brightVal) return std::nullopt;

    std::string effLower = toLowerCopy(*effectStr);
    bool isOff = (effLower == "off");
    auto eff = effectFromString(*effectStr);
    if (!eff) return std::nullopt;

    std::vector<std::string> zonesHex;
    {
        std::regex zonesBlock(R"REGEX("zones"\s*:\s*\[([^\]]*)\])REGEX", std::regex::icase);
        std::smatch m;
        if (std::regex_search(content, m, zonesBlock)) {
            std::string inside = m[1].str();
            std::regex item(R"REGEX("([0-9A-Fa-f]{6})")REGEX");
            for (auto it = std::sregex_iterator(inside.begin(), inside.end(), item);
                 it != std::sregex_iterator(); ++it) {
                zonesHex.push_back((*it)[1].str());
            }
        }
    }

    LAParams p;
    p.effect = *eff;
    p.speed = (uint8_t)std::max(1, std::min(4, *speedVal));
    p.brightness = (uint8_t)std::max(1, std::min(2, *brightVal));
    p.waveDir = waveStr ? waveDirFromString(*waveStr) : LAWaveDir::None;

    if (isOff) {
        p.effect = LAEffect::Static;
        p.zones = {LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}};
        p.waveDir = LAWaveDir::None;
        p.speed = 1;
        p.brightness = (uint8_t)std::max(1, std::min(2, *brightVal));
        return p;
    }

    if (p.effect == LAEffect::Static || p.effect == LAEffect::Breath) {
        zonesHex = normalizeHexColors(std::move(zonesHex));
        if (zonesHex.size() != 4) return std::nullopt;
        for (int i = 0; i < 4; ++i) {
            auto c = parseHexRGB(zonesHex[i]);
            if (!c) return std::nullopt;
            p.zones[i] = *c;
        }
    } else {
        p.zones = {LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}, LAColor{0,0,0}};
    }

    return p;
}

bool LegionAura::open() {
    if (ctx_) return true;

    if (libusb_init(&ctx_) != 0) {
        std::cerr << "libusb initialization failed.\n";
        return false;
    }

    dev_ = libusb_open_device_with_vid_pid(ctx_, vid_, pid_);
    if (!dev_) {

        const bool present = devicePresentOnBus(ctx_, vid_, pid_);
        if (!present) {
            std::cerr <<
                "Device open failed.\n"
                "No supported device found at VID:PID "
                << std::hex << std::showbase << vid_ << ":" << pid_ << std::dec << "\n\n"
                "If your keyboard shows up in 'lsusb' with a different PID, use auto-detect\n"
                "(the GUI does this automatically) or ensure devices.json includes your PID.\n";
        } else {
            // Permission/busy hint:
            std::cerr <<
                "Device open failed.\n"
                "The device exists, but cannot be opened. This is usually a permissions issue\n"
                "or another driver/program is holding the interface.\n\n"
                "Solutions:\n"
                "  1) Run the command with sudo:\n"
                "       sudo legionaura <command>\n\n"
                "  2) Or install the udev rules for non-root access:\n"
                "       sudo cp udev/10-legionaura.rules /etc/udev/rules.d/\n"
                "       sudo udevadm control --reload-rules\n"
                "       sudo udevadm trigger\n\n"
                "After installing the rules, unplug and reconnect the keyboard.\n";
        }

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

    const uint16_t VID = kLenovoIteVid;

    for (auto it = begin; it != end; ++it) {
        std::string hex = (*it)[1];
        try {
            uint16_t pid = (uint16_t)std::stoi(hex, nullptr, 16);
            list.emplace_back(VID, pid);
        } catch (...) {
            // Ignore malformed entries
        }
    }

    return list;
}

bool LegionAura::autoDetect() {
    bool createdCtx = false;
    if (!ctx_) {
        if (libusb_init(&ctx_) != 0) return false;
        createdCtx = true;
    }

    std::vector<std::pair<uint16_t, uint16_t>> devices;
    if (auto path = resolveDevicesJsonPath()) {
        devices = loadSupportedDevices(*path);
    }
    if (devices.empty()) {
        devices = builtInSupportedDevices();
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
        const bool softDim = (bright == 1);
        const float scale = softDim ? 0.6f : 1.0f;
        for (auto& z : p.zones) {
            const uint8_t r = softDim ? clampByte((int)(z.r * scale)) : z.r;
            const uint8_t g = softDim ? clampByte((int)(z.g * scale)) : z.g;
            const uint8_t b = softDim ? clampByte((int)(z.b * scale)) : z.b;
            d.push_back(r); d.push_back(g); d.push_back(b);
        }
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