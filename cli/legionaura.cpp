#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "legionaura.h"
#include <cctype>


static std::vector<std::string> normalize_colors(const std::vector<std::string>& in) {
    std::vector<std::string> out = in;

    // Trim/normalize hex case (optional)
    auto norm = [](std::string s) {
        // allow HEX like ff00ff or RGB like 255,0,255; just lowercase hex for consistency
        std::string t = s;
        std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return std::tolower(c); });
        return t;
    };
    for (auto& s : out) s = norm(s);

    if (out.empty()) return out;               // let caller decide if empty is allowed
    if (out.size() == 1) out = {out[0], out[0], out[0], out[0]};
    else if (out.size() == 2) out = {out[0], out[1], out[1], out[1]};
    else if (out.size() == 3) out = {out[0], out[1], out[2], out[2]};
    else if (out.size() > 4)  out.resize(4);   // ignore extras gracefully

    return out;
}

static void usage(const char* prog){
    std::cerr <<
      "Usage:\n"
      "  " << prog << " static <Z1> <Z2> <Z3> <Z4> [--brightness 1|2]\n"
      "  " << prog << " breath <Z1> <Z2> <Z3> <Z4> [--speed 1..4] [--brightness 1|2]\n"
      "  " << prog << " wave <ltr|rtl> [--speed 1..4] [--brightness 1|2]\n"
      "  " << prog << " hue [--speed 1..4] [--brightness 1|2]\n"
      "  " << prog << " off\n"
      "Notes: Z1..Z4 are hex colors RRGGBB.\n";
}

int main(int argc, char** argv){
    if (argc < 2){ usage(argv[0]); return 1; }

    std::string cmd = argv[1];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    uint8_t speed = 1, brightness = 1;
    LAWaveDir wdir = LAWaveDir::None;
    LAEffect eff;

    // We will fill this *after* normalization
    std::array<LAColor,4> zones;

    int i = 2;

    // Collect all colors into a vector<string> (raw)
    auto parseColorsDynamic = [&](std::vector<std::string>& rawColors) {
        while (i < argc && argv[i][0] != '-') {
            rawColors.push_back(argv[i]);
            i++;
        }
    };

    // Build final zones[] from normalized color strings
    auto fillZones = [&](const std::vector<std::string>& cvec){
        for (int z=0; z<4; ++z){
            auto c = LegionAura::parseHexRGB(cvec[z].c_str());
            if (!c){
                std::cerr << "Invalid color: " << cvec[z] << "\n";
                exit(2);
            }
            zones[z] = *c;
        }
    };

    // Command handling
    std::vector<std::string> userColors;

    if (cmd == "static") {
        eff = LAEffect::Static;
        parseColorsDynamic(userColors);
        if (userColors.empty()){
            std::cerr << "Error: static requires at least one color.\n";
            return 2;
        }
        userColors = normalize_colors(userColors);
        std::cerr << "Zones = [" << userColors[0] << ", " << userColors[1]
                  << ", " << userColors[2] << ", " << userColors[3] << "]\n";
        fillZones(userColors);

    } else if (cmd == "breath") {
        eff = LAEffect::Breath;
        parseColorsDynamic(userColors);
        if (userColors.empty()){
            std::cerr << "Error: breath requires at least one color.\n";
            return 2;
        }
        userColors = normalize_colors(userColors);
        std::cerr << "Zones = [" << userColors[0] << ", " << userColors[1]
                  << ", " << userColors[2] << ", " << userColors[3] << "]\n";
        fillZones(userColors);

    } else if (cmd == "wave") {
        eff = LAEffect::Wave;
        if (i >= argc){ std::cerr << "wave requires direction: ltr|rtl\n"; return 2; }
        std::string dir = argv[i++];
        if (dir == "ltr") wdir = LAWaveDir::LTR;
        else if (dir == "rtl") wdir = LAWaveDir::RTL;
        else { std::cerr << "Invalid wave direction: " << dir << "\n"; return 2; }

    } else if (cmd == "hue") {
        eff = LAEffect::Hue;

    } else if (cmd == "off") {
        LegionAura kb;
        if (!kb.open()){ std::cerr << "Device open failed.\n"; return 3; }
        bool ok = kb.off();
        std::cout << (ok ? "OK\n" : "FAIL\n");
        return ok ? 0 : 4;

    } else if (cmd == "brightness") {
    eff = LAEffect::None;   // special “no-mode-change” effect
    if (i >= argc) {
        std::cerr << "brightness requires level 1 or 2\n";
        return 2;
    }
        brightness = (uint8_t)std::stoi(argv[i++]);
    if (brightness < 1 || brightness > 2) {
        std::cerr << "brightness must be 1 or 2\n";
        return 2;
    }

    // We still fill zones with dummy data but apply() will ignore them
    for (auto& z : zones) z = LAColor{0,0,0};
    }

    else {
        usage(argv[0]);
        return 1;
    }

    // optional flags
    while (i < argc){
        std::string f = argv[i++];
        if (f == "--speed" && i<argc) {
            speed = (uint8_t)std::stoi(argv[i++]);
            if (speed<1 || speed>4){ std::cerr << "speed must be 1..4\n"; return 2; }
        } else if (f == "--brightness" && i<argc) {
            brightness = (uint8_t)std::stoi(argv[i++]);
            if (brightness<1 || brightness>2){ std::cerr << "brightness must be 1 or 2\n"; return 2; }
        } else {
            std::cerr << "Unknown arg: " << f << "\n"; 
            return 2;
        }
    }

    // Build params and apply
    LAParams p{eff, speed, brightness, zones, wdir};

    LegionAura kb;
    if (!kb.open()){
        std::cerr << "Device open failed. Are udev rules installed?\n";
        return 3;
    }

    bool ok = kb.apply(p);
    std::cout << (ok ? "OK\n" : "FAIL\n");
    return ok ? 0 : 4;
}
