// ===== src/ProcessTableAction.h =====
#pragma once
#include <QObject>

class QAction;
class QWidget;

// Optional helper that adds an action to open `nohang --tasks` output.
// In the skeleton we expose a factory method, the TrayApp can add it to the SNI menu later.
class ProcessTableAction : public QObject {
    Q_OBJECT
public:
    explicit ProcessTableAction(QObject* parent = nullptr);
    QAction* makeAction(QWidget* parentWidget, const QString& configPath);

private slots:
    void runTasks();

private:
    QString m_cfgPath;
};
