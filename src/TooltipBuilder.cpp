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

    // RAM
    s += "RAM:\n";
    s += "  available: " + fmtMiB(snap.mem().memAvailableMiB) + " (" + fmtPct(snap.mem().memAvailablePercent) + ")\n";
    if (th.warn_mem_free.percent) s += "  warn if free < " + fmtPct(*th.warn_mem_free.percent) + " (≈ " + (th.warn_mem_free.mib ? fmtMiB(*th.warn_mem_free.mib) : "?") + ")\n";
    if (th.soft_mem_free.percent) s += "  soft action if free < " + fmtPct(*th.soft_mem_free.percent) + " (≈ " + (th.soft_mem_free.mib ? fmtMiB(*th.soft_mem_free.mib) : "?") + ")\n";
    if (th.hard_mem_free.percent) s += "  hard action if free < " + fmtPct(*th.hard_mem_free.percent) + " (≈ " + (th.hard_mem_free.mib ? fmtMiB(*th.hard_mem_free.mib) : "?") + ")\n";

    // Swap
    s += "Swap:\n";
    s += "  free: " + fmtMiB(snap.mem().swapFreeMiB) + " (" + fmtPct(snap.mem().swapFreePercent) + ")\n";
    if (th.warn_swap_free.percent) s += "  warn if free < " + fmtPct(*th.warn_swap_free.percent) + " (≈ " + (th.warn_swap_free.mib ? fmtMiB(*th.warn_swap_free.mib) : "?") + ")\n";
    if (th.soft_swap_free.percent) s += "  soft action if free < " + fmtPct(*th.soft_swap_free.percent) + " (≈ " + (th.soft_swap_free.mib ? fmtMiB(*th.soft_swap_free.mib) : "?") + ")\n";
    if (th.hard_swap_free.percent) s += "  hard action if free < " + fmtPct(*th.hard_swap_free.percent) + " (≈ " + (th.hard_swap_free.mib ? fmtMiB(*th.hard_swap_free.mib) : "?") + ")\n";

    // ZRAM
    if (snap.zram().present) {
        s += "ZRAM:\n";
        s += "  size: " + fmtMiB(snap.zram().diskSizeMiB) + "\n";
        s += "  logical used: " + fmtMiB(snap.zram().origDataMiB) + " (" + fmtPct(snap.zram().logicalUsedPercent) + ")\n";
        s += "  physical used: " + fmtMiB(snap.zram().memUsedTotalMiB) + "\n";
        if (th.warn_zram_used.percent) s += "  warn if used > " + fmtPct(*th.warn_zram_used.percent) + "\n";
        if (th.soft_zram_used.percent) s += "  soft action if used > " + fmtPct(*th.soft_zram_used.percent) + "\n";
        if (th.hard_zram_used.percent) s += "  hard action if used > " + fmtPct(*th.hard_zram_used.percent) + "\n";
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
