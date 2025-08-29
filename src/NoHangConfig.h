// ===== src/NoHangConfig.h =====
#pragma once
#include <QObject>
#include <QString>
#include <optional>

// Parsed threshold values, percentages are stored as double in [0,100],
// MiB values are stored as double MiB. Only a subset is needed for the tooltip.
struct ThresholdsPercent {
    std::optional<double> warn_mem_percent;
    std::optional<double> warn_swap_percent_free;
    std::optional<double> warn_zram_percent_used;
    std::optional<double> warn_psi;

    std::optional<double> soft_mem_percent;
    std::optional<double> soft_swap_percent_free;
    std::optional<double> soft_zram_percent_used;
    std::optional<double> soft_psi;

    std::optional<double> hard_mem_percent;
    std::optional<double> hard_swap_percent_free;
    std::optional<double> hard_zram_percent_used;
    std::optional<double> hard_psi;

    QString psi_metrics;                  // e.g. "full_avg10"
    std::optional<double> psi_duration;   // seconds
};

class NoHangConfig : public QObject {
    Q_OBJECT
public:
    explicit NoHangConfig(QObject* parent = nullptr);

    void ensureParsed(const QString& cfgPath);    // no-op if already parsed or unchanged
    const ThresholdsPercent& thresholds() const { return m_t; }
    QString sourcePath() const { return m_srcPath; }

private:
    void parseFile(const QString& path);          // simple line parser, ignore comments and @ lines
    static std::optional<double> parsePercentOrMiB(const QString& raw); // percent positive, MiB as negative sentinel

    ThresholdsPercent m_t;
    QString m_srcPath;
    qint64  m_lastMtime {0};
};
