#include "Game.h"
struct virtual_machine {
	void* connection;
};
int main() {
	Game game;
	game.start();
	char c = game.chooseConnectionType();
	if (c == 'S') {
		game.startServer();
		game.createRoom();
	}
	else if (c == 'C') {
		game.startClient();
		game.joinRoom();
	}
	game.networkGame();

	return 0;
}