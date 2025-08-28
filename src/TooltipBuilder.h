// ===== src/TooltipBuilder.h =====
#pragma once
#include <QObject>
#include <QString>

class NoHangConfig;
class SystemSnapshot;

class TooltipBuilder : public QObject {
    Q_OBJECT
public:
    explicit TooltipBuilder(QObject* parent = nullptr);

    // Compose a multi line tooltip that shows
    // 1) status and config path
    // 2) thresholds with percent and MiB equivalents
    // 3) current values and a short hint about the next action
    QString build(const NoHangConfig& cfg,
                  const SystemSnapshot& snap,
                  bool active,
                  const QString& cfgPath) const;
};
