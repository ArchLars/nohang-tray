#!/usr/bin/env bash
# scripts/common.sh
# Shared helpers and defaults for Codex container scripts

set -euo pipefail
IFS=$'\n\t'

# Versions and prefixes (override via env before calling the scripts)
: "${QT_VER:=6.9.0}"                    # Fixed: Qt 6.9.0 exists, 6.9.2 does not
: "${QT_ARCH:=linux_gcc_64}"            # Fixed: Use linux_gcc_64 for Qt 6.x on Linux
: "${QT_ROOT:=/opt/qt}"
: "${KF_VER:=6.14.0}"                   # KDE Frameworks version to build
: "${KF_PREFIX:=/opt/kf6}"
: "${SRC_DIR:=/tmp/kf6-src}"

# Derived
QT_PREFIX="${QT_ROOT}/${QT_VER}/${QT_ARCH}"
KF_DIR="${KF_VER%.*}"

# URLs for tarballs
ECM_URL="https://download.kde.org/stable/frameworks/${KF_DIR}/extra-cmake-modules-${KF_VER}.tar.xz"
KWS_URL="https://download.kde.org/stable/frameworks/${KF_DIR}/kwindowsystem-${KF_VER}.tar.xz"
KSI_URL="https://download.kde.org/stable/frameworks/${KF_DIR}/kstatusnotifieritem-${KF_VER}.tar.xz"

log() {
  printf '[%s] %s\n' "$(date -u +'%Y-%m-%dT%H:%M:%SZ')" "$*" >&2
}

retry() {
  local n=0 max=${2:-5} delay=${3:-2}
  until eval "$1"; do  # Add eval here
    n=$((n+1))
    if (( n >= max )); then
      log "command failed after ${n} attempts: $1"
      return 1
    fi
    sleep "${delay}"
  done
}

ensure_root() {
  if [[ ${EUID} -ne 0 ]]; then
    log "re-executing with sudo"
    exec sudo -E bash "$0" "$@"
  fi
}

ensure_path() {
  case ":${PATH}:" in
    *:"$1":*) ;; # already present
    *) export PATH="$1:${PATH}" ;;
  esac
}

write_env_file() {
  local f="/etc/profile.d/qt-kf6.sh"
  log "writing environment file to ${f}"
  cat > "${f}" <<EOF
# Qt and KF6 environment for nohang-tray CI
export QT_ROOT="${QT_ROOT}"
export QT_VER="${QT_VER}"
export QT_PREFIX="${QT_PREFIX}"
export KF_PREFIX="${KF_PREFIX}"
export CMAKE_PREFIX_PATH="${QT_PREFIX}/lib/cmake:${KF_PREFIX}/lib/cmake:\${CMAKE_PREFIX_PATH:-}"
export PKG_CONFIG_PATH="${QT_PREFIX}/lib/pkgconfig:${KF_PREFIX}/lib/pkgconfig:\${PKG_CONFIG_PATH:-}"
export PATH="${QT_PREFIX}/bin:\${PATH}"
EOF
  chmod 0644 "${f}"
}

install_base_build_tools() {
  export DEBIAN_FRONTEND=noninteractive
  retry "apt-get update -y"
  retry "apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build pkg-config ccache \
    python3 python3-pip curl ca-certificates xz-utils git gnupg"
}

install_x11_wayland_dev() {
  retry "apt-get install -y --no-install-recommends \
    libx11-dev libxext-dev libxcb1-dev libxkbcommon-dev \
    libsm-dev libice-dev libxfixes-dev libxrender-dev \
    libwayland-dev wayland-protocols"
}

install_aqt() {
  # use the python that pip installs into
  python3 -m pip install --no-cache-dir --upgrade pip
  python3 -m pip install --no-cache-dir aqtinstall
  # ensure 'python3 -m aqt' is callable
  python3 -c "import aqt" >/dev/null 2>&1 || { log "aqt module not importable"; exit 1; }
}

install_qt() {
  mkdir -p "${QT_ROOT}"
  # Use module invocation to avoid PATH issues
  python3 -m aqt install-qt linux desktop "${QT_VER}" "${QT_ARCH}" -O "${QT_ROOT}"
  # quick sanity
  test -x "${QT_PREFIX}/bin/qmake6" || { log "Qt not found at ${QT_PREFIX}"; exit 1; }
}

fetch_tarball() {
  local url="$1"
  local destdir="$2"
  mkdir -p "${destdir}"
  log "downloading ${url}"
  curl -fsSL --retry 5 --retry-delay 2 -o "${destdir}/$(basename "${url}")" "${url}"
}

build_and_install() {
  local tar="$1" name="$2"
  local bdir srcdir
  srcdir="${SRC_DIR}/${name}"
  bdir="${SRC_DIR}/build-${name}"
  rm -rf "${srcdir}" "${bdir}"
  mkdir -p "${srcdir}" "${bdir}"
  log "extracting $(basename "${tar}") to ${srcdir}"
  tar -C "${srcdir}" --strip-components=1 -xf "${tar}"
  log "configuring ${name}"
  cmake -S "${srcdir}" -B "${bdir}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${KF_PREFIX}" \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}/lib/cmake" \
    -DBUILD_TESTING=OFF
  log "building ${name}"
  cmake --build "${bdir}" -j"$(nproc)"
  log "installing ${name}"
  cmake --install "${bdir}"
}

probe_cmake_find() {
  local probe_dir="/tmp/qt-kf6-probe"
  rm -rf "${probe_dir}" "${probe_dir}-build"
  mkdir -p "${probe_dir}"
  cat > "${probe_dir}/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.22)
project(probe LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(KF6 REQUIRED COMPONENTS WindowSystem StatusNotifierItem)
message(STATUS "Qt6 found at: ${Qt6_DIR}")
message(STATUS "KF6 found WindowSystem at: ${KF6WindowSystem_DIR}")
message(STATUS "KF6 found StatusNotifierItem at: ${KF6StatusNotifierItem_DIR}")
EOF
  cmake -S "${probe_dir}" -B "${probe_dir}-build" -G Ninja \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}/lib/cmake:${KF_PREFIX}/lib/cmake"
}
