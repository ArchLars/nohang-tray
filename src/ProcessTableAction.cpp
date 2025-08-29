// ===== src/ProcessTableAction.cpp =====
#include "pch.h"
#include "ProcessTableAction.h"
#include <QAction>
#include <QProcess>
#include <QTextEdit>
#include <QDialog>
#include <QVBoxLayout>
#include <QTimer>

ProcessTableAction::ProcessTableAction(QObject* parent) : QObject(parent) {}

QAction* ProcessTableAction::makeAction(QWidget* parentWidget, const QString& configPath) {
    m_cfgPath = configPath;
    auto act = new QAction(tr("Show nohang tasks"), parentWidget);
    connect(act, &QAction::triggered, this, &ProcessTableAction::runTasks);
    return act;
}

void ProcessTableAction::runTasks() {
    // This is a simple viewer, no privilege escalation. Users can adjust later.
    auto* proc = new QProcess(this);
    QStringList args{QStringLiteral("--tasks")};
    if (!m_cfgPath.isEmpty()) {
        args << QStringLiteral("-c") << m_cfgPath;
    }

    auto showDialog = [](const QString& text) {
        auto dlg = new QDialog();
        dlg->setWindowTitle(QStringLiteral("nohang --tasks"));
        auto* edit = new QTextEdit(dlg);
        edit->setReadOnly(true);
        edit->setPlainText(text);
        auto* lay = new QVBoxLayout(dlg);
        lay->addWidget(edit);
        dlg->resize(800, 500);
        dlg->show();
    };

    auto* timer = new QTimer(proc);
    timer->setSingleShot(true);
    timer->setInterval(3000);
    connect(timer, &QTimer::timeout, this, [proc, showDialog]() {
        proc->disconnect();
        proc->kill();
        showDialog(QObject::tr("nohang --tasks timed out"));
        proc->deleteLater();
    });

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [proc, timer, showDialog](int exitCode, QProcess::ExitStatus exitStatus) {
                timer->stop();
                QString text;
                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    text = QObject::tr("nohang failed (exit code %1)").arg(exitCode);
                } else {
                    text = QString::fromUtf8(proc->readAllStandardOutput());
                }
                showDialog(text);
                proc->deleteLater();
            });

    connect(proc, &QProcess::errorOccurred, this, [proc, timer, showDialog](QProcess::ProcessError) {
        timer->stop();
        proc->disconnect();
        showDialog(QObject::tr("failed to start nohang"));
        proc->deleteLater();
    });

    proc->start(QStringLiteral("nohang"), args);
    timer->start();
}
