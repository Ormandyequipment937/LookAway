#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QSoundEffect>
#include "SettingsManager.h"
#include "TimerEngine.h"

class AudioManager : public QObject {
    Q_OBJECT

public:
    explicit AudioManager(SettingsManager* settings, TimerEngine* timerEngine, QObject* parent = nullptr);

public slots:
    void playWorkCompleteChime();
    void playBreakCompleteChime();
    void playTestChime();

private slots:
    void syncSettings();

private:
    SettingsManager* m_settings;
    TimerEngine* m_timerEngine;
    QSoundEffect m_workSound;
    QSoundEffect m_breakSound;
};

#endif // AUDIOMANAGER_H
