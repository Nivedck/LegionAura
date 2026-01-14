## LegionAura

LegionAura is a lightweight RGB keyboard lighting controller for Lenovo LOQ, Legion, and IdeaPad Gaming laptops on Linux.

It controls the built-in **4-zone ITE RGB keyboard controller** over USB (HID `SET_REPORT`) and provides:
- A C++ library (`legionaura_lib`)
- A command-line tool (`legionaura`)
- A Qt6 GUI (`legionaura-gui`)

This project is not affiliated with Lenovo.

## Features

- 4-zone RGB lighting
- Effects: Static, Breath, Wave, Hue
- Per-zone colors (hex `RRGGBB`)
- Speed control (1-4)
- Brightness control (1-2)
- Wave direction (LTR/RTL)
- Auto-detects supported Lenovo ITE keyboard PIDs

## Supported devices

LegionAura targets Lenovo laptops using the ITE RGB keyboard controller (VID `048d`) with known keyboard PIDs.

The PID list is stored in `devices/devices.json`. If your device is not listed but uses the same ITE controller, adding your PID there is usually enough.

## Installation

### Arch-based distributions (AUR)

The AUR package name is `legionaura`.

Using `yay`:
```bash
yay -S legionaura
```

Using `paru`:
```bash
paru -S legionaura
```

### Build from source (other distributions)

Dependencies:
- C++17 compiler, CMake (>= 3.16)
- `libusb-1.0`
- Qt6 Widgets (for GUI)

Examples:

Debian/Ubuntu:
```bash
sudo apt update
sudo apt install build-essential cmake libusb-1.0-0-dev qt6-base-dev git
```

Fedora:
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake libusb1-devel qt6-qtbase-devel git
```

Build and install:
```bash
git clone https://github.com/nivedck/LegionAura.git
cd LegionAura

cmake -S . -B build
cmake --build build -j
sudo cmake --install build
```

Installed files (typical):
- `legionaura` and `legionaura-gui` in `/usr/local/bin` (or your CMake prefix)
- Desktop entry in `/usr/share/applications`
- Icon in `/usr/share/icons/hicolor/256x256/apps`
- Device list in `/usr/share/legionaura/devices.json`
- udev rule in `/usr/lib/udev/rules.d/10-legionaura.rules`

### udev rules (recommended)

To run as a normal user, you need permission to access the USB device.

If you installed with `cmake --install`, the udev rule file is installed automatically. Reload udev rules:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Then unplug/replug the laptop keyboard device (or reboot).

## Usage

### CLI

```text
legionaura static <colors...> [--brightness 1|2]
legionaura breath <colors...> [--speed 1..4] [--brightness 1|2]
legionaura wave <ltr|rtl> [--speed 1..4] [--brightness 1|2]
legionaura hue [--speed 1..4] [--brightness 1|2]
legionaura off
legionaura --brightness 1|2
```

Examples:
```bash
legionaura static ff0000
legionaura breath ff0000 00ff00 0000ff --speed 2
legionaura wave ltr --speed 2
legionaura off
```

### GUI

Launch from the application menu or run:
```bash
legionaura-gui
```

## Applying settings on login

If you want a fixed color/effect at every login, create a user systemd service.

Example (`~/.config/systemd/user/legionaura.service`):
```ini
[Unit]
Description=Apply LegionAura keyboard lighting

[Service]
Type=oneshot
ExecStart=/usr/local/bin/legionaura static ff0000

[Install]
WantedBy=default.target
```

Enable it:
```bash
systemctl --user daemon-reload
systemctl --user enable --now legionaura.service
```

Adjust the path to `legionaura` if you installed to a different prefix.

## Troubleshooting

### 1) Confirm the device is visible
```bash
lsusb | grep 048d
```

You should see at least one `048d:xxxx` entry for the keyboard controller.

### 2) Permissions
Check permissions for the matching USB node (BUS/DEV come from `lsusb` output):
```bash
ls -l /dev/bus/usb/BUS/DEV
```

If you cannot open the device as a normal user, ensure the udev rules are installed and reload them:
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### 3) PID mismatch / auto-detect

If the GUI works but the CLI does not, it is usually a PID mismatch in older versions.
Recent versions of the CLI auto-detect supported PIDs (same as the GUI).

### 4) Override the devices list (advanced)

If your distro installs `devices.json` to a non-standard location, you can override it:
```bash
LEGIONAURA_DEVICES_JSON=/path/to/devices.json legionaura static ff0000
```

### 5) Still not working?

Please open an issue with:
1) `lsusb | grep 048d`
2) Whether `sudo legionaura static ff0000` works
3) Output of `ls -l /dev/bus/usb/BUS/DEV` for the keyboard device

## Contributing

Bug reports and pull requests are welcome.

If your laptop uses the same controller but has a different PID, add it to `devices/devices.json`.

## Disclaimer

This tool changes keyboard lighting settings via USB commands. Use at your own risk.

## License

MIT. See `LICENSE`.
