#include "Player.h"
#include "Board.h"

#include <iostream>

void Player::move() {
	int time_now = clock();
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && data.x > 0 && time_now - last_move_time > move_delay) {
		std::cout << "Left\n";
		data.x--;
		last_move_time = time_now;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && data.x < board->width - 1 && time_now - last_move_time > move_delay) {
		std::cout << "Right\n";
		data.x++;
		last_move_time = time_now;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && data.y > 0 && time_now - last_move_time > move_delay) {
		std::cout << "Top\n";
		data.y--;
		last_move_time = time_now;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && data.y < board->height - 1 && time_now - last_move_time > move_delay) {
		std::cout << "Down\n";
		data.y++;
		last_move_time = time_now;
	}
}