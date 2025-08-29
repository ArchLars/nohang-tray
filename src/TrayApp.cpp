// ===== src/TrayApp.cpp =====
#include "TrayApp.h"
#include "NoHangConfig.h"
#include "NoHangUnit.h"
#include "ProcessTableAction.h"
#include "SystemSnapshot.h"
#include "Thresholds.h"
#include "TooltipBuilder.h"
#include "pch.h"

#include <KStatusNotifierItem>
#include <QAction>
#include <QFileInfo>
#include <QTimer>

static constexpr int kPollMs = 5000;
static constexpr int kCfgWatchMs = 3000;

TrayApp::~TrayApp() = default;

TrayApp::TrayApp(QObject *parent) : QObject(parent) {}

QString TrayApp::escapePercent(const QString &s) { return s; }

static bool below(double current, const std::optional<double> &threshold) {
  return threshold.has_value() && current < *threshold;
}

static bool above(double current, const std::optional<double> &threshold) {
  return threshold.has_value() && current > *threshold;
}

QString TrayApp::iconNameFor(const NoHangConfig &cfg,
                             const SystemSnapshot &snap) {
  const ThresholdSet th = Thresholds::compute(cfg.thresholds(), snap);

  // Choose PSI metric if thresholds configure one; default to full_avg10
  double psiVal = snap.psi().full_avg10;
  if (th.psi_metrics == QStringLiteral("some"))
    psiVal = snap.psi().some_avg10;
  else if (th.psi_metrics == QStringLiteral("full"))
    psiVal = snap.psi().full_avg10;

  const bool critical =
      below(snap.mem().memAvailableMiB, th.hard_mem_free.mib) ||
      below(snap.mem().swapFreeMiB, th.hard_swap_free.mib) ||
      above(snap.zram().origDataMiB, th.hard_zram_used.mib) ||
      (th.hard_psi && psiVal > *th.hard_psi);

  if (critical)
    return QStringLiteral("security-high");

  const bool warning =
      below(snap.mem().memAvailableMiB, th.soft_mem_free.mib) ||
      below(snap.mem().memAvailableMiB, th.warn_mem_free.mib) ||
      below(snap.mem().swapFreeMiB, th.soft_swap_free.mib) ||
      below(snap.mem().swapFreeMiB, th.warn_swap_free.mib) ||
      above(snap.zram().origDataMiB, th.soft_zram_used.mib) ||
      above(snap.zram().origDataMiB, th.warn_zram_used.mib) ||
      (th.soft_psi && psiVal > *th.soft_psi) ||
      (th.warn_psi && psiVal > *th.warn_psi);

  if (warning)
    return QStringLiteral("security-medium");

  return QStringLiteral("security-low");
}

void TrayApp::start() {
  ensureModels();
  setupStatusItem();
  setupTimers();
  tick();
}

void TrayApp::ensureModels() {
  if (!m_unit)
    m_unit = std::make_unique<NoHangUnit>(this);
  if (!m_cfg)
    m_cfg = std::make_unique<NoHangConfig>(this);
  if (!m_snapshot)
    m_snapshot = std::make_unique<SystemSnapshot>(this);
  if (!m_tooltip)
    m_tooltip = std::make_unique<TooltipBuilder>(this);
  if (!m_procAction)
    m_procAction = std::make_unique<ProcessTableAction>(this);
}

void TrayApp::setupStatusItem() {
  m_sni = std::make_unique<KStatusNotifierItem>(this);
  m_sni->setCategory(KStatusNotifierItem::SystemServices);
  m_sni->setTitle(QStringLiteral("nohang"));
  // Active or passive icon will be set in refreshIcon
  m_sni->setStatus(KStatusNotifierItem::Active);
  if (auto *menu = m_sni->contextMenu()) {
    QAction *act = m_procAction->makeAction(menu, m_unit->resolvedConfigPath());
    menu->addAction(act);
  }
}

void TrayApp::setupTimers() {
  m_pollTimer = new QTimer(this);
  m_pollTimer->setInterval(kPollMs);
  connect(m_pollTimer, &QTimer::timeout, this, &TrayApp::tick);
  m_pollTimer->start();

  m_cfgWatchTimer = new QTimer(this);
  m_cfgWatchTimer->setInterval(kCfgWatchMs);
  connect(m_cfgWatchTimer, &QTimer::timeout, this,
          &TrayApp::onConfigMaybeChanged);
  m_cfgWatchTimer->start();
}

void TrayApp::tick() {
  // Detect running unit and config path
  const bool active = m_unit->isActive();
  const QString cfgPath = m_unit->configPath();
  if (cfgPath != m_configPathCache) {
    m_configPathCache = cfgPath;
    m_configMtimeCache = 0; // force re-parse
  }

  // Parse thresholds from the current config, or defaults
  m_cfg->ensureParsed(cfgPath);

  // Read live system data
  m_snapshot->refresh();

  // Update UI
  refreshIcon();
  refreshTooltip();
}

void TrayApp::refreshIcon() {
  const bool active = m_unit->isActive();
  const QString icon = active ? iconNameFor(*m_cfg, *m_snapshot)
                              : QStringLiteral("security-low");
  m_sni->setIconByName(icon);
  m_sni->setStatus(active ? KStatusNotifierItem::Active
                          : KStatusNotifierItem::Passive);
  m_sni->setTitle(active ? QStringLiteral("nohang, active")
                         : QStringLiteral("nohang, inactive"));
}

void TrayApp::refreshTooltip() {
  // Build "configured vs current" text for RAM, swap, zram, PSI
  const QString tipTitle = QStringLiteral("nohang status");
  const QString tipIcon = QStringLiteral("security-medium");
  const QString tipText = m_tooltip->build(
      *m_cfg, *m_snapshot, m_unit->isActive(), m_unit->resolvedConfigPath());

  // KStatusNotifierItem tooltips take icon-name, title, subtitle
  m_sni->setToolTip(tipIcon, tipTitle, tipText);
}

void TrayApp::onConfigMaybeChanged() {
  const QString path = m_unit->configPath();
  if (path.isEmpty())
    return;
  QFileInfo fi(path);
  if (fi.exists()) {
    const qint64 mt = fi.lastModified().toSecsSinceEpoch();
    if (mt != m_configMtimeCache) {
      m_configMtimeCache = mt;
      m_cfg->ensureParsed(path);
      refreshTooltip();
    }
  }
}
