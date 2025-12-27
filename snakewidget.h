#ifndef SNAKEWIDGET_H
#define SNAKEWIDGET_H

#include <QWidget>
#include <QTimer>
#include "game.h"

class SnakeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SnakeWidget(QWidget *parent = nullptr);
    void startGameDirectly();

signals:
    void backToMenu();
    void requestFullscreen(bool enable);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void gameLoop();

private:
    Game game;
    QTimer timer;
    int cellSize;
    bool isFullscreen;
    int bestScore;
    bool waitingStart;

    void toggleFullscreen();
};

#endif // SNAKEWIDGET_H
