#include "menuwidget.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QKeyEvent>

MenuWidget::MenuWidget(QWidget *parent)
    : QWidget(parent), currentLevel(1)
{
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    setupUI();
}

void MenuWidget::setupUI()
{
    playButton = new QPushButton("JOUER", this);
    levelButton = new QPushButton("LEVEL : 1", this);
    quitButton = new QPushButton("QUITTER", this);

    QString playStyle = getButtonStyle("rgb(0, 220, 120)", "rgb(0, 255, 150)");
    QString levelStyle = getButtonStyle("rgb(0, 160, 200)", "rgb(0, 200, 240)");
    QString quitStyle = getButtonStyle("rgb(220, 60, 60)", "rgb(255, 90, 90)");

    playButton->setStyleSheet(playStyle);
    levelButton->setStyleSheet(levelStyle);
    quitButton->setStyleSheet(quitStyle);

    QFont buttonFont("Consolas", 18, QFont::Bold);
    playButton->setFont(buttonFont);
    levelButton->setFont(buttonFont);
    quitButton->setFont(buttonFont);

    playButton->setFixedSize(300, 70);
    levelButton->setFixedSize(300, 70);
    quitButton->setFixedSize(300, 70);

    playButton->setCursor(Qt::PointingHandCursor);
    levelButton->setCursor(Qt::PointingHandCursor);
    quitButton->setCursor(Qt::PointingHandCursor);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->addStretch();
    layout->addWidget(playButton, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(levelButton, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(quitButton, 0, Qt::AlignCenter);
    layout->addStretch();
    setLayout(layout);

    connect(playButton, &QPushButton::clicked, this, &MenuWidget::onPlayClicked);
    connect(levelButton, &QPushButton::clicked, this, &MenuWidget::onLevelClicked);
    connect(quitButton, &QPushButton::clicked, this, &MenuWidget::onQuitClicked);
}

QString MenuWidget::getButtonStyle(const QString &color, const QString &hoverColor)
{
    return QString(
               "QPushButton {"
               "  background-color: %1;"
               "  color: white;"
               "  border: 2px solid rgba(255, 255, 255, 0.3);"
               "  border-radius: 10px;"
               "  padding: 10px;"
               "}"
               "QPushButton:hover {"
               "  background-color: %2;"
               "  border: 2px solid rgba(255, 255, 255, 0.6);"
               "}"
               "QPushButton:pressed {"
               "  background-color: rgba(0, 0, 0, 0.3);"
               "}"
               ).arg(color, hoverColor);
}

void MenuWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rect(), QColor(15, 15, 30));

    p.setPen(QPen(QColor(0, 60, 60, 100), 1));
    int gridSize = 30;
    for (int x = 0; x < width(); x += gridSize)
        p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += gridSize)
        p.drawLine(0, y, width(), y);

    p.setPen(QColor(0, 220, 120));
    p.setFont(QFont("Consolas", 48, QFont::Bold));
    p.drawText(rect().adjusted(0, 50, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "SNAKE - GI3");

    p.setPen(QColor(255, 255, 255));
    p.setFont(QFont("Consolas", 16));
    p.drawText(rect().adjusted(0, 130, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "Menu Principal");

    p.setFont(QFont("Consolas", 10));
    p.drawText(rect().adjusted(0, 0, 0, -20), Qt::AlignHCenter | Qt::AlignBottom, "F11 : Plein écran");
}

void MenuWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QWidget::resizeEvent(event);
}

void MenuWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11)
    {
        bool isCurrentlyFullscreen = window()->isFullScreen();
        emit requestFullscreen(!isCurrentlyFullscreen);
        return;
    }

    QWidget::keyPressEvent(event);
}

void MenuWidget::onPlayClicked()
{
    emit startGame(currentLevel);
}

void MenuWidget::onLevelClicked()
{
    currentLevel = (currentLevel % 3) + 1;
    levelButton->setText(QString("LEVEL : %1").arg(currentLevel));
}

void MenuWidget::onQuitClicked()
{
    emit quitGame();
}
