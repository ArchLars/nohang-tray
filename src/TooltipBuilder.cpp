// ===== src/TooltipBuilder.cpp =====
#include "pch.h"
#include "TooltipBuilder.h"
#include "NoHangConfig.h"
#include "SystemSnapshot.h"
#include "Thresholds.h"
#include <QStringBuilder>

TooltipBuilder::TooltipBuilder(QObject* parent) : QObject(parent) {}

static QString fmtMiB(double v) { return QString::number(v, 'f', 0) + " MiB"; }
static QString fmtPct(double v) { return QString::number(v, 'f', 1) + " %"; }

QString TooltipBuilder::build(const NoHangConfig& cfg,
                              const SystemSnapshot& snap,
                              bool active,
                              const QString& cfgPath) const
{
    const ThresholdSet th = Thresholds::compute(cfg.thresholds(), snap);

    QString s;
    s += (active ? "status: active\n" : "status: inactive\n");
    if (!cfgPath.isEmpty()) s += "config: " + cfgPath + "\n";

    auto appendThreshold = [&](const QString& label, const ThresholdValue& tv) {
        if (tv.percent || tv.mib) {
            s += label;
            if (tv.percent) {
                s += fmtPct(*tv.percent);
                if (tv.mib) s += QStringLiteral(" (≈ ") + fmtMiB(*tv.mib) + QStringLiteral(")");
            } else if (tv.mib) {
                s += fmtMiB(*tv.mib);
            }
            s += QStringLiteral("\n");
        }
    };

    // RAM
    s += "RAM:\n";
    s += "  available: " + fmtMiB(snap.mem().memAvailableMiB) + " (" + fmtPct(snap.mem().memAvailablePercent) + ")\n";
    appendThreshold("  warn if free < ", th.warn_mem_free);
    appendThreshold("  soft action if free < ", th.soft_mem_free);
    appendThreshold("  hard action if free < ", th.hard_mem_free);

    // Swap
    s += "Swap:\n";
    s += "  total: " + fmtMiB(snap.mem().swapTotalMiB) + "\n";
    s += "  free: " + fmtMiB(snap.mem().swapFreeMiB) + " (" + fmtPct(snap.mem().swapFreePercent) + ")\n";
    appendThreshold("  warn if free < ", th.warn_swap_free);
    appendThreshold("  soft action if free < ", th.soft_swap_free);
    appendThreshold("  hard action if free < ", th.hard_swap_free);

    // ZRAM
    if (snap.zram().present) {
        s += "ZRAM:\n";
        s += "  size: " + fmtMiB(snap.zram().diskSizeMiB) + "\n";
        s += "  logical used: " + fmtMiB(snap.zram().origDataMiB) + " (" + fmtPct(snap.zram().logicalUsedPercent) + ")\n";
        s += "  physical used: " + fmtMiB(snap.zram().memUsedTotalMiB) + "\n";
        appendThreshold("  warn if used > ", th.warn_zram_used);
        appendThreshold("  soft action if used > ", th.soft_zram_used);
        appendThreshold("  hard action if used > ", th.hard_zram_used);
    }

    // PSI
    s += "PSI:\n";
    s += "  full avg10: " + QString::number(snap.psi().full_avg10, 'f', 2) + ", some avg10: " + QString::number(snap.psi().some_avg10, 'f', 2) + "\n";
    if (!cfg.thresholds().psi_metrics.isEmpty()) s += "  metric: " + cfg.thresholds().psi_metrics + "\n";
    if (th.warn_psi) s += "  warn if > " + QString::number(*th.warn_psi, 'f', 0) + "\n";
    if (th.soft_psi) s += "  soft action if > " + QString::number(*th.soft_psi, 'f', 0) + "\n";
    if (th.hard_psi) s += "  hard action if > " + QString::number(*th.hard_psi, 'f', 0) + "\n";
    if (th.psi_duration) s += "  duration: " + QString::number(*th.psi_duration, 'f', 0) + " s\n";

    // Short hint on actions
    s += "Actions:\n";
    s += "  soft: ask a target to exit cleanly, hard: force kill if pressure persists.\n";

    return s;
}
