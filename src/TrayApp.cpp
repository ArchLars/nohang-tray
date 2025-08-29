// ===== src/TrayApp.cpp =====
#include "pch.h"
#include "TrayApp.h"
#include "NoHangUnit.h"
#include "NoHangConfig.h"
#include "SystemSnapshot.h"
#include "TooltipBuilder.h"
#include "ProcessTableAction.h"
#include "Thresholds.h"

#include <QTimer>
#include <QFileInfo>
#include <QAction>
#include <KStatusNotifierItem>

static constexpr int kPollMs = 5000;
static constexpr int kCfgWatchMs = 3000;

TrayApp::~TrayApp() = default;

TrayApp::TrayApp(QObject* parent) : QObject(parent) {}

void TrayApp::start() {
    ensureModels();
    setupStatusItem();
    setupTimers();
    tick();
}

void TrayApp::ensureModels() {
    if (!m_unit)    m_unit    = std::make_unique<NoHangUnit>(this);
    if (!m_cfg)     m_cfg     = std::make_unique<NoHangConfig>(this);
    if (!m_snapshot)m_snapshot= std::make_unique<SystemSnapshot>(this);
    if (!m_tooltip) m_tooltip = std::make_unique<TooltipBuilder>(this);
    if (!m_procAction) m_procAction = std::make_unique<ProcessTableAction>(this);
}

void TrayApp::setupStatusItem() {
    m_sni = std::make_unique<KStatusNotifierItem>(this);
    m_sni->setCategory(KStatusNotifierItem::SystemServices);
    m_sni->setTitle(QStringLiteral("nohang"));
    // Active or passive icon will be set in refreshIcon
    m_sni->setStatus(KStatusNotifierItem::Active);
    if (auto* menu = m_sni->contextMenu()) {
        QAction* act = m_procAction->makeAction(menu, m_unit->resolvedConfigPath());
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
    connect(m_cfgWatchTimer, &QTimer::timeout, this, &TrayApp::onConfigMaybeChanged);
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
    if (active) {
        m_sni->setIconByName(QStringLiteral("security-medium"));
        m_sni->setStatus(KStatusNotifierItem::Active);
        m_sni->setTitle(QStringLiteral("nohang, active"));
    } else {
        m_sni->setIconByName(QStringLiteral("security-low"));
        m_sni->setStatus(KStatusNotifierItem::Passive);
        m_sni->setTitle(QStringLiteral("nohang, inactive"));
    }
}

void TrayApp::refreshTooltip() {
    // Build "configured vs current" text for RAM, swap, zram, PSI
    const QString tipTitle = QStringLiteral("nohang status");
    const QString tipIcon  = QStringLiteral("security-medium");
    const QString tipText  = m_tooltip->build(*m_cfg, *m_snapshot, m_unit->isActive(), m_unit->resolvedConfigPath());

    // KStatusNotifierItem tooltips take icon-name, title, subtitle
    m_sni->setToolTip(tipIcon, tipTitle, tipText);
}

void TrayApp::onConfigMaybeChanged() {
    const QString path = m_unit->configPath();
    if (path.isEmpty()) return;
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
