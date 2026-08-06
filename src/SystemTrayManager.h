#ifndef SYSTEMTRAYMANAGER_H
#define SYSTEMTRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "TimerEngine.h"
#include "SettingsManager.h"

class SystemTrayManager : public QObject {
    Q_OBJECT

public:
    explicit SystemTrayManager(TimerEngine* timerEngine, SettingsManager* settings, QObject* parent = nullptr);

    void showNotification(const QString& title, const QString& message, QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);

signals:
    void showDashboardRequested();
    void showSettingsRequested();

private slots:
    void handleStateChanged(TimerEngine::State newState, TimerEngine::State oldState);
    void handleTick(int secondsRemaining, int totalSeconds);
    void handleWorkCompleted();
    void handleBreakCompleted();
    void handleTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void createTrayMenu();
    void updateTrayIcon();

    TimerEngine* m_timerEngine;
    SettingsManager* m_settings;
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;

    QAction* m_actionTogglePlayPause;
    QAction* m_actionSkipBreak;
    QAction* m_actionShowDashboard;
    QAction* m_actionSettings;
    QAction* m_actionQuit;
};

#endif // SYSTEMTRAYMANAGER_H
