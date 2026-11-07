# LegionAura

[![AUR version](https://shields.io)](https://archlinux.org)
[![License](https://shields.io)](LICENSE)

A lightweight C++ and Qt6 RGB keyboard lighting controller for Lenovo LOQ, Legion, and IdeaPad Gaming laptops running Linux.

> [!WARNING]
> **LegionAura is only compatible with laptops equipped with a 4-zone RGB keyboard.** It is **not compatible** with single-color/white backlit keyboards, 1-zone RGB keyboards, per-key RGB keyboards, or other keyboard configurations. Check your laptop's exact keyboard hardware before installing or using this tool.

LegionAura communicates with the built-in 4-zone ITE RGB keyboard controller via USB (`HID SET_REPORT`), allowing you to completely customize your keyboard lighting without heavy background daemons.

## Features

- **Dual Interface:** Comes with both a command-line utility (`legionaura`) and a clean graphical interface (`legionaura-gui`).
- **Screen Sync:** Smoothly mirror four screen regions onto the four keyboard zones while the GUI is running.
- **Hardware-Level Control:** Interfaces directly with the ITE controller using `hidapi`.
- **Lightweight:** Built natively in C++ and Qt6 for near-zero memory footprint.

## Compatibility

LegionAura is designed **exclusively for 4-zone RGB keyboards** controlled by a compatible ITE RGB controller.

Before installing, verify that your laptop has a 4-zone RGB keyboard. Laptop model names alone are not sufficient to determine compatibility because the same model may be sold with different keyboard configurations.

The following keyboard types are **not supported**:

- Single-color or white backlit keyboards
- 1-zone RGB keyboards
- Per-key RGB keyboards
- 3-zone RGB keyboards
- Any keyboard without a compatible ITE RGB controller

If you are unsure whether your device is compatible, check the exact keyboard configuration in your laptop's specifications or open an issue with your full laptop model and keyboard details before using LegionAura.

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
- Qt **6.5 or newer**: `qt6-base` and `qt6-multimedia` development packages (Ubuntu/Debian: `qt6-base-dev` and `qt6-multimedia-dev`)
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

### Screen Sync

Select **Screen Sync** and press **Apply**. On Wayland, choose a monitor in the desktop screen-sharing picker. On X11, the primary monitor is used. Each vertical quarter of that screen drives the matching keyboard zone, left to right. Colors blend smoothly and update at the capture/device rate by default, without an artificial FPS cap. Slow USB transfers retain only the newest pending colors, and unchanged colors skip USB writes. The brightness slider remains available.

Screen Sync needs the GUI running (minimized is fine). Switching effects, pressing Off, reconnecting, or closing the app stops capture. Closing leaves the last applied colors on the keyboard. Screen Sync does not replace your saved lighting profile or automatically request capture at login. Frames are processed in memory and are not saved.

Wayland requires a working PipeWire service and an XDG Desktop Portal backend with ScreenCast support for your desktop (for example, `xdg-desktop-portal-gnome` on GNOME). Qt Multimedia must have its FFmpeg backend available. If screen sharing is denied or stopped, or a keyboard write fails, the status bar reports it; press Apply to retry after resolving the problem.

### GNOME Wayland fullscreen capture

Some Mutter versions stop delivering CPU-memory capture frames when fullscreen content enters direct scanout. Mutter 50.5 includes the [upstream fix](https://github.com/GNOME/mutter/commit/817424e85c3c4ae1a242978c34d210c6fc7b4c0c); a distribution backport also works. On affected desktops, use a maximized window / theater mode until the compositor is updated. LegionAura cannot update lighting while the compositor is not supplying frames.

### Development checks

After building, run `ctest --test-dir build --output-on-failure` for synthetic color sampling and smoothing checks. For integration testing with a supported keyboard:

- Display four distinct vertical color bands and confirm left-to-right mapping and the live preview.
- Change to black and bright content; check smooth transitions and brightness adjustment.
- Cancel the Wayland picker, stop sharing through the desktop, and retry with Apply.
- While syncing, switch effects, press Off, reconnect, and close the app; confirm no later screen updates override the action.
- Unplug the keyboard while syncing; confirm an error and a responsive GUI, then reconnect and retry.
- Check both GNOME Wayland and X11, existing firmware effects, and saved-profile restoration. USB update rate and visual quality need validation on hardware.

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
