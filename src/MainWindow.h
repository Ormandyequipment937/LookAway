#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QTabWidget>
#include <QCloseEvent>
#include "TimerEngine.h"
#include "SettingsManager.h"
#include "AudioManager.h"
#include "BreakOverlayWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(TimerEngine* timerEngine, SettingsManager* settings, AudioManager* audioManager, QWidget* parent = nullptr);
    ~MainWindow();

    void showSettingsTab();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateUiForState(TimerEngine::State newState, TimerEngine::State oldState);
    void updateCountdown(int secondsRemaining, int totalSeconds);
    void updateStatsDisplay();
    void saveSettingsFromUi();
    void loadSettingsToUi();
    void applyPreset(int workMins, int breakSecs);

private:
    void setupUi();
    QWidget* createDashboardTab();
    QWidget* createSettingsTab();
    void applyTheme();

    TimerEngine* m_timerEngine;
    SettingsManager* m_settings;
    AudioManager* m_audioManager;
    BreakOverlayWidget* m_breakOverlay;

    QTabWidget* m_tabWidget;
    bool m_isUpdatingUi;

    // Dashboard UI
    QLabel* m_statusBadgeLabel;
    QLabel* m_countdownLabel;
    QProgressBar* m_progressBar;
    QPushButton* m_btnPlayPause;
    QPushButton* m_btnReset;
    QPushButton* m_btnSkipBreak;

    // Stats UI
    QLabel* m_lblStatCompleted;
    QLabel* m_lblStatSkipped;
    QLabel* m_lblStatRestTime;

    // Settings UI
    QSpinBox* m_spinWorkDuration;
    QSpinBox* m_spinBreakDuration;
    QCheckBox* m_chkAudioEnabled;
    QSlider* m_sliderVolume;
    QLabel* m_lblVolumeVal;
    QPushButton* m_btnTestAudio;
    QCheckBox* m_chkNotificationsEnabled;
    QCheckBox* m_chkCloseToTray;
    QCheckBox* m_chkAutostart;
    QCheckBox* m_chkStrictMode;
    QCheckBox* m_chkIdleDetection;
    QSpinBox* m_spinIdleThreshold;
};

#endif // MAINWINDOW_H
