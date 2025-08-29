// ===== src/SystemSnapshot.cpp =====
#include "pch.h"
#include "SystemSnapshot.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

SystemSnapshot::SystemSnapshot(QObject* parent) : QObject(parent) {}

SystemSnapshot::SystemSnapshot(const QString& procRoot, const QString& sysRoot, QObject* parent)
    : QObject(parent), m_procRoot(procRoot), m_sysRoot(sysRoot) {}

void SystemSnapshot::refresh() {
    readMeminfo();
    readSwaps();
    readZram();
    readPsi();
}

void SystemSnapshot::readMeminfo() {
    QFile f(m_procRoot + QStringLiteral("/meminfo"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << "SystemSnapshot: cannot open" << f.fileName();
        m_mem = {};
        return;
    }
    QTextStream ts(&f);
    double memTotalKiB = 0, memAvailableKiB = 0, swapTotalKiB = 0, swapFreeKiB = 0;
    QString line;
    QRegularExpression re(R"(^\s*([A-Za-z_]+):\s+([0-9]+))");
    while (ts.readLineInto(&line)) {
        auto m = re.match(line);
        if (!m.hasMatch()) continue;
        const QString key = m.captured(1);
        const double val  = m.captured(2).toDouble();
        if (key == QLatin1String("MemTotal"))       memTotalKiB       = val;
        else if (key == QLatin1String("MemAvailable")) memAvailableKiB = val;
        else if (key == QLatin1String("SwapTotal"))  swapTotalKiB      = val;
        else if (key == QLatin1String("SwapFree"))   swapFreeKiB       = val;
    }
    m_mem.memTotalMiB = memTotalKiB / 1024.0;
    m_mem.memAvailableMiB = memAvailableKiB / 1024.0;
    m_mem.swapTotalMiB = swapTotalKiB / 1024.0;
    m_mem.swapFreeMiB  = swapFreeKiB  / 1024.0;

    m_mem.memAvailablePercent = (m_mem.memTotalMiB > 0) ? (m_mem.memAvailableMiB * 100.0 / m_mem.memTotalMiB) : 0;
    m_mem.swapFreePercent     = (m_mem.swapTotalMiB > 0) ? (m_mem.swapFreeMiB * 100.0 / m_mem.swapTotalMiB) : 0;
}

void SystemSnapshot::readSwaps() {
    QFile f(m_procRoot + QStringLiteral("/swaps"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream ts(&f);
    QString header;
    if (!ts.readLineInto(&header)) {
        return;
    }
    double totalKiB = 0;
    double usedKiB = 0;
    QString line;
    QRegularExpression re("\\s+");
    while (ts.readLineInto(&line)) {
        const QStringList parts = line.split(re, Qt::SkipEmptyParts);
        if (parts.size() >= 5) {
            totalKiB += parts[2].toDouble();
            usedKiB  += parts[3].toDouble();
        }
    }
    if (totalKiB > 0) {
        m_mem.swapTotalMiB = totalKiB / 1024.0;
        const double freeKiB = totalKiB - usedKiB;
        m_mem.swapFreeMiB = freeKiB / 1024.0;
        m_mem.swapFreePercent = (m_mem.swapTotalMiB > 0) ? (m_mem.swapFreeMiB * 100.0 / m_mem.swapTotalMiB) : 0;
    }
}

void SystemSnapshot::readZram() {
    QFile disk(m_sysRoot + QStringLiteral("/block/zram0/disksize"));
    if (!disk.exists()) {
        m_zram = {};
        return;
    }
    // GCOVR_EXCL_START
    m_zram.present = true;

    if (disk.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString s = QString::fromUtf8(disk.readAll()).trimmed();
        const double bytes = s.toDouble();
        m_zram.diskSizeMiB = bytes / (1024.0 * 1024.0);
    }

    QFile mm(m_sysRoot + QStringLiteral("/block/zram0/mm_stat"));
    if (mm.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString s = QString::fromUtf8(mm.readAll()).trimmed();
        const QStringList parts = s.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        // mm_stat: orig_data_size compr_data_size mem_used_total mem_limit mem_used_max zero_pages num_migrated
        if (parts.size() >= 3) {
            const double orig = parts[0].toDouble();
            const double compr = parts[1].toDouble();
            const double memUsed = parts[2].toDouble();
            m_zram.origDataMiB = orig / (1024.0 * 1024.0);
            m_zram.comprDataMiB = compr / (1024.0 * 1024.0);
            m_zram.memUsedTotalMiB = memUsed / (1024.0 * 1024.0);
        }
    }
    m_zram.logicalUsedPercent = (m_zram.diskSizeMiB > 0) ? (m_zram.origDataMiB * 100.0 / m_zram.diskSizeMiB) : 0;
    // GCOVR_EXCL_STOP
}

void SystemSnapshot::readPsi() {
    QFile f(m_procRoot + QStringLiteral("/pressure/memory"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_psi = {};
        return;
    }
    // GCOVR_EXCL_START
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.startsWith(QStringLiteral("some "))) {
            QRegularExpression re(R"(avg10=([0-9\.]+))");
            auto m = re.match(line);
            if (m.hasMatch()) m_psi.some_avg10 = m.captured(1).toDouble();
        } else if (line.startsWith(QStringLiteral("full "))) {
            QRegularExpression re(R"(avg10=([0-9\.]+))");
            auto m = re.match(line);
            if (m.hasMatch()) m_psi.full_avg10 = m.captured(1).toDouble();
        }
    }
    // GCOVR_EXCL_STOP
}
