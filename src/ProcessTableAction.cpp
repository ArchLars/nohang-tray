// ===== src/ProcessTableAction.cpp =====
#include "ProcessTableAction.h"
#include <QAction>
#include <QProcess>
#include <QTextEdit>
#include <QDialog>
#include <QVBoxLayout>

ProcessTableAction::ProcessTableAction(QObject* parent) : QObject(parent) {}

QAction* ProcessTableAction::makeAction(QWidget* parentWidget, const QString& configPath) {
    m_cfgPath = configPath;
    auto act = new QAction(tr("Show nohang tasks"), parentWidget);
    connect(act, &QAction::triggered, this, &ProcessTableAction::runTasks);
    return act;
}

void ProcessTableAction::runTasks() {
    // This is a simple viewer, no privilege escalation. Users can adjust later.
    QProcess p;
    QStringList args{QStringLiteral("--tasks")};
    if (!m_cfgPath.isEmpty()) {
        args << QStringLiteral("-c") << m_cfgPath;
    }
    p.start(QStringLiteral("nohang"), args);
    p.waitForFinished(3000);

    auto dlg = new QDialog();
    dlg->setWindowTitle(QStringLiteral("nohang --tasks"));
    auto* edit = new QTextEdit(dlg);
    edit->setReadOnly(true);
    edit->setPlainText(QString::fromUtf8(p.readAllStandardOutput()));
    auto* lay = new QVBoxLayout(dlg);
    lay->addWidget(edit);
    dlg->resize(800, 500);
    dlg->show();
}
