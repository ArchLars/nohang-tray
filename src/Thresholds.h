// ===== src/Thresholds.h =====
#pragma once
#include "NoHangConfig.h"
#include "SystemSnapshot.h"

// ThresholdSet holds absolute values calculated from percents and totals.
// For each dimension, store configured threshold and computed MiB.
struct ThresholdValue {
    std::optional<double> percent; // if configured as percent
    std::optional<double> mib;     // absolute MiB derived from totals, or configured directly
};

struct ThresholdSet {
    ThresholdValue warn_mem_free;     // free RAM floor
    ThresholdValue warn_swap_free;
    ThresholdValue warn_zram_used;    // used percent of zram logical size
    std::optional<double> warn_psi;

    ThresholdValue soft_mem_free;
    ThresholdValue soft_swap_free;
    ThresholdValue soft_zram_used;
    std::optional<double> soft_psi;

    ThresholdValue hard_mem_free;
    ThresholdValue hard_swap_free;
    ThresholdValue hard_zram_used;
    std::optional<double> hard_psi;

    QString psi_metrics;
    std::optional<double> psi_duration;
};

class Thresholds {
public:
    static ThresholdSet compute(const ThresholdsPercent& t, const SystemSnapshot& snap);
};
