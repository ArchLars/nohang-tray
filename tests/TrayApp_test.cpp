#include "pch.h"
#include <gtest/gtest.h>
#define private public
#include "NoHangConfig.h"
#include "SystemSnapshot.h"
#undef private
#include "TrayApp.h"

TEST(TrayAppTest, EscapePercent) {
  const QString input = "value 42 %";
  const QString output = TrayApp::escapePercent(input);
  EXPECT_EQ(QStringLiteral("value 42 %"), output);
}

TEST(TrayAppTest, IconReflectsMemorySeverity) {
  NoHangConfig cfg;
  cfg.m_t.warn_mem_percent = 40.0;
  cfg.m_t.soft_mem_percent = 30.0;
  cfg.m_t.hard_mem_percent = 20.0;

  SystemSnapshot snap;
  snap.m_mem.memTotalMiB = 100.0;

  snap.m_mem.memAvailableMiB = 50.0;
  EXPECT_EQ(QStringLiteral("security-low"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.memAvailableMiB = 35.0;
  EXPECT_EQ(QStringLiteral("security-medium"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.memAvailableMiB = 15.0;
  EXPECT_EQ(QStringLiteral("security-high"), TrayApp::iconNameFor(cfg, snap));
}

TEST(TrayAppTest, IconHandlesMiBThresholds) {
  NoHangConfig cfg;
  cfg.m_t.soft_swap_percent_free = -500.0; // 500 MiB soft threshold

  SystemSnapshot snap;
  snap.m_mem.memTotalMiB = 1000.0;
  snap.m_mem.memAvailableMiB = 1000.0;
  snap.m_mem.swapTotalMiB = 1000.0;

  snap.m_mem.swapFreeMiB = 600.0;
  EXPECT_EQ(QStringLiteral("security-low"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.swapFreeMiB = 400.0;
  EXPECT_EQ(QStringLiteral("security-high"), TrayApp::iconNameFor(cfg, snap));
}
