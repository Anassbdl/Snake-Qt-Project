#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QVector>
#include <QRandomGenerator>

// Dimensions du jeu (en cases)
#define WIDTH 40
#define HEIGHT 25

enum Direction
{
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
};

enum FruitType
{
    APPLE,
    BANANA,
    PINEAPPLE
};

struct SnakeNode
{
    int x;
    int y;
    SnakeNode *next;
};

struct Obstacle
{
    int x;
    int y;
};

class Game : public QObject
{
    Q_OBJECT

public:
    static const int FOOD_COUNT = 3;

    explicit Game(QObject *parent = nullptr);
    ~Game();

    void reset();
    void updateGame();
    void changeDirection(Direction dir);

    // NOUVEAU : gestion du niveau
    void setLevel(int level);
    int getLevel() const { return currentLevel; }
    int getSpeed() const;  // Retourne la vitesse selon le niveau

    SnakeNode *snakeHead() const { return head; }
    int getScore() const { return score; }
    int getLength() const { return length; }
    int foodX(int i) const { return food_x[i]; }
    int foodY(int i) const { return food_y[i]; }
    FruitType foodType(int i) const { return food_type[i]; }
    int foodCount() const { return FOOD_COUNT; }
    bool isGameOver() const { return gameOver; }
    Direction getDirection() const { return direction; }
    const QVector<Obstacle> &getObstacles() const { return obstacles; }

    int getLastFruitEaten() const { return lastFruitEaten; }
    void clearLastFruitEaten() { lastFruitEaten = -1; }

signals:
    void fruitEaten(int x, int y, int points, FruitType type);

private:
    SnakeNode *head;
    int length;
    Direction direction;
    Direction nextDirection;
    int food_x[FOOD_COUNT];
    int food_y[FOOD_COUNT];
    FruitType food_type[FOOD_COUNT];
    int score;
    bool gameOver;
    QVector<Obstacle> obstacles;
    int lastFruitEaten;
    int currentLevel;  // NOUVEAU : niveau actuel (1, 2, ou 3)

    SnakeNode *createNode(int x, int y);
    void addSegment(int x, int y);
    void removeLastSegment();
    void freeSnake();
    void generateFood();
    void generateSingleFood(int index);
    void generateObstacles();
    bool isPositionObstacle(int x, int y);
    bool isPositionOnSnake(int x, int y);
    int checkCollision();
    int checkFoodCollision();
    void moveSnake();
};

#endif // GAME_H
