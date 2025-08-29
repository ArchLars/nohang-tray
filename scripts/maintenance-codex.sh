#!/usr/bin/env bash
# scripts/maintenance-codex.sh
# Run in resumed Codex containers after checkout. Verifies toolchains and rehydrates env.

set -euo pipefail
IFS=$'\n\t'

# shellcheck source=scripts/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# Reapply environment if file exists
if [[ -f /etc/profile.d/qt-kf6.sh ]]; then
  # shellcheck disable=SC1091
  source /etc/profile.d/qt-kf6.sh
else
  log "/etc/profile.d/qt-kf6.sh not found, writing a fresh one from defaults"
  write_env_file
  # shellcheck disable=SC1091
  source /etc/profile.d/qt-kf6.sh
fi

# Validate Qt
if [[ ! -x "${QT_PREFIX}/bin/qmake6" ]]; then
  log "Qt not present at ${QT_PREFIX}, reinstalling via aqt"
  install_base_build_tools
  install_aqt
  install_qt
fi
ensure_path "${QT_PREFIX}/bin"

# Validate KDE prefixes
mkdir -p "${KF_PREFIX}"

# Ensure X11 and Wayland headers if not present
if [[ ! -f /usr/include/X11/Xlib.h || ! -f /usr/include/wayland-client.h ]]; then
  log "missing X11 or Wayland headers, installing"
  install_x11_wayland_dev
fi

# Probe CMake find_package for Qt and KF6
log "running cmake probe for Qt and KF6"
probe_cmake_find

# Optional quick configure to prime CMake cache for this repo
if [[ -f CMakeLists.txt ]]; then
  log "priming this repo build directory"
  cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}/lib/cmake:${KF_PREFIX}/lib/cmake"
fi

log "maintenance complete"
