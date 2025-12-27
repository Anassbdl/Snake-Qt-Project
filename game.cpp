#include "game.h"

Game::Game(QObject *parent)
    : QObject(parent),
    head(nullptr),
    length(0),
    direction(RIGHT),
    nextDirection(RIGHT),
    score(0),
    gameOver(false)
{
    reset();
}

Game::~Game()
{
    freeSnake();
}

SnakeNode *Game::createNode(int x, int y)
{
    SnakeNode *node = new SnakeNode;
    node->x = x;
    node->y = y;
    node->next = nullptr;
    return node;
}

void Game::freeSnake()
{
    SnakeNode *current = head;
    while (current != nullptr)
    {
        SnakeNode *tmp = current;
        current = current->next;
        delete tmp;
    }
    head = nullptr;
    length = 0;
}

void Game::addSegment(int x, int y)
{
    SnakeNode *newNode = createNode(x, y);
    if (!head)
    {
        head = newNode;
    }
    else
    {
        SnakeNode *cur = head;
        while (cur->next)
            cur = cur->next;
        cur->next = newNode;
    }
    ++length;
}

void Game::removeLastSegment()
{
    if (!head)
        return;
    if (!head->next)
    {
        delete head;
        head = nullptr;
    }
    else
    {
        SnakeNode *cur = head;
        while (cur->next->next)
            cur = cur->next;
        delete cur->next;
        cur->next = nullptr;
    }
    --length;
}

bool Game::isPositionOnSnake(int x, int y)
{
    SnakeNode *cur = head;
    while (cur)
    {
        if (cur->x == x && cur->y == y)
            return true;
        cur = cur->next;
    }
    return false;
}

bool Game::isPositionObstacle(int x, int y)
{
    for (const Obstacle &o : obstacles)
    {
        if (o.x == x && o.y == y)
            return true;
    }
    return false;
}

void Game::generateSingleFood(int index)
{
    int x, y;
    bool ok = false;
    while (!ok)
    {
        x = QRandomGenerator::global()->bounded(1, WIDTH - 1);
        y = QRandomGenerator::global()->bounded(1, HEIGHT - 1);
        ok = true;
        if (isPositionOnSnake(x, y))
            ok = false;
        if (isPositionObstacle(x, y))
            ok = false;

        for (int i = 0; i < FOOD_COUNT; ++i)
        {
            if (i == index) continue;
            if (food_x[i] == x && food_y[i] == y)
            {
                ok = false;
                break;
            }
        }
    }
    food_x[index] = x;
    food_y[index] = y;

    int randType = QRandomGenerator::global()->bounded(0, 3);
    food_type[index] = static_cast<FruitType>(randType);
}

void Game::generateFood()
{
    for (int i = 0; i < FOOD_COUNT; ++i)
        generateSingleFood(i);
}

void Game::generateObstacles()
{
    obstacles.clear();
    QRandomGenerator *rg = QRandomGenerator::global();
    const int nbObs = 8;
    for (int i = 0; i < nbObs; ++i)
    {
        int x, y;
        bool ok = false;
        while (!ok)
        {
            x = rg->bounded(2, WIDTH - 2);
            y = rg->bounded(2, HEIGHT - 2);
            ok = true;
            if (isPositionOnSnake(x, y))
                ok = false;
            if (isPositionObstacle(x, y))
                ok = false;
            for (int k = 0; k < FOOD_COUNT; ++k)
            {
                if (food_x[k] == x && food_y[k] == y)
                {
                    ok = false;
                    break;
                }
            }
        }
        obstacles.push_back({x, y});
    }
}

int Game::checkCollision()
{
    SnakeNode *h = head;
    if (!h)
        return 1;
    if (isPositionObstacle(h->x, h->y))
        return 1;

    SnakeNode *cur = head;
    if (cur && cur->next && cur->next->next)
        cur = cur->next->next;
    else
        return 0;

    while (cur)
    {
        if (h->x == cur->x && h->y == cur->y)
            return 1;
        cur = cur->next;
    }
    return 0;
}

int Game::checkFoodCollision()
{
    if (!head)
        return -1;
    for (int i = 0; i < FOOD_COUNT; ++i)
    {
        if (head->x == food_x[i] && head->y == food_y[i])
            return i;
    }
    return -1;
}

void Game::moveSnake()
{
    direction = nextDirection;
    int newX = head->x;
    int newY = head->y;

    switch (direction)
    {
    case UP: --newY; break;
    case DOWN: ++newY; break;
    case LEFT: --newX; break;
    case RIGHT: ++newX; break;
    }

    // wrapping
    if (newX < 0) newX = WIDTH - 1;
    if (newX >= WIDTH) newX = 0;
    if (newY < 0) newY = HEIGHT - 1;
    if (newY >= HEIGHT) newY = 0;

    SnakeNode *newHead = createNode(newX, newY);
    newHead->next = head;
    head = newHead;
    ++length;

    int foodIndex = checkFoodCollision();
    if (foodIndex != -1)
    {
        FruitType ft = food_type[foodIndex];
        if (ft == APPLE)
            score += 10;
        else if (ft == BANANA)
            score += 15;
        else if (ft == PINEAPPLE)
            score += 25;

        generateSingleFood(foodIndex);
    }
    else
    {
        removeLastSegment();
    }

    if (checkCollision())
        gameOver = true;
}

void Game::updateGame()
{
    if (!gameOver)
        moveSnake();
}

void Game::changeDirection(Direction dir)
{
    if ((direction == UP && dir == DOWN) ||
        (direction == DOWN && dir == UP) ||
        (direction == LEFT && dir == RIGHT) ||
        (direction == RIGHT && dir == LEFT))
        return;
    nextDirection = dir;
}

void Game::reset()
{
    freeSnake();
    head = createNode(WIDTH / 2, HEIGHT / 2);
    length = 1;
    direction = RIGHT;
    nextDirection = RIGHT;
    score = 0;
    gameOver = false;
    addSegment(WIDTH / 2 - 1, HEIGHT / 2);
    addSegment(WIDTH / 2 - 2, HEIGHT / 2);
    generateFood();
    generateObstacles();
}
