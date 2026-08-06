#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QIcon>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QIntValidator>

MainWindow::MainWindow(TimerEngine* timerEngine, SettingsManager* settings, AudioManager* audioManager, QWidget* parent)
    : QMainWindow(parent),
      m_timerEngine(timerEngine),
      m_settings(settings),
      m_audioManager(audioManager),
      m_isUpdatingUi(false) {

    setWindowIcon(QIcon(":/icons/app_icon.svg"));
    setWindowTitle("LookAway - 20-20-20 Eye Care");
    setFixedSize(460, 580);

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
    qDeleteAll(m_breakOverlays);
    m_breakOverlays.clear();
}

int MainWindow::durationToSeconds(QComboBox* valCombo, QComboBox* unitCombo) const {
    int val = valCombo->currentText().toInt();
    if (val <= 0) val = 1;
    QString unit = unitCombo->currentText();
    if (unit == "Hours") {
        return val * 3600;
    } else if (unit == "Minutes") {
        return val * 60;
    }
    return val; // Seconds
}

void MainWindow::secondsToUi(int totalSeconds, QComboBox* valCombo, QComboBox* unitCombo) {
    if (totalSeconds <= 0) {
        valCombo->setCurrentText("1");
        unitCombo->setCurrentText("Seconds");
        return;
    }

    if (totalSeconds % 3600 == 0) {
        valCombo->setCurrentText(QString::number(totalSeconds / 3600));
        unitCombo->setCurrentText("Hours");
    } else if (totalSeconds % 60 == 0) {
        valCombo->setCurrentText(QString::number(totalSeconds / 60));
        unitCombo->setCurrentText("Minutes");
    } else {
        valCombo->setCurrentText(QString::number(totalSeconds));
        unitCombo->setCurrentText("Seconds");
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

    // Work Interval controls
    QHBoxLayout* workLayout = new QHBoxLayout();
    m_comboWorkVal = new QComboBox();
    m_comboWorkVal->setEditable(true);
    m_comboWorkVal->setValidator(new QIntValidator(1, 9999, m_comboWorkVal));
    m_comboWorkVal->addItems({"5", "10", "15", "20", "25", "30", "45", "50", "60"});
    m_comboWorkUnit = new QComboBox();
    m_comboWorkUnit->addItems({"Seconds", "Minutes", "Hours"});
    m_comboWorkUnit->setFixedWidth(110);
    workLayout->addWidget(m_comboWorkVal, 1);
    workLayout->addWidget(m_comboWorkUnit);

    // Break Duration controls
    QHBoxLayout* breakLayout = new QHBoxLayout();
    m_comboBreakVal = new QComboBox();
    m_comboBreakVal->setEditable(true);
    m_comboBreakVal->setValidator(new QIntValidator(1, 9999, m_comboBreakVal));
    m_comboBreakVal->addItems({"5", "10", "15", "20", "25", "30", "45", "50", "60"});
    m_comboBreakUnit = new QComboBox();
    m_comboBreakUnit->addItems({"Seconds", "Minutes", "Hours"});
    m_comboBreakUnit->setFixedWidth(110);
    breakLayout->addWidget(m_comboBreakVal, 1);
    breakLayout->addWidget(m_comboBreakUnit);

    timerForm->addRow("Work Interval:", workLayout);
    timerForm->addRow("Break Duration:", breakLayout);
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
    m_comboIdleVal = new QComboBox();
    m_comboIdleVal->setEditable(true);
    m_comboIdleVal->setValidator(new QIntValidator(1, 9999, m_comboIdleVal));
    m_comboIdleVal->addItems({"1", "2", "3", "5", "10", "15"});
    m_comboIdleVal->setFixedWidth(80);
    m_comboIdleUnit = new QComboBox();
    m_comboIdleUnit->addItems({"Seconds", "Minutes", "Hours"});
    m_comboIdleUnit->setFixedWidth(110);
    idleLayout->addWidget(m_comboIdleVal);
    idleLayout->addWidget(m_comboIdleUnit);
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

    connect(m_comboWorkVal, &QComboBox::currentTextChanged, saveLambda);
    connect(m_comboWorkUnit, &QComboBox::currentTextChanged, saveLambda);
    connect(m_comboBreakVal, &QComboBox::currentTextChanged, saveLambda);
    connect(m_comboBreakUnit, &QComboBox::currentTextChanged, saveLambda);
    connect(m_chkAudioEnabled, &QCheckBox::toggled, saveLambda);
    connect(m_chkNotificationsEnabled, &QCheckBox::toggled, saveLambda);
    connect(m_chkStrictMode, &QCheckBox::toggled, saveLambda);
    connect(m_chkCloseToTray, &QCheckBox::toggled, saveLambda);
    connect(m_chkAutostart, &QCheckBox::toggled, saveLambda);
    connect(m_chkIdleDetection, &QCheckBox::toggled, [this, saveLambda](bool checked) {
        m_comboIdleVal->setEnabled(checked);
        m_comboIdleUnit->setEnabled(checked);
        saveLambda();
    });
    connect(m_comboIdleVal, &QComboBox::currentTextChanged, saveLambda);
    connect(m_comboIdleUnit, &QComboBox::currentTextChanged, saveLambda);

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
    secondsToUi(m_settings->workDurationSeconds(), m_comboWorkVal, m_comboWorkUnit);
    secondsToUi(m_settings->breakDurationSeconds(), m_comboBreakVal, m_comboBreakUnit);
    m_chkAudioEnabled->setChecked(m_settings->audioEnabled());
    m_sliderVolume->setValue(m_settings->volume());
    m_lblVolumeVal->setText(QString("%1%").arg(m_settings->volume()));
    m_chkNotificationsEnabled->setChecked(m_settings->notificationsEnabled());
    m_chkStrictMode->setChecked(m_settings->strictModeEnabled());
    m_chkCloseToTray->setChecked(m_settings->closeToTray());
    m_chkAutostart->setChecked(m_settings->autostart());
    m_chkIdleDetection->setChecked(m_settings->idleDetectionEnabled());
    secondsToUi(m_settings->idleThresholdSeconds(), m_comboIdleVal, m_comboIdleUnit);
    m_comboIdleVal->setEnabled(m_settings->idleDetectionEnabled());
    m_comboIdleUnit->setEnabled(m_settings->idleDetectionEnabled());
    m_isUpdatingUi = false;
}

void MainWindow::saveSettingsFromUi() {
    m_settings->setWorkDurationSeconds(durationToSeconds(m_comboWorkVal, m_comboWorkUnit));
    m_settings->setBreakDurationSeconds(durationToSeconds(m_comboBreakVal, m_comboBreakUnit));
    m_settings->setAudioEnabled(m_chkAudioEnabled->isChecked());
    m_settings->setVolume(m_sliderVolume->value());
    m_settings->setNotificationsEnabled(m_chkNotificationsEnabled->isChecked());
    m_settings->setStrictModeEnabled(m_chkStrictMode->isChecked());
    m_settings->setCloseToTray(m_chkCloseToTray->isChecked());
    m_settings->setAutostart(m_chkAutostart->isChecked());
    m_settings->setIdleDetectionEnabled(m_chkIdleDetection->isChecked());
    m_settings->setIdleThresholdSeconds(durationToSeconds(m_comboIdleVal, m_comboIdleUnit));
}

void MainWindow::applyPreset(int workMins, int breakSecs) {
    secondsToUi(workMins * 60, m_comboWorkVal, m_comboWorkUnit);
    secondsToUi(breakSecs, m_comboBreakVal, m_comboBreakUnit);
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
        qDeleteAll(m_breakOverlays);
        m_breakOverlays.clear();
        break;
 
    case TimerEngine::State::Breaking:
        m_statusBadgeLabel->setText("LOOK 20 FEET AWAY! 👁️");
        m_statusBadgeLabel->setStyleSheet("background-color: #d97706; color: #ffffff;");
        m_btnPlayPause->setText("Pause");
        m_btnSkipBreak->setEnabled(true);
        qDeleteAll(m_breakOverlays);
        m_breakOverlays.clear();
        if (m_settings->strictModeEnabled()) {
            const QList<QScreen*> screens = QGuiApplication::screens();
            for (QScreen* screen : screens) {
                BreakOverlayWidget* overlay = new BreakOverlayWidget();
                m_breakOverlays.append(overlay);
                connect(overlay, &BreakOverlayWidget::skipRequested, m_timerEngine, &TimerEngine::skipBreak);
                overlay->setGeometry(screen->geometry());
                overlay->showFullScreen();
                overlay->raise();
                overlay->activateWindow();
            }
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
        qDeleteAll(m_breakOverlays);
        m_breakOverlays.clear();
        break;
 
    case TimerEngine::State::Idle:
    default:
        m_statusBadgeLabel->setText("READY TO WORK");
        m_statusBadgeLabel->setStyleSheet("background-color: #334155; color: #94a3b8;");
        m_btnPlayPause->setText("Start");
        m_btnSkipBreak->setEnabled(false);
        qDeleteAll(m_breakOverlays);
        m_breakOverlays.clear();
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

    if (m_timerEngine->state() == TimerEngine::State::Breaking) {
        for (BreakOverlayWidget* overlay : m_breakOverlays) {
            overlay->updateCountdown(secondsRemaining, totalSeconds);
        }
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
        QSpinBox, QSlider, QComboBox {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 4px;
            padding: 4px;
            color: #f8fafc;
        }
        QComboBox QAbstractItemView {
            background-color: #0f172a;
            color: #f8fafc;
            border: 1px solid #334155;
            selection-background-color: #0284c7;
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
