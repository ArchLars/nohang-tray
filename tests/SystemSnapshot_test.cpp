#include "pch.h"
#include <gtest/gtest.h>
#include "SystemSnapshot.h"
#include <QTemporaryDir>
#include <QTest>

TEST(SystemSnapshotTest, RefreshPopulatesFields)
{
    SystemSnapshot snap;
    snap.refresh();
    EXPECT_GT(snap.mem().memTotalMiB, 0);
    EXPECT_GT(snap.mem().memAvailableMiB, 0);
    EXPECT_GT(snap.mem().memAvailablePercent, 0);
    EXPECT_LE(snap.mem().memAvailablePercent, 100);
    EXPECT_GE(snap.mem().swapFreeMiB, 0);
    EXPECT_GE(snap.psi().some_avg10, 0);
    EXPECT_GE(snap.psi().full_avg10, 0);
}

TEST(SystemSnapshotTest, MissingMeminfoLogsWarning)
{
    QTemporaryDir procDir;
    QTemporaryDir sysDir;
    SystemSnapshot snap(procDir.path(), sysDir.path());

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("SystemSnapshot: cannot open .*meminfo"));
    snap.refresh();

    EXPECT_DOUBLE_EQ(0.0, snap.mem().memTotalMiB);
}

TEST(SystemSnapshotTest, ParsesMeminfoAndHandlesMissingPsi)
{
    QTemporaryDir procDir;
    QTemporaryDir sysDir;

    QFile meminfo(procDir.filePath("meminfo"));
    ASSERT_TRUE(meminfo.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream ts(&meminfo);
    ts << "MemTotal:       2048 kB\n";
    ts << "MemAvailable:   1024 kB\n";
    ts << "SwapTotal:       512 kB\n";
    ts << "SwapFree:        256 kB\n";
    meminfo.close();

    SystemSnapshot snap(procDir.path(), sysDir.path());
    snap.refresh();

    EXPECT_DOUBLE_EQ(2.0, snap.mem().memTotalMiB);
    EXPECT_DOUBLE_EQ(1.0, snap.mem().memAvailableMiB);
    EXPECT_DOUBLE_EQ(0.5, snap.mem().swapTotalMiB);
    EXPECT_DOUBLE_EQ(0.25, snap.mem().swapFreeMiB);
    EXPECT_DOUBLE_EQ(50.0, snap.mem().memAvailablePercent);
    EXPECT_DOUBLE_EQ(50.0, snap.mem().swapFreePercent);
    EXPECT_DOUBLE_EQ(0.0, snap.psi().some_avg10);
    EXPECT_DOUBLE_EQ(0.0, snap.psi().full_avg10);
}

TEST(SystemSnapshotTest, ParsesMeminfoWithLeadingSpaces)
{
    QTemporaryDir procDir;
    QTemporaryDir sysDir;

    QFile meminfo(procDir.filePath("meminfo"));
    ASSERT_TRUE(meminfo.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream ts(&meminfo);
    ts << "   MemTotal:       2048 kB\n";
    ts << "   MemAvailable:   1024 kB\n";
    ts << "   SwapTotal:       512 kB\n";
    ts << "   SwapFree:        256 kB\n";
    meminfo.close();

    SystemSnapshot snap(procDir.path(), sysDir.path());
    snap.refresh();

    EXPECT_DOUBLE_EQ(2.0, snap.mem().memTotalMiB);
    EXPECT_DOUBLE_EQ(1.0, snap.mem().memAvailableMiB);
    EXPECT_DOUBLE_EQ(0.5, snap.mem().swapTotalMiB);
    EXPECT_DOUBLE_EQ(0.25, snap.mem().swapFreeMiB);
}

TEST(SystemSnapshotTest, ParsesSwapsWhenMeminfoMissingSwap)
{
    QTemporaryDir procDir;
    QTemporaryDir sysDir;

    // meminfo without swap stats
    QFile meminfo(procDir.filePath("meminfo"));
    ASSERT_TRUE(meminfo.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream ts(&meminfo);
    ts << "MemTotal:       2048 kB\n";
    ts << "MemAvailable:   1024 kB\n";
    meminfo.close();

    QFile swaps(procDir.filePath("swaps"));
    ASSERT_TRUE(swaps.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream tsw(&swaps);
    tsw << "Filename\tType\tSize\tUsed\tPriority\n";
    tsw << "/dev/zram0\tpartition\t512\t256\t100\n";
    swaps.close();

    SystemSnapshot snap(procDir.path(), sysDir.path());
    snap.refresh();

    EXPECT_DOUBLE_EQ(0.5, snap.mem().swapTotalMiB);
    EXPECT_DOUBLE_EQ(0.25, snap.mem().swapFreeMiB);
    EXPECT_DOUBLE_EQ(50.0, snap.mem().swapFreePercent);
}

TEST(SystemSnapshotTest, ParsesMultipleSwapEntries)
{
    QTemporaryDir procDir;
    QTemporaryDir sysDir;

    QFile meminfo(procDir.filePath("meminfo"));
    ASSERT_TRUE(meminfo.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream ts(&meminfo);
    ts << "MemTotal:       2048 kB\n";
    ts << "MemAvailable:   1024 kB\n";
    meminfo.close();

    QFile swaps(procDir.filePath("swaps"));
    ASSERT_TRUE(swaps.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream tsw(&swaps);
    tsw << "Filename Type Size Used Priority\n";
    tsw << "/dev/zram0 partition 512 128 100\n";
    tsw << "/swapfile file 1024 512 -2\n";
    swaps.close();

    SystemSnapshot snap(procDir.path(), sysDir.path());
    snap.refresh();

    EXPECT_DOUBLE_EQ(1.5, snap.mem().swapTotalMiB);
    EXPECT_DOUBLE_EQ(0.875, snap.mem().swapFreeMiB);
    EXPECT_NEAR(58.3333, snap.mem().swapFreePercent, 0.001);
}
