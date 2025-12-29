# 🐍 PRÉSENTATION DÉTAILLÉE DU JEU SNAKE EN Qt

## Vue d'ensemble

Le jeu **SNAKE** est une application Qt (C++) qui implémente le jeu classique du serpent avec :

- **3 niveaux de difficulté** (Facile, Moyen, Difficile)
- **3 types de fruits** (Pomme, Banane, Ananas) avec des points différents
- **Obstacles** qui augmentent avec la difficulté
- **Bordures qui se lient** (wrap-around) : sortir d'un côté = entrer de l'autre
- **Système de pause** et menu principal
- **Meilleur score** mémorisation
- **Interface graphique professionnelle** avec animations

---

## Architecture Générale

### Composants Principaux

```
APPLICATION
    ├── QApplication (Point d'entrée)
    └── QStackedWidget (Gestionnaire d'écrans)
            ├── MenuWidget (Menu principal)
            └── GameContainer + SnakeWidget (Jeu)
                    └── Game (Logique de jeu)
```

### Hiérarchie des Classes

```
QObject
├── Game (Logique métier)
│   ├── Gestion du serpent
│   ├── Gestion des fruits
│   ├── Gestion des obstacles
│   └── Détection des collisions
│
QWidget
├── MenuWidget (Interface du menu)
│   ├── Boutons (Jouer, Niveau, Quitter)
│   └── Sélection du niveau
│
└── SnakeWidget (Interface du jeu)
    ├── Rendu du jeu (paintEvent)
    ├── Gestion des entrées (keyPressEvent)
    └── Gestion des boutons (Pause, GameOver)
```

---

### 📄 **game.h** - Déclaration de la logique de jeu

**Responsabilités:**

- Définition des structures de données
- Déclaration des enums (Direction, FruitType)
- Interface publique pour la gestion du jeu

**Contenu principal:**

```cpp
struct SnakeNode {
    int x, y;
    SnakeNode *next;  // Liste chaînée
};

SnakeNode *Game::createNode(int x, int y)
{
    SnakeNode *node = new SnakeNode;
    node->x = x;
    node->y = y;
    node->next = nullptr;
    return node;
}

struct Obstacle {
    int x, y;
};

enum Direction { UP, DOWN, LEFT, RIGHT };
enum FruitType { APPLE, BANANA, PINEAPPLE };

class Game : public QObject {
    // Gestion du serpent
    // Gestion des fruits (3 fruits simultanés)
    // Gestion des obstacles
    // Détection des collisions
    // Système de niveaux
};
```

**Dimensions du terrain:**

- **Largeur:** 40 cases
- **Hauteur:** 25 cases

**Constantes:**

- `FOOD_COUNT = 3` : Toujours 3 fruits à l'écran

---

### 🔧 **game.cpp** - Implémentation de la logique de jeu

#### 1. **Initialisation du serpent**

```cpp
void Game::reset() {
    // Crée la tête au centre: (20, 12)
    head = createNode(WIDTH/2, HEIGHT/2);  // (20, 12)

    // Ajoute 2 segments supplémentaires (queue)
    addSegment(19, 12);   // Derrière la tête
    addSegment(18, 12);   // Queue

    length = 3;  // Longueur initiale
}
```

**Structure initiale:**

```
[Tête(20,12)] → [Segment(19,12)] → [Segment(18,12)]
     ↓                                        ↓
   Direction: DROITE              next = nullptr (queue)
```

---

#### 2. **Génération des fruits**

**Processus:** `generateFood()` → `generateSingleFood(index)`

```cpp
void Game::generateSingleFood(int index) {
    int x, y;
    bool ok = false;

    // Boucle jusqu'à trouver une position valide
    while (!ok) {
        x = QRandomGenerator::global()->bounded(1, WIDTH - 1);   // 1 à 38
        y = QRandomGenerator::global()->bounded(1, HEIGHT - 1);  // 1 à 23

        ok = true;

        // Vérifications
        if (isPositionOnSnake(x, y))      // ❌ Sur le serpent
            ok = false;
        if (isPositionObstacle(x, y))     // ❌ Sur un obstacle
            ok = false;

        // Vérifie qu'aucun autre fruit n'est à cette position
        for (int i = 0; i < FOOD_COUNT; ++i) {
            if (i == index) continue;
            if (food_x[i] == x && food_y[i] == y) {
                ok = false;
                break;
            }
        }
    }

    // Sauvegarde la position et le type
    food_x[index] = x;
    food_y[index] = y;

    // Assigne le type selon l'index
    if (index == 0) food_type[index] = APPLE;       // 10 points
    else if (index == 1) food_type[index] = BANANA; // 15 points
    else food_type[index] = PINEAPPLE;              // 25 points
}
```

**Fruits disponibles:**
| Fruit | Points | Couleur |
|-------|--------|---------|
| 🍎 Pomme | 10 | Rouge |
| 🍌 Banane | 15 | Jaune |
| 🍍 Ananas | 25 | Orange |

**Règles de génération:**

- Position aléatoire dans les limites (1 à WIDTH-1, 1 à HEIGHT-1)
- Pas sur le serpent
- Pas sur un obstacle
- Pas en double (chaque fruit unique)
- La génération réessaie jusqu'à trouver une position valide

---

**Tableau Statique pour les Fruits**

```cpp
class Game {
    static const int FOOD_COUNT = 3;

    int food_x[FOOD_COUNT];       // Positions X
    int food_y[FOOD_COUNT];       // Positions Y
    FruitType food_type[FOOD_COUNT]; // Types
};
```

**Avantages:**

- ✅ Accès O(1) direct
- ✅ Mémoire fixe et prédictible
- ✅ Cache-friendly
- ✅ Simple à gérer

---

**Score total:**

```
Score = Σ(points de chaque fruit mangé)
```

**Meilleur score:**

- Mémorisé pendant la session de jeu
- Réinitialisé si l'application est fermée
- Affiché dans le HUD

---

#### 3. **Mouvement du serpent et consommation**

**Opérations:**

```cpp
// Ajouter un segment
void Game::addSegment(int x, int y) {
    SnakeNode *newNode = createNode(x, y);
    if (!head) head = newNode;
    else {
        SnakeNode *cur = head;
        while (cur->next) cur = cur->next;  // Trouve la queue
        cur->next = newNode;
    }
    ++length;
}

// Retirer le dernier segment
void Game::removeLastSegment() {
    if (!head) return;
    if (!head->next) {
        delete head;
        head = nullptr;
    } else {
        SnakeNode *cur = head;
        while (cur->next->next) cur = cur->next;  // Prédécesseur
        delete cur->next;
        cur->next = nullptr;
    }
    --length;
}
```

---

**Fonction clé:** `moveSnake()`

```cpp
void Game::moveSnake() {
    direction = nextDirection;  // Applique la direction suivante

    // Calcule la nouvelle position de la tête
    int newX = head->x;
    int newY = head->y;

    switch (direction) {
        case UP:    --newY; break;
        case DOWN:  ++newY; break;
        case LEFT:  --newX; break;
        case RIGHT: ++newX; break;
    }

    // ⭕ BORDURES QUI SE LIENT (Wrap-around)
    if (newX < 0)      newX = WIDTH - 1;      // Gauche → Droite
    if (newX >= WIDTH) newX = 0;              // Droite → Gauche
    if (newY < 0)      newY = HEIGHT - 1;     // Haut → Bas
    if (newY >= HEIGHT) newY = 0;             // Bas → Haut

    // Crée une nouvelle tête
    SnakeNode *newHead = createNode(newX, newY);
    newHead->next = head;  // Lie à l'ancienne tête
    head = newHead;        // Nouvelle tête
    ++length;              // Temporairement, longueur augmente

    // 🍎 Détecte la collision avec un fruit
    int foodIndex = checkFoodCollision();

    if (foodIndex != -1) {
        // ✅ ALIMENT MANGÉ !
        FruitType ft = food_type[foodIndex];
        int points = 0;

        if (ft == APPLE)      points = 10;
        else if (ft == BANANA) points = 15;
        else if (ft == PINEAPPLE) points = 25;

        score += points;

        // 📢 Signal émis pour l'animation des points
        emit fruitEaten(food_x[foodIndex], food_y[foodIndex], points, ft);

        // 🆕 Génère un nouvel aliment
        generateSingleFood(foodIndex);

        // Le serpent garde la nouvelle tête (+1 segment)
        // La longueur reste augmentée

    } else {
        // ❌ Pas d'aliment mangé
        // Retire le dernier segment pour garder la longueur
        removeLastSegment();
    }

    // Vérifie si le serpent s'est écrasé
    if (checkCollision()) {
        gameOver = true;
    }
}
```

**Flux de mouvement:**

```
Itération N:
  Tête = [3, 5], Direction = DROITE
  Segments = [3,5] → [2,5] → [1,5]

  ↓ moveSnake() appelé

  Nouvelle tête = [4, 5]
  Segments = [4,5] → [3,5] → [2,5] → [1,5]   (longueur temporaire = 4)

  ↓ Pas d'aliment

  Retire la queue
  Segments = [4,5] → [3,5] → [2,5]           (longueur = 3)
```

**Consommation d'un fruit:**

```
Avant:
  Segments = [20,10] → [19,10] → [18,10]
  Fruit à [21,10]

  ↓ moveSnake() et collision détectée

Après:
  Segments = [21,10] → [20,10] → [19,10] → [18,10]
  ✅ Longueur = 4 (gain de 1 segment)
  ✅ Points ajoutés au score
  ✅ Nouveau fruit généré
```

---

#### 4. **Génération des obstacles**

```cpp
void Game::generateObstacles() {
    obstacles.clear();

    int nbObs;  // Nombre d'obstacles selon le niveau
    switch (currentLevel) {
        case 1: nbObs = 5;   break;   // Facile
        case 2: nbObs = 8;   break;   // Moyen
        case 3: nbObs = 12;  break;   // Difficile
        default: nbObs = 8;  break;
    }

    // Place les obstacles aléatoirement
    for (int i = 0; i < nbObs; ++i) {
        int x, y;
        bool ok = false;

        while (!ok) {
            x = rg->bounded(2, WIDTH - 2);    // 2 à 37
            y = rg->bounded(2, HEIGHT - 2);   // 2 à 22

            ok = true;

            // Vérifie que la position est libre
            if (isPositionOnSnake(x, y))
                ok = false;
            if (isPositionObstacle(x, y))
                ok = false;

            // Vérifie pas sur un fruit
            for (int k = 0; k < FOOD_COUNT; ++k) {
                if (food_x[k] == x && food_y[k] == y) {
                    ok = false;
                    break;
                }
            }
        }

        obstacles.push_back({x, y});
    }
}
```

📊 **Système de Niveaux**

**3 niveaux disponibles :**

| Niveau | Difficulté   | Vitesse | Obstacles | Score Cible   |
| ------ | ------------ | ------- | --------- | ------------- |
| 1      | 🟢 Facile    | 200ms   | 5         | Débutants     |
| 2      | 🟡 Moyen     | 140ms   | 8         | Intermédiaire |
| 3      | 🔴 Difficile | 90ms    | 12        | Experts       |

**Impact:**

- **Vitesse** : Affecte le timer du `gameLoop()`
  - Niveau 1 : 5 mouvements/seconde
  - Niveau 2 : 7 mouvements/seconde
  - Niveau 3 : 11 mouvements/seconde
- **Obstacles** : Générés au démarrage selon le niveau
  - Bloquent les mouvements
  - Causent Game Over si heurtés
  - Augmentent la difficulté

---

#### 5. **Détection des collisions**

```cpp
int Game::checkCollision() {
    SnakeNode *h = head;

    // Tête = nullptr (erreur)
    if (!h) return 1;  // Game Over

    // Tête heurte un obstacle
    if (isPositionObstacle(h->x, h->y)) return 1;  // Game Over

    // Vérifie l'auto-collision (serpent qui se mord lui-même)
    // Commence à partir du 3e segment (la tête ne peut pas
    // heurter ses 2 premiers segments en 1 mouvement)
    SnakeNode *cur = head;
    if (cur && cur->next && cur->next->next)
        cur = cur->next->next;  // Saute head et son premier voisin
    else
        return 0;  // Pas assez de segments

    // Vérifie si la tête heurte l'un des segments
    while (cur) {
        if (h->x == cur->x && h->y == cur->y)
            return 1;  // Auto-collision détectée → Game Over
        cur = cur->next;
    }

    return 0;  // Pas de collision
}
```

**Cas de Game Over:**

1. ❌ Tête heurte un obstacle
2. ❌ Tête heurte le corps du serpent (sauf les 2 premiers segments)
3. ✅ Les bordures NE causent PAS de Game Over (wrap-around)

---

### 🎨 **menuwidget.h / menuwidget.cpp** - Menu principal

**Responsabilités:**

- Affichage du menu principal
- Sélection du niveau
- Navigation vers le jeu

**Interface du menu:**

```
╔════════════════════════════════════╗
║        SNAKE - GI3                 ║
║     Menu Principal                 ║
║                                    ║
║        [ JOUER ]                   ║
║        [ LEVEL : 1 ]               ║
║        [ QUITTER ]                 ║
║                                    ║
║  F11 : Plein écran                 ║
╚════════════════════════════════════╝
```

**Fonctionnalités:**

- Bouton "JOUER" : Lance le jeu avec le niveau sélectionné
- Bouton "LEVEL" : Bascule entre les niveaux (1 → 2 → 3 → 1)
- Bouton "QUITTER" : Ferme l'application
- F11 : Bascule plein écran
- Arrière-plan avec grille de style

**Signaux émis:**

- `startGame(int level)` : Commence le jeu avec le niveau
- `quitGame()` : Ferme l'application
- `requestFullscreen(bool)` : Demande le mode plein écran

---

#### **Gestion des entrées (keyPressEvent)**

```cpp
void SnakeWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        // Direction du serpent
        case Qt::Key_Up:     game.changeDirection(UP);    break;
        case Qt::Key_Down:   game.changeDirection(DOWN);  break;
        case Qt::Key_Left:   game.changeDirection(LEFT);  break;
        case Qt::Key_Right:  game.changeDirection(RIGHT); break;

        // Pause
        case Qt::Key_P:  togglePause();  break;

        // Menu
        case Qt::Key_Escape:  emit backToMenu();  break;

        // Plein écran
        case Qt::Key_F11:  toggleFullscreen();  break;
    }
}
```

**Contrôles:**
| Touche | Action |
|--------|--------|
| ⬆️ Flèche haut | Serpent vers le haut |
| ⬇️ Flèche bas | Serpent vers le bas |
| ⬅️ Flèche gauche | Serpent vers la gauche |
| ➡️ Flèche droite | Serpent vers la droite |
| P | Pause / Reprendre |
| ESC | Retour au menu |
| F11 | Plein écran |

**Sécurité des mouvements:**

```cpp
void Game::changeDirection(Direction dir) {
    // Empêche le serpent de faire demi-tour immédiatement
    if ((direction == UP && dir == DOWN) ||
        (direction == DOWN && dir == UP) ||
        (direction == LEFT && dir == RIGHT) ||
        (direction == RIGHT && dir == LEFT))
        return;  // Ignore le mouvement impossible

    nextDirection = dir;  // Applique au prochain cycle
}
```

---

**Boutons Game Over (affichés quand gameOver == true):**

- "REJOUER" : Recommence le jeu (reset)
- "MENU" : Retour au menu

```
game.updateGame() détecte collision
    ↓
gameOver = true
    ↓
gameLoop() s'arrête (timer.stop())
    ↓
setupGameOverButtons() affiche
    ├─ "REJOUER"
    └─ "MENU"
    ↓
Utilisateur choisit une action
    ├─ "REJOUER" → startGameDirectly()  (Nouvelle partie)
    └─ "MENU" → emit backToMenu()  (Retour)
```

---

**Boutons Pause (affichés quand isPaused == true):**

- "REPRENDRE" : Continue le jeu
- "RECOMMENCER" : Recommence le jeu
- "MENU PRINCIPAL" : Retour au menu

```
Utilisateur appuie P
    ↓
togglePause()
    ├─ isPaused = true
    ├─ timer.stop()  // Arrête la boucle
    └─ setupPauseButtons()  // Affiche les boutons
    ↓
paintEvent() affiche voile + "PAUSE"
    ↓
Utilisateur clique "REPRENDRE"
    ↓
togglePause()
    ├─ isPaused = false
    ├─ hidePauseButtons()
    └─ timer.start()  // Reprend la boucle
```

---

**Boucle de jeu (Game Loop)**

**Chaque `game.getSpeed()` millisecondes :**

```
gameLoop() appelé (via timer)
    ↓
game.updateGame()
    ├─ game.moveSnake()
    │   ├─ direction = nextDirection
    │   ├─ Calcule nouvelle tête
    │   ├─ Applique wrap-around
    │   ├─ Ajoute nouvelle tête
    │   ├─ Détecte collision fruit
    │   │   ├─ OUI → Pas de removeLastSegment()
    │   │   │       Génère nouveau fruit
    │   │   │       Émet signal fruitEaten()
    │   │   └─ NON → removeLastSegment()
    │   └─ Détecte collision (obstacles, self)
    │       └─ OUI → gameOver = true
    └─ update()  (Demande un repaint)
    ↓
paintEvent() appelé
    ├─ Dessine le fond
    ├─ Dessine les fruits
    ├─ Dessine le serpent
    ├─ Dessine les obstacles
    ├─ Dessine les popups
    └─ Dessine le HUD
    ↓
Si gameOver : affiche boutons Game Over
```

---

## Évènements Importants

### **Signal `fruitEaten(x, y, points, type)`**

Émis quand le serpent mange un fruit.

```cpp
void SnakeWidget::onFruitEaten(int x, int y, int points, FruitType type) {
    // Crée un popup d'animation
    ScorePopup popup;
    popup.x = x;
    popup.y = y;
    popup.points = points;
    popup.alpha = 255;
    popup.offsetY = 0;
    popup.fruitType = type;

    scorePopups.append(popup);
}
```

**Animations:**

- Popup "+10", "+15", "+25" au-dessus du fruit
- Remonte progressivement pendant 3 secondes
- Transparence en augmentation
- Couleur selon le fruit

---

## Résumé Exécutif

### **Ce que le code fait bien :**

✅ Mécaniques de jeu solides et précises
✅ Système de niveaux fonctionnel
✅ Interface Qt élégante et réactive
✅ Structure orientée objet claire
✅ Gestion mémoire propre (listes chaînées, vecteurs)
✅ Bordures qui se lient (pas de murs)
✅ Pause et reprise fluides
✅ Animations de score visuellement attrayantes

### **Éléments éducatifs importants :**

📚 **Listes chaînées** : Implémentation classique pour le serpent
📚 **Tableaux statiques** : Gestion simple des fruits
📚 **Vecteurs Qt** : Obstacles dynamiques
📚 **Programmation orientée objet** : Séparation logique/UI
📚 **Signaux/Slots Qt** : Communication inter-composants
📚 **Gestion d'événements** : Clavier, souris, timer
📚 **Détection de collisions** : Algorithme O(n)
📚 **Rendu graphique** : Utilisation de QPainter

---

## Questions Fréquentes

**Q: Pourquoi une liste chaînée pour le serpent ?**
R: Elle permet une insertion O(1) en tête (nouvelle tête) et une suppression O(n) à la queue. C'est efficace pour ce cas d'usage.

**Q: Comment le wrap-around fonctionne ?**
R: Avant de créer la nouvelle tête, les coordonnées sont validées avec `if (x < 0) x = WIDTH-1`. Pas de Game Over.

**Q: Pourquoi 3 fruits simultanés ?**
R: Ça augmente la difficulté et offre plus de choix au joueur. Chaque fruit a une valeur différente.

**Q: Comment eviter l'auto-collision ?**
R: Le check commence au 3e segment (pas les 2 premiers) car en 1 mouvement, la tête ne peut pas heurter les segments immédiats.

**Q: Comment les obstacles affectent la difficulté ?**
R: Plus de obstacles = moins d'espace. Niveau 3 en a 12 sur un terrain 40×25.

**Q: Le score est-il persistant ?**
R: Non, c'est une session. Pour la persistance, il faudrait un fichier ou une base de données.

---

**Merci Pour votre attention**

**Snake-GI3**
