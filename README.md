# Sine Installer

<a href="https://github.com/CosmoCreeper/Sine/">
  <img src="https://github.com/user-attachments/assets/87b7dede-1ac7-4122-bcd9-fc18d3dffeb1" alt="Sine logo" width="80"/>
</a>

The official auto-installer for [Sine](https://github.com/CosmoCreeper/Sine/) - a Firefox browser customization system.

## Features

- Cross-platform support (Linux, Windows, macOS)
- Graphical installer with ImGui
- Automatic browser and profile detection
- Support for multiple Firefox-based browsers (Firefox, Floorp, Mullvad, Waterfox, Zen)
- Wayland-compatible privilege escalation on Linux

## Supported Browsers

| Browser | Versions |
|---------|----------|
| Firefox | Stable, Developer Edition, Nightly |
| Floorp | Stable |
| Mullvad | Stable, Alpha |
| Waterfox | Stable |
| Zen | Beta, Twilight |

---

## Building from Source

### Prerequisites

#### All Platforms
- CMake 3.16+
- C++17 compatible compiler
- Git

#### Linux
```bash
# Debian/Ubuntu
sudo apt install build-essential cmake git libglfw3-dev libcurl4-openssl-dev libgl1-mesa-dev unzip

# Fedora
sudo dnf install gcc-c++ cmake git glfw-devel libcurl-devel mesa-libGL-devel unzip

# Arch Linux
sudo pacman -S base-devel cmake git glfw curl mesa unzip
```

#### Windows
- Visual Studio 2019+ with C++ workload
- [vcpkg](https://github.com/microsoft/vcpkg) package manager

#### macOS
```bash
brew install cmake glfw curl
```

### Clone the Repository

```bash
git clone --recursive https://github.com/sineorg/installer.git
cd installer
```

If you forgot `--recursive`, initialize submodules:
```bash
git submodule update --init --recursive
```

### Build Instructions

#### Linux

```bash
# Create build directory
mkdir -p build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)
```

**Output binaries:**
- `build/sine_installer` - Main GUI installer
- `build/sine-installer-helper` - Privileged helper (Linux only)

#### Windows (with vcpkg)

```powershell
# Set vcpkg root (adjust path as needed)
$env:VCPKG_ROOT = "C:\vcpkg"

# Install dependencies
vcpkg install glfw3:x64-windows curl:x64-windows

# Create build directory
mkdir build
cd build

# Configure with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

# Build
cmake --build . --config Release
```

#### macOS

```bash
mkdir -p build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

---

## Running the Installer

### Linux

Both binaries must be in the same directory:

```bash
cd build
./sine_installer
```

**Note:** On Wayland, if admin privileges are needed, a terminal window will open asking for your sudo password. This is expected behavior.

### Windows

```powershell
.\sine_installer.exe
```

### macOS

```bash
./sine_installer
```

---

## Project Structure

```
installer/
├── src/
│   ├── main.cpp          # Main GUI application
│   ├── helper.cpp        # Privileged helper (Linux)
│   ├── data.cpp          # Browser configuration data
│   └── data.h
├── external/
│   ├── glad/             # OpenGL loader
│   ├── imgui/            # Dear ImGui
│   └── fonts/            # Embedded fonts
├── build/                # Build output (generated)
├── CMakeLists.txt
├── vcpkg.json            # vcpkg manifest
└── README.md
```

---

## How It Works

### Installation Flow

1. **Select Browser** - Choose from supported Firefox-based browsers
2. **Select Version** - Pick the browser edition (Stable, Nightly, etc.)
3. **Confirm Paths** - Verify browser and profile locations
4. **Configure Options** - Choose install/uninstall, keep data, etc.
5. **Download** - Downloads required files to ~/Downloads
6. **Install** - Extracts and configures with elevated privileges if needed
7. **Done** - Restart browser to see Sine

### Privilege Escalation (Linux)

The installer uses a two-binary approach for security:

- **sine_installer** - GUI application, runs as normal user
- **sine-installer-helper** - Handles privileged file operations only

On **X11**: Uses `pkexec` for native polkit authentication dialog

On **Wayland**: Opens a terminal emulator with `sudo` (polkit doesn't work reliably on Wayland)

Supported terminal emulators (checked in order):
- kitty, alacritty, foot, gnome-terminal, konsole, xfce4-terminal, xterm, tilix, mate-terminal, lxterminal, terminator

---

## Runtime Dependencies

- `unzip` - Required for extracting downloaded archives

---

## Development

### Updating Browser Data

Browser paths and versions are defined in `src/data.cpp`. To add a new browser or version, update the `browsers` vector.

### Updating Sine/Bootloader Versions

Edit these constants in `src/data.cpp`:
```cpp
const std::string bootVersion = "0.1.1";
const std::string sineVersion = "2.3c";
```

---

## License

See the [Sine repository](https://github.com/CosmoCreeper/Sine/) for license information.

---

## Links

- [Sine](https://github.com/CosmoCreeper/Sine/) - The main Sine project
- [Bootloader](https://github.com/sineorg/bootloader/) - Sine bootloader

