# LegionAura

<p align="center">
  <img src="https://raw.githubusercontent.com/nivedck/LegionAura/main/assets/logo.png" alt="LegionAura Logo" width="200"/>
</p>

<p align="center">
  <strong>An open-source RGB keyboard lighting controller for Lenovo LOQ, Legion, and IdeaPad Gaming laptops on Linux.</strong>
</p>

<p align="center">
  <a href="https://github.com/nivedck/LegionAura/blob/main/LICENSE"><img src="https://img.shields.io/github/license/nivedck/LegionAura" alt="License"></a>
  <a href="https://github.com/nivedck/LegionAura/issues"><img src="https://img.shields.io/github/issues/nivedck/LegionAura" alt="Issues"></a>
  <a href="https://github.com/nivedck/LegionAura/stargazers"><img src="https://img.shields.io/github/stars/nivedck/LegionAura" alt="Stars"></a>
</p>

> LegionAura provides full control over the built-in 4-zone RGB ITE keyboard without requiring Lenovo Vantage or Windows. It is lightweight, fast, and designed to work entirely through USB HID control transfers, replicating the behavior of Lenovo’s firmware-level lighting commands.

---

## 📋 Table of Contents

- [✨ Features](#-features)
- [🎯 Why LegionAura Exists](#-why-legionaura-exists)
- [🖥️ Supported Devices](#️-supported-devices)
- [GUI Screenshots](#-gui-screenshots)
- [🛠️ How It Works](#️-how-it-works)
- [🚀 Getting Started](#-getting-started)
  - [Installation](#installation)
  - [Building from Source](#building-from-source)
- [💡 Usage](#-usage)
- [🤝 Contributing](#-contributing)
- [📜 License](#-license)

---

## ✨ Features

* ✅ 4-zone RGB lighting control
* ✅ Static, Breath, Wave, and Hue effects
* ✅ Per-zone custom colors (HEX RRGGBB)
* ✅ Animation speed control (1–4)
* ✅ Brightness control (1–2)
* ✅ Wave direction (LTR / RTL)
* ✅ Brightness-only mode
* ✅ Safe color auto-fill
    * 1 color → applies to all 4 zones
    * 2 colors → Z1, Z2, Z2, Z2
    * 3 colors → Z1, Z2, Z3, Z3
* ✅ Simple CLI with human-friendly commands
* ✅ GUI (Qt6) for easy control
* ✅ C++17/libusb backend

---

## 🎯 Why LegionAura Exists

Lenovo does not officially provide Linux support for multi-zone RGB keyboard lighting on LOQ/Legion/IdeaPad gaming laptops.

Most devices expose only raw HID/USB interfaces with undocumented control packets. LegionAura implements a clean, fully-open library and CLI based on reverse-engineering and community research.

The goal is to provide:
* a stable command-line controller
* a reusable C++ library
* a GUI that mirrors Lenovo Vantage’s lighting controls
* support for multiple Lenovo gaming models

---

## 🖥️ Supported Devices

All laptops using the **ITE 8295 RGB controller** over USB HID are supported.

While the tool should be compatible with a wide range of Lenovo gaming laptops, it has been tested and confirmed to work with the following models:

*   **Legion Series (2020-2024)**
    *   Legion Pro
    *   Legion Regular/Slim
*   **LOQ Series (2023-2024)**
*   **IdeaPad Gaming Series (2021-2022)**

A more detailed list of models includes:
- Legion Pro (2024)
- Legion Regular/Slim (2024)
- Lenovo LOQ (2024)
- Legion Pro (2023)
- Legion Slim (2023)
- Lenovo LOQ (2023)
- Legion Pro/Regular (2022)
- IdeaPad Gaming (2022)
- Legion Pro/Regular (2021)
- IdeaPad Gaming (2021)
- Legion Pro/Regular (2020)

If your device is not on this list but uses an ITE 8295 controller, it will likely work. You can contribute by adding your device's PID to `devices/devices.json` and submitting a pull request.

---

## GUI Screenshots

> Add screenshot file here

---

## 🛠️ How It Works

LegionAura communicates with the keyboard’s ITE controller using a single USB `SET_REPORT` control transfer.

---

## 🚀 Getting Started

There are two ways to install LegionAura: through an AUR helper (for Arch-based distributions) or by building it manually.

### ✅ Method 1 — Install from the AUR (Arch Linux)

If you are on Arch Linux or an Arch-based distribution, you can install LegionAura from the [Arch User Repository (AUR)](https://aur.archlinux.org/packages/legionaura-git).

Use your favorite AUR helper.

**Using `yay`:**
```bash
yay -S legionaura
```

**Using `paru`:**
```bash
paru -S legionaura
```

The package will automatically handle dependencies, build the project, and install it to your system.

### ✅ Method 2 — Build Manually (Any Linux Distribution)

If you are not on an Arch-based distribution or prefer to build the project yourself, follow these steps.

**1. Install Dependencies**

First, you need to install the required build tools and libraries.

*   **A C++17 compatible compiler:** `gcc` or `clang`
*   **CMake:** Version 3.16 or later
*   **libusb:** Version 1.0 or later
*   **Qt6:** For the GUI
*   **Git:** To clone the repository

On **Debian/Ubuntu-based** distributions, you can install them with:
```bash
sudo apt update
sudo apt install build-essential cmake libusb-1.0-0-dev qt6-base-dev git
```

On **Fedora/RHEL-based** distributions, you can install them with:
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake libusb1-devel qt6-qtbase-devel git
```

**2. Clone the Repository**

```bash
git clone https://github.com/nivedck/LegionAura.git
cd LegionAura
```

**3. Build and Install**

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

This will compile the project and install the `legionaura` executable to `/usr/local/bin` and the GUI to `/usr/local/bin/legionaura-gui`, making it available system-wide.

**4. Set Up udev Rules**

To allow LegionAura to access the keyboard without running it as root, you need to install a `udev` rule. This gives your user account permission to control the device.

```bash
sudo cp ../udev/10-legionaura.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

After this, unplug and reconnect your keyboard (or reboot your system) for the changes to take full effect. You can now run `legionaura` as a regular user.

---

## 💡 Usage

### CLI

```
Usage:
  legionaura static <colors...> [--brightness 1|2]
  legionaura breath <colors...> [--speed 1..4] [--brightness 1|2]
  legionaura wave <ltr|rtl> [--speed 1..4] [--brightness 1|2]
  legionaura hue [--speed 1..4] [--brightness 1|2]
  legionaura off
  legionaura --brightness 1|2    (brightness only)
```

**Examples:**

* Set a static color for all zones:
  ```bash
  ./build/cli/legionaura static ff0000
  ```

* Set a breathing effect with custom colors:
  ```bash
  ./build/cli/legionaura breath ff0000 00ff00 0000ff
  ```

* Set a wave effect from left to right:
  ```bash
  ./build/cli/legionaura wave ltr --speed 2
  ```

### GUI

You can also use the GUI for easy control. Launch it from your application menu or by running `legionaura-gui` in your terminal.

---

## 🤝 Contributing

Contributions are welcome! If you have a feature request, bug report, or want to contribute to the code, please open an issue or a pull request.

---

## ⚠️ Disclaimer

This tool modifies your keyboard's firmware settings. The author is not responsible for any damage that may occur to your device. **Use at your own risk.**

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.