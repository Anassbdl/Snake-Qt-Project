#ifndef SNAKEWIDGET_H
#define SNAKEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include "game.h"

struct ScorePopup
{
    int x;
    int y;
    int points;
    int alpha;
    int offsetY;
};

class SnakeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SnakeWidget(QWidget *parent = nullptr);
    void startGameDirectly();

signals:
    void backToMenu();
    void requestFullscreen(bool fullscreen);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void gameLoop();
    void onRestartClicked();
    void onMenuClicked();
    void onFruitEaten(int x, int y, int points);
    void updateScorePopups();

private:
    Game game;
    QTimer timer;
    QTimer popupTimer;
    int cellSize;
    bool isFullscreen;
    int bestScore;
    bool waitingStart;
    int lastScore;

    QPushButton *restartButton;
    QPushButton *menuButton;

    QVector<ScorePopup> scorePopups;

    void toggleFullscreen();
    void drawSnakeSegment(QPainter &p, const QRect &rect, bool isHead,
                          float segmentRatio, Direction dir);
    void drawFruit(QPainter &p, const QRect &rect, FruitType type);
    QString getButtonStyle(const QString &color, const QString &hoverColor);
    void setupGameOverButtons();
    void hideGameOverButtons();
};

#endif // SNAKEWIDGET_H
