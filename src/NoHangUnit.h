// ===== src/NoHangUnit.h =====
#pragma once
#include <QObject>
#include <QString>

// NoHangUnit talks to systemd to learn if the service is active,
// and discovers the ExecStart to find the --config path in use.
class NoHangUnit : public QObject {
    Q_OBJECT
public:
    explicit NoHangUnit(QObject* parent = nullptr);

    bool isActive() const;                           // systemctl is-active nohang-desktop.service
    QString configPath(bool refresh = false) const;  // parse ExecStart for --config
    QString resolvedConfigPath() const;              // cached, or fallback to defaults

protected:
    virtual QString readExecStart() const;
    QString parseConfigFromExec(const QString& execStart) const;

private:
    mutable QString m_cachedConfig;
    mutable bool m_haveCached {false};
    mutable QString m_lastExecStart;
};
