#pragma once
#include "SFML\Graphics.hpp"

#include "Entity.h"

class Board;

class Player {
public:
	int x;
	int y;
	int move_delay;
	int last_move_time;
	char id;
	std::string address;
	enum class Teams { red, blue, unspecified };
	Teams team;
	Board* board;

	Player(int px, int py, char pid, Board* pboard, std::string paddress) {
		x = px;
		y = py;
		id = pid;
		board = pboard;
		address = paddress;
		team = Teams::unspecified;
		move_delay = 333; // miliseconds
		last_move_time = 0;
	}
	void move();
};
