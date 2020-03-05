#pragma once
#include "Connection.h"
#include "Player.h"
#include "Board.h"

#include <ctime>
#include <random>

class Game
{
public:
	Board board;
	Connection* connection;
	int playerID;
	bool gameStarted; 

	Game() : playerID(-1), gameStarted(false), connection(nullptr) {}

	void start();
	void startServer();
	void startClient();
	void startClient(int id, unsigned short p, char* a);

	void randomMove();
	char chooseConnectionType();

	void createRoom();
	void deleteRoom();
	void joinRoom();
	void addPlayer(int x, int y, int id, bool connected = true);
	void removePlayer(int id);
	void addBoard();

	void processNetworkData();
	void networkGame();
};

