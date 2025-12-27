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
    FruitType fruitType;
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
    void onFruitEaten(int x, int y, int points, FruitType type);
    void updateScorePopups();

    // NOUVEAUX SLOTS POUR PAUSE
    void onPauseResumeClicked();
    void onPauseRestartClicked();
    void onPauseMenuClicked();

private:
    Game game;
    QTimer timer;
    QTimer popupTimer;
    int cellSize;
    bool isFullscreen;
    int bestScore;
    bool waitingStart;
    int lastScore;

    // NOUVEAU : état de pause
    bool isPaused;

    QPushButton *restartButton;
    QPushButton *menuButton;

    // NOUVEAUX BOUTONS PAUSE
    QPushButton *pauseResumeButton;
    QPushButton *pauseRestartButton;
    QPushButton *pauseMenuButton;

    QVector<ScorePopup> scorePopups;

    void toggleFullscreen();
    void togglePause();  // NOUVEAU
    void drawSnakeSegment(QPainter &p, const QRect &rect, bool isHead,
                          float segmentRatio, Direction dir);
    void drawFruit(QPainter &p, const QRect &rect, FruitType type);
    QString getButtonStyle(const QString &color, const QString &hoverColor);
    void setupGameOverButtons();
    void hideGameOverButtons();
    void setupPauseButtons();   // NOUVEAU
    void hidePauseButtons();    // NOUVEAU
};

#endif // SNAKEWIDGET_H
