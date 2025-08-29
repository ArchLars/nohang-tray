// ===== src/TrayApp.h =====
#pragma once
#include <QObject>
#include <QString>
#include <memory>

class QTimer;
class KStatusNotifierItem;
class NoHangUnit;
class NoHangConfig;
class SystemSnapshot;
class TooltipBuilder;
class ProcessTableAction;
struct ThresholdSet; // from Thresholds.h

// TrayApp wires everything together.
// 1) Discovers the running nohang service and its config path
// 2) Parses thresholds (or defaults) from that path
// 3) Reads live system snapshot, RAM, swap, zram, PSI
// 4) Builds a concise tooltip that shows "configured vs current"
// 5) Shows a shield icon when the daemon is active
class TrayApp : public QObject {
  Q_OBJECT
public:
  explicit TrayApp(QObject *parent = nullptr);
  ~TrayApp(); // out-of-line definition in the .cpp
  void start();

  // Utility method exposed for testing; currently returns the input string
  // unchanged. Retained for compatibility if tooltips require escaping in
  // the future.
  static QString escapePercent(const QString &s);

  // Determine icon name based on current thresholds and system snapshot.
  // This is exposed for testing of severity mapping logic.
  static QString iconNameFor(const NoHangConfig &cfg,
                             const SystemSnapshot &snap);

private slots:
  void tick();           // periodic refresh
  void refreshIcon();    // sets icon based on active state
  void refreshTooltip(); // composes tooltip text from models
  void onConfigMaybeChanged();

private:
  void setupStatusItem();
  void setupTimers();
  void ensureModels();

  std::unique_ptr<NoHangUnit> m_unit;
  std::unique_ptr<NoHangConfig> m_cfg;
  std::unique_ptr<SystemSnapshot> m_snapshot;
  std::unique_ptr<TooltipBuilder> m_tooltip;
  std::unique_ptr<ProcessTableAction> m_procAction;

  std::unique_ptr<KStatusNotifierItem> m_sni;
  QTimer *m_pollTimer{nullptr};
  QTimer *m_cfgWatchTimer{nullptr};

  QString m_configPathCache;
  qint64 m_configMtimeCache{0};

  QString m_lastIcon;
};
