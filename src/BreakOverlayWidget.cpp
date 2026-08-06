#include "BreakOverlayWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>

BreakOverlayWidget::BreakOverlayWidget(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {

    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);

    setupUi();
}

void BreakOverlayWidget::setupUi() {
    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    // Full screen semi-transparent background frame
    QFrame* bgFrame = new QFrame(this);
    bgFrame->setStyleSheet(R"(
        QFrame {
            background-color: rgba(15, 23, 42, 235);
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(bgFrame);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    // Eye icon
    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(QIcon(":/icons/app_icon.svg").pixmap(80, 80));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    m_lblTitle = new QLabel("TIME FOR AN EYE BREAK");
    m_lblTitle->setStyleSheet("font-size: 28px; font-weight: 800; color: #38bdf8; letter-spacing: 2px;");
    m_lblTitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_lblTitle);

    m_lblSubtitle = new QLabel("Look at an object at least 20 feet (6 meters) away to relax your eye muscles.");
    m_lblSubtitle->setStyleSheet("font-size: 16px; color: #94a3b8; font-weight: 500;");
    m_lblSubtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_lblSubtitle);

    m_lblCountdown = new QLabel("00:20");
    m_lblCountdown->setStyleSheet("font-size: 72px; font-weight: 900; color: #ffffff; font-family: 'Consolas', monospace;");
    m_lblCountdown->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_lblCountdown);

    m_progressBar = new QProgressBar();
    m_progressBar->setFixedWidth(360);
    m_progressBar->setFixedHeight(10);
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 5px;
        }
        QProgressBar::chunk {
            background-color: #38bdf8;
            border-radius: 4px;
        }
    )");
    layout->addWidget(m_progressBar, 0, Qt::AlignCenter);

    m_btnSkip = new QPushButton("Skip Break (Esc)");
    m_btnSkip->setFixedSize(160, 42);
    m_btnSkip->setCursor(Qt::PointingHandCursor);
    m_btnSkip->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(51, 65, 85, 200);
            color: #f8fafc;
            border: 1px solid #475569;
            border-radius: 6px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: rgba(71, 85, 105, 240);
        }
    )");
    connect(m_btnSkip, &QPushButton::clicked, this, &BreakOverlayWidget::skipRequested);
    layout->addWidget(m_btnSkip, 0, Qt::AlignCenter);

    outerLayout->addWidget(bgFrame);
}

void BreakOverlayWidget::updateCountdown(int secondsRemaining, int totalSeconds) {
    int mins = secondsRemaining / 60;
    int secs = secondsRemaining % 60;
    m_lblCountdown->setText(QString("%1:%2")
                                .arg(mins, 2, 10, QChar('0'))
                                .arg(secs, 2, 10, QChar('0')));

    if (totalSeconds > 0) {
        int pct = static_cast<int>((static_cast<double>(secondsRemaining) / totalSeconds) * 100.0);
        m_progressBar->setValue(pct);
    }
}

void BreakOverlayWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        emit skipRequested();
    } else {
        QWidget::keyPressEvent(event);
    }
}
