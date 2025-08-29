# Coding Guide

This repository builds a small Qt/KDE tray utility. Key points:

* **Build**: `cmake -G Ninja -S . -B build && cmake --build build -j$(nproc)`
AND: force regular log output by adding flags such as:

cmake --build build -j -- -v      # Ninja: show each command line
ctest --test-dir build --progress # Tests: show per‑test progress
Use a keep-alive loop for very quiet steps
Wrap long-running commands that produce little output:

while sleep 60; do echo "still building…"; done &
cmake --build build -j -- -v
Increase the step timeout if needed
In GitHub Actions, set timeout-minutes on the job or individual step when a build legitimately takes longer.****

* **Tests**: `ctest --test-dir build`
* **Entry point**: `src/main.cpp` boots `TrayApp`, which wires up the modules.
* **Modules**:
  * `SystemSnapshot` – reads RAM/swap/zram/PSI from `/proc`.
  * `NoHangUnit` – queries `systemctl` for the running service and config path.
  * `NoHangConfig` – parses thresholds from the resolved config.
  * `Thresholds` – converts percentages to MiB and compares against live totals.
  * `TooltipBuilder` – formats the status tooltip.
  * `ProcessTableAction` – optional QAction to show `nohang --tasks` output.
* **Tests** live in `tests/` and each module has a matching `*_test.cpp`.

Follow TDD: add or adjust tests before changing implementation.
