# ===== CMakeLists.txt =====
cmake_minimum_required(VERSION 3.24)
project(nohang-tray VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
set(CMAKE_C_COMPILER_LAUNCHER ccache)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# KDE Frameworks 6, Status Notifier Item
# Docs show: find_package(KF6StatusNotifierItem) then link KF6::StatusNotifierItem
find_package(KF6StatusNotifierItem REQUIRED)

add_library(nohang_core STATIC
  src/TrayApp.cpp
  src/NoHangUnit.cpp
  src/NoHangConfig.cpp
  src/SystemSnapshot.cpp
  src/Thresholds.cpp
  src/TooltipBuilder.cpp
  src/ProcessTableAction.cpp
)
target_include_directories(nohang_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(nohang_core
  PUBLIC Qt6::Core Qt6::Widgets
  PRIVATE KF6::StatusNotifierItem)

add_executable(nohang-tray src/main.cpp)
target_link_libraries(nohang-tray PRIVATE nohang_core Qt6::Core Qt6::Widgets KF6::StatusNotifierItem)

install(TARGETS nohang-tray RUNTIME DESTINATION bin)
install(FILES data/org.archlars.nohangtray.desktop DESTINATION share/applications)

# Optional, for packaging
include(GNUInstallDirs)

if (BUILD_TESTING)
  find_package(GTest REQUIRED)

  add_executable(NoHangConfig_test tests/NoHangConfig_test.cpp)
  target_link_libraries(NoHangConfig_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  add_test(NAME NoHangConfig_test COMMAND NoHangConfig_test)

  add_executable(Thresholds_test tests/Thresholds_test.cpp)
  target_link_libraries(Thresholds_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  add_test(NAME Thresholds_test COMMAND Thresholds_test)

  add_executable(SystemSnapshot_test tests/SystemSnapshot_test.cpp)
  target_link_libraries(SystemSnapshot_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  add_test(NAME SystemSnapshot_test COMMAND SystemSnapshot_test)

  add_executable(NoHangUnit_test tests/NoHangUnit_test.cpp)
  target_link_libraries(NoHangUnit_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  add_test(NAME NoHangUnit_test COMMAND NoHangUnit_test)

  add_executable(TooltipBuilder_test tests/TooltipBuilder_test.cpp)
  target_link_libraries(TooltipBuilder_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  add_test(NAME TooltipBuilder_test COMMAND TooltipBuilder_test)

  add_executable(ProcessTableAction_test tests/ProcessTableAction_test.cpp)
  target_link_libraries(ProcessTableAction_test PRIVATE nohang_core Qt6::Core Qt6::Widgets GTest::gtest GTest::gtest_main)
  add_test(NAME ProcessTableAction_test COMMAND ProcessTableAction_test)
endif()
