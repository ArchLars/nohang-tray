// ===== src/NoHangConfig.cpp =====
#include "pch.h"
#include "NoHangConfig.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>

NoHangConfig::NoHangConfig(QObject* parent) : QObject(parent) {}

void NoHangConfig::ensureParsed(const QString& cfgPath) {
    QString path = cfgPath.isEmpty() ? QStringLiteral("/etc/nohang/nohang-desktop.conf") : cfgPath;
    QFileInfo fi(path);
    if (!fi.exists()) {
        // Fallback to distro defaults
        path = QStringLiteral("/usr/share/nohang/nohang.conf");
        fi = QFileInfo(path);
    }
    if (!fi.exists()) {
        m_srcPath.clear();
        m_t = {};
        return;
    }
    const qint64 mt = fi.lastModified().toSecsSinceEpoch();
    if (path == m_srcPath && mt == m_lastMtime) return;

    parseFile(path);
    m_srcPath = path;
    m_lastMtime = mt;
}

void NoHangConfig::parseFile(const QString& path) {
    ThresholdsPercent out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_t = {};
        return;
    }
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('@')) continue;

        auto setIf = [&](const QString& key, std::optional<double>& slot){
            if (line.startsWith(key)) {
                const int eq = line.indexOf('=');
                if (eq > 0) {
                    const QString raw = line.mid(eq + 1).trimmed();
                    slot = parsePercentOrMiB(raw);
                }
            }
        };

        setIf(QStringLiteral("warning_threshold_min_mem"),  out.warn_mem_percent);
        setIf(QStringLiteral("warning_threshold_min_swap"), out.warn_swap_percent_free);
        setIf(QStringLiteral("warning_threshold_max_zram"), out.warn_zram_percent_used);
        setIf(QStringLiteral("warning_threshold_max_psi"),  out.warn_psi);

        setIf(QStringLiteral("soft_threshold_min_mem"),     out.soft_mem_percent);
        setIf(QStringLiteral("soft_threshold_min_swap"),    out.soft_swap_percent_free);
        setIf(QStringLiteral("soft_threshold_max_zram"),    out.soft_zram_percent_used);
        setIf(QStringLiteral("soft_threshold_max_psi"),     out.soft_psi);

        setIf(QStringLiteral("hard_threshold_min_mem"),     out.hard_mem_percent);
        setIf(QStringLiteral("hard_threshold_min_swap"),    out.hard_swap_percent_free);
        setIf(QStringLiteral("hard_threshold_max_zram"),    out.hard_zram_percent_used);
        setIf(QStringLiteral("hard_threshold_max_psi"),     out.hard_psi);

        if (line.startsWith(QStringLiteral("psi_metrics"))) {
            const int eq = line.indexOf('=');
            if (eq > 0) out.psi_metrics = line.mid(eq + 1).trimmed();
        }
        if (line.startsWith(QStringLiteral("psi_excess_duration"))) {
            const int eq = line.indexOf('=');
            if (eq > 0) out.psi_duration = line.mid(eq + 1).trimmed().toDouble();
        }
    }
    m_t = out;
}

std::optional<double> NoHangConfig::parsePercentOrMiB(const QString& raw) {
    // Accept "10 %", "10%", "512 M", "512M"
    const QString s = raw.simplified();
    QRegularExpression rePercent(R"(^([0-9]+(\.[0-9]+)?)\s*%$)");
    QRegularExpression reMiB(R"(^([0-9]+(\.[0-9]+)?)\s*M(i?B)?$)", QRegularExpression::CaseInsensitiveOption);

    if (auto m = rePercent.match(s); m.hasMatch()) {
        return m.captured(1).toDouble(); // store percent as percent
    }
    if (auto m = reMiB.match(s); m.hasMatch()) {
        // Store MiB as a negative sentinel or keep as positive and tag elsewhere.
        // Simpler, keep positive MiB and let Thresholds convert with totals.
        return m.captured(1).toDouble(); // MiB value
    }
    // If it is a bare number, treat as percent
    bool ok = false;
    const double v = s.toDouble(&ok);
    if (ok) return v;
    return std::nullopt;
}
