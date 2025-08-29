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
