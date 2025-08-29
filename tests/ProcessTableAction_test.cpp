#include <gtest/gtest.h>
#include <QApplication>
#include <QAction>
#define private public
#include "ProcessTableAction.h"
#undef private

TEST(ProcessTableActionTest, StoresConfigPathAndCreatesAction)
{
    int argc = 0;
    char** argv = nullptr;
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);

    ProcessTableAction act;
    QAction* qact = act.makeAction(nullptr, "/etc/nohang/foo.conf");
    ASSERT_NE(nullptr, qact);
    EXPECT_EQ("/etc/nohang/foo.conf", act.m_cfgPath);
    EXPECT_EQ(QString("Show nohang tasks"), qact->text());
}
