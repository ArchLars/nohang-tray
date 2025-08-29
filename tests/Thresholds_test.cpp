#include "pch.h"
#include <gtest/gtest.h>
#include "Thresholds.h"

// Stub SystemSnapshot with controllable totals
class StubSnapshot : public SystemSnapshot {
public:
    StubSnapshot(double memTotal, double swapTotal, double zramDisk)
    {
        auto &memInfo = const_cast<MemInfo&>(mem());
        memInfo.memTotalMiB = memTotal;
        memInfo.swapTotalMiB = swapTotal;

        auto &zramInfo = const_cast<ZramInfo&>(zram());
        zramInfo.diskSizeMiB = zramDisk;
    }
};

TEST(ThresholdsTest, ComputesPercentAndMiB)
{
    ThresholdsPercent t;
    t.warn_mem_percent = 10.0;
    t.warn_swap_percent_free = 20.0;
    t.warn_zram_percent_used = 30.0;
    t.soft_mem_percent = 40.0;
    t.soft_swap_percent_free = 50.0;
    t.soft_zram_percent_used = 60.0;
    t.hard_mem_percent = 70.0;
    t.hard_swap_percent_free = 80.0;
    t.hard_zram_percent_used = 90.0;

    StubSnapshot snap{1000.0, 2000.0, 500.0};

    const ThresholdSet out = Thresholds::compute(t, snap);

    ASSERT_TRUE(out.warn_mem_free.percent.has_value());
    EXPECT_DOUBLE_EQ(10.0, out.warn_mem_free.percent.value());
    ASSERT_TRUE(out.warn_mem_free.mib.has_value());
    EXPECT_DOUBLE_EQ(100.0, out.warn_mem_free.mib.value());

    ASSERT_TRUE(out.warn_swap_free.percent.has_value());
    EXPECT_DOUBLE_EQ(20.0, out.warn_swap_free.percent.value());
    ASSERT_TRUE(out.warn_swap_free.mib.has_value());
    EXPECT_DOUBLE_EQ(400.0, out.warn_swap_free.mib.value());

    ASSERT_TRUE(out.warn_zram_used.percent.has_value());
    EXPECT_DOUBLE_EQ(30.0, out.warn_zram_used.percent.value());
    ASSERT_TRUE(out.warn_zram_used.mib.has_value());
    EXPECT_DOUBLE_EQ(150.0, out.warn_zram_used.mib.value());

    ASSERT_TRUE(out.soft_mem_free.percent.has_value());
    EXPECT_DOUBLE_EQ(40.0, out.soft_mem_free.percent.value());
    ASSERT_TRUE(out.soft_mem_free.mib.has_value());
    EXPECT_DOUBLE_EQ(400.0, out.soft_mem_free.mib.value());

    ASSERT_TRUE(out.soft_swap_free.percent.has_value());
    EXPECT_DOUBLE_EQ(50.0, out.soft_swap_free.percent.value());
    ASSERT_TRUE(out.soft_swap_free.mib.has_value());
    EXPECT_DOUBLE_EQ(1000.0, out.soft_swap_free.mib.value());

    ASSERT_TRUE(out.soft_zram_used.percent.has_value());
    EXPECT_DOUBLE_EQ(60.0, out.soft_zram_used.percent.value());
    ASSERT_TRUE(out.soft_zram_used.mib.has_value());
    EXPECT_DOUBLE_EQ(300.0, out.soft_zram_used.mib.value());

    ASSERT_TRUE(out.hard_mem_free.percent.has_value());
    EXPECT_DOUBLE_EQ(70.0, out.hard_mem_free.percent.value());
    ASSERT_TRUE(out.hard_mem_free.mib.has_value());
    EXPECT_DOUBLE_EQ(700.0, out.hard_mem_free.mib.value());

    ASSERT_TRUE(out.hard_swap_free.percent.has_value());
    EXPECT_DOUBLE_EQ(80.0, out.hard_swap_free.percent.value());
    ASSERT_TRUE(out.hard_swap_free.mib.has_value());
    EXPECT_DOUBLE_EQ(1600.0, out.hard_swap_free.mib.value());

    ASSERT_TRUE(out.hard_zram_used.percent.has_value());
    EXPECT_DOUBLE_EQ(90.0, out.hard_zram_used.percent.value());
    ASSERT_TRUE(out.hard_zram_used.mib.has_value());
    EXPECT_DOUBLE_EQ(450.0, out.hard_zram_used.mib.value());
}
