#!/usr/bin/env bash
# scripts/setup-codex.sh
# One-shot setup for OpenAI Codex containers, run from repo root.

set -euo pipefail
IFS=$'\n\t'

# shellcheck source=scripts/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

ensure_root "$@"

log "installing base build tools"
install_base_build_tools

log "installing X11 and Wayland dev headers"
install_x11_wayland_dev

log "installing aqt"
install_aqt

log "installing Qt ${QT_VER} (${QT_ARCH}) to ${QT_ROOT}"
install_qt

log "writing environment exports"
write_env_file
# apply for current shell too
# shellcheck disable=SC1091
source /etc/profile.d/qt-kf6.sh

log "creating sources dir ${SRC_DIR} and prefix ${KF_PREFIX}"
mkdir -p "${SRC_DIR}" "${KF_PREFIX}"

log "fetching KDE Frameworks ${KF_VER} tarballs"
fetch_tarball "${ECM_URL}" "${SRC_DIR}"
fetch_tarball "${KWS_URL}" "${SRC_DIR}"
fetch_tarball "${KSI_URL}" "${SRC_DIR}"

log "building and installing extra-cmake-modules"
build_and_install "${SRC_DIR}/$(basename "${ECM_URL}")" "extra-cmake-modules"

log "building and installing kwindowsystem"
build_and_install "${SRC_DIR}/$(basename "${KWS_URL}")" "kwindowsystem"

log "building and installing kstatusnotifieritem"
build_and_install "${SRC_DIR}/$(basename "${KSI_URL}")" "kstatusnotifieritem"

log "probing CMake find_package"
probe_cmake_find

log "setup complete. Qt in ${QT_PREFIX}, KF6 in ${KF_PREFIX}"
