#pragma once
#include "Connection.h"
#include "Player.h"
#include "Board.h"

#include <ctime>
#include <random>

class Game
{
public:
	Board board;				// contains players
	Connection* connection;		// server or client
	int playerID;				// index of your player in board.players
	bool gameStarted;			// set to true if server accepts you

	Game() : playerID(-1), gameStarted(false), connection(nullptr) {}
	
	
	void start();				// does nothing so far
	void startServer();
	void startClient();
	void startClient(int id, unsigned short p, char* a);

	void randomMove();			// not implemented

	char chooseConnectionType();	// set your connection to Server or Client

	void createRoom();			// only if you are server
	void deleteRoom();			// not implemented
	void joinRoom();			// only if you are client

	void addPlayer(int x, int y, int id, bool connected = true);
	void removePlayer(int id);	// if someone left the room
	void addBoard();			// not implemented

	void updateRealTimeVariables();	// update real time variables
	void networkGame();				// game loop
};

