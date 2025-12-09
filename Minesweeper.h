#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class TileState { Hidden, Revealed, Flagged };
enum class FaceState { Happy, Win, Lose };

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
    void updateTimer();
    void resetTimer();

private:
    int rows;
    int cols;
    int numMines;
    int tileSize;

    std::vector<std::vector<Tile>> grid;
    sf::Texture texHidden, texRevealed, texMine, texFlag;
    sf::Font font;
    sf::Texture numberTextures[9];   // index 1–8 used

    // Face button & state
    sf::Texture faceHappy, faceWin, faceLose;
    sf::Sprite faceSprite;
    FaceState faceState = FaceState::Happy;
    sf::FloatRect faceBounds;

    // Timer
    sf::Clock clock;
    int elapsedSeconds = 0;
    bool gameOver = false;

    void placeMines();
    void calculateAdjacency();
    void revealTile(int r, int c);
    bool checkWin();
};
