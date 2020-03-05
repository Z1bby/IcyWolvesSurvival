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
		else
			choice = 'U';
	}
	return 0;
}

void Game::joinRoom() {
	// default server data in constructor
	char option;
	std::cout << "Join room: ";
	std::cin >> option;
	if (option == 'l')
		strcpy_s(connection->serverIP, 16, sf::IpAddress::getLocalAddress().toString().c_str());
	else if (option == '3')
		strcpy_s(connection->serverIP, 16, "192.168.8.103");
	else if (option == '6')
		strcpy_s(connection->serverIP, 16, "192.168.8.106");
	connection->serverPORT = 54000;
	connection->bind(connection->port);
	connection->startListening();
	char c = 'j';
	connection->socket.send(&c, 1, connection->serverIP, connection->serverPORT);
}

void Game::createRoom() {
	connection->segmentData = connection->allData;
	connection->bind(54000);
	addPlayer(rand() % board.width, rand() % board.height, 0);
	static_cast<Server*>(connection)->occupiedSize = DATA_SEGMENT_SIZE;
	board.player_count = 1;
	playerID = 0;
	gameStarted = true;
	connection->startListening();
	// could read a message here.
	connection->save_message(reinterpret_cast<char*>(&board.players[playerID]->data), 0, nullptr, DATA_SEGMENT_SIZE);
}

void Game::addPlayer(int x, int y, int id, bool connected) {
	board.players[id] = new Player(x, y, id, &board, connected);
}

void Game::removePlayer(int id) {

}

void Game::processNetworkData() {
	// max 10 players
	for (int i = 0; i < 10; i++) {
		if (reinterpret_cast<pData*>(&connection->allData[i * DATA_SEGMENT_SIZE])->id != i)
			return;
		if (board.players.find(i) == board.players.end()) {
			if (connection->connectionID == i) {
				playerID = i;
				addPlayer(rand() % board.width, rand() % board.height, i);
				gameStarted = true;
				board.player_count++;
				connection->segmentData = &connection->allData[i * DATA_SEGMENT_SIZE];
				connection->socket.send(connection->segmentData, DATA_SEGMENT_SIZE , connection->serverIP, connection->serverPORT);
			}
			else {
				addPlayer(0, 0, i, false);
				board.player_count++;
			}
		}
		else if(i != playerID) {
			memcpy(reinterpret_cast<char*>(&board.players[i]->data), &connection->allData[i * DATA_SEGMENT_SIZE], DATA_SEGMENT_SIZE);
		}
	}
}

void Game::networkGame() {
	while (!gameStarted) {
		processNetworkData();
		std::this_thread::sleep_for(100ms);
	}
	std::cout << "Game Started:\n";
	bool game_over = false;
	while (game_over == false) {
		int time_now = clock();
		processNetworkData();
		board.players[playerID]->move();
		if (time_now - board.last_print_time > board.print_delay) {
			board.print();
			board.last_print_time = time_now;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_over = true;
			break;
		}
		
		if (connection->isServer()) {
			connection->save_message(reinterpret_cast<char*>(&board.players[playerID]->data), 0, nullptr, DATA_SEGMENT_SIZE);
			for (std::pair<int, cData> p : connection->connections) {
				std::cout << "Sending to " << p.second.address << ": " << p.second.port << '\n';
				connection->socket.send(connection->allData, ALL_DATA_SIZE, p.second.address, p.second.port);
			}
		}
		else {
			std::cout << "Sending to " << connection->serverIP << ": " << connection->serverPORT << '\n';
			connection->socket.send(reinterpret_cast<char*>(&board.players[playerID]->data), DATA_SEGMENT_SIZE, static_cast<Client*>(connection)->serverIP, static_cast<Client*>(connection)->serverPORT);
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
	std::cout << "Address: \n";
	char address[16];
	strcpy_s(address, sf::IpAddress::getLocalAddress().toString().c_str());
	connection = new Client(-1, port, address);
}

void Game::startClient(int connectionID, unsigned short port, char* address) {
	connection = new Client(connectionID, port, address);
}
