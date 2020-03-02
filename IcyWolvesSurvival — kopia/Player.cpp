#include "Player.h"
#include "Board.h"

#include <iostream>

void Player::move() {
	int time_now = clock();
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && x > 0 && time_now - last_move_time > move_delay) {
		std::cout << "Left\n";
		x--;
		last_move_time = time_now;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && x < board->width - 1 && time_now - last_move_time > move_delay) {
		std::cout << "Right\n";
		x++;
		last_move_time = time_now;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && y > 0 && time_now - last_move_time > move_delay) {
		std::cout << "Top\n";
		y--;
		last_move_time = time_now;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && y < board->height - 1 && time_now - last_move_time > move_delay) {
		std::cout << "Down\n";
		y++;
		last_move_time = time_now;
	}
}