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

    QStackedWidget mainStack;
    mainStack.setWindowTitle("Snake - GI3");
    mainStack.resize(800, 600);

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

    mainStack.addWidget(menu);
    mainStack.addWidget(gameContainer);

    // Connexion pour démarrer le jeu
    QObject::connect(menu, &MenuWidget::startGame, game,
                     [game, gameContainer, &mainStack](int level) {
                         Q_UNUSED(level);
                         game->startGameDirectly();
                         mainStack.setCurrentWidget(gameContainer);
                         game->setFocus();
                     });

    // Connexion pour quitter
    QObject::connect(menu, &MenuWidget::quitGame, &mainStack, &QWidget::close);

    // Connexion pour retourner au menu
    QObject::connect(game, &SnakeWidget::backToMenu, menu,
                     [menu, &mainStack]() {
                         mainStack.setCurrentWidget(menu);
                         menu->setFocus();
                     });

    // **Connexion F11 depuis le JEU**
    QObject::connect(game, &SnakeWidget::requestFullscreen, &mainStack,
                     [&mainStack](bool fullscreen) {
                         if (fullscreen) {
                             mainStack.showFullScreen();
                         } else {
                             mainStack.showNormal();
                         }
                     });

    // **Connexion F11 depuis le MENU**
    QObject::connect(menu, &MenuWidget::requestFullscreen, &mainStack,
                     [&mainStack](bool fullscreen) {
                         if (fullscreen) {
                             mainStack.showFullScreen();
                         } else {
                             mainStack.showNormal();
                         }
                     });

    mainStack.show();
    return a.exec();
}
