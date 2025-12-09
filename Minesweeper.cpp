#include "Minesweeper.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>

Minesweeper::Minesweeper(int r, int c, int mines, int tSize)
    : rows(r), cols(c), numMines(mines), tileSize(tSize)
{
    //std::vector<Tile>(cols);
    std::cout << "Minesweeper created!" << rows << "x" << cols << std::endl;


    // Load textures
    if (!texHidden.loadFromFile("../images/tile_hidden.png") ||
        !texRevealed.loadFromFile("../images/tile_revealed.png") ||
        !texMine.loadFromFile("../images/mine.png") ||
        !texFlag.loadFromFile("../images/flag.png") ||
        !font.openFromFile("../font.ttf"))
    {
        std::cerr << "Failed to load assets!\n";
    }
    // Load number textures
    for (int i = 1; i <= 8; i++) {
        std::cout << i << std::endl;
        std::string path = "../images/number_" + std::to_string(i) + ".png";
        std::cout << path << std::endl;
        if (!numberTextures[i].loadFromFile(path)) {
            std::cout << "⚠ Failed to load: " << path << "\n";
        }
    }
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

void Minesweeper::revealTile(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;
    Tile& t = grid[r][c];
    if (t.state == TileState::Revealed || t.state == TileState::Flagged) return;

    t.state = TileState::Revealed;
    t.sprite.setTexture(texRevealed);

    // Auto reveal empty neighbors
    if (t.adjacentMines == 0 && !t.isMine) {
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                if (dr != 0 || dc != 0) revealTile(r + dr, c + dc);
    }
}

void Minesweeper::handleClick(sf::Vector2i mousePos, sf::Mouse::Button button) {
    int c = mousePos.x / tileSize;
    int r = mousePos.y / tileSize;
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;

    Tile& t = grid[r][c];

    if (button == sf::Mouse::Button::Left) {
        if (t.isMine) {
            std::cout << "Game Over!\n";
            t.sprite.setTexture(texMine);
        } else {
            revealTile(r, c);
        }
    }
    else if (button == sf::Mouse::Button::Right) {
        if (t.state == TileState::Hidden) {
            t.state = TileState::Flagged;
            t.sprite.setTexture(texFlag);
        } else if (t.state == TileState::Flagged) {
            t.state = TileState::Hidden;
            t.sprite.setTexture(texHidden);
        }
    }
}

void Minesweeper::draw(sf::RenderWindow& window) {
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            window.draw(grid[r][c].sprite);

            // Draw number if revealed
            Tile& t = grid[r][c];
            if (t.state == TileState::Revealed && t.adjacentMines > 0 && !t.isMine) {
                std::cout << "revealed" << t.adjacentMines;
                sf::Sprite numSprite(numberTextures[t.adjacentMines]);

                numSprite.setPosition(
                    sf::Vector2f(c * tileSize, r * tileSize)
                );
                window.draw(numSprite);
            }
        }
}


