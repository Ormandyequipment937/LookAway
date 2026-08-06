#ifndef BREAKOVERLAYWIDGET_H
#define BREAKOVERLAYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

class BreakOverlayWidget : public QWidget {
    Q_OBJECT

public:
    explicit BreakOverlayWidget(QWidget* parent = nullptr);

    void updateCountdown(int secondsRemaining, int totalSeconds);

signals:
    void skipRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUi();

    QLabel* m_lblTitle;
    QLabel* m_lblSubtitle;
    QLabel* m_lblCountdown;
    QProgressBar* m_progressBar;
    QPushButton* m_btnSkip;
};

#endif // BREAKOVERLAYWIDGET_H
