#pragma once
#include "Entity.h"

#include <iostream>
#include <windows.h>
#include <list>

class Player;

class Board {
public:
	int width;
	int height;
	std::map<int, Player*> players;
	int player_count;

	int print_delay;
	int last_print_time;

	Board() {
		width = 10;
		height = 10;
		player_count = 0;
		print_delay = 50;
		last_print_time = 0;
	}
	Board(int w, int h) {
		width = w;
		height = h;
		player_count = 0;
		print_delay = 50;
		last_print_time = 0;
	}
	void addPlayer(int id, Player& p) { players[id] = &p; }
	void print();
	void update_players();
};