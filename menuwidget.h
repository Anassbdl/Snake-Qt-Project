#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include <QPushButton>

class MenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MenuWidget(QWidget *parent = nullptr);

signals:
    void startGame(int level);
    void quitGame();
    void requestFullscreen(bool fullscreen);

private:
    QPushButton *playButton;
    QPushButton *levelButton;
    QPushButton *quitButton;
    int currentLevel;
    bool isFullscreen;

    void setupUI();
    QString getButtonStyle(const QString &color, const QString &hoverColor);
    void toggleFullscreen();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onPlayClicked();
    void onLevelClicked();
    void onQuitClicked();
};

#endif // MENUWIDGET_H
