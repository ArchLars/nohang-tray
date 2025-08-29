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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

# KDE Frameworks 6, Status Notifier Item
# Docs show: find_package(KF6StatusNotifierItem) then link KF6::StatusNotifierItem
find_package(KF6StatusNotifierItem REQUIRED)

add_library(nohang_core STATIC
  src/NoHangUnit.cpp
  src/NoHangConfig.cpp
  src/SystemSnapshot.cpp
  src/Thresholds.cpp
  src/TooltipBuilder.cpp
)
target_include_directories(nohang_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(nohang_core PUBLIC Qt6::Core)
target_precompile_headers(nohang_core PRIVATE src/pch.h)

add_library(tray_ui STATIC
  src/TrayApp.cpp
  src/ProcessTableAction.cpp
)
target_include_directories(tray_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(tray_ui PUBLIC Qt6::Widgets KF6::StatusNotifierItem)
target_precompile_headers(tray_ui PRIVATE src/pch.h)

add_executable(nohang-tray src/main.cpp)
target_link_libraries(nohang-tray PRIVATE tray_ui nohang_core)
target_precompile_headers(nohang-tray PRIVATE src/pch.h)

install(TARGETS nohang-tray RUNTIME DESTINATION bin)
install(FILES data/org.archlars.nohangtray.desktop DESTINATION share/applications)

# Optional, for packaging
include(GNUInstallDirs)

if (BUILD_TESTING)
  include(FetchContent)
  find_package(GTest QUIET)
  if (NOT GTest_FOUND)
    message(STATUS "GTest not found, fetching...")
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    )
    # Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(NoHangConfig_test tests/NoHangConfig_test.cpp)
  target_link_libraries(NoHangConfig_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  target_precompile_headers(NoHangConfig_test PRIVATE src/pch.h)
  add_test(NAME NoHangConfig_test COMMAND NoHangConfig_test)

  add_executable(Thresholds_test tests/Thresholds_test.cpp)
  target_link_libraries(Thresholds_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  target_precompile_headers(Thresholds_test PRIVATE src/pch.h)
  add_test(NAME Thresholds_test COMMAND Thresholds_test)

  add_executable(SystemSnapshot_test tests/SystemSnapshot_test.cpp)
  target_link_libraries(SystemSnapshot_test PRIVATE nohang_core Qt6::Core Qt6::Test GTest::gtest GTest::gtest_main)
  target_precompile_headers(SystemSnapshot_test PRIVATE src/pch.h)
  add_test(NAME SystemSnapshot_test COMMAND SystemSnapshot_test)

  add_executable(NoHangUnit_test tests/NoHangUnit_test.cpp)
  target_link_libraries(NoHangUnit_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  target_precompile_headers(NoHangUnit_test PRIVATE src/pch.h)
  add_test(NAME NoHangUnit_test COMMAND NoHangUnit_test)

  add_executable(TooltipBuilder_test tests/TooltipBuilder_test.cpp)
  target_link_libraries(TooltipBuilder_test PRIVATE nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  target_precompile_headers(TooltipBuilder_test PRIVATE src/pch.h)
  add_test(NAME TooltipBuilder_test COMMAND TooltipBuilder_test)

  add_executable(ProcessTableAction_test tests/ProcessTableAction_test.cpp)
  target_link_libraries(ProcessTableAction_test PRIVATE tray_ui nohang_core GTest::gtest GTest::gtest_main)
  target_precompile_headers(ProcessTableAction_test PRIVATE src/pch.h)
  add_test(NAME ProcessTableAction_test COMMAND ProcessTableAction_test)

  add_executable(TrayApp_test tests/TrayApp_test.cpp)
  target_link_libraries(TrayApp_test PRIVATE tray_ui nohang_core Qt6::Core GTest::gtest GTest::gtest_main)
  target_precompile_headers(TrayApp_test PRIVATE src/pch.h)
  add_test(NAME TrayApp_test COMMAND TrayApp_test)
endif()
