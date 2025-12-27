#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "menuwidget.h"
#include "snakewidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    QStackedWidget *mainStack = new QStackedWidget();
    mainStack->setWindowTitle("Snake - GI3");
    mainStack->resize(800, 600);

    MenuWidget *menu = new MenuWidget();
    SnakeWidget *game = new SnakeWidget();

    QWidget *gameContainer = new QWidget();
    gameContainer->setStyleSheet("background-color: rgb(15, 15, 30);");
    QVBoxLayout *vLayout = new QVBoxLayout(gameContainer);
    QHBoxLayout *hLayout = new QHBoxLayout();

    hLayout->addStretch();
    hLayout->addWidget(game);
    hLayout->addStretch();

    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    vLayout->addStretch();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    mainStack->addWidget(menu);
    mainStack->addWidget(gameContainer);

    QObject::connect(menu, &MenuWidget::startGame, mainStack,
                     [game, gameContainer, mainStack](int level) {
                         Q_UNUSED(level);
                         game->startGameDirectly();
                         mainStack->setCurrentWidget(gameContainer);
                         game->setFocus();
                     });

    QObject::connect(menu, &MenuWidget::quitGame, mainStack, &QWidget::close);

    QObject::connect(game, &SnakeWidget::backToMenu, mainStack,
                     [menu, mainStack]() {
                         mainStack->setCurrentWidget(menu);
                         menu->setFocus();
                     });

    QObject::connect(game, &SnakeWidget::requestFullscreen, mainStack,
                     [mainStack](bool fullscreen) {
                         if (fullscreen) {
                             mainStack->showFullScreen();
                         } else {
                             mainStack->showNormal();
                         }
                     });

    QObject::connect(menu, &MenuWidget::requestFullscreen, mainStack,
                     [mainStack](bool fullscreen) {
                         if (fullscreen) {
                             mainStack->showFullScreen();
                         } else {
                             mainStack->showNormal();
                         }
                     });

    mainStack->show();
    return a.exec();
}
