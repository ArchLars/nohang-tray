# ===== CMakeLists.txt =====
cmake_minimum_required(VERSION 3.24)
project(nohang-tray VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# KDE Frameworks 6, Status Notifier Item
# Docs show: find_package(KF6StatusNotifierItem) then link KF6::StatusNotifierItem
find_package(KF6StatusNotifierItem REQUIRED)

add_executable(nohang-tray
  src/main.cpp
  src/TrayApp.cpp
  src/NoHangUnit.cpp
  src/NoHangConfig.cpp
  src/SystemSnapshot.cpp
  src/Thresholds.cpp
  src/TooltipBuilder.cpp
  src/ProcessTableAction.cpp
)

target_include_directories(nohang-tray PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(nohang-tray
  PRIVATE
    Qt6::Core
    Qt6::Widgets
    KF6::StatusNotifierItem
)

install(TARGETS nohang-tray RUNTIME DESTINATION bin)
install(FILES data/org.archlars.nohangtray.desktop DESTINATION share/applications)

# Optional, for packaging
include(GNUInstallDirs)
