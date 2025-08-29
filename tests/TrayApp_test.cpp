#include "pch.h"
#include <gtest/gtest.h>
#include "TrayApp.h"

TEST(TrayAppTest, EscapePercent)
{
    const QString input = "value 42 %";
    const QString output = TrayApp::escapePercent(input);
    EXPECT_EQ(QStringLiteral("value 42 %%"), output);
}
