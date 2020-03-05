#pragma once
#include "SFML\Graphics.hpp"

#include "Entity.h"

class Board;

struct pData {
	int x;
	int y;
	int hp;
	int id;
	char body;
	bool connected;
};

class Player {
public:
	pData data;
	int move_delay;
	int last_move_time;

	enum class Teams { red, blue, unspecified };
	Teams team;
	Board* board;

	Player(char px, char py, int pid, Board* pboard, bool pconnected) {
		data.x = px;
		data.y = py;
		data.id = pid;
		data.hp = 100;
		data.connected = pconnected;
		data.body = 'A' + pid;
		board = pboard;
		team = Teams::unspecified;
		move_delay = 150;		// miliseconds
		last_move_time = 0;
	}
	void move();
};
