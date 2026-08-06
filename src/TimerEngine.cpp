#include "TimerEngine.h"
#include <QTime>

#ifdef Q_OS_WIN
#include <windows.h>

static qint64 getSystemIdleTimeMs() {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);
    if (GetLastInputInfo(&lii)) {
        DWORD tickCount = GetTickCount();
        return static_cast<qint64>(tickCount - lii.dwTime);
    }
    return 0;
}
#endif

TimerEngine::TimerEngine(SettingsManager* settings, QObject* parent)
    : QObject(parent),
      m_settings(settings),
      m_state(State::Idle),
      m_previousState(State::Idle),
      m_secondsRemaining(0),
      m_totalDurationSeconds(0),
      m_wasPausedForIdle(false) {

    connect(&m_timer, &QTimer::timeout, this, &TimerEngine::handleOneSecondTick);
    connect(m_settings, &SettingsManager::settingsChanged, this, &TimerEngine::handleSettingsChanged);

    m_secondsRemaining = m_settings->workDurationSeconds();
    m_totalDurationSeconds = m_secondsRemaining;
}

TimerEngine::State TimerEngine::state() const {
    return m_state;
}

int TimerEngine::secondsRemaining() const {
    return m_secondsRemaining;
}

int TimerEngine::totalDurationSeconds() const {
    return m_totalDurationSeconds;
}

bool TimerEngine::isPausedForIdle() const {
    return m_wasPausedForIdle;
}

QString TimerEngine::formattedTimeRemaining() const {
    int mins = m_secondsRemaining / 60;
    int secs = m_secondsRemaining % 60;
    return QString("%1:%2")
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}

void TimerEngine::start() {
    m_wasPausedForIdle = false;
    if (m_state == State::Idle || m_state == State::Paused) {
        if (m_state == State::Idle) {
            m_totalDurationSeconds = m_settings->workDurationSeconds();
            m_secondsRemaining = m_totalDurationSeconds;
            setState(State::Working);
        } else {
            setState(m_previousState == State::Idle ? State::Working : m_previousState);
        }
        m_timer.start(1000);
        emit tick(m_secondsRemaining, m_totalDurationSeconds);
    }
}

void TimerEngine::pause() {
    m_wasPausedForIdle = false;
    if (m_state == State::Working || m_state == State::Breaking) {
        m_timer.stop();
        m_previousState = m_state;
        setState(State::Paused);
    }
}

void TimerEngine::resume() {
    m_wasPausedForIdle = false;
    if (m_state == State::Paused) {
        setState(m_previousState == State::Idle ? State::Working : m_previousState);
        m_timer.start(1000);
        emit tick(m_secondsRemaining, m_totalDurationSeconds);
    }
}

void TimerEngine::stop() {
    m_wasPausedForIdle = false;
    m_timer.stop();
    m_previousState = State::Idle;
    m_totalDurationSeconds = m_settings->workDurationSeconds();
    m_secondsRemaining = m_totalDurationSeconds;
    setState(State::Idle);
    emit tick(m_secondsRemaining, m_totalDurationSeconds);
}

void TimerEngine::skipBreak() {
    if (m_state == State::Breaking || (m_state == State::Paused && m_previousState == State::Breaking)) {
        m_settings->incrementBreaksSkipped();
        m_wasPausedForIdle = false;
        m_timer.stop();
        m_totalDurationSeconds = m_settings->workDurationSeconds();
        m_secondsRemaining = m_totalDurationSeconds;
        setState(State::Working);
        m_timer.start(1000);
        emit tick(m_secondsRemaining, m_totalDurationSeconds);
    }
}

void TimerEngine::setState(State newState) {
    if (m_state != newState) {
        State oldState = m_state;
        m_state = newState;
        emit stateChanged(m_state, oldState);
    }
}

void TimerEngine::handleOneSecondTick() {
    checkIdleDetection();

    if (m_state == State::Paused) {
        return;
    }

    if (m_secondsRemaining > 0) {
        m_secondsRemaining--;
        emit tick(m_secondsRemaining, m_totalDurationSeconds);
    }

    if (m_secondsRemaining <= 0) {
        if (m_state == State::Working) {
            m_settings->incrementBreaksCompleted(m_settings->breakDurationSeconds());
            emit workCompleted();
            m_totalDurationSeconds = m_settings->breakDurationSeconds();
            m_secondsRemaining = m_totalDurationSeconds;
            setState(State::Breaking);
            emit tick(m_secondsRemaining, m_totalDurationSeconds);
        } else if (m_state == State::Breaking) {
            emit breakCompleted();
            m_totalDurationSeconds = m_settings->workDurationSeconds();
            m_secondsRemaining = m_totalDurationSeconds;
            setState(State::Working);
            emit tick(m_secondsRemaining, m_totalDurationSeconds);
        }
    }
}

void TimerEngine::checkIdleDetection() {
#ifdef Q_OS_WIN
    if (!m_settings->idleDetectionEnabled()) {
        return;
    }

    qint64 idleMs = getSystemIdleTimeMs();
    qint64 thresholdMs = static_cast<qint64>(m_settings->idleThresholdSeconds()) * 1000;

    if (m_state == State::Working && idleMs >= thresholdMs) {
        m_wasPausedForIdle = true;
        m_previousState = m_state;
        setState(State::Paused);
        emit idlePauseTriggered();
    } else if (m_state == State::Paused && m_wasPausedForIdle && idleMs < 2000) {
        m_wasPausedForIdle = false;
        setState(m_previousState == State::Idle ? State::Working : m_previousState);
        emit idleResumeTriggered();
    }
#endif
}

void TimerEngine::handleSettingsChanged() {
    if (m_state == State::Idle) {
        m_totalDurationSeconds = m_settings->workDurationSeconds();
        m_secondsRemaining = m_totalDurationSeconds;
        emit tick(m_secondsRemaining, m_totalDurationSeconds);
    }
}
