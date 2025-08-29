#include "pch.h"
#include <gtest/gtest.h>
#include "SystemSnapshot.h"

TEST(SystemSnapshotTest, RefreshPopulatesFields)
{
    SystemSnapshot snap;
    snap.refresh();
    EXPECT_GE(snap.mem().memTotalMiB, 0);
    EXPECT_GE(snap.mem().memAvailableMiB, 0);
    EXPECT_GE(snap.mem().memAvailablePercent, 0);
    EXPECT_LE(snap.mem().memAvailablePercent, 100);
    EXPECT_GE(snap.mem().swapFreeMiB, 0);
    EXPECT_GE(snap.psi().some_avg10, 0);
    EXPECT_GE(snap.psi().full_avg10, 0);
}
