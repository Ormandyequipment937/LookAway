#include "AudioManager.h"
#include <QUrl>
#include <QDebug>

AudioManager::AudioManager(SettingsManager* settings, TimerEngine* timerEngine, QObject* parent)
    : QObject(parent),
      m_settings(settings),
      m_timerEngine(timerEngine) {

    m_workSound.setSource(QUrl("qrc:/sounds/chime_work.wav"));
    m_breakSound.setSource(QUrl("qrc:/sounds/chime_break.wav"));

    syncSettings();

    connect(m_settings, &SettingsManager::settingsChanged, this, &AudioManager::syncSettings);
    connect(m_timerEngine, &TimerEngine::workCompleted, this, &AudioManager::playWorkCompleteChime);
    connect(m_timerEngine, &TimerEngine::breakCompleted, this, &AudioManager::playBreakCompleteChime);
}

void AudioManager::syncSettings() {
    float vol = static_cast<float>(m_settings->volume()) / 100.0f;
    m_workSound.setVolume(vol);
    m_breakSound.setVolume(vol);
}

void AudioManager::playWorkCompleteChime() {
    if (m_settings->audioEnabled()) {
        m_workSound.play();
    }
}

void AudioManager::playBreakCompleteChime() {
    if (m_settings->audioEnabled()) {
        m_breakSound.play();
    }
}

void AudioManager::playTestChime() {
    m_workSound.play();
}
