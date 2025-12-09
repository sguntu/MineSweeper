#include "Minesweeper.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>

Minesweeper::Minesweeper(int r, int c, int mines, int tSize)
    : rows(r), cols(c), numMines(mines), tileSize(tSize), faceSprite(faceHappy)
{
    //std::vector<Tile>(cols);
    std::cout << "Minesweeper created!" << rows << "x" << cols << std::endl;

   // faceSprite = sf::Sprite(faceHappy);
    //faceSprite.setTexture(faceHappy);
    // Load textures
    if (!texHidden.loadFromFile("../images/tile_hidden.png") ||
        !texRevealed.loadFromFile("../images/tile_revealed.png") ||
        !texMine.loadFromFile("../images/mine.png") ||
        !texFlag.loadFromFile("../images/flag.png") ||
        !font.openFromFile("../font.ttf"))
    {
        std::cerr << "Failed to load images!\n";
    }
    // Load number textures
    for (int i = 1; i <= 8; i++) {
        std::cout << i << std::endl;
        std::string path = "../images/number_" + std::to_string(i) + ".png";
        std::cout << path << std::endl;
        if (!numberTextures[i].loadFromFile(path)) {
            std::cout << "Failed to load numbers: " << path << "\n";
        }
    }

    if (!faceHappy.loadFromFile("../images/face_happy.png") ||
      !faceWin.loadFromFile("../images/face_win.png") ||
      !faceLose.loadFromFile("../images/face_lose.png"))
    {
        std::cerr << "Error loading face textures.\n";
    }
    faceSprite.setTexture(faceHappy);
    faceSprite.setPosition(sf::Vector2f((cols * tileSize) / 2.f - 16.f, 5.f));
    faceBounds = faceSprite.getGlobalBounds();

    grid.resize(rows);

    for (int r = 0; r < rows; ++r) {
        grid.emplace_back();
        grid[r].reserve(cols);
        for (int c = 0; c < cols; ++c) {
            grid[r].emplace_back(texHidden); // provide texture for each tile
            grid[r][c].sprite.setPosition(sf::Vector2f(c * tileSize, r * tileSize));
        }
    }


    // Initialize sprites
    /*for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            grid[r][c].sprite.setTexture(texHidden);
            //grid[r][c].sprite.setPosition(c * tileSize, r * tileSize);
            grid[r][c].sprite.setPosition(sf::Vector2f(c * tileSize, r * tileSize)); // ✅

        }*/

    srand(static_cast<unsigned>(time(nullptr)));
    placeMines();
    calculateAdjacency();
}

void Minesweeper::placeMines() {
    int placed = 0;
    while (placed < numMines) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (!grid[r][c].isMine) {
            grid[r][c].isMine = true;
            ++placed;
        }
    }
}

void Minesweeper::calculateAdjacency() {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (grid[r][c].isMine) continue;

            int count = 0;
            for (int dr = -1; dr <= 1; ++dr)
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols)
                        if (grid[nr][nc].isMine) ++count;
                }
            grid[r][c].adjacentMines = count;
        }
    }
}

bool Minesweeper::checkWin() {
    for (auto &row : grid) {
        for (auto &t : row) {
            if (!t.isMine && t.state != TileState::Revealed)
                return false;
        }
    }
    return true;
}

void Minesweeper::revealTile(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;

    Tile &t = grid[r][c];
    if (t.state != TileState::Hidden) return;

    t.state = TileState::Revealed;
    t.sprite.setTexture(texRevealed);

    if (t.isMine) {
        t.sprite.setTexture(texMine);
        gameOver = true;
        faceState = FaceState::Lose;
        faceSprite.setTexture(faceLose);
        return;
    }

    if (t.adjacentMines == 0) {
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                if (dr != 0 || dc != 0)
                    revealTile(r + dr, c + dc);
    }

    if (checkWin()) {
        gameOver = true;
        faceState = FaceState::Win;
        faceSprite.setTexture(faceWin);
    }
}


void Minesweeper::handleClick(sf::Vector2i mouse, sf::Mouse::Button btn) {
    if (gameOver) return;

    // Check if face clicked (reset)
    if (faceBounds.contains(static_cast<sf::Vector2f>(mouse))) {
        gameOver = false;
        for (auto &row : grid)
            for (auto &t : row) {
                t.isMine = false;
                t.adjacentMines = 0;
                t.state = TileState::Hidden;
                t.sprite.setTexture(texHidden);
            }
        placeMines();
        calculateAdjacency();
        clock.restart();
        elapsedSeconds = 0;
        faceState = FaceState::Happy;
        faceSprite.setTexture(faceHappy);
        return;
    }

    int r = (mouse.y - 50) / tileSize;
    int c = mouse.x / tileSize;
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;

    Tile &t = grid[r][c];

    if (btn == sf::Mouse::Button::Left) {
        revealTile(r, c);
    }
    else if (btn == sf::Mouse::Button::Right) {
        if (t.state == TileState::Hidden) {
            t.state = TileState::Flagged;
            t.sprite.setTexture(texFlag);
        } else if (t.state == TileState::Flagged) {
            t.state = TileState::Hidden;
            t.sprite.setTexture(texHidden);
        }
    }
}

void Minesweeper::updateTimer() {
    if (!gameOver) {
        elapsedSeconds = static_cast<int>(clock.getElapsedTime().asSeconds());
    }
}

void Minesweeper::draw(sf::RenderWindow &window) {
    for (auto &row : grid) {
        for (auto &t : row) {
            window.draw(t.sprite);
            if (t.state == TileState::Revealed &&
                t.adjacentMines > 0 && !t.isMine)
            {
                sf::Sprite num(numberTextures[t.adjacentMines]);
                num.setPosition(t.sprite.getPosition());
                window.draw(num);
            }
        }
    }

    window.draw(faceSprite);

  //  static sf::Font font("../font.ttf");
    sf::Text timerText(font);
    timerText.setCharacterSize(20);
    timerText.setFillColor(sf::Color::White);
    timerText.setString(std::to_string(elapsedSeconds));
    timerText.setPosition(sf::Vector2f(5.f, 5.f));

    window.draw(timerText);
}


