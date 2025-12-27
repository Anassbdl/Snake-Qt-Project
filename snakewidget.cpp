#include "snakewidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTime>
#include <QResizeEvent>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

SnakeWidget::SnakeWidget(QWidget *parent)
    : QWidget(parent),
    cellSize(25),
    isFullscreen(false),
    bestScore(0),
    waitingStart(true),
    showScorePopup(false),
    scorePopupAlpha(0)
{
    setFocusPolicy(Qt::StrongFocus);
    int preferredWidth = WIDTH * cellSize;
    int preferredHeight = (HEIGHT + 3) * cellSize;
    setMinimumSize(preferredWidth, preferredHeight);
    resize(preferredWidth, preferredHeight);
    connect(&timer, &QTimer::timeout, this, &SnakeWidget::gameLoop);
    game.reset();
    timer.stop();

    // Créer les boutons Game Over
    restartButton = new QPushButton("REJOUER", this);
    menuButton = new QPushButton("MENU", this);

    QString restartStyle = getButtonStyle("rgb(0, 220, 120)", "rgb(0, 255, 150)");
    QString menuStyle = getButtonStyle("rgb(0, 160, 200)", "rgb(0, 200, 240)");

    restartButton->setStyleSheet(restartStyle);
    menuButton->setStyleSheet(menuStyle);

    QFont buttonFont("Consolas", 16, QFont::Bold);
    restartButton->setFont(buttonFont);
    menuButton->setFont(buttonFont);

    restartButton->setFixedSize(200, 60);
    menuButton->setFixedSize(200, 60);

    restartButton->setCursor(Qt::PointingHandCursor);
    menuButton->setCursor(Qt::PointingHandCursor);

    connect(restartButton, &QPushButton::clicked, this, &SnakeWidget::onRestartClicked);
    connect(menuButton, &QPushButton::clicked, this, &SnakeWidget::onMenuClicked);

    hideGameOverButtons();
}

QString SnakeWidget::getButtonStyle(const QString &color, const QString &hoverColor)
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

void SnakeWidget::setupGameOverButtons()
{
    int gameWidth = WIDTH * cellSize;
    int gameHeight = HEIGHT * cellSize;
    int offsetX = (width() - gameWidth) / 2;
    int offsetY = (height() - (HEIGHT + 3) * cellSize) / 2;
    if (offsetY < 0) offsetY = 0;

    int centerX = offsetX + gameWidth / 2;
    int centerY = offsetY + gameHeight / 2;

    // Plus d'espace en bas pour les boutons
    restartButton->move(centerX - 220, centerY + 120);
    menuButton->move(centerX + 20, centerY + 120);

    restartButton->show();
    menuButton->show();
    restartButton->setFocus();
}

void SnakeWidget::hideGameOverButtons()
{
    restartButton->hide();
    menuButton->hide();
}

void SnakeWidget::onRestartClicked()
{
    hideGameOverButtons();
    waitingStart = false;
    game.reset();
    timer.start(160);
    setFocus();
    update();
}

void SnakeWidget::onMenuClicked()
{
    hideGameOverButtons();
    timer.stop();
    emit backToMenu();
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

    if (game.isGameOver()) {
        setupGameOverButtons();
    }
}

void SnakeWidget::drawSnakeSegment(QPainter &p, const QRect &rect, bool isHead,
                                   float segmentRatio, Direction dir)
{
    QRect shadowRect = rect.adjusted(3, 3, 3, 3);
    p.setBrush(QColor(0, 0, 0, 40));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(shadowRect.adjusted(2, 2, -2, -2), 6, 6);

    if (isHead)
    {
        QRadialGradient gradient(rect.center(), cellSize * 0.6);
        gradient.setColorAt(0, QColor(0, 255, 140));
        gradient.setColorAt(0.7, QColor(0, 220, 120));
        gradient.setColorAt(1, QColor(0, 180, 100));
        p.setBrush(gradient);
        p.setPen(QPen(QColor(0, 150, 90), 2));
        p.drawRoundedRect(rect.adjusted(1, 1, -1, -1), 8, 8);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(100, 255, 200, 100), 1));
        p.drawRoundedRect(rect.adjusted(3, 3, -3, -3), 6, 6);

        int eyeSize = cellSize / 6;
        int eyeOffset = cellSize / 4;
        QPoint center = rect.center();
        QPoint leftEye, rightEye;

        switch (dir)
        {
        case UP:
            leftEye = QPoint(center.x() - eyeOffset, center.y() - eyeOffset);
            rightEye = QPoint(center.x() + eyeOffset, center.y() - eyeOffset);
            break;
        case DOWN:
            leftEye = QPoint(center.x() - eyeOffset, center.y() + eyeOffset);
            rightEye = QPoint(center.x() + eyeOffset, center.y() + eyeOffset);
            break;
        case LEFT:
            leftEye = QPoint(center.x() - eyeOffset, center.y() - eyeOffset);
            rightEye = QPoint(center.x() - eyeOffset, center.y() + eyeOffset);
            break;
        case RIGHT:
            leftEye = QPoint(center.x() + eyeOffset, center.y() - eyeOffset);
            rightEye = QPoint(center.x() + eyeOffset, center.y() + eyeOffset);
            break;
        }

        p.setBrush(Qt::white);
        p.setPen(Qt::NoPen);
        p.drawEllipse(leftEye, eyeSize, eyeSize);
        p.drawEllipse(rightEye, eyeSize, eyeSize);

        p.setBrush(QColor(20, 20, 40));
        p.drawEllipse(leftEye, eyeSize/2, eyeSize/2);
        p.drawEllipse(rightEye, eyeSize/2, eyeSize/2);

        p.setBrush(QColor(255, 255, 255, 180));
        p.drawEllipse(leftEye.x() - eyeSize/4, leftEye.y() - eyeSize/4,
                      eyeSize/3, eyeSize/3);
        p.drawEllipse(rightEye.x() - eyeSize/4, rightEye.y() - eyeSize/4,
                      eyeSize/3, eyeSize/3);
    }
    else
    {
        int baseGreen = 220 - (segmentRatio * 80);
        int darkGreen = 140 - (segmentRatio * 60);

        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, QColor(0, baseGreen, baseGreen * 0.55));
        gradient.setColorAt(0.5, QColor(0, darkGreen, darkGreen * 0.55));
        gradient.setColorAt(1, QColor(0, baseGreen - 20, (baseGreen - 20) * 0.55));

        p.setBrush(gradient);
        p.setPen(QPen(QColor(0, darkGreen - 20, (darkGreen - 20) * 0.5), 1.5));
        p.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 5, 5);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(0, baseGreen + 20, (baseGreen + 20) * 0.6, 60), 1));
        int scaleSize = cellSize / 3;
        for (int sx = 0; sx < 2; ++sx)
        {
            for (int sy = 0; sy < 2; ++sy)
            {
                QPoint scaleCenter(rect.left() + scaleSize + sx * scaleSize,
                                   rect.top() + scaleSize + sy * scaleSize);
                p.drawEllipse(scaleCenter, scaleSize/3, scaleSize/3);
            }
        }

        QLinearGradient shine(rect.topLeft(),
                              QPoint(rect.left(), rect.top() + cellSize/3));
        shine.setColorAt(0, QColor(255, 255, 255, 40));
        shine.setColorAt(1, QColor(255, 255, 255, 0));
        p.setBrush(shine);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect.adjusted(3, 3, -3, -cellSize/2), 4, 4);
    }
}

void SnakeWidget::drawFruit(QPainter &p, const QRect &rect, FruitType type)
{
    QPoint center = rect.center();

    switch (type)
    {
    case APPLE:
    {
        QRadialGradient appleGrad(center, cellSize * 0.6);
        appleGrad.setColorAt(0, QColor(255, 80, 80));
        appleGrad.setColorAt(0.6, QColor(220, 30, 30));
        appleGrad.setColorAt(1, QColor(180, 20, 20));
        p.setBrush(appleGrad);
        p.setPen(QPen(QColor(200, 40, 40), 2));
        p.drawEllipse(rect.adjusted(2, 2, -2, -2));

        p.setBrush(QColor(255, 255, 255, 150));
        p.setPen(Qt::NoPen);
        p.drawEllipse(center.x() - cellSize/4, center.y() - cellSize/3,
                      cellSize/3, cellSize/3);

        p.setPen(QPen(QColor(100, 60, 20), 2));
        p.drawLine(center.x(), center.y() - cellSize/3,
                   center.x(), center.y() - cellSize/2);

        p.setBrush(QColor(50, 150, 50));
        p.setPen(Qt::NoPen);
        QPoint leaf[3] = {
            QPoint(center.x(), center.y() - cellSize/2),
            QPoint(center.x() + cellSize/5, center.y() - cellSize/2.5),
            QPoint(center.x() + cellSize/6, center.y() - cellSize/3)
        };
        p.drawPolygon(leaf, 3);
    }
    break;

    case BANANA:
    {
        QPainterPath bananaBody;
        bananaBody.moveTo(center.x() - cellSize/3, center.y() + cellSize/5);
        bananaBody.cubicTo(
            center.x() - cellSize/2.5, center.y() - cellSize/8,
            center.x() - cellSize/6, center.y() - cellSize/2.5,
            center.x() + cellSize/4, center.y() - cellSize/3
            );
        bananaBody.lineTo(center.x() + cellSize/3, center.y() - cellSize/4);
        bananaBody.cubicTo(
            center.x() + cellSize/5, center.y() - cellSize/2.2,
            center.x() - cellSize/8, center.y() - cellSize/6,
            center.x() - cellSize/3.5, center.y() + cellSize/4
            );
        bananaBody.closeSubpath();

        QLinearGradient bananaGrad(
            center.x() - cellSize/3, center.y(),
            center.x() + cellSize/3, center.y()
            );
        bananaGrad.setColorAt(0, QColor(240, 210, 70));
        bananaGrad.setColorAt(0.3, QColor(255, 235, 100));
        bananaGrad.setColorAt(0.5, QColor(255, 245, 120));
        bananaGrad.setColorAt(0.7, QColor(255, 230, 80));
        bananaGrad.setColorAt(1, QColor(230, 195, 50));
        p.setBrush(bananaGrad);
        p.setPen(Qt::NoPen);
        p.drawPath(bananaBody);

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(180, 140, 30), 2));
        p.drawPath(bananaBody);
    }
    break;

    case PINEAPPLE:
    {
        QLinearGradient pineGrad(rect.topLeft(), rect.bottomRight());
        pineGrad.setColorAt(0, QColor(255, 200, 50));
        pineGrad.setColorAt(0.5, QColor(240, 160, 0));
        pineGrad.setColorAt(1, QColor(200, 120, 0));
        p.setBrush(pineGrad);
        p.setPen(QPen(QColor(180, 100, 0), 2));

        QRect body = rect.adjusted(3, cellSize/4, -3, -2);
        p.drawEllipse(body);

        p.setPen(QPen(QColor(150, 80, 0), 1));
        for (int y = 0; y < 3; ++y)
        {
            for (int x = 0; x < 2; ++x)
            {
                int px = center.x() - cellSize/5 + x * cellSize/2.5;
                int py = center.y() - cellSize/6 + y * cellSize/4;
                p.drawLine(px - cellSize/8, py, px, py - cellSize/8);
                p.drawLine(px, py - cellSize/8, px + cellSize/8, py);
                p.drawLine(px + cellSize/8, py, px, py + cellSize/8);
                p.drawLine(px, py + cellSize/8, px - cellSize/8, py);
            }
        }

        p.setBrush(QColor(50, 180, 50));
        p.setPen(QPen(QColor(30, 140, 30), 2));
        for (int i = 0; i < 5; ++i)
        {
            int angle = -60 + i * 30;
            float rad = angle * 3.14159 / 180.0;
            int x1 = center.x();
            int y1 = rect.top() + cellSize/4;
            int x2 = x1 + qSin(rad) * cellSize/2;
            int y2 = y1 - qCos(rad) * cellSize/2.5;
            QPoint leaf[3] = {
                QPoint(x1, y1),
                QPoint(x2, y2),
                QPoint(x1 + qSin(rad) * cellSize/4, y1 - cellSize/6)
            };
            p.drawPolygon(leaf, 3);
        }
    }
    break;
    }
}

void SnakeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Fond avec dégradé
    QLinearGradient bgGradient(0, 0, 0, height());
    bgGradient.setColorAt(0, QColor(15, 15, 35));
    bgGradient.setColorAt(1, QColor(10, 10, 25));
    p.fillRect(rect(), bgGradient);

    int gameWidth = WIDTH * cellSize;
    int gameHeight = HEIGHT * cellSize;
    int offsetX = (width() - gameWidth) / 2;
    int offsetY = (height() - (HEIGHT + 3) * cellSize) / 2;
    if (offsetY < 0) offsetY = 0;

    QRect gameRect(offsetX, offsetY, gameWidth, gameHeight);

    // **Game Over**
    if (game.isGameOver())
    {
        // Fond identique au menu général (OPAQUE)
        p.fillRect(gameRect, QColor(15, 15, 30));

        // Grille décorative
        p.setPen(QPen(QColor(0, 60, 60, 100), 1));
        int gridSize = 30;
        for (int x = 0; x < gameRect.width(); x += gridSize)
            p.drawLine(gameRect.left() + x, gameRect.top(),
                       gameRect.left() + x, gameRect.bottom());
        for (int y = 0; y < gameRect.height(); y += gridSize)
            p.drawLine(gameRect.left(), gameRect.top() + y,
                       gameRect.right(), gameRect.top() + y);

        // Bordure rouge
        p.setPen(QPen(QColor(255, 50, 50), 4));
        p.drawRect(gameRect.adjusted(5, 5, -5, -5));

        int startY = gameRect.top() + 120;

        // Titre GAME OVER
        p.setPen(QColor(255, 100, 100));
        p.setFont(QFont("Consolas", 42, QFont::Bold));
        QRect titleRect(gameRect.left(), startY, gameRect.width(), 60);
        p.drawText(titleRect, Qt::AlignCenter, "GAME OVER");

        startY += 90;

        bool isNewRecord = (game.getScore() >= bestScore && game.getScore() > 0);

        if (isNewRecord)
        {
            p.setPen(QColor(255, 215, 0));
            p.setFont(QFont("Consolas", 18, QFont::Bold));
            QRect recordRect(gameRect.left(), startY, gameRect.width(), 40);
            p.drawText(recordRect, Qt::AlignCenter, "NOUVEAU RECORD !");
            startY += 60;
        }
        else
        {
            startY += 30;
        }

        // Score final
        p.setPen(Qt::white);
        p.setFont(QFont("Consolas", 22));
        QRect scoreRect(gameRect.left(), startY, gameRect.width(), 40);
        p.drawText(scoreRect, Qt::AlignCenter,
                   QString("Score final : %1").arg(game.getScore()));

        startY += 50;

        // Meilleur score
        p.setPen(QColor(255, 200, 0));
        p.setFont(QFont("Consolas", 18));
        QRect bestRect(gameRect.left(), startY, gameRect.width(), 40);
        p.drawText(bestRect, Qt::AlignCenter,
                   QString("Meilleur score : %1").arg(bestScore));

        return; // SORTIR ICI
    }

    // Fond de jeu
    QLinearGradient gameGradient(gameRect.topLeft(), gameRect.bottomRight());
    gameGradient.setColorAt(0, QColor(5, 5, 20));
    gameGradient.setColorAt(1, QColor(8, 8, 25));
    p.fillRect(gameRect, gameGradient);

    // Bordure
    p.setPen(QPen(QColor(0, 220, 170), 3));
    p.drawRect(gameRect.adjusted(1, 1, -1, -1));
    p.setPen(QPen(QColor(0, 255, 200, 100), 1));
    p.drawRect(gameRect.adjusted(3, 3, -3, -3));

    // Grille
    p.setPen(QPen(QColor(0, 100, 100), 1));
    for (int x = 1; x < WIDTH; ++x)
        p.drawLine(offsetX + x * cellSize, offsetY,
                   offsetX + x * cellSize, offsetY + gameHeight);
    for (int y = 1; y < HEIGHT; ++y)
        p.drawLine(offsetX, offsetY + y * cellSize,
                   offsetX + gameWidth, offsetY + y * cellSize);

    // Écran d'attente
    if (waitingStart)
    {
        p.setPen(QPen(QColor(0, 255, 180), 2));
        p.setFont(QFont("Consolas", 28, QFont::Bold));
        p.drawText(gameRect.adjusted(0, -40, 0, -40), Qt::AlignCenter, "SNAKE PRO");

        p.setPen(Qt::white);
        p.setFont(QFont("Consolas", 16));
        p.drawText(gameRect.adjusted(0, 0, 0, -80), Qt::AlignCenter,
                   "Appuie sur ENTRER pour commencer");

        p.setFont(QFont("Consolas", 12));
        p.setPen(QColor(255, 80, 80));
        p.drawText(gameRect.adjusted(0, 60, 0, 0), Qt::AlignCenter,
                   "Pomme = 10 pts");
        p.setPen(QColor(255, 220, 0));
        p.drawText(gameRect.adjusted(0, 85, 0, 0), Qt::AlignCenter,
                   "Banane = 15 pts");
        p.setPen(QColor(255, 180, 0));
        p.drawText(gameRect.adjusted(0, 110, 0, 0), Qt::AlignCenter,
                   "Ananas = 25 pts");

        return;
    }

    // Obstacles
    for (const Obstacle &o : game.getObstacles())
    {
        QRect r(offsetX + o.x * cellSize,
                offsetY + o.y * cellSize,
                cellSize, cellSize);

        p.fillRect(r.adjusted(2, 2, 2, 2), QColor(0, 0, 0, 50));

        QLinearGradient obsGrad(r.topLeft(), r.bottomRight());
        obsGrad.setColorAt(0, QColor(140, 60, 180));
        obsGrad.setColorAt(1, QColor(100, 30, 140));
        p.setBrush(obsGrad);
        p.setPen(QPen(QColor(200, 120, 255), 2));
        p.drawRect(r.adjusted(1, 1, -1, -1));

        p.setBrush(QColor(255, 255, 255, 30));
        p.setPen(Qt::NoPen);
        p.drawRect(r.adjusted(3, 3, -cellSize/2, -cellSize/2));
    }

    // Fruits
    for (int i = 0; i < game.foodCount(); ++i)
    {
        QRect foodRect(offsetX + game.foodX(i) * cellSize,
                       offsetY + game.foodY(i) * cellSize,
                       cellSize, cellSize);
        drawFruit(p, foodRect, game.foodType(i));
    }

    // Serpent
    Direction snakeDir = game.getDirection();
    SnakeNode *cur = game.snakeHead();
    int segmentIndex = 0;
    int totalLength = game.getLength();

    while (cur)
    {
        QRect r(offsetX + cur->x * cellSize,
                offsetY + cur->y * cellSize,
                cellSize, cellSize);
        bool isHead = (segmentIndex == 0);
        float segmentRatio = static_cast<float>(segmentIndex) / totalLength;
        drawSnakeSegment(p, r, isHead, segmentRatio, snakeDir);
        cur = cur->next;
        segmentIndex++;
    }

    // **HUD EN BAS (sans bordure)**
    if (game.getScore() > bestScore)
        bestScore = game.getScore();

    // Position juste sous le plateau
    int hudY = gameRect.bottom() + 40;

    // Fond de la barre HUD
    QRect hudRect(offsetX, gameRect.bottom() + 15, gameWidth, 50);
    QLinearGradient hudGrad(hudRect.topLeft(), hudRect.bottomLeft());
    hudGrad.setColorAt(0, QColor(10, 10, 30, 200));
    hudGrad.setColorAt(1, QColor(5, 5, 20, 200));
    p.fillRect(hudRect, hudGrad);

    // Score
    p.setPen(QColor(0, 255, 180));
    p.setFont(QFont("Consolas", 16, QFont::Bold));
    p.drawText(offsetX + 20, hudY,
               QString("Score : %1").arg(game.getScore()));

    // Longueur
    p.setPen(QColor(100, 200, 255));
    p.drawText(offsetX + gameWidth/2 - 80, hudY,
               QString("Longueur : %1").arg(game.getLength()));

    // Best
    p.setPen(QColor(255, 200, 0));
    p.drawText(offsetX + gameWidth - 150, hudY,
               QString("Best : %1").arg(bestScore));

    // Contrôles en bas du HUD
    p.setPen(QColor(150, 150, 150));
    p.setFont(QFont("Consolas", 10));
    p.drawText(offsetX + 20, hudY + 25,
               "F11 : plein ecran | ESC : menu | Fleches : direction");
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
            lastScore = 0;
            timer.start(160);
            update();
        }
        return;
    }

    if (game.isGameOver())
    {
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

    if (game.isGameOver())
    {
        timer.stop();
        setupGameOverButtons();
    }

    update();
}
