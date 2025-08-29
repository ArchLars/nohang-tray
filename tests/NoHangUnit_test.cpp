#include "pch.h"
#include <gtest/gtest.h>
#define private public
#include "NoHangUnit.h"
#undef private

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
