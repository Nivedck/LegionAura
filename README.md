# LegionAura

A Linux RGB keyboard controller for supported Lenovo Legion, LOQ and IdeaPad Gaming laptops.

LegionAura lets you control your keyboard lighting through a CLI or GUI, with support for four-zone colours, lighting effects and screen-based lighting.

## Features

- **Four-zone RGB:** Set individual colours for each keyboard zone.
- **Lighting effects:** Static, Breath, Wave, Hue and Off.
- **Screen Sync:** Match keyboard lighting to the colours on your screen.
- **Wallpaper Accent:** Set keyboard lighting using your wallpaper's colours.
- **CLI and GUI:** Control lighting from the terminal or a graphical interface.
- **Brightness control:** Adjust keyboard brightness.

## Compatibility

LegionAura requires a compatible Lenovo laptop with a **4-zone RGB keyboard**.

It does not support single-colour backlighting, white backlighting, 1-zone, 3-zone or per-key RGB keyboards. Compatibility depends on the keyboard hardware, not just the laptop model.

## Installation

### Arch Linux

```bash
yay -S legionaura
```

### Build from source

Dependencies: C++17 compiler, CMake, Qt 6.5+, Qt Multimedia and hidapi.

```bash
git clone https://github.com/Nivedck/LegionAura.git
cd LegionAura

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

## Usage

Start the graphical interface:

```bash
legionaura-gui
```

Or use the command-line interface:

```bash
legionaura --help
```

Configuration is stored in `~/.config/legionaura/config.json`.

## Notes

- Screen Sync requires the GUI to remain open. On Wayland, screen capture uses PipeWire and the desktop portal.
- Wallpaper Accent is currently in beta.
- USB access is handled through the installed udev rules.

## Contributing

Contributions, bug reports and suggestions are welcome. Feel free to open an issue or submit a pull request.

## License

[MIT License](LICENSE)