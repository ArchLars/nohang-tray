# nohang-tray

[nohang](https://github.com/hakavlad/nohang) is a daemon that watches system memory pressure and takes action before the kernel freezes. `nohang-tray` is a KDE Plasma tray utility that lets you see whether `nohang-desktop.service` is active and, on hover, review the actions `nohang` will take based on the current configuration and live system values.

## Build

### Arch Linux
1. Install dependencies:
   ```bash
   sudo pacman -S --needed base-devel cmake qt6-base kstatusnotifieritem
   ```
2. Generate the build directory and compile:
   ```bash
   cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j$(nproc)
   ```

### Other distributions
- **Debian/Ubuntu**
  ```bash
  sudo apt install build-essential cmake qt6-base-dev libkf6statusnotifieritem-dev
  ```
- **Fedora**
  ```bash
  sudo dnf install @development-tools cmake qt6-qtbase-devel kf6-kstatusnotifieritem-devel
  ```

After installing dependencies, generate the build directory and compile:

```bash
cmake -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Usage

`nohang-desktop.service` must be active for the tray icon to appear:
```bash
systemctl --user enable --now nohang-desktop.service
```

### Run manually
```bash
./build/nohang-tray &
```

### Autostart on login
```bash
mkdir -p ~/.config/autostart
cp data/org.archlars.nohangtray.desktop ~/.config/autostart/
```

## Notes

* The icon appears only when `nohang` is protecting your system.
* Hovering the icon shows memory limits from your configuration alongside current usage.
* This helps you gauge how close you are to running out of memory.
* Robust `/proc/meminfo` parsing tolerates leading whitespace, and `/proc/swaps` totals ensure swap usage is always reported.

## Technical Details

* Discovers config via `systemctl show -p ExecStart nohang-desktop.service`.
* Parses thresholds from the discovered config, falling back to `/etc/nohang/nohang-desktop.conf` and `/usr/share/nohang/nohang.conf`.
* Reads `/proc/meminfo`, `/proc/swaps`, `/proc/pressure/memory`, and `/sys/block/zram0/{disksize,mm_stat}` to populate the tooltip.
* Logs a warning if `/proc/meminfo` cannot be opened.

## Layout
```bash
nohang-tray/
  CMakeLists.txt                 (Qt 6, KF6 find_package, release flags)
  src/
    main.cpp                     (QApplication, TrayApp bootstrap)
    TrayApp.h/.cpp               (KStatusNotifierItem setup, timers, icon)
    NoHangUnit.h/.cpp            (discover ExecStart, resolve config path, isActive)
    NoHangConfig.h/.cpp          (parse thresholds from the found config, fallback to /usr/share defaults)
    SystemSnapshot.h/.cpp        (read /proc/meminfo, /proc/swaps, /sys/block/zram0/*, /proc/pressure/memory)
    Thresholds.h/.cpp            (convert % to MiB using live totals, compare current vs thresholds)
    TooltipBuilder.h/.cpp        (format multi-line tooltip with numbers and explanations)
    ProcessTableAction.h/.cpp    (optional action to run `sudo nohang --tasks -c <cfg>` in a viewer)
  data/
    org.archlars.nohangtray.desktop   (optional autostart entry)
  packaging/
    PKGBUILD                     (depends: qt6-base, kstatusnotifieritem)
  README.md                      (how it works, build steps, permissions)
  LICENSE                        (MIT)
```

## License

MIT
