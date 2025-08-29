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

  snap.m_mem.memAvailableMiB = 25.0;
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
  EXPECT_EQ(QStringLiteral("security-medium"), TrayApp::iconNameFor(cfg, snap));
}

TEST(TrayAppTest, IconBoundaryAtWarnThreshold) {
  NoHangConfig cfg;
  cfg.m_t.warn_mem_percent = 10.0;

  SystemSnapshot snap;
  snap.m_mem.memTotalMiB = 1000.0;

  snap.m_mem.memAvailableMiB = 100.0; // exactly at threshold
  EXPECT_EQ(QStringLiteral("security-low"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.memAvailableMiB = 99.9; // just below
  EXPECT_EQ(QStringLiteral("security-medium"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.memAvailableMiB = 100.1; // just above
  EXPECT_EQ(QStringLiteral("security-low"), TrayApp::iconNameFor(cfg, snap));
}

TEST(TrayAppTest, IconThresholdPrecedence) {
  NoHangConfig cfg;
  cfg.m_t.warn_mem_percent = 80.0;
  cfg.m_t.soft_mem_percent = 70.0;
  cfg.m_t.hard_mem_percent = 60.0;

  SystemSnapshot snap;
  snap.m_mem.memTotalMiB = 100.0;

  snap.m_mem.memAvailableMiB = 75.0; // warn only
  EXPECT_EQ(QStringLiteral("security-medium"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.memAvailableMiB = 65.0; // warn + soft
  EXPECT_EQ(QStringLiteral("security-medium"), TrayApp::iconNameFor(cfg, snap));

  snap.m_mem.memAvailableMiB = 55.0; // hard
  EXPECT_EQ(QStringLiteral("security-high"), TrayApp::iconNameFor(cfg, snap));
}

TEST(TrayAppTest, IconIgnoresMissingSwapAndZram) {
  NoHangConfig cfg;
  cfg.m_t.warn_swap_percent_free = 10.0;
  cfg.m_t.warn_zram_percent_used = 50.0;

  SystemSnapshot snap;
  snap.m_mem.memTotalMiB = 1000.0;
  snap.m_mem.memAvailableMiB = 1000.0;
  snap.m_mem.swapTotalMiB = 0.0; // no swap configured
  snap.m_mem.swapFreeMiB = 0.0;
  snap.m_zram.present = false;    // zram not available

  EXPECT_EQ(QStringLiteral("security-low"), TrayApp::iconNameFor(cfg, snap));
}

TEST(TrayAppTest, IconUsesPsiMetric) {
  NoHangConfig cfg;
  cfg.m_t.warn_psi = 10.0;
  cfg.m_t.hard_psi = 20.0;
  cfg.m_t.psi_metrics = QStringLiteral("some");

  SystemSnapshot snap;
  snap.m_psi.some_avg10 = 25.0; // above hard
  snap.m_psi.full_avg10 = 5.0;
  EXPECT_EQ(QStringLiteral("security-high"), TrayApp::iconNameFor(cfg, snap));

  snap.m_psi.some_avg10 = 15.0; // between warn and hard
  EXPECT_EQ(QStringLiteral("security-medium"), TrayApp::iconNameFor(cfg, snap));

  snap.m_psi.some_avg10 = 5.0; // below warn
  EXPECT_EQ(QStringLiteral("security-low"), TrayApp::iconNameFor(cfg, snap));
}
