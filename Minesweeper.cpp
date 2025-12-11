#include "Minesweeper.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>

#include "GameWindow.h"

Minesweeper::Minesweeper(int r, int c, int mines, int tSize)
    : rows(r), cols(c), numMines(mines), tileSize(tSize)
{
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

    // Number images
    for (int i = 1; i <= 8; i++) {
        std::string path = "../images/number_" + std::to_string(i) + ".png";
        if (!numberTextures[i].loadFromFile(path)) {
            std::cout << "Failed to load: " << path << "\n";
        }
    }

    //bottom images
    if (!faceHappy.loadFromFile("../images/face_happy.png"))
        std::cout << "Error loading face_happy.png\n";
    if (!faceLose.loadFromFile("../images/face_lose.png"))
        std::cout << "Error loading face_lose.png\n";
    if (!faceWin.loadFromFile("../images/face_win.png"))
        std::cout << "Error loading face_win.png\n";
    if (!debug.loadFromFile("../images/debug.png"))
        std::cout << "Error loading debug.png\n";
    if (!play.loadFromFile("../images/play.png"))
        std::cout << "Error loading play.png\n";
    if (!pause.loadFromFile("../images/pause.png"))
        std::cout << "Error loading pause.png\n";
    if (!leaderboard.loadFromFile("../images/leaderboard.png"))
        std::cout << "Error loading leaderboard.png\n";
    if (!minesCounter.loadFromFile("../images/digits.png"))
        std::cout << "Error loading digits.png\n";

    minesCounterSprite.reserve(3);
    for (int i = 0; i < 3; i++) {
        minesCounterSprite.emplace_back(minesCounter);
        minesCounterSprite[i].setTextureRect(
        sf::IntRect(sf::Vector2i{21, 0}, sf::Vector2i{21, 32}));
    }

    faceSprite = sf::Sprite(faceHappy);
    debugSprite = sf::Sprite(debug);
    playTypeSprite = sf::Sprite(play);
    leaderboardSprite = sf::Sprite(leaderboard);

    //Timer
    startTime = std::chrono::steady_clock::now();
    timerRunning = true;
    isPaused = false;
    pausedDuration = std::chrono::seconds(0);


    grid.resize(rows);

    for (int r = 0; r < rows; ++r) {
        grid.emplace_back();
        grid[r].reserve(cols);
        for (int c = 0; c < cols; ++c) {
            grid[r].emplace_back(texHidden); // provide texture for each tile
            grid[r][c].sprite.setPosition(sf::Vector2f(c * tileSize, r * tileSize));
        }
    }

    srand(static_cast<unsigned>(time(nullptr)));
    placeMines();
    calculateAdjacentMines();
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

void Minesweeper::calculateAdjacentMines() {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (grid[r][c].isMine) {
                continue;
            }

            int count = 0;
            for (int dr = -1; dr <= 1; ++dr)
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (grid[nr][nc].isMine) ++count;
                    }
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

void Minesweeper::handleClick(sf::Vector2i mousePos, sf::Mouse::Button button, sf::RenderWindow& window) {

    if (playTypeSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
        isPaused = !isPaused;
        if (isPaused) {
            pausedStart = std::chrono::steady_clock::now();
            playTypeSprite.setTexture(play);
        }
        else {
            pausedDuration += std::chrono::steady_clock::now() - pausedStart;
            playTypeSprite.setTexture(pause);
        }
        return;
    }

    if (faceSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
        resetGame();
    }

    if (leaderboardSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
        displayLeaderboard();
    }

    const int c = mousePos.x / tileSize;
    const int r = mousePos.y / tileSize;
    if (r < 0 || r >= rows || c < 0 || c >= cols) return;

    Tile& t = grid[r][c];

    if (button == sf::Mouse::Button::Left) {
        if (t.isMine) {
            std::cout << "Game Over!\n";
            t.sprite.setTexture(texMine);
            timerRunning = false;
            gameOver = true;
            faceSprite.setTexture(faceLose);
            playTypeSprite.setTexture(play, true);
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

void Minesweeper::updateTimer() {
    if (timerRunning && !isPaused) {
        const auto now = std::chrono::steady_clock::now();
        elapsedTime = now - startTime - pausedDuration;
    }else {
        elapsedTime = pausedStart - startTime - pausedDuration;
    }
}

void Minesweeper::resetGame() {
    placeMines();
    calculateAdjacentMines();
    gameOver = false;
    isPaused = false;
    timerRunning = false;
}

void Minesweeper::togglePlay() {


}

void Minesweeper::setCounter(sf::Sprite& sprite, int digit) {
    int digitWidth = 21;
    int digitHeight = 32;
    //sprite.setTextureRect({digit*digitWidth, 0, digitWidth, digitHeight});
    sprite.setTextureRect(
            sf::IntRect(sf::Vector2i{digit * digitWidth, 0}, sf::Vector2i{digitWidth, digitHeight})
        );
}

void Minesweeper::displayLeaderboard() {

    std::ifstream file("../leaderboard.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open leaderboard file\n";
    }

    std::string players;
    std::string plist;

    int i = 1;
    while (std::getline(file, players)) {
        std::stringstream ss(players);
        std::string name, score;
        std::getline(ss, name, ',');
        std::getline(ss, score);
        plist += std::to_string(i) + ".\t" + name + "\t" + score +"\n\n";
        i++;
    }

    std::cout << plist << std::endl;

    unsigned height = rows * 32 + 100;
    unsigned width  = cols * 32;

    sf::RenderWindow leaderBoardWindow(
        sf::VideoMode({static_cast<unsigned>(width/2.f), static_cast<unsigned>(height/2.f)}), "Minesweeper");

    while (leaderBoardWindow.isOpen()) {
        while (auto ev = leaderBoardWindow.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                leaderBoardWindow.close();
            }
            if (auto key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    leaderBoardWindow.close();
                }
            }
        }

        leaderBoardWindow.clear(sf::Color::Blue);

        sf::Text title(font, "LEADER BOARD", 20);
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold | sf::Text::Underlined);
        title.setPosition(sf::Vector2f{width/4.f-80.f, height/8.f-80.f});
        leaderBoardWindow.draw(title);

        sf::Text scores(font, plist, 18);
        scores.setFillColor(sf::Color::White);
        scores.setStyle(sf::Text::Bold);
        scores.setPosition(sf::Vector2f{width/4.f-100, height/8.f-20});
        leaderBoardWindow.draw(scores);

        leaderBoardWindow.display();
    }
}

void Minesweeper::draw(sf::RenderWindow& window) {

    updateTimer();
    // 1. Draw grid
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            window.draw(grid[r][c].sprite);

    // 2. Draw number overlays
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            Tile& t = grid[r][c];
            if (t.state == TileState::Revealed &&
                t.adjacentMines > 0 && !t.isMine)
            {
                sf::Sprite num(numberTextures[t.adjacentMines]);
                num.setPosition(sf::Vector2f{static_cast<float>(c * tileSize),
                    static_cast<float>(r * tileSize)});
                window.draw(num);
            }
        }

    // 3. Bottom bar (after grid)
    sf::RectangleShape bottomBar(
        { float(cols * tileSize), 100.f }
    );
    bottomBar.setFillColor(sf::Color::White);//(50, 150, 50));
    bottomBar.setPosition(sf::Vector2f{0.f, static_cast<float>(rows * tileSize)});
    window.draw(bottomBar);

    // 4. Draw face centered inside the bar
    faceSprite.setPosition(sf::Vector2f{cols*32/2.f-32.f, 32*(rows+0.5f)});
    window.draw(faceSprite);
    debugSprite.setPosition(sf::Vector2f(cols*32-304.f, 32*(rows+0.5f)));
    window.draw(debugSprite);
    playTypeSprite.setPosition(sf::Vector2f(cols*32-240.f, 32*(rows+0.5f)));
    window.draw(playTypeSprite);
    leaderboardSprite.setPosition(sf::Vector2f(cols*32-176.f, 32*(rows+0.5f)));
    window.draw(leaderboardSprite);

    // Draw timer
    int totalSeconds = static_cast<int>(elapsedTime.count());
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << minutes
       << ":" << std::setw(2) << std::setfill('0') << seconds;

    sf::Text timerText(font);
    timerText.setCharacterSize(26);
    timerText.setFillColor(sf::Color::Black);
    //std::cout << ss.str() << std::endl;
    timerText.setString(ss.str());
    timerText.setPosition(sf::Vector2f(cols*32-97.f, 32*(rows+0.5f)+16.f));
    window.draw(timerText);

    //Mines counter
    // Draw mines counter
    int flagsUsed = 0;
    for (auto& row : grid)
        for (auto& t : row)
            if (t.state == TileState::Flagged)
                flagsUsed++;

    int minesLeft = numMines - flagsUsed;
    if (minesLeft < 0) minesLeft = 0;
    if (minesLeft > 999) minesLeft = 999;

    int hundreds = minesLeft/100;
    int tens = (minesLeft/10)%10;
    int ones = minesLeft%10;
   // std::cout << hundreds << " " << tens << " " << ones << std::endl;

    setCounter(minesCounterSprite[0], hundreds);
    setCounter(minesCounterSprite[1], tens);
    setCounter(minesCounterSprite[2], ones);
   // int i =0;

//    for (auto& ds : minesCounterSprite) {
    for (int i =0; i < 3; i++){
        //std::cout << "in for loop to display mines count";
        minesCounterSprite[i].setPosition(sf::Vector2f(33.f+(i*21.f), 32.f*(rows+0.5f)+16.f));
        minesCounterSprite[i].setColor(sf::Color::Black);
        window.draw(minesCounterSprite[i]);
    }

}