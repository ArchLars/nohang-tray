#include "pch.h"
#include <gtest/gtest.h>
#define protected public
#define private public
#include "NoHangUnit.h"
#undef private
#undef protected

TEST(NoHangUnitTest, ParseConfigFromExec)
{
    NoHangUnit unit;
    QString exec = "ExecStart={ path=/usr/bin/nohang ; argv[]=/usr/bin/nohang --monitor --config /etc/nohang/custom.conf ; }";
    EXPECT_EQ("/etc/nohang/custom.conf", unit.parseConfigFromExec(exec));
}

TEST(NoHangUnitTest, FallbacksWhenUnitAbsent)
{
    NoHangUnit unit;
    EXPECT_FALSE(unit.isActive());
    QString path = unit.configPath();
    EXPECT_EQ("/etc/nohang/nohang-desktop.conf", path);
    EXPECT_EQ(path, unit.resolvedConfigPath());
}

TEST(NoHangUnitTest, ConfigPathRefreshesWhenExecChanges)
{
    struct MockUnit : public NoHangUnit {
        QString exec;
        QString readExecStart() const override { return exec; }
    };

    MockUnit unit;
    unit.exec = "ExecStart={ path=/usr/bin/nohang ; argv[]=/usr/bin/nohang --monitor --config /etc/nohang/first.conf ; }";
    EXPECT_EQ("/etc/nohang/first.conf", unit.configPath());

    unit.exec = "ExecStart={ path=/usr/bin/nohang ; argv[]=/usr/bin/nohang --monitor --config /etc/nohang/second.conf ; }";
    EXPECT_EQ("/etc/nohang/second.conf", unit.configPath());

    unit.exec = "ExecStart={ path=/usr/bin/nohang ; argv[]=/usr/bin/nohang --monitor --config /etc/nohang/third.conf ; }";
    EXPECT_EQ("/etc/nohang/third.conf", unit.configPath(true));
}
