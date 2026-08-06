#include <QApplication>
#include <QCommandLineParser>
#include "SettingsManager.h"
#include "TimerEngine.h"
#include "AudioManager.h"
#include "SystemTrayManager.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("LookAway");
    QApplication::setOrganizationName("LookAway");
    QApplication::setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription("LookAway - 20-20-20 Eye Care Background Utility");
    parser.addHelpOption();

    QCommandLineOption startMinimizedOption("minimized", "Start application hidden in system tray.");
    parser.addOption(startMinimizedOption);
    parser.process(app);

    SettingsManager settings;
    TimerEngine timerEngine(&settings);
    AudioManager audioManager(&settings, &timerEngine);
    SystemTrayManager trayManager(&timerEngine, &settings);
    MainWindow mainWindow(&timerEngine, &settings, &audioManager);

    // Auto-start timer on launch by default 
    timerEngine.start();

    QObject::connect(&trayManager, &SystemTrayManager::showDashboardRequested, [&mainWindow]() {
        mainWindow.show();
        mainWindow.raise();
        mainWindow.activateWindow();
    });

    QObject::connect(&trayManager, &SystemTrayManager::showSettingsRequested, [&mainWindow]() {
        mainWindow.showSettingsTab();
    });

    if (!parser.isSet(startMinimizedOption)) {
        mainWindow.show();
    }

    return app.exec();
}
