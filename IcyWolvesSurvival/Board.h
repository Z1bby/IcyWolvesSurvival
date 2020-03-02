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
	Player* players[10];
	char player_id;
	int player_index;
	char* all_data;
	int player_count;

	int print_delay;
	int last_print_time;
	
	Board(int w, int h) {
		width = w;
		height = h;
		player_count = 0;
		player_id = -1;
		player_index = -1;
		all_data = nullptr;
		for (int i = 0; i < 10; i++)
			players[i] = nullptr;
		print_delay = 50;
		last_print_time = 0;
	}
	void print();
	void update_players();
};