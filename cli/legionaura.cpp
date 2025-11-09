/LegionAura/cli/legionaura.cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include "legionaura.h"


// Nivedck -- @2025

#ifndef LEGIONAURA_VERSION
#define LEGIONAURA_VERSION "unknown"
#endif


// ------------------------------------------------------
// Normalize colors (1->4, 2->4, 3->4, >4->trim)
// ------------------------------------------------------
static std::vector<std::string> normalize_colors(const std::vector<std::string>& in) {
    if (in.empty()) return in;
    std::vector<std::string> out = in;

    for (auto& s : out) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
    }

    if (out.size() == 1) out = {out[0], out[0], out[0], out[0]};
    else if (out.size() == 2) out = {out[0], out[1], out[1], out[1]};
    else if (out.size() == 3) out = {out[0], out[1], out[2], out[2]};
    else if (out.size() > 4) out.resize(4);

    return out;
}

static void usage(const char* prog){
    std::cerr <<
      "Usage:\n\n"
      "  " << prog << " static <colors...> [--brightness 1|2]\n"
      "  " << prog << " breath <colors...> [--speed 1..4] [--brightness 1|2]\n"
      "  " << prog << " wave <ltr|rtl> [--speed 1..4] [--brightness 1|2]\n"
      "  " << prog << " hue [--speed 1..4] [--brightness 1|2]\n"
      "  " << prog << " off\n"
      "  " << prog << " --brightness 1|2        (brightness only)\n\n"
      "Notes:\n"
      "  • Colors must be hex RRGGBB (example: ff0000)\n"
      "  • If only 1–3 colors are given, remaining zones are auto-filled\n"
      "  • Brightness: 1 = low, 2 = high\n";
}

// ------------------------------------------------------
// MAIN FUNCTION ---- Nivedck
// ------------------------------------------------------
int main(int argc, char** argv){
    if (argc < 2){ usage(argv[0]); return 1; }




    std::string cmd = argv[1];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        // Help flags
    if (cmd == "-h" || cmd == "--help") {
        usage(argv[0]);
        return 0;
    }

    // Version flags
    if (cmd == "-v" || cmd == "--version") {
        std::cout << "LegionAura " << LEGIONAURA_VERSION << "\n";
        return 0;
    }


    uint8_t speed = 1, brightness = 1;
    LAWaveDir wdir = LAWaveDir::None;
    LAEffect eff = LAEffect::Static;
    std::array<LAColor,4> zones {LAColor{0,0,0},LAColor{0,0,0},LAColor{0,0,0},LAColor{0,0,0}};

    int i = 2;

    // ------------------------------------------------------
    // BRIGHTNESS ONLY MODE ( --brightness 2 )
    // ------------------------------------------------------
    if (cmd == "--brightness") {
        if (argc < 3) {
            std::cerr << "brightness requires 1 or 2\n";
            return 2;
        }

        brightness = (uint8_t)std::stoi(argv[2]);
        if (brightness < 1 || brightness > 2) {
            std::cerr << "brightness must be 1 or 2\n";
            return 2;
        }

        LegionAura kb;
        if (!kb.open()){ std::cerr << "Device open failed.\n"; return 3; }
        bool ok = kb.setBrightnessOnly(brightness);

        std::cout << (ok ? "OK\n" : "FAIL\n");
        return ok ? 0 : 4;
    }

    // ------------------------------------------------------
    // Helper lambdas
    // ------------------------------------------------------
    std::vector<std::string> rawColors;

    auto parseDynamicColors = [&](std::vector<std::string>& out){
        while (i < argc && argv[i][0] != '-') {
            out.push_back(argv[i]);
            i++;
        }
    };

    auto fillZones = [&](const std::vector<std::string>& cvec){
        for (int z=0; z<4; ++z){
            auto c = LegionAura::parseHexRGB(cvec[z]);
            if (!c){
                std::cerr << "Invalid color: " << cvec[z] << "\n";
                exit(2);
            }
            zones[z] = *c;
        }
    };

    // ------------------------------------------------------
    // COMMANDS
    // ------------------------------------------------------
    if (cmd == "static") {
        eff = LAEffect::Static;
        parseDynamicColors(rawColors);
        if (rawColors.empty()){ std::cerr << "static needs at least 1 color\n"; return 2; }
        rawColors = normalize_colors(rawColors);
        fillZones(rawColors);

    } else if (cmd == "breath") {
        eff = LAEffect::Breath;
        parseDynamicColors(rawColors);
        if (rawColors.empty()){ std::cerr << "breath needs 1+ colors\n"; return 2; }
        rawColors = normalize_colors(rawColors);
        fillZones(rawColors);

    } else if (cmd == "wave") {
        eff = LAEffect::Wave;
        if (i >= argc){
            std::cerr << "wave requires direction ltr|rtl\n";
            return 2;
        }
        std::string d = argv[i++];
        if (d == "ltr") wdir = LAWaveDir::LTR;
        else if (d == "rtl") wdir = LAWaveDir::RTL;
        else { std::cerr << "Invalid direction\n"; return 2; }

    } else if (cmd == "hue") {
        eff = LAEffect::Hue;

    } else if (cmd == "off") {
        LegionAura kb;
        if (!kb.open()){ std::cerr << "Device open failed.\n"; return 3; }
        bool ok = kb.off();
        std::cout << (ok ? "OK\n" : "FAIL\n");
        return ok ? 0 : 4;

    } else {
        usage(argv[0]);
        return 1;
    }

    // ------------------------------------------------------
    // OPTIONAL FLAGS
    // ------------------------------------------------------
    while (i < argc){
        std::string f = argv[i++];
        if (f == "--speed" && i<argc) {
            speed = (uint8_t)std::stoi(argv[i++]);
            if (speed<1 || speed>4){
                std::cerr << "speed must be 1..4\n";
                return 2;
            }
        } else if (f == "--brightness" && i<argc) {
            brightness = (uint8_t)std::stoi(argv[i++]);
            if (brightness<1 || brightness>2){
                std::cerr << "brightness must be 1 or 2\n";
                return 2;
            }
        } else {
            std::cerr << "Unknown arg: " << f << "\n";
            return 2;
        }
    }

    // ------------------------------------------------------
    // APPLY FINAL PARAMETERS
    // ------------------------------------------------------
    LAParams p{eff, speed, brightness, zones, wdir};

    LegionAura kb;
    if (!kb.open()){
        std::cerr << "Device open failed.\n";
        return 3;
    }

    bool ok = kb.apply(p);
    std::cout << (ok ? "OK\n" : "FAIL\n");
    return ok ? 0 : 4;
}