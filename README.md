<!-- ===== README.md ===== -->
# nohang-tray

KDE Plasma tray icon that indicates whether `nohang-desktop.service` is active, and on hover shows what nohang will do on this system, based on the running config and live values.

## Build
```bash
sudo pacman -S --needed base-devel cmake qt6-base kstatusnotifieritem
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build
./build/nohang-tray
```
## Notes

* Discovers config via systemctl show -p ExecStart nohang-desktop.service.

* Parses key thresholds from the discovered config, falls back to /etc/nohang/nohang-desktop.conf, then /usr/share/nohang/nohang.conf.

* Reads /proc/meminfo, /proc/pressure/memory, /sys/block/zram0/{disksize,mm_stat}.

* Shows a shield icon when active, tooltip lists thresholds and current values.


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
  LICENSE                        (MIT or BSD-2)
```
