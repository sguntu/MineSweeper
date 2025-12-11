#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <string>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Minesweeper.h"

void setText(sf::Text &text, float a, float b) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(sf::Vector2f(
        textRect.position.x + textRect.size.x / 2.0f,
        textRect.position.y + textRect.size.y / 2.0f
    ));
    text.setPosition(sf::Vector2f(a, b));
}

enum class AppState {
    NameInput,
    Game
};

int main() {

    int numOfColumns = 0;
    int numOfRows = 0;
    int numOfMines = 0;

    AppState state = AppState::NameInput;

    sf::Font font("../font.ttf");
    std::ifstream configFile("../config.cfg");
    if (configFile) {
        configFile >> numOfColumns >> numOfRows >> numOfMines;
    }

    unsigned height = numOfRows * 32 + 100;
    unsigned width  = numOfColumns * 32;

    sf::RenderWindow window(sf::VideoMode({width, height}), "Minesweeper");

    sf::Text text(font, "WELCOME TO MINESWEEPER!", 24);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    setText(text, width / 2.f, height / 2.f - 150);

    sf::Text label(font, "Enter your name:", 20);
    label.setFillColor(sf::Color::White);
    label.setStyle(sf::Text::Bold);
    setText(label, width / 2.f, height / 2.f - 75);

    std::string input;

    sf::Text userip(font, "|", 18);
    userip.setFillColor(sf::Color::White);
    setText(userip, width / 2.f, height / 2.f - 45);

    Minesweeper* game = nullptr;

    while (window.isOpen()) {

        while (auto ev = window.pollEvent()) {
            if (!ev.has_value()) continue;

            if (ev->is<sf::Event::Closed>()) {
                window.close();
                break;
            }

            if (state == AppState::NameInput) {

                // --- Text Input ---
                if (auto evt = ev->getIf<sf::Event::TextEntered>()) {
                    char32_t ch = evt->unicode;

                    // Backspace
                    if (ch == 8) {
                        if (!input.empty())
                            input.pop_back();
                        userip.setString(input + "|");
                        continue;
                    }

                    // Only alphabet letters (ASCII A-Z or a-z)
                    if (ch < 128 && std::isalpha(static_cast<unsigned char>(ch))) {
                        //(ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {

                        if (input.size() < 10) {

                            char letter = static_cast<char>(ch);

                            // Capitalize first letter, lowercase the rest
                            if (input.empty())
                                letter = std::toupper(letter);
                            else
                                letter = std::tolower(letter);

                            input.push_back(letter);
                        }
                    }
                    userip.setString(input + "|");
                }

                if (auto kp = ev->getIf<sf::Event::KeyPressed>()) {
                    if (kp->code == sf::Keyboard::Key::Enter) {

                        if (input.empty()) {
                            continue;
                        }

                        window.create(
                            sf::VideoMode({width, height}),
                            "Minesweeper - Player: " + input
                        );

                        game = new Minesweeper(numOfRows, numOfColumns, numOfMines);
                        state = AppState::Game;
                    }
                }
            }

            else if (state == AppState::Game) {
                if (auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                    game->handleClick(
                        sf::Mouse::getPosition(window),
                        mb->button, window
                    );
                }
            }
        }

        window.clear(sf::Color::Blue);

        if (state == AppState::NameInput) {
            window.draw(text);
            window.draw(label);
            window.draw(userip);
        }
        else {
            game->draw(window);
        }

        window.display();
    }

    delete game;
    return 0;
}