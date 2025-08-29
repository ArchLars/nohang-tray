#include "pch.h"
#include <gtest/gtest.h>
#include <QApplication>
#include <QAction>
#include <QDialog>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QFile>
#include <QFileInfo>
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

TEST(ProcessTableActionTest, RunTasksDisplaysOutput)
{
    // prepare dummy 'nohang' executable
    QTemporaryDir dir;
    QFile script(dir.filePath("nohang"));
    ASSERT_TRUE(script.open(QIODevice::WriteOnly | QIODevice::Text));
    script.write("#!/bin/sh\necho dummy output\n");
    script.close();
    QFile::setPermissions(script.fileName(), QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther |
                                        QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    QByteArray newPath = dir.path().toUtf8() + ':' + qgetenv("PATH");
    qputenv("PATH", newPath);

    int argc = 0;
    char** argv = nullptr;
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);

    int before = QApplication::topLevelWidgets().size();

    ProcessTableAction act;
    act.runTasks();
    QApplication::processEvents();

    EXPECT_EQ(before + 1, QApplication::topLevelWidgets().size());

    bool found = false;
    for (QWidget* w : QApplication::topLevelWidgets()) {
        if (auto* dlg = qobject_cast<QDialog*>(w)) {
            if (dlg->windowTitle() == "nohang --tasks") {
                QTextEdit* edit = dlg->findChild<QTextEdit*>();
                ASSERT_NE(nullptr, edit);
                EXPECT_EQ(QStringLiteral("dummy output\n"), edit->toPlainText());
                dlg->close();
                found = true;
            }
        }
    }
    EXPECT_TRUE(found);

    QApplication::processEvents();
    EXPECT_EQ(before, QApplication::topLevelWidgets().size());
}
