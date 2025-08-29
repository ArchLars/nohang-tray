# Coding Guide

This repository builds a small Qt/KDE tray utility. Key points:

* **Build**: `cmake -S . -B build && cmake --build build`
* **Tests**: `ctest --test-dir build`
* **Main entry**: `src/TrayApp.cpp` wires together the modules.
* **Modules**:
  * `SystemSnapshot` – reads RAM/swap/zram/PSI from `/proc`.
  * `NoHangUnit` – queries `systemctl` for the running service and config path.
  * `TooltipBuilder` – formats the status tooltip.
  * `ProcessTableAction` – optional QAction to show `nohang --tasks` output.
* **Tests** live in `tests/` and each module has a matching `*_test.cpp`.

Follow TDD: add or adjust tests before changing implementation.
