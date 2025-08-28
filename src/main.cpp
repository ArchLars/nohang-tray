// ===== src/main.cpp =====
#include <QApplication>
#include "TrayApp.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nohang-tray"));
    app.setOrganizationName(QStringLiteral("ArchLars"));
    app.setOrganizationDomain(QStringLiteral("github.com/ArchLars"));

    TrayApp tray;
    tray.start(); // sets up the SNI, timers, and first refresh

    return app.exec();
}
