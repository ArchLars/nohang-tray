// ===== src/Thresholds.cpp =====
#include "pch.h"
#include "Thresholds.h"

static ThresholdValue makeVal(std::optional<double> raw, double totalMiB) {
    ThresholdValue v;
    if (raw) {
        if (*raw < 0) {
            v.mib = -*raw;
        } else {
            v.percent = raw;
            v.mib = totalMiB > 0 ? std::optional<double>(*raw * totalMiB / 100.0) : std::nullopt;
        }
    }
    return v;
}

ThresholdSet Thresholds::compute(const ThresholdsPercent& t, const SystemSnapshot& snap) {
    ThresholdSet out;
    // RAM and Swap thresholds are free space floors
    out.warn_mem_free  = makeVal(t.warn_mem_percent,  snap.mem().memTotalMiB);
    out.warn_swap_free = makeVal(t.warn_swap_percent_free, snap.mem().swapTotalMiB);
    // ZRAM thresholds are used percent of logical disksize
    out.warn_zram_used = makeVal(t.warn_zram_percent_used, snap.zram().diskSizeMiB);
    out.warn_psi       = t.warn_psi;

    out.soft_mem_free  = makeVal(t.soft_mem_percent,  snap.mem().memTotalMiB);
    out.soft_swap_free = makeVal(t.soft_swap_percent_free, snap.mem().swapTotalMiB);
    out.soft_zram_used = makeVal(t.soft_zram_percent_used, snap.zram().diskSizeMiB);
    out.soft_psi       = t.soft_psi;

    out.hard_mem_free  = makeVal(t.hard_mem_percent,  snap.mem().memTotalMiB);
    out.hard_swap_free = makeVal(t.hard_swap_percent_free, snap.mem().swapTotalMiB);
    out.hard_zram_used = makeVal(t.hard_zram_percent_used, snap.zram().diskSizeMiB);
    out.hard_psi       = t.hard_psi;

    out.psi_metrics    = t.psi_metrics;
    out.psi_duration   = t.psi_duration;
    return out;
}
