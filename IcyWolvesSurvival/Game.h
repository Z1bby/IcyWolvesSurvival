#pragma once
#include "Connection.h"
#include "Player.h"
#include "Board.h"

#include <ctime>
#include <random>

class Game
{
public:
	Connection* connection;
	char serverIP[17];
	Board* board;
	
	void start();
	Connection::Types choose_connection_type();
	void join_a_room();
	void create_a_room();
	void add_player(const char* address, int x = -1, int y = -1, char id = 0);
	void remove_player(char id = 0);
	void add_board();
	void deal_with_message(struct message msg);
	void server_game();
	void client_game();
};

