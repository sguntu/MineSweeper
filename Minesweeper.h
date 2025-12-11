#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>

enum class TileState { Hidden, Revealed, Flagged };
enum class FaceState { Happy, Win, Lose };
enum class PlayType { Play, Pause };

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

    void handleClick(sf::Vector2i mousePos, sf::Mouse::Button button, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    void displayLeaderboard();
private:
    int rows;
    int cols;
    int numMines;
    int tileSize;

    std::vector<std::vector<Tile>> grid;
    sf::Texture texHidden, texRevealed, texMine, texFlag;
    sf::Font font;
    sf::Texture numberTextures[9];

    // Face button & state
    sf::Texture faceHappy, faceWin, faceLose;
    sf::Texture debug, play, pause, leaderboard;
    sf::Sprite faceSprite = sf::Sprite(faceHappy);
    sf::Sprite debugSprite = sf::Sprite(debug);
    sf::Sprite playTypeSprite = sf::Sprite(play);
    //sf::Sprite pauseSprite = sf::Sprite(pause);
    sf::Sprite leaderboardSprite = sf::Sprite(leaderboard);
    //sf::Sprite digitSprite = sf::Sprite(digits);

    FaceState faceState = FaceState::Happy;
    PlayType playType = PlayType::Play;

    sf::FloatRect faceBounds;

    //Timer
    std::chrono::steady_clock::time_point startTime;
    std::chrono::duration<float> elapsedTime;
    bool timerRunning = true;
    bool gameOver = false;

    std::chrono::steady_clock::time_point pausedStart;
    std::chrono::duration<float> pausedDuration = std::chrono::seconds(0);
    bool isPaused = false;

    //Mines counter
    sf::Texture minesCounter;
    std::vector<sf::Sprite> minesCounterSprite;

    void placeMines();
    void calculateAdjacentMines();
    void revealTile(int r, int c);
    void updateTimer();
    void togglePlay();
    void resetGame();
    void setCounter(sf::Sprite& sprite, int digit);
};