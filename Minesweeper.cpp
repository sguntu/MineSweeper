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

    loadAndSetImages();
    readLeaderBoard();
    //mines counter
    minesCounterSprite.reserve(3);
    for (int i = 0; i < 3; i++) {
        minesCounterSprite.emplace_back(minesCounter);
        minesCounterSprite[i].setTextureRect(
        sf::IntRect(sf::Vector2i{21, 0}, sf::Vector2i{21, 32}));
    }
    //Timer
    startTimer();

    grid.resize(rows);

    for (int gr = 0; gr < rows; ++gr) {
        grid.emplace_back();
        grid[gr].reserve(cols);
        for (int gc = 0; gc < cols; ++gc) {
            grid[gr].emplace_back(texHidden); // provide texture for each tile
            grid[gr][gc].sprite.setPosition(sf::Vector2f(gc * tileSize, gr * tileSize));
        }
    }

    srand(static_cast<unsigned>(time(nullptr)));
    placeMines();
    countAdjacentMines();
}

void Minesweeper::loadAndSetImages() {

    // Tile images
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

    //Bottom board images
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

    faceSprite = sf::Sprite(faceHappy);
    debugSprite = sf::Sprite(debug);
    playTypeSprite = sf::Sprite(pause);
    leaderboardSprite = sf::Sprite(leaderboard);
}
void Minesweeper::placeMines() {
    int placed = 0;
    while (placed < numMines) {
        int rr = rand() % rows;
        int rc = rand() % cols;
        if (!grid[rr][rc].isMine) {
            grid[rr][rc].isMine = true;
            ++placed;
        }
    }
}

void Minesweeper::countAdjacentMines() {
    for (int rr = 0; rr < rows; ++rr) {
        for (int cc = 0; cc < cols; ++cc) {
            if (grid[rr][cc].isMine) {
                continue;
            }

            int count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = rr + dr;
                    int nc = cc + dc;

                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        if (grid[nr][nc].isMine) ++count;
                    }
                }
            }
            grid[rr][cc].adjacentMines = count;
        }
    }
}

void Minesweeper::showTile(int ir, int ic) {
    if (ir < 0 || ir >= rows || ic < 0 || ic >= cols) return;
    Tile& t = grid[ir][ic];
    if (t.state == TileState::Revealed || t.state == TileState::Flagged) return;

    t.state = TileState::Revealed;
    t.sprite.setTexture(texRevealed);

    if (t.adjacentMines == 0 && !t.isMine) {
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                if (dr != 0 || dc != 0) {
                    showTile(ir + dr, ic + dc);
                }
    }
}

void Minesweeper::handleClick(sf::Vector2i mousePos, sf::Mouse::Button button, sf::String playerName) {

    if (!gameOver) {
        if (playTypeSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
            isPaused = !isPaused;
            if (isPaused) {
                timerRunning = false;
                pausedStart = std::chrono::steady_clock::now();
                pauseTimer();
                playTypeSprite.setTexture(play);
            }
            else {
                timerRunning = true;
                pausedDuration += std::chrono::steady_clock::now() - pausedStart;
                resumeTimer();
                playTypeSprite.setTexture(pause);
            }
            return;
        }

        const int c = mousePos.x / tileSize;
        const int r = mousePos.y / tileSize;
        if (r < 0 || r >= rows || c < 0 || c >= cols) return;

        Tile& t = grid[r][c];

        if (button == sf::Mouse::Button::Left) {
            if (t.isMine) {
                std::cout << "Game Over, it mine!\n";
                t.sprite.setTexture(texMine);
                timerRunning = false;
                gameOver = true;
                faceSprite.setTexture(faceLose);
                playTypeSprite.setTexture(play, true);
            } else {
                showTile(r, c);
                if (finishGame()) {
                    gameOver = true;
                    timerRunning = false;
                    faceSprite.setTexture(faceWin);
                    int timeTaken = static_cast<int>(elapsedTime.count());
                    std::cout << "Game Over!" << timeTaken << std::endl;
                    if (timeTaken < winnerTime) {
                        std::cout << "You are the new champion!\n";
                        updateLeaderboard(playerName, timeTaken);
                    }
                }
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
    if (faceSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
        resetGame();
    }

    if (debugSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
        resetGame();
    }
    if (leaderboardSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
        displayLeaderboard();
    }
}
void Minesweeper::startTimer() {
    startTime = std::chrono::steady_clock::now();
    pausedDuration = std::chrono::seconds(0);
    elapsedTime = std::chrono::seconds(0);
    timerRunning = true;
    isPaused = false;
}
void Minesweeper::pauseTimer() {
    if (timerRunning && !isPaused) {
        pausedStart = std::chrono::steady_clock::now();
        isPaused = true;
    }
}
void Minesweeper::resumeTimer() {
    if (timerRunning && isPaused) {
        auto now = std::chrono::steady_clock::now();
        pausedDuration += now - pausedStart;
        isPaused = false;
    }
}

void Minesweeper::updateTimer() {
    if (!timerRunning) {
        return;
    }

    if (!isPaused) {
        auto now = std::chrono::steady_clock::now();
        elapsedTime = now - startTime - pausedDuration;
    }
}

void Minesweeper::resetGame() {
    gameOver = false;
    isPaused = false;
    timerRunning = false;

    faceSprite.setTexture(faceHappy);

    // Timer
    elapsedTime = std::chrono::seconds(0);
    pausedDuration= std::chrono::seconds(0);
    timerRunning = true;
    isPaused = false;
    startTime = std::chrono::steady_clock::now();


    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            Tile& t = grid[r][c];
            t.isMine = false;
            t.state = TileState::Hidden;
            t.adjacentMines = 0;
            t.sprite.setTexture(texHidden);
        }
    }

    // clear tiles
    placeMines();
    countAdjacentMines();
}

void Minesweeper::setCounter(sf::Sprite& sprite, int digit) {
    int digitWidth = 21;
    int digitHeight = 32;
    sprite.setTextureRect(
            sf::IntRect(sf::Vector2i{digit * digitWidth, 0}, sf::Vector2i{digitWidth, digitHeight})
        );
}
bool Minesweeper::finishGame() {
    int openedTiles = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            Tile& t = grid[r][c];

            if (!t.isMine && t.state == TileState::Revealed)
                openedTiles++;
        }
    }

    return openedTiles == (rows * cols - numMines);
}
int Minesweeper::convertToSeconds(std::string& minutesecond) {
    int minutes = 0, seconds = 0;

    size_t colon = minutesecond.find(':');
    if (colon != std::string::npos) {
        minutes = std::stoi(minutesecond.substr(0, colon));
        seconds = std::stoi(minutesecond.substr(colon + 1));
    }

    return minutes * 60 + seconds;
}
void Minesweeper::readLeaderBoard() {
    std::ifstream file("../leaderboard.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open leaderboard file\n";
    }
    std::string players;

    int i = 1;
    while (std::getline(file, players)) {
        std::stringstream ss(players);
        std::string name, gameTime;
        std::getline(ss, gameTime, ',');
        std::getline(ss, name);
        if (i == 1) {
            winnerTime = convertToSeconds(gameTime);
        }
        winnerslist += gameTime + "\t" + name +"\n\n";//std::to_string(i) + ".\t" +
        ++i;
    }
    std::cout << winnerTime << std::endl;
    std::cout << winnerslist << std::endl;
}
void Minesweeper::updateLeaderboard(std::string playerName, int gameTime) {
    int minutes = gameTime / 60;
    int seconds = gameTime % 60;

    std::ostringstream minuteSeconds;
    minuteSeconds << std::setw(2) << std::setfill('0') << minutes
       << ":" << std::setw(2) << std::setfill('0') << seconds;

    std::size_t lastNewline = winnerslist.rfind("\n\n");
    std::size_t secondLastNewline = winnerslist.rfind("\n\n", lastNewline-1);
    std::string finalLeaderboard = minuteSeconds.str() + "\t" + playerName + "\n\n" + winnerslist.substr(0, secondLastNewline);
    std::cout << finalLeaderboard << std::endl;

    std::ofstream out("../leaderboard.txt", std::ios::trunc);
    out << finalLeaderboard << "\n";
}
void Minesweeper::displayLeaderboard() {
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

        sf::Text scores(font, winnerslist, 18);
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
   /* if (minesLeft < 0) minesLeft = 0;
    if (minesLeft > 999) minesLeft = 999;

    int hundreds = minesLeft/100;
    int tens = (minesLeft/10)%10;
    int ones = minesLeft%10;

    setCounter(minesCounterSprite[0], hundreds);
    setCounter(minesCounterSprite[1], tens);
    setCounter(minesCounterSprite[2], ones);

    for (int i =0; i < 3; i++){
        //std::cout << "in for loop to display mines count";
        minesCounterSprite[i].setPosition(sf::Vector2f(33.f+(i*21.f), 32.f*(rows+0.5f)+16.f));
        minesCounterSprite[i].setColor(sf::Color::Black);
        window.draw(minesCounterSprite[i]);
    }*/
    sf::Text minesCountText(font);
    minesCountText.setCharacterSize(26);
    minesCountText.setFillColor(sf::Color::Black);
    minesCountText.setString(std::to_string(minesLeft));
    minesCountText.setPosition(sf::Vector2f(33.f+21.f, 32.f*(rows+0.5f)+16.f));
    window.draw(minesCountText);

}