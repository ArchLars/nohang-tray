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
    s += "RAM: available " + fmtMiB(snap.mem().memAvailableMiB) + " (" + fmtPct(snap.mem().memAvailablePercent) + ")\n";

    // Swap
    s += "Swap: total " + fmtMiB(snap.mem().swapTotalMiB) + ", free " + fmtMiB(snap.mem().swapFreeMiB) + " (" + fmtPct(snap.mem().swapFreePercent) + ")\n";

    // ZRAM
    if (snap.zram().present) {
        s += "ZRAM: size " + fmtMiB(snap.zram().diskSizeMiB) + ", logical used " + fmtMiB(snap.zram().origDataMiB) + " (" + fmtPct(snap.zram().logicalUsedPercent) + "), physical used " + fmtMiB(snap.zram().memUsedTotalMiB) + "\n";
    }

    // PSI
    s += "PSI: full avg10 " + QString::number(snap.psi().full_avg10, 'f', 2) + ", some avg10 " + QString::number(snap.psi().some_avg10, 'f', 2);
    if (!cfg.thresholds().psi_metrics.isEmpty()) s += ", metric " + cfg.thresholds().psi_metrics;
    if (th.psi_duration) s += ", duration " + QString::number(*th.psi_duration, 'f', 0) + " s";
    s += "\n";

    // Thresholds after current values
    s += "Thresholds:\n";
    appendThreshold("  RAM warn if free < ", th.warn_mem_free);
    appendThreshold("  RAM soft action if free < ", th.soft_mem_free);
    appendThreshold("  RAM hard action if free < ", th.hard_mem_free);
    appendThreshold("  Swap warn if free < ", th.warn_swap_free);
    appendThreshold("  Swap soft action if free < ", th.soft_swap_free);
    appendThreshold("  Swap hard action if free < ", th.hard_swap_free);
    if (snap.zram().present) {
        appendThreshold("  ZRAM warn if used > ", th.warn_zram_used);
        appendThreshold("  ZRAM soft action if used > ", th.soft_zram_used);
        appendThreshold("  ZRAM hard action if used > ", th.hard_zram_used);
    }
    if (th.warn_psi)
        s += "  PSI warn if > " + QString::number(*th.warn_psi, 'f', 0) + "\n";
    if (th.soft_psi)
        s += "  PSI soft action if > " + QString::number(*th.soft_psi, 'f', 0) + "\n";
    if (th.hard_psi)
        s += "  PSI hard action if > " + QString::number(*th.hard_psi, 'f', 0) + "\n";

    return s;
}
