#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QDate>

class SettingsManager : public QObject {
    Q_OBJECT

public:
    explicit SettingsManager(QObject* parent = nullptr);

    int workDurationSeconds() const;
    void setWorkDurationSeconds(int seconds);

    int breakDurationSeconds() const;
    void setBreakDurationSeconds(int seconds);

    bool audioEnabled() const;
    void setAudioEnabled(bool enabled);

    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool enabled);

    int volume() const;
    void setVolume(int volumePercent);

    bool closeToTray() const;
    void setCloseToTray(bool enabled);

    bool autostart() const;
    void setAutostart(bool enabled);

    bool strictModeEnabled() const;
    void setStrictModeEnabled(bool enabled);

    bool idleDetectionEnabled() const;
    void setIdleDetectionEnabled(bool enabled);

    int idleThresholdSeconds() const;
    void setIdleThresholdSeconds(int seconds);

    // Statistics
    int breaksCompletedToday() const;
    int breaksSkippedToday() const;
    int eyeRestSecondsToday() const;
    void incrementBreaksCompleted(int breakDurationSecs);
    void incrementBreaksSkipped();
    void resetStatsIfNewDay();

signals:
    void settingsChanged();
    void statsUpdated();

private:
    QSettings m_settings;
    void applyAutostart(bool enabled);
};

#endif // SETTINGSMANAGER_H
