// ===== src/NoHangUnit.cpp =====
#include "NoHangUnit.h"
#include <QProcess>
#include <QStringList>

static const char* kUnit = "nohang-desktop.service";

NoHangUnit::NoHangUnit(QObject* parent) : QObject(parent) {}

bool NoHangUnit::isActive() const {
    QProcess p;
    p.start(QStringLiteral("systemctl"), {QStringLiteral("is-active"), QString::fromLatin1(kUnit)});
    p.waitForFinished(1500);
    return p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0;
}

QString NoHangUnit::configPath() const {
    if (!m_haveCached) {
        const auto exec = readExecStart();
        m_cachedConfig = parseConfigFromExec(exec);
        if (m_cachedConfig.isEmpty()) {
            // Fallbacks, first etc, then distro defaults
            m_cachedConfig = QStringLiteral("/etc/nohang/nohang-desktop.conf");
        }
        m_haveCached = true;
    }
    return m_cachedConfig;
}

QString NoHangUnit::resolvedConfigPath() const {
    return configPath();
}

QString NoHangUnit::readExecStart() const {
    QProcess p;
    p.start(QStringLiteral("systemctl"), {QStringLiteral("show"), QString::fromLatin1(kUnit), QStringLiteral("-p"), QStringLiteral("ExecStart")});
    p.waitForFinished(1500);
    return QString::fromUtf8(p.readAllStandardOutput());
}

QString NoHangUnit::parseConfigFromExec(const QString& execStart) const {
    // Example line: ExecStart={ path=/usr/bin/nohang ; argv[]=/usr/bin/nohang --monitor --config /etc/nohang/nohang-desktop.conf ; ... }
    // Skeleton parser, keep simple, search for "--config" and take the next token
    const QString flag = QStringLiteral("--config");
    const QStringList toks = execStart.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (int i = 0; i < toks.size(); ++i) {
        if (toks[i] == flag && i + 1 < toks.size()) {
            return toks[i + 1];
        }
    }
    return {};
}
