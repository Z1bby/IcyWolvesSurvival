#pragma once
#include "SFML\Graphics.hpp"

#include "Entity.h"

class Board;

class Player {
public:
	char x;
	char y;
	char hp;
	char id;

	int move_delay;
	int last_move_time;
	
	char* buffer;
	bool connected;
	std::string address;
	unsigned short port;

	enum class Teams { red, blue, unspecified };
	Teams team;
	Board* board;

	void update_player();

	Player(char px, char py, char pid, Board* pboard, unsigned short pport, std::string paddress, bool pconnected) {
		x = px;
		y = py;
		id = pid;
		hp = 100;
		board = pboard;
		team = Teams::unspecified;
		move_delay = 150;		// miliseconds
		last_move_time = 0;
		buffer = new char[32];
		connected = pconnected;
		port = pport;
		address = paddress;
	}
	~Player() {
		delete[] buffer;
	}
	void move();
};
