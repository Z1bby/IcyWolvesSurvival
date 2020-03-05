#include "Game.h"

char Game::chooseConnectionType() {
	char choice = 0;
	while (choice == 0) {
		std::cout << "Choose connection [C - client, S - server]\n";

		std::cin >> choice;
		if (choice == 'C' || choice == 'c') {
			return 'C';
		}
		else if (choice == 'S' || choice == 's') {
			return 'S';
		}
	}
	return 0;
}

void Game::joinRoom() {
	std::cout << "Join room.\nAddress: ";
	std::cin >> connection->serverIP;			// specify server address. first char is used to assign addres by program
	switch (connection->serverIP[0]) {
	case 'l':
		strcpy_s(connection->serverIP, 16, sf::IpAddress::getLocalAddress().toString().c_str());
		break;
	case '3':
		strcpy_s(connection->serverIP, 16, "192.168.8.103");
		break;
	case '6':
		strcpy_s(connection->serverIP, 16, "192.168.8.106");
		break;
	}
	connection->serverPORT = 54000;
	connection->bind(connection->port);		// important. udp socket is now ready for comunication
	connection->startListening();			// create new thread to receive messages
	char c = 'j';							// could receive message before sends join request
	connection->socket.send(&c, 1, connection->serverIP, connection->serverPORT);
}

void Game::createRoom() {
	//playerData is at index 0,
	//server
	connection->playerData = connection->allData;
	connection->bind(54000);
	addPlayer(rand() % board.width, rand() % board.height, 0);
	static_cast<Server*>(connection)->occupiedSize = PLAYER_DATA_SIZE;
	board.player_count = 1;
	playerID = 0;
	gameStarted = true;
	connection->startListening();
	// could read a message here.
	connection->save_message(reinterpret_cast<char*>(&board.players[playerID]->data), 0, nullptr, PLAYER_DATA_SIZE);
}

void Game::addPlayer(int x, int y, int id, bool connected) {
	board.players[id] = new Player(x, y, id, &board, connected);
}

void Game::removePlayer(int id) {

}

void Game::updateRealTimeVariables() {
	// max 10 players
	for (int i = 0; i < 10; i++) {
		// if allData[i] doesn't have right index that means it's empty, no more players in server buffer
		if (reinterpret_cast<pData*>(&connection->allData[i * PLAYER_DATA_SIZE])->id != i)
			return;
		// if there is an index but there is no player object in the game
		if (board.players.find(i) == board.players.end()) {
			// if it's you, then you're happy to join the server
			if (connection->connectionID == i) {
				playerID = i;
				addPlayer(rand() % board.width, rand() % board.height, i);
				gameStarted = true;
				board.player_count++;
				connection->playerData = &connection->allData[i * PLAYER_DATA_SIZE];
				connection->socket.send(connection->playerData, PLAYER_DATA_SIZE , connection->serverIP, connection->serverPORT);
			}
			// else, add empty player, and wait for first update
			else {
				addPlayer(0, 0, i, false);
				board.player_count++;
			}
		}
		else if(i != playerID) {
			memcpy(reinterpret_cast<char*>(&board.players[i]->data), &connection->allData[i * PLAYER_DATA_SIZE], PLAYER_DATA_SIZE);
		}
	}
}

void Game::networkGame() {
	while (!gameStarted) {
		updateRealTimeVariables();
		std::this_thread::sleep_for(100ms);
	}
	std::cout << "Game Started:\n";
	bool game_over = false;
	while (game_over == false) {
		int time_now = clock();
		updateRealTimeVariables();
		board.players[playerID]->move();

		if (time_now - board.last_print_time > board.print_delay) {
			board.print();
			board.last_print_time = time_now;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_over = true;
			break;
		}
		// if server, then update allData with your variables and send allData to everyone
		if (connection->isServer()) {
			connection->save_message(reinterpret_cast<char*>(&board.players[playerID]->data), 0, nullptr, PLAYER_DATA_SIZE);
			for (std::pair<int, cData> p : connection->connections) {
				std::cout << "Sending to " << p.second.address << ": " << p.second.port << '\n';
				connection->socket.send(connection->allData, ALL_DATA_SIZE, p.second.address, p.second.port);
			}
		}
		//else send your variables to server
		else {
			std::cout << "Sending to " << connection->serverIP << ": " << connection->serverPORT << '\n';
			connection->socket.send(reinterpret_cast<char*>(&board.players[playerID]->data), PLAYER_DATA_SIZE, static_cast<Client*>(connection)->serverIP, static_cast<Client*>(connection)->serverPORT);
		}
		std::this_thread::sleep_for(300ms);
		std::cin.get();
	}
	connection->stopListening();
}

void Game::start()
{
	srand(time(NULL));
	std::cout << "Hello. Your IP: " << sf::IpAddress::getLocalAddress() << '\n';
}

void Game::startServer() {
	connection = new Server(54000, sf::IpAddress::getLocalAddress().toString().c_str());
}

void Game::startClient () {
	unsigned short port;
	std::cout << "Port: 54000 + ";
	std::cin >> port;
	port += 54000;
	char address[16];
	strcpy_s(address, sf::IpAddress::getLocalAddress().toString().c_str());
	connection = new Client(-1, port, address);
}

void Game::startClient(int connectionID, unsigned short port, char* address) {
	connection = new Client(connectionID, port, address);
}
