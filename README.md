# Layout

```swift
nohang-tray/
  CMakeLists.txt
  src/
    main.cpp
    TrayApp.h
    TrayApp.cpp
    NoHangStatus.h
    NoHangStatus.cpp        (checks systemd service state)
    ConfigModel.h
    ConfigModel.cpp         (parses /etc/nohang/nohang-desktop.conf subset)
    SystemInfo.h
    SystemInfo.cpp          (reads /proc/meminfo, /proc/pressure/memory,
                             /sys/block/zram0/{disksize,mm_stat}, /proc/swaps)
    TooltipBuilder.h
    TooltipBuilder.cpp      (formats values into KStatusNotifierItem tooltip)
  data/
    org.archlars.nohangtray.desktop  (optional autostart entry)
  packaging/
    PKGBUILD                 (Arch package skeleton)
  README.md
  LICENSE
```
