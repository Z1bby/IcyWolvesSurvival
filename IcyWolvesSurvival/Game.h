#pragma once
#include "Connection.h"
#include "Player.h"
#include "Board.h"

#include <ctime>
#include <random>
#include <mutex>

class Game
{
public:
	Connection* connection;
	std::string serverIP;
	Board* board;

	bool gameStarted;

	Game() : connection(nullptr), serverIP(""), board(nullptr), gameStarted(false) { }
	
	void start();
	char choose_connection_type();
	void join_a_room();
	void create_a_room();
	void add_player(int x, int y, char id, int i);
	void remove_player(char id = 0);
	void add_board();
	void process_network_data();
	void network_game();
	void client_game();
};

