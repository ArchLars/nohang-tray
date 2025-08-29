#include "pch.h"
#include <gtest/gtest.h>
#include <QApplication>
#include <QAction>
#include <QDialog>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QFile>
#include <QFileInfo>
#define private public
#include "ProcessTableAction.h"
#undef private

static QDialog* waitForDialog(const QString& title, int timeoutMs = 5000)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        for (QWidget* w : QApplication::topLevelWidgets()) {
            if (auto* dlg = qobject_cast<QDialog*>(w)) {
                if (dlg->windowTitle() == title) {
                    return dlg;
                }
            }
        }
    }
    return nullptr;
}

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

    ProcessTableAction act;
    act.runTasks();
    QDialog* dlg = waitForDialog("nohang --tasks");
    ASSERT_NE(nullptr, dlg);
    QTextEdit* edit = dlg->findChild<QTextEdit*>();
    ASSERT_NE(nullptr, edit);
    EXPECT_EQ(QStringLiteral("dummy output\n"), edit->toPlainText());
    dlg->close();
}

TEST(ProcessTableActionTest, RunTasksTimesOutShowsMessage)
{
    QTemporaryDir dir;
    QFile script(dir.filePath("nohang"));
    ASSERT_TRUE(script.open(QIODevice::WriteOnly | QIODevice::Text));
    script.write("#!/bin/sh\nsleep 5\necho done\n");
    script.close();
    QFile::setPermissions(script.fileName(), QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther |
                                        QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    QByteArray newPath = dir.path().toUtf8() + ':' + qgetenv("PATH");
    qputenv("PATH", newPath);

    int argc = 0;
    char** argv = nullptr;
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);

    ProcessTableAction act;
    act.runTasks();
    QDialog* dlg = waitForDialog("nohang --tasks", 7000);
    ASSERT_NE(nullptr, dlg);
    QTextEdit* edit = dlg->findChild<QTextEdit*>();
    ASSERT_NE(nullptr, edit);
    EXPECT_TRUE(edit->toPlainText().contains("timed out"));
    dlg->close();
}

TEST(ProcessTableActionTest, RunTasksDisplaysErrorOnFailure)
{
    QTemporaryDir dir;
    QFile script(dir.filePath("nohang"));
    ASSERT_TRUE(script.open(QIODevice::WriteOnly | QIODevice::Text));
    script.write("#!/bin/sh\necho oops >&2\nexit 1\n");
    script.close();
    QFile::setPermissions(script.fileName(), QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther |
                                        QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    QByteArray newPath = dir.path().toUtf8() + ':' + qgetenv("PATH");
    qputenv("PATH", newPath);

    int argc = 0;
    char** argv = nullptr;
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);

    ProcessTableAction act;
    act.runTasks();
    QDialog* dlg = waitForDialog("nohang --tasks");
    ASSERT_NE(nullptr, dlg);
    QTextEdit* edit = dlg->findChild<QTextEdit*>();
    ASSERT_NE(nullptr, edit);
    EXPECT_TRUE(edit->toPlainText().contains("failed"));
    EXPECT_TRUE(edit->toPlainText().contains("exit code 1"));
    dlg->close();
}
