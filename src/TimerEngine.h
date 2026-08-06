#ifndef TIMERENGINE_H
#define TIMERENGINE_H

#include <QObject>
#include <QTimer>
#include "SettingsManager.h"

class TimerEngine : public QObject {
    Q_OBJECT

public:
    enum class State {
        Idle,
        Working,
        Breaking,
        Paused
    };
    Q_ENUM(State)

    explicit TimerEngine(SettingsManager* settings, QObject* parent = nullptr);

    State state() const;
    int secondsRemaining() const;
    int totalDurationSeconds() const;
    QString formattedTimeRemaining() const;
    bool isPausedForIdle() const;

public slots:
    void start();
    void pause();
    void resume();
    void stop();
    void skipBreak();

signals:
    void stateChanged(TimerEngine::State newState, TimerEngine::State oldState);
    void tick(int secondsRemaining, int totalDurationSeconds);
    void workCompleted();
    void breakCompleted();
    void idlePauseTriggered();
    void idleResumeTriggered();

private slots:
    void handleOneSecondTick();
    void handleSettingsChanged();

private:
    void setState(State newState);
    void checkIdleDetection();

    SettingsManager* m_settings;
    QTimer m_timer;
    State m_state;
    State m_previousState;
    int m_secondsRemaining;
    int m_totalDurationSeconds;
    bool m_wasPausedForIdle;
};

#endif // TIMERENGINE_H
