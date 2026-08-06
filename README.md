# LookAway 👁️⚡

A lightweight, cross-platform background utility built with **C++17** and **Qt 6** to enforce the **20-20-20 eye care rule**: *Every 20 minutes of screen work, look at something 20 feet (6 meters) away for 20 seconds.*

---

## 🌟 Key Features

* **⏱️ Intelligent Background Engine:** Runs unobtrusively in your system tray with minimal CPU/RAM footprint. Automatically starts countdown on launch.
* **📌 System Tray Presence:** Lives in the Windows taskbar system tray / Linux status area. Shows dynamic status icons and live countdown hover tooltips.
* **🛡️ Strict / Gentle Break Overlay:** Optional full-screen translucent overlay during 20-second break sessions with eye relaxation prompts.
* **💤 Automatic System Idle Detection:** Pauses the work timer if mouse or keyboard activity is idle for 3+ minutes, so you aren't prompted to take a break when returning to your desk.
* **🔔 Gentle Audio & Toast Notifications:** Low-latency chime sounds on session transitions paired with native OS toast popups.
* **⚡ One-Click Presets:** Instantly switch between `20-20-20 (Eye Care)`, `25-5 (Pomodoro)`, and `50-10 (Deep Work)`.
* **📊 Daily Health Statistics:** Tracks completed breaks, skipped breaks, and total eye-rest minutes today.
* **⚙️ Persistent Configuration:** Remembers your custom durations, audio volume, overlay preferences, and startup behavior across reboots via `QSettings`.

---

## 🛠️ Technology Stack

* **Language:** C++17
* **GUI Framework:** Qt 6 (Qt Widgets, Qt Multimedia)
* **Build System:** CMake 3.16+
* **Compiler Support:** GCC / MinGW 13+, MSVC 2022, Clang
* **OS Compatibility:** Windows 10/11 & Linux (X11 / Wayland)

---

## 📂 Project Architecture

```
LookAway/
├── CMakeLists.txt              # CMake build configuration
├── src/
│   ├── main.cpp                # Application entry point & CLI parser
│   ├── TimerEngine.h/.cpp      # Core timer engine & state machine
│   ├── SystemTrayManager.h/.cpp# QSystemTrayIcon & tray context menu
│   ├── MainWindow.h/.cpp       # Qt Dashboard & Settings window
│   ├── BreakOverlayWidget.h/.cpp# Full-screen break overlay window
│   ├── AudioManager.h/.cpp     # QSoundEffect chime playback
│   └── SettingsManager.h/.cpp  # QSettings persistence & stats tracking
└── resources/
    ├── resources.qrc           # Qt resource manifest
    ├── icons/                  # SVG icons (app, tray_work, tray_break, tray_paused)
    └── sounds/                 # Embedded 16-bit PCM chime WAV files
```

---

## 🚀 Building from Source

### Prerequisites

* **Qt 6.x** (Widgets & Multimedia modules)
* **CMake 3.16+**
* **C++17 Compiler** (GCC/MinGW, MSVC, or Clang)
* **Ninja** or **Make**

### Build Instructions (Windows - MinGW / MSVC)

1. **Clone the repository:**
   ```bash
   git clone https://github.com/your-username/LookAway.git
   cd LookAway
   ```

2. **Configure CMake:**
   ```powershell
   cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/mingw_64"
   ```

3. **Compile the binary:**
   ```powershell
   cmake --build build
   ```

4. **Deploy Qt DLL dependencies (for standalone execution):**
   ```powershell
   windeployqt build/LookAway.exe
   ```

### Build Instructions (Linux)

1. **Install dependencies (Ubuntu/Debian):**
   ```bash
   sudo apt update
   sudo apt install qt6-base-dev qt6-multimedia-dev cmake build-essential ninja-build
   ```

2. **Configure and compile:**
   ```bash
   cmake -B build -G Ninja
   cmake --build build
   ```

---

## 🎮 Usage & CLI Flags

Run the compiled binary:
```bash
./build/LookAway
```

Start minimized directly in the system tray:
```bash
./build/LookAway --minimized
```

### System Tray Context Menu
Right-click the **LookAway** icon in your system tray to quickly access:
- 👁️ **Show Dashboard**
- ⏯️ **Pause / Resume Timer**
- ⏭️ **Skip Break**
- ⚙️ **Settings...**
- ❌ **Quit LookAway**

---

## 📄 License

This project is licensed under the **MIT License**. Feel free to use, modify, and distribute.
