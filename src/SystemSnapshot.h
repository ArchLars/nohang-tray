// ===== src/SystemSnapshot.h =====
#pragma once
#include <QObject>
#include <QString>
#include <optional>

// Live system values, read on each poll
struct MemInfo {
    double memTotalMiB {0};
    double memAvailableMiB {0};
    double memAvailablePercent {0};
    double swapTotalMiB {0};
    double swapFreeMiB {0};
    double swapFreePercent {0};
};

struct ZramInfo {
    bool   present {false};
    double diskSizeMiB {0};        // logical capacity
    double origDataMiB {0};        // logical used, uncompressed
    double comprDataMiB {0};       // compressed size
    double memUsedTotalMiB {0};    // physical RAM used
    double logicalUsedPercent {0}; // origDataMiB / diskSizeMiB
};

struct PsiInfo {
    double some_avg10 {0};
    double full_avg10 {0};
};

class SystemSnapshot : public QObject {
    Q_OBJECT
public:
    explicit SystemSnapshot(QObject* parent = nullptr);

    void refresh();

    const MemInfo& mem() const { return m_mem; }
    const ZramInfo& zram() const { return m_zram; }
    const PsiInfo& psi() const { return m_psi; }

private:
    void readMeminfo();
    void readSwaps();
    void readZram();
    void readPsi();

    MemInfo m_mem;
    ZramInfo m_zram;
    PsiInfo m_psi;
};
