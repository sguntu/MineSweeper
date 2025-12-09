#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class TileState { Hidden, Revealed, Flagged };

struct Tile {
    bool isMine = false;
    int adjacentMines = 0;
    TileState state = TileState::Hidden;
    sf::Sprite sprite;

    // Explicit constructor that takes a texture
    Tile(const sf::Texture& texture) : sprite(texture) {}
};



class Minesweeper {
public:
    Minesweeper(int rows, int cols, int numMines, int tileSize = 32);

    void handleClick(sf::Vector2i mousePos, sf::Mouse::Button button);
    void draw(sf::RenderWindow& window);

private:
    int rows;
    int cols;
    int numMines;
    int tileSize;

    std::vector<std::vector<Tile>> grid;
    sf::Texture texHidden, texRevealed, texMine, texFlag;
    sf::Font font;
    sf::Texture numberTextures[9];   // index 1–8 used

    void placeMines();
    void calculateAdjacency();
    void revealTile(int r, int c);
};
