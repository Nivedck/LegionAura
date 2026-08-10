# LegionAura

[![AUR version](https://shields.io)](https://archlinux.org)
[![License](https://shields.io)](LICENSE)

A lightweight C++ and Qt6 RGB keyboard lighting controller for Lenovo LOQ, Legion, and IdeaPad Gaming laptops running Linux. 

LegionAura communicates with the built-in 4-zone ITE RGB keyboard controller via USB (`HID SET_REPORT`), allowing you to completely customize your keyboard lighting without heavy background daemons.

## Features

- **Dual Interface:** Comes with both a command-line utility (`legionaura`) and a clean graphical interface (`legionaura-gui`).
- **Hardware-Level Control:** Interfaces directly with the ITE controller using `hidapi`.
- **Lightweight:** Built natively in C++ and Qt6 for near-zero memory footprint.

## Architecture

The project is structured into three main components:
1. `legionaura_lib`: Core C++ hardware library handling low-level HID communication.
2. `legionaura`: CLI application for scripting and fast profile switches.
3. `legionaura-gui`: Desktop application for interactive 4-zone lighting customization.

## Installation

### Arch Linux (AUR)
If you are on Arch Linux, LegionAura is available natively in the Arch User Repository. You can quickly install it using an AUR helper like `yay`:

```bash
yay -S legionaura
```

### Other Linux Distributions (Building From Source)
*Note: Direct packages for other distributions are not currently provided. If you are not on Arch Linux, you can manually build the application from source code.*

#### Dependencies
Ensure your distribution's package manager has a working C++ toolchain, CMake, and the development headers for the following libraries installed:
- `qt6-base` (or `qt6-base-dev` / `qt6-base-devel`)
- `hidapi` (or `libhidapi-dev` / `hidapi-devel`)

#### Build Commands
```bash
git clone https://github.com
cd LegionAura
mkdir build && cd build
cmake ..
make
sudo make install
```


## Usage

### CLI Layout
Run the command-line utility to quickly cycle zones or apply profiles:
```bash
legionaura --help
```

### Graphical Interface
Launch the GUI application from your desktop application launcher or via terminal:
```bash
legionaura-gui
```

## System Integration
When installed, the application places the following files onto your system:
- **Binaries:** `/usr/local/bin/legionaura` and `/usr/local/bin/legionaura-gui`
- **Desktop Entry:** `/usr/share/applications/`
- **Hardware Rules:** `/usr/lib/udev/rules.d/10-legionaura.rules` (Handles rootless USB access)
- **Autostart:** `/usr/share/xdg/autostart/legionaura-autostart.desktop`

## Contributing
Contributions are welcome! Please feel free to submit issues, add laptop models to `devices.json`, or submit Pull Requests.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
