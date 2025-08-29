#include "pch.h"
#include <gtest/gtest.h>
#define private public
#include "NoHangConfig.h"
#include "SystemSnapshot.h"
#undef private
#include "TooltipBuilder.h"

TEST(TooltipBuilderTest, BuildsSummary)
{
    NoHangConfig cfg;
    cfg.m_t.warn_mem_percent = 10.0;
    cfg.m_t.warn_swap_percent_free = 20.0;
    cfg.m_t.warn_zram_percent_used = 30.0;
    cfg.m_t.warn_psi = 50.0;
    cfg.m_t.psi_metrics = "full_avg10";
    cfg.m_t.psi_duration = 60.0;

    SystemSnapshot snap;
    snap.m_mem.memAvailableMiB = 1000.0;
    snap.m_mem.memTotalMiB = 2000.0;
    snap.m_mem.memAvailablePercent = 50.0;
    snap.m_mem.swapFreeMiB = 500.0;
    snap.m_mem.swapTotalMiB = 1000.0;
    snap.m_mem.swapFreePercent = 50.0;
    snap.m_zram.present = true;
    snap.m_zram.diskSizeMiB = 100.0;
    snap.m_zram.origDataMiB = 50.0;
    snap.m_zram.memUsedTotalMiB = 10.0;
    snap.m_zram.logicalUsedPercent = 50.0;
    snap.m_psi.some_avg10 = 1.2;
    snap.m_psi.full_avg10 = 0.3;

    TooltipBuilder tb;
    QString out = tb.build(cfg, snap, true, "/path.cfg");
    EXPECT_TRUE(out.contains("status: active"));
    EXPECT_TRUE(out.contains("config: /path.cfg"));

    const int swapPos = out.indexOf("Swap:\n  total: 1000 MiB\n  free: 500 MiB (50.0 %)\n");
    const int threshPos = out.indexOf("Thresholds:\n");
    EXPECT_NE(-1, swapPos);
    EXPECT_NE(-1, threshPos);
    EXPECT_LT(swapPos, threshPos);

    EXPECT_TRUE(out.contains("Thresholds:\n"));
    EXPECT_TRUE(out.contains("RAM warn if free < 10.0 %"));
    EXPECT_TRUE(out.contains("ZRAM warn if used > 30.0 %"));
    EXPECT_TRUE(out.contains("PSI:"));
    EXPECT_TRUE(out.contains("metric: full_avg10"));
}

TEST(TooltipBuilderTest, IncludesAllSections)
{
    NoHangConfig cfg;
    cfg.m_t.warn_mem_percent = 10.0;
    cfg.m_t.soft_mem_percent = 5.0;
    cfg.m_t.hard_mem_percent = 1.0;
    cfg.m_t.warn_swap_percent_free = 20.0;
    cfg.m_t.soft_swap_percent_free = 15.0;
    cfg.m_t.hard_swap_percent_free = 5.0;
    cfg.m_t.warn_zram_percent_used = 30.0;
    cfg.m_t.soft_zram_percent_used = 40.0;
    cfg.m_t.hard_zram_percent_used = 50.0;
    cfg.m_t.warn_psi = 10.0;
    cfg.m_t.soft_psi = 20.0;
    cfg.m_t.hard_psi = 30.0;
    cfg.m_t.psi_metrics = "some";
    cfg.m_t.psi_duration = 60.0;

    SystemSnapshot snap;
    snap.m_mem.memAvailableMiB = 800.0;
    snap.m_mem.memTotalMiB = 1000.0;
    snap.m_mem.memAvailablePercent = 80.0;
    snap.m_mem.swapFreeMiB = 200.0;
    snap.m_mem.swapTotalMiB = 400.0;
    snap.m_mem.swapFreePercent = 50.0;
    snap.m_zram.present = true;
    snap.m_zram.diskSizeMiB = 100.0;
    snap.m_zram.origDataMiB = 10.0;
    snap.m_zram.memUsedTotalMiB = 5.0;
    snap.m_zram.logicalUsedPercent = 10.0;
    snap.m_psi.some_avg10 = 1.2;
    snap.m_psi.full_avg10 = 0.3;

    TooltipBuilder tb;
    const QString out = tb.build(cfg, snap, true, "/etc/nohang.cfg");
    EXPECT_NE(-1, out.indexOf("RAM:\n"));
    EXPECT_NE(-1, out.indexOf("Swap:\n"));
    EXPECT_NE(-1, out.indexOf("ZRAM:\n"));
    EXPECT_NE(-1, out.indexOf("PSI:\n"));
    EXPECT_NE(-1, out.indexOf("Thresholds:\n"));
    EXPECT_NE(-1, out.indexOf("Actions:\n"));
    EXPECT_TRUE(out.contains("RAM hard action if free <"));
    EXPECT_TRUE(out.contains("Swap hard action if free <"));
    EXPECT_TRUE(out.contains("ZRAM hard action if used >"));
    EXPECT_TRUE(out.contains("PSI hard action if >"));
}

TEST(TooltipBuilderTest, OmitsZramSectionWhenAbsent)
{
    NoHangConfig cfg;
    SystemSnapshot snap;
    snap.m_zram.present = false;

    TooltipBuilder tb;
    const QString out = tb.build(cfg, snap, false, QString());
    EXPECT_EQ(-1, out.indexOf("ZRAM:"));
}
