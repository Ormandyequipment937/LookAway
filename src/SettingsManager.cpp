#include "SettingsManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QDate>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
      m_settings("LookAway", "LookAwayApp") {
    resetStatsIfNewDay();
}

int SettingsManager::workDurationSeconds() const {
    return m_settings.value("timer/workDuration", 1200).toInt(); // 20 mins default
}

void SettingsManager::setWorkDurationSeconds(int seconds) {
    if (seconds <= 0) seconds = 1200;
    m_settings.setValue("timer/workDuration", seconds);
    emit settingsChanged();
}

int SettingsManager::breakDurationSeconds() const {
    return m_settings.value("timer/breakDuration", 20).toInt(); // 20 secs default
}

void SettingsManager::setBreakDurationSeconds(int seconds) {
    if (seconds <= 0) seconds = 20;
    m_settings.setValue("timer/breakDuration", seconds);
    emit settingsChanged();
}

bool SettingsManager::audioEnabled() const {
    return m_settings.value("audio/enabled", true).toBool();
}

void SettingsManager::setAudioEnabled(bool enabled) {
    m_settings.setValue("audio/enabled", enabled);
    emit settingsChanged();
}

bool SettingsManager::notificationsEnabled() const {
    return m_settings.value("notifications/enabled", true).toBool();
}

void SettingsManager::setNotificationsEnabled(bool enabled) {
    m_settings.setValue("notifications/enabled", enabled);
    emit settingsChanged();
}

int SettingsManager::volume() const {
    return m_settings.value("audio/volume", 80).toInt();
}

void SettingsManager::setVolume(int volumePercent) {
    if (volumePercent < 0) volumePercent = 0;
    if (volumePercent > 100) volumePercent = 100;
    m_settings.setValue("audio/volume", volumePercent);
    emit settingsChanged();
}

bool SettingsManager::closeToTray() const {
    return m_settings.value("ui/closeToTray", true).toBool(); // Default true
}

void SettingsManager::setCloseToTray(bool enabled) {
    m_settings.setValue("ui/closeToTray", enabled);
    emit settingsChanged();
}

bool SettingsManager::autostart() const {
    return m_settings.value("system/autostart", false).toBool();
}

void SettingsManager::setAutostart(bool enabled) {
    m_settings.setValue("system/autostart", enabled);
    applyAutostart(enabled);
    emit settingsChanged();
}

bool SettingsManager::strictModeEnabled() const {
    return m_settings.value("ui/strictMode", true).toBool();
}

void SettingsManager::setStrictModeEnabled(bool enabled) {
    m_settings.setValue("ui/strictMode", enabled);
    emit settingsChanged();
}

bool SettingsManager::idleDetectionEnabled() const {
    return m_settings.value("system/idleDetection", true).toBool();
}

void SettingsManager::setIdleDetectionEnabled(bool enabled) {
    m_settings.setValue("system/idleDetection", enabled);
    emit settingsChanged();
}

int SettingsManager::idleThresholdSeconds() const {
    return m_settings.value("system/idleThreshold", 180).toInt(); // 3 mins default
}

void SettingsManager::setIdleThresholdSeconds(int seconds) {
    if (seconds < 30) seconds = 30;
    m_settings.setValue("system/idleThreshold", seconds);
    emit settingsChanged();
}

int SettingsManager::breaksCompletedToday() const {
    return m_settings.value("stats/completedToday", 0).toInt();
}

int SettingsManager::breaksSkippedToday() const {
    return m_settings.value("stats/skippedToday", 0).toInt();
}

int SettingsManager::eyeRestSecondsToday() const {
    return m_settings.value("stats/eyeRestSecondsToday", 0).toInt();
}

void SettingsManager::incrementBreaksCompleted(int breakDurationSecs) {
    resetStatsIfNewDay();
    int completed = breaksCompletedToday() + 1;
    int restSecs = eyeRestSecondsToday() + breakDurationSecs;
    m_settings.setValue("stats/completedToday", completed);
    m_settings.setValue("stats/eyeRestSecondsToday", restSecs);
    emit statsUpdated();
}

void SettingsManager::incrementBreaksSkipped() {
    resetStatsIfNewDay();
    int skipped = breaksSkippedToday() + 1;
    m_settings.setValue("stats/skippedToday", skipped);
    emit statsUpdated();
}

void SettingsManager::resetStatsIfNewDay() {
    QString todayStr = QDate::currentDate().toString(Qt::ISODate);
    QString lastReset = m_settings.value("stats/lastResetDate", "").toString();
    if (lastReset != todayStr) {
        m_settings.setValue("stats/lastResetDate", todayStr);
        m_settings.setValue("stats/completedToday", 0);
        m_settings.setValue("stats/skippedToday", 0);
        m_settings.setValue("stats/eyeRestSecondsToday", 0);
        emit statsUpdated();
    }
}

void SettingsManager::applyAutostart(bool enabled) {
#ifdef Q_OS_WIN
    QSettings autoRunSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    if (enabled) {
        autoRunSettings.setValue("LookAway", "\"" + appPath + "\" --minimized");
    } else {
        autoRunSettings.remove("LookAway");
    }
#endif
}
