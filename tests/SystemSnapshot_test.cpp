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

TEST(SystemSnapshotTest, ParsesZramStatsFromSeparateFiles)
{
    QTemporaryDir procDir;
    QTemporaryDir sysDir;

    QDir zramDir(sysDir.path());
    ASSERT_TRUE(zramDir.mkpath("block/zram0"));
    const QString base = sysDir.path() + "/block/zram0/";

    QFile disksize(base + "disksize");
    ASSERT_TRUE(disksize.open(QIODevice::WriteOnly | QIODevice::Text));
    disksize.write("4194304\n"); // 4 MiB
    disksize.close();

    QFile orig(base + "orig_data_size");
    ASSERT_TRUE(orig.open(QIODevice::WriteOnly | QIODevice::Text));
    orig.write("2097152\n"); // 2 MiB
    orig.close();

    QFile compr(base + "compr_data_size");
    ASSERT_TRUE(compr.open(QIODevice::WriteOnly | QIODevice::Text));
    compr.write("1048576\n"); // 1 MiB
    compr.close();

    QFile memused(base + "mem_used_total");
    ASSERT_TRUE(memused.open(QIODevice::WriteOnly | QIODevice::Text));
    memused.write("1572864\n"); // 1.5 MiB
    memused.close();

    SystemSnapshot snap(procDir.path(), sysDir.path());
    snap.refresh();

    EXPECT_TRUE(snap.zram().present);
    EXPECT_DOUBLE_EQ(4.0, snap.zram().diskSizeMiB);
    EXPECT_DOUBLE_EQ(2.0, snap.zram().origDataMiB);
    EXPECT_DOUBLE_EQ(1.0, snap.zram().comprDataMiB);
    EXPECT_DOUBLE_EQ(1.5, snap.zram().memUsedTotalMiB);
    EXPECT_DOUBLE_EQ(50.0, snap.zram().logicalUsedPercent);
}
