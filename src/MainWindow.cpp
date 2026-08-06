#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QIcon>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(TimerEngine* timerEngine, SettingsManager* settings, AudioManager* audioManager, QWidget* parent)
    : QMainWindow(parent),
      m_timerEngine(timerEngine),
      m_settings(settings),
      m_audioManager(audioManager),
      m_breakOverlay(nullptr),
      m_isUpdatingUi(false) {

    setWindowIcon(QIcon(":/icons/app_icon.svg"));
    setWindowTitle("LookAway - 20-20-20 Eye Care");
    setFixedSize(460, 580);

    m_breakOverlay = new BreakOverlayWidget();
    connect(m_breakOverlay, &BreakOverlayWidget::skipRequested, m_timerEngine, &TimerEngine::skipBreak);

    setupUi();
    applyTheme();

    connect(m_timerEngine, &TimerEngine::stateChanged, this, &MainWindow::updateUiForState);
    connect(m_timerEngine, &TimerEngine::tick, this, &MainWindow::updateCountdown);
    connect(m_settings, &SettingsManager::statsUpdated, this, &MainWindow::updateStatsDisplay);

    loadSettingsToUi();
    updateStatsDisplay();
    updateUiForState(m_timerEngine->state(), TimerEngine::State::Idle);
    updateCountdown(m_timerEngine->secondsRemaining(), m_timerEngine->totalDurationSeconds());
}

MainWindow::~MainWindow() {
    if (m_breakOverlay) {
        delete m_breakOverlay;
        m_breakOverlay = nullptr;
    }
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createDashboardTab(), QIcon(":/icons/app_icon.svg"), "Dashboard");
    m_tabWidget->addTab(createSettingsTab(), "Settings");

    mainLayout->addWidget(m_tabWidget);
}

QWidget* MainWindow::createDashboardTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(14);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(":/icons/app_icon.svg").pixmap(36, 36));
    QLabel* titleLabel = new QLabel("LookAway");
    titleLabel->setObjectName("headerTitle");
    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    // Status Badge
    m_statusBadgeLabel = new QLabel("READY TO WORK");
    m_statusBadgeLabel->setObjectName("statusBadge");
    m_statusBadgeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_statusBadgeLabel);

    // Countdown Display Box
    QFrame* timerCard = new QFrame();
    timerCard->setObjectName("timerCard");
    QVBoxLayout* cardLayout = new QVBoxLayout(timerCard);
    cardLayout->setContentsMargins(16, 18, 16, 18);

    m_countdownLabel = new QLabel("20:00");
    m_countdownLabel->setObjectName("countdownDisplay");
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(m_countdownLabel);

    m_progressBar = new QProgressBar();
    m_progressBar->setObjectName("sessionProgress");
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(100);
    m_progressBar->setTextVisible(false);
    cardLayout->addWidget(m_progressBar);

    layout->addWidget(timerCard);

    // Controls
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_btnPlayPause = new QPushButton("Start");
    m_btnPlayPause->setObjectName("btnPrimary");
    m_btnPlayPause->setFixedHeight(40);

    m_btnReset = new QPushButton("Reset");
    m_btnReset->setObjectName("btnSecondary");
    m_btnReset->setFixedHeight(40);

    m_btnSkipBreak = new QPushButton("Skip Break");
    m_btnSkipBreak->setObjectName("btnSecondary");
    m_btnSkipBreak->setFixedHeight(40);
    m_btnSkipBreak->setEnabled(false);

    btnLayout->addWidget(m_btnPlayPause);
    btnLayout->addWidget(m_btnReset);
    btnLayout->addWidget(m_btnSkipBreak);
    layout->addLayout(btnLayout);

    // Presets Row
    QHBoxLayout* presetLayout = new QHBoxLayout();
    QLabel* lblPreset = new QLabel("Presets:");
    lblPreset->setStyleSheet("font-weight: 600; color: #94a3b8;");
    presetLayout->addWidget(lblPreset);

    QPushButton* btnPreset20 = new QPushButton("20-20-20");
    btnPreset20->setObjectName("btnSmall");
    QPushButton* btnPreset25 = new QPushButton("25-5 Pomo");
    btnPreset25->setObjectName("btnSmall");
    QPushButton* btnPreset50 = new QPushButton("50-10 Work");
    btnPreset50->setObjectName("btnSmall");

    connect(btnPreset20, &QPushButton::clicked, [this]() { applyPreset(20, 20); });
    connect(btnPreset25, &QPushButton::clicked, [this]() { applyPreset(25, 300); });
    connect(btnPreset50, &QPushButton::clicked, [this]() { applyPreset(50, 600); });

    presetLayout->addWidget(btnPreset20);
    presetLayout->addWidget(btnPreset25);
    presetLayout->addWidget(btnPreset50);
    presetLayout->addStretch();
    layout->addLayout(presetLayout);

    // Daily Stats Box
    QGroupBox* statsGroup = new QGroupBox("Daily Eye Care Summary");
    QHBoxLayout* statsLayout = new QHBoxLayout(statsGroup);

    QVBoxLayout* col1 = new QVBoxLayout();
    m_lblStatCompleted = new QLabel("0");
    m_lblStatCompleted->setStyleSheet("font-size: 20px; font-weight: 700; color: #38bdf8;");
    m_lblStatCompleted->setAlignment(Qt::AlignCenter);
    QLabel* cap1 = new QLabel("Breaks Taken");
    cap1->setStyleSheet("font-size: 11px; color: #94a3b8;");
    cap1->setAlignment(Qt::AlignCenter);
    col1->addWidget(m_lblStatCompleted);
    col1->addWidget(cap1);

    QVBoxLayout* col2 = new QVBoxLayout();
    m_lblStatSkipped = new QLabel("0");
    m_lblStatSkipped->setStyleSheet("font-size: 20px; font-weight: 700; color: #f59e0b;");
    m_lblStatSkipped->setAlignment(Qt::AlignCenter);
    QLabel* cap2 = new QLabel("Skipped");
    cap2->setStyleSheet("font-size: 11px; color: #94a3b8;");
    cap2->setAlignment(Qt::AlignCenter);
    col2->addWidget(m_lblStatSkipped);
    col2->addWidget(cap2);

    QVBoxLayout* col3 = new QVBoxLayout();
    m_lblStatRestTime = new QLabel("0m");
    m_lblStatRestTime->setStyleSheet("font-size: 20px; font-weight: 700; color: #10b981;");
    m_lblStatRestTime->setAlignment(Qt::AlignCenter);
    QLabel* cap3 = new QLabel("Rest Time");
    cap3->setStyleSheet("font-size: 11px; color: #94a3b8;");
    cap3->setAlignment(Qt::AlignCenter);
    col3->addWidget(m_lblStatRestTime);
    col3->addWidget(cap3);

    statsLayout->addLayout(col1);
    statsLayout->addLayout(col2);
    statsLayout->addLayout(col3);
    layout->addWidget(statsGroup);

    layout->addStretch();

    // Connect Action Buttons
    connect(m_btnPlayPause, &QPushButton::clicked, [this]() {
        if (m_timerEngine->state() == TimerEngine::State::Working || m_timerEngine->state() == TimerEngine::State::Breaking) {
            m_timerEngine->pause();
        } else {
            m_timerEngine->start();
        }
    });

    connect(m_btnReset, &QPushButton::clicked, [this]() {
        m_timerEngine->stop();
    });

    connect(m_btnSkipBreak, &QPushButton::clicked, [this]() {
        m_timerEngine->skipBreak();
    });

    return tab;
}

QWidget* MainWindow::createSettingsTab() {
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(16, 16, 16, 16);

    QGroupBox* timerGroup = new QGroupBox("Timer Durations");
    QFormLayout* timerForm = new QFormLayout(timerGroup);

    m_spinWorkDuration = new QSpinBox();
    m_spinWorkDuration->setRange(1, 120);
    m_spinWorkDuration->setSuffix(" minutes");

    m_spinBreakDuration = new QSpinBox();
    m_spinBreakDuration->setRange(5, 300);
    m_spinBreakDuration->setSuffix(" seconds");

    timerForm->addRow("Work Interval:", m_spinWorkDuration);
    timerForm->addRow("Break Duration:", m_spinBreakDuration);
    layout->addWidget(timerGroup);

    QGroupBox* audioGroup = new QGroupBox("Audio & Notifications");
    QVBoxLayout* audioLayout = new QVBoxLayout(audioGroup);

    m_chkAudioEnabled = new QCheckBox("Play sound on interval finish");
    audioLayout->addWidget(m_chkAudioEnabled);

    QHBoxLayout* volLayout = new QHBoxLayout();
    volLayout->addWidget(new QLabel("Volume:"));
    m_sliderVolume = new QSlider(Qt::Horizontal);
    m_sliderVolume->setRange(0, 100);
    m_lblVolumeVal = new QLabel("80%");
    m_btnTestAudio = new QPushButton("Test");
    m_btnTestAudio->setObjectName("btnSmall");

    volLayout->addWidget(m_sliderVolume);
    volLayout->addWidget(m_lblVolumeVal);
    volLayout->addWidget(m_btnTestAudio);
    audioLayout->addLayout(volLayout);

    m_chkNotificationsEnabled = new QCheckBox("Show system popups (tray notifications)");
    audioLayout->addWidget(m_chkNotificationsEnabled);

    m_chkStrictMode = new QCheckBox("Show full-screen break overlay window");
    audioLayout->addWidget(m_chkStrictMode);

    layout->addWidget(audioGroup);

    QGroupBox* appGroup = new QGroupBox("Application Preferences");
    QVBoxLayout* appLayout = new QVBoxLayout(appGroup);

    m_chkCloseToTray = new QCheckBox("Minimize to system tray on close");
    m_chkAutostart = new QCheckBox("Launch automatically at system startup");
    m_chkIdleDetection = new QCheckBox("Auto-pause timer when system is idle");

    QHBoxLayout* idleLayout = new QHBoxLayout();
    idleLayout->addWidget(new QLabel("   Idle threshold:"));
    m_spinIdleThreshold = new QSpinBox();
    m_spinIdleThreshold->setRange(1, 30);
    m_spinIdleThreshold->setSuffix(" minutes");
    idleLayout->addWidget(m_spinIdleThreshold);
    idleLayout->addStretch();

    appLayout->addWidget(m_chkCloseToTray);
    appLayout->addWidget(m_chkAutostart);
    appLayout->addWidget(m_chkIdleDetection);
    appLayout->addLayout(idleLayout);
    layout->addWidget(appGroup);

    layout->addStretch();

    // Connect Settings UI controls to save
    auto saveLambda = [this]() {
        if (!m_isUpdatingUi) {
            saveSettingsFromUi();
        }
    };

    connect(m_spinWorkDuration, QOverload<int>::of(&QSpinBox::valueChanged), saveLambda);
    connect(m_spinBreakDuration, QOverload<int>::of(&QSpinBox::valueChanged), saveLambda);
    connect(m_chkAudioEnabled, &QCheckBox::toggled, saveLambda);
    connect(m_chkNotificationsEnabled, &QCheckBox::toggled, saveLambda);
    connect(m_chkStrictMode, &QCheckBox::toggled, saveLambda);
    connect(m_chkCloseToTray, &QCheckBox::toggled, saveLambda);
    connect(m_chkAutostart, &QCheckBox::toggled, saveLambda);
    connect(m_chkIdleDetection, &QCheckBox::toggled, [this, saveLambda](bool checked) {
        m_spinIdleThreshold->setEnabled(checked);
        saveLambda();
    });
    connect(m_spinIdleThreshold, QOverload<int>::of(&QSpinBox::valueChanged), saveLambda);

    connect(m_sliderVolume, &QSlider::valueChanged, [this, saveLambda](int val) {
        m_lblVolumeVal->setText(QString("%1%").arg(val));
        saveLambda();
    });

    connect(m_btnTestAudio, &QPushButton::clicked, [this]() {
        m_audioManager->playTestChime();
    });

    return tab;
}

void MainWindow::loadSettingsToUi() {
    m_isUpdatingUi = true;
    m_spinWorkDuration->setValue(m_settings->workDurationSeconds() / 60);
    m_spinBreakDuration->setValue(m_settings->breakDurationSeconds());
    m_chkAudioEnabled->setChecked(m_settings->audioEnabled());
    m_sliderVolume->setValue(m_settings->volume());
    m_lblVolumeVal->setText(QString("%1%").arg(m_settings->volume()));
    m_chkNotificationsEnabled->setChecked(m_settings->notificationsEnabled());
    m_chkStrictMode->setChecked(m_settings->strictModeEnabled());
    m_chkCloseToTray->setChecked(m_settings->closeToTray());
    m_chkAutostart->setChecked(m_settings->autostart());
    m_chkIdleDetection->setChecked(m_settings->idleDetectionEnabled());
    m_spinIdleThreshold->setValue(m_settings->idleThresholdSeconds() / 60);
    m_spinIdleThreshold->setEnabled(m_settings->idleDetectionEnabled());
    m_isUpdatingUi = false;
}

void MainWindow::saveSettingsFromUi() {
    m_settings->setWorkDurationSeconds(m_spinWorkDuration->value() * 60);
    m_settings->setBreakDurationSeconds(m_spinBreakDuration->value());
    m_settings->setAudioEnabled(m_chkAudioEnabled->isChecked());
    m_settings->setVolume(m_sliderVolume->value());
    m_settings->setNotificationsEnabled(m_chkNotificationsEnabled->isChecked());
    m_settings->setStrictModeEnabled(m_chkStrictMode->isChecked());
    m_settings->setCloseToTray(m_chkCloseToTray->isChecked());
    m_settings->setAutostart(m_chkAutostart->isChecked());
    m_settings->setIdleDetectionEnabled(m_chkIdleDetection->isChecked());
    m_settings->setIdleThresholdSeconds(m_spinIdleThreshold->value() * 60);
}

void MainWindow::applyPreset(int workMins, int breakSecs) {
    m_spinWorkDuration->setValue(workMins);
    m_spinBreakDuration->setValue(breakSecs);
    saveSettingsFromUi();
    if (m_timerEngine->state() == TimerEngine::State::Idle) {
        m_timerEngine->stop();
    }
}

void MainWindow::updateStatsDisplay() {
    m_lblStatCompleted->setText(QString::number(m_settings->breaksCompletedToday()));
    m_lblStatSkipped->setText(QString::number(m_settings->breaksSkippedToday()));
    double restMins = static_cast<double>(m_settings->eyeRestSecondsToday()) / 60.0;
    m_lblStatRestTime->setText(QString("%1m").arg(restMins, 0, 'f', 1));
}

void MainWindow::showSettingsTab() {
    show();
    raise();
    activateWindow();
    m_tabWidget->setCurrentIndex(1);
}

void MainWindow::updateUiForState(TimerEngine::State newState, TimerEngine::State oldState) {
    Q_UNUSED(oldState);
    switch (newState) {
    case TimerEngine::State::Working:
        m_statusBadgeLabel->setText("WORKING SESSION");
        m_statusBadgeLabel->setStyleSheet("background-color: #0284c7; color: #ffffff;");
        m_btnPlayPause->setText("Pause");
        m_btnSkipBreak->setEnabled(false);
        if (m_breakOverlay) m_breakOverlay->hide();
        break;

    case TimerEngine::State::Breaking:
        m_statusBadgeLabel->setText("LOOK 20 FEET AWAY! 👁️");
        m_statusBadgeLabel->setStyleSheet("background-color: #d97706; color: #ffffff;");
        m_btnPlayPause->setText("Pause");
        m_btnSkipBreak->setEnabled(true);
        if (m_settings->strictModeEnabled() && m_breakOverlay) {
            m_breakOverlay->showFullScreen();
            m_breakOverlay->raise();
            m_breakOverlay->activateWindow();
        }
        break;

    case TimerEngine::State::Paused:
        if (m_timerEngine->isPausedForIdle()) {
            m_statusBadgeLabel->setText("PAUSED (SYSTEM IDLE)");
            m_statusBadgeLabel->setStyleSheet("background-color: #64748b; color: #ffffff;");
        } else {
            m_statusBadgeLabel->setText("PAUSED");
            m_statusBadgeLabel->setStyleSheet("background-color: #475569; color: #ffffff;");
        }
        m_btnPlayPause->setText("Resume");
        m_btnSkipBreak->setEnabled(m_timerEngine->secondsRemaining() == m_settings->breakDurationSeconds() || oldState == TimerEngine::State::Breaking);
        if (m_breakOverlay) m_breakOverlay->hide();
        break;

    case TimerEngine::State::Idle:
    default:
        m_statusBadgeLabel->setText("READY TO WORK");
        m_statusBadgeLabel->setStyleSheet("background-color: #334155; color: #94a3b8;");
        m_btnPlayPause->setText("Start");
        m_btnSkipBreak->setEnabled(false);
        if (m_breakOverlay) m_breakOverlay->hide();
        break;
    }
}

void MainWindow::updateCountdown(int secondsRemaining, int totalSeconds) {
    int mins = secondsRemaining / 60;
    int secs = secondsRemaining % 60;
    m_countdownLabel->setText(QString("%1:%2")
                                   .arg(mins, 2, 10, QChar('0'))
                                   .arg(secs, 2, 10, QChar('0')));

    if (totalSeconds > 0) {
        int pct = static_cast<int>((static_cast<double>(secondsRemaining) / totalSeconds) * 100.0);
        m_progressBar->setValue(pct);
    } else {
        m_progressBar->setValue(100);
    }

    if (m_breakOverlay && m_timerEngine->state() == TimerEngine::State::Breaking) {
        m_breakOverlay->updateCountdown(secondsRemaining, totalSeconds);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_settings->closeToTray()) {
        event->ignore();
        hide();
    } else {
        event->accept();
        QApplication::quit();
    }
}

void MainWindow::applyTheme() {
    QString qss = R"(
        QMainWindow {
            background-color: #0f172a;
        }
        QWidget {
            color: #f8fafc;
            font-family: 'Segoe UI', system-ui, sans-serif;
            font-size: 13px;
        }
        QTabWidget::pane {
            border: 1px solid #334155;
            background-color: #1e293b;
            border-radius: 8px;
        }
        QTabBar::tab {
            background-color: #0f172a;
            color: #94a3b8;
            padding: 8px 16px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            font-weight: 600;
        }
        QTabBar::tab:selected {
            background-color: #1e293b;
            color: #38bdf8;
            border-bottom: 2px solid #38bdf8;
        }
        #headerTitle {
            font-size: 20px;
            font-weight: 700;
            color: #f8fafc;
        }
        #statusBadge {
            border-radius: 6px;
            padding: 6px 12px;
            font-weight: 700;
            letter-spacing: 1px;
            font-size: 12px;
        }
        #timerCard {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 12px;
        }
        #countdownDisplay {
            font-size: 50px;
            font-weight: 700;
            color: #38bdf8;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        QProgressBar#sessionProgress {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 4px;
            height: 8px;
        }
        QProgressBar#sessionProgress::chunk {
            background-color: #38bdf8;
            border-radius: 3px;
        }
        QPushButton#btnPrimary {
            background-color: #0284c7;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            font-weight: 700;
            font-size: 14px;
        }
        QPushButton#btnPrimary:hover {
            background-color: #0369a1;
        }
        QPushButton#btnPrimary:pressed {
            background-color: #075985;
        }
        QPushButton#btnSecondary {
            background-color: #334155;
            color: #f8fafc;
            border: none;
            border-radius: 6px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton#btnSecondary:hover {
            background-color: #475569;
        }
        QPushButton#btnSmall {
            background-color: #334155;
            color: #38bdf8;
            border: 1px solid #475569;
            border-radius: 4px;
            padding: 4px 10px;
            font-weight: 600;
        }
        QPushButton#btnSmall:hover {
            background-color: #0284c7;
            color: #ffffff;
        }
        QGroupBox {
            font-weight: 700;
            border: 1px solid #334155;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
            color: #38bdf8;
        }
        QSpinBox, QSlider {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 4px;
            padding: 4px;
            color: #f8fafc;
        }
        QCheckBox {
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 3px;
            border: 1px solid #475569;
            background-color: #0f172a;
        }
        QCheckBox::indicator:checked {
            background-color: #0284c7;
            border-color: #38bdf8;
        }
    )";
    setStyleSheet(qss);
}
