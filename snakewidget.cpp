#include "snakewidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTime>
#include <QResizeEvent>

SnakeWidget::SnakeWidget(QWidget *parent)
    : QWidget(parent),
    cellSize(25),
    isFullscreen(false),
    bestScore(0),
    waitingStart(true)
{
    setFocusPolicy(Qt::StrongFocus);
    int preferredWidth = WIDTH * cellSize;
    int preferredHeight = (HEIGHT + 3) * cellSize;
    setMinimumSize(preferredWidth, preferredHeight);
    resize(preferredWidth, preferredHeight);
    connect(&timer, &QTimer::timeout, this, &SnakeWidget::gameLoop);
    game.reset();
    timer.stop();
}

void SnakeWidget::startGameDirectly()
{
    waitingStart = false;
    game.reset();
    timer.start(160);
    update();
}

void SnakeWidget::toggleFullscreen()
{
    isFullscreen = !isFullscreen;
    emit requestFullscreen(isFullscreen);
}

void SnakeWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (isFullscreen) {
        cellSize = qMin(width() / WIDTH, (height() - 50) / HEIGHT);
        if (cellSize < 5) cellSize = 5;
    } else {
        cellSize = 25;
    }
}

void SnakeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(15, 15, 30));

    int gameWidth = WIDTH * cellSize;
    int gameHeight = HEIGHT * cellSize;
    int offsetX = (width() - gameWidth) / 2;
    int offsetY = (height() - (HEIGHT + 3) * cellSize) / 2;
    if (offsetY < 0) offsetY = 0;

    QRect gameRect(offsetX, offsetY, gameWidth, gameHeight);
    p.fillRect(gameRect, QColor(5, 5, 15));
    p.setPen(QPen(QColor(0, 200, 150), 2));
    p.drawRect(gameRect.adjusted(1, 1, -1, -1));

    p.setPen(QPen(QColor(0, 60, 60), 1));
    for (int x = 1; x < WIDTH; ++x)
        p.drawLine(offsetX + x * cellSize, offsetY,
                   offsetX + x * cellSize, offsetY + gameHeight);
    for (int y = 1; y < HEIGHT; ++y)
        p.drawLine(offsetX, offsetY + y * cellSize,
                   offsetX + gameWidth, offsetY + y * cellSize);

    if (waitingStart)
    {
        p.setPen(Qt::white);
        p.setFont(QFont("Consolas", 24, QFont::Bold));
        p.drawText(gameRect, Qt::AlignCenter,
                   "SNAKE PRO\n\nAppuie sur ENTRER pour commencer");
        return;
    }

    for (const Obstacle &o : game.getObstacles())
    {
        QRect r(offsetX + o.x * cellSize,
                offsetY + o.y * cellSize,
                cellSize, cellSize);
        p.fillRect(r, QColor(120, 40, 160));
        p.setPen(QPen(QColor(200, 120, 255), 1));
        p.drawRect(r.adjusted(1, 1, -1, -1));
    }

    int t = QTime::currentTime().msec();
    int alpha = 180 + (qSin(t / 150.0) * 50);
    for (int i = 0; i < game.foodCount(); ++i)
    {
        QRect foodRect(offsetX + game.foodX(i) * cellSize,
                       offsetY + game.foodY(i) * cellSize,
                       cellSize, cellSize);
        p.setBrush(QColor(255, 80, 0, alpha));
        p.setPen(Qt::NoPen);
        p.drawEllipse(foodRect.adjusted(2, 2, -2, -2));
    }

    SnakeNode *cur = game.snakeHead();
    bool headDone = false;
    while (cur)
    {
        QRect r(offsetX + cur->x * cellSize,
                offsetY + cur->y * cellSize,
                cellSize, cellSize);
        if (!headDone)
        {
            p.setBrush(QColor(0, 220, 120));
            headDone = true;
        }
        else
        {
            p.setBrush(QColor(0, 140, 80));
        }
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(r.adjusted(2, 2, -2, -2), 4, 4);
        cur = cur->next;
    }

    if (game.getScore() > bestScore)
        bestScore = game.getScore();

    p.setPen(Qt::white);
    p.setFont(QFont("Consolas", 14, QFont::Bold));
    int hudY = gameRect.bottom() + 30;
    p.drawText(offsetX, hudY,
               QString("Score : %1").arg(game.getScore()));
    p.drawText(offsetX + 200, hudY,
               QString("Longueur : %1").arg(game.getLength()));
    p.drawText(offsetX + 420, hudY,
               QString("Best : %1").arg(bestScore));

    p.setFont(QFont("Consolas", 10));
    p.drawText(offsetX, hudY + 20,
               "F11 : plein ecran | ESC : menu | Fleches : direction");

    if (game.isGameOver())
    {
        p.fillRect(gameRect, QColor(0, 0, 0, 160));
        p.setPen(Qt::white);
        p.setFont(QFont("Consolas", 24, QFont::Bold));
        p.drawText(gameRect, Qt::AlignCenter,
                   QString("GAME OVER\nScore : %1\nENTRER pour revenir au menu")
                       .arg(game.getScore()));
    }
}

void SnakeWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11)
    {
        toggleFullscreen();
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        emit backToMenu();
        return;
    }

    if (waitingStart)
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            waitingStart = false;
            game.reset();
            timer.start(160);
            update();
        }
        return;
    }

    if (game.isGameOver())
    {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            emit backToMenu();
            timer.stop();
            return;
        }
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Up: game.changeDirection(UP); break;
    case Qt::Key_Down: game.changeDirection(DOWN); break;
    case Qt::Key_Left: game.changeDirection(LEFT); break;
    case Qt::Key_Right: game.changeDirection(RIGHT); break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void SnakeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    toggleFullscreen();
}

void SnakeWidget::gameLoop()
{
    game.updateGame();
    update();
}
