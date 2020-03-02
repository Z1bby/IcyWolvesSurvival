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
	std::list<Player*> players;
	int player_count;
	
	Board(int w, int h) {
		width = w;
		height = h;
		player_count = 0;
	}
	void print();
};