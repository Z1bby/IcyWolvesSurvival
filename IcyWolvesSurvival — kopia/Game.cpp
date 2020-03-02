#include "Game.h"

Connection::Types Game::choose_connection_type() {
	char choice = 0;
	while (choice == 0) {
		std::cout << "Choose connection [C - client, S - server]\n";

		std::cin >> choice;
		if (choice == 'C' || choice == 'c') {
			return Connection::Types::client;
		}
		else if (choice == 'S' || choice == 's') {
			return Connection::Types::server;
		}
		else
			choice = 0;
	}
	return Connection::Types::undefined;
}

void Game::join_a_room() {
	// gett answer from server that you can play;
	unsigned short port;
	std::cout << "Server address: \n";
	serverIP = sf::IpAddress::getLocalAddress().toString();
	std::cout << "Server port: \n";
	std::cin >> port;
	port += 54000;
	connection->bind(port);
	connection->startListening();
	char buffer[8] = "jo";
	// connection->socket.send(buffer, 3, serverIP, port);
	connection->socket.send(buffer, 3, serverIP, 54000);
	gameStarted = true;
}

void Game::create_a_room() {
	serverIP = sf::IpAddress::getLocalAddress().toString();
	std::cout << "IP = " << connection->address << '\n'
		<< "Port = 54000\n";
	std::cin.get();
	std::cin.get();
	board->players.front()->address = connection->address;
	board->players.front()->port = 54000;
	board->players.front()->x = rand() % board->width;
	board->players.front()->y = rand() % board->height;
	board->players.front()->id = 'A';
	connection->bind(54000);
	connection->startListening();
}

void Game::add_player(std::string address, unsigned short port, int x, int y, char id) {
	// later board will specify the starting position of 
	if (id == 0) {
		x = std::rand() % board->width;
		y = std::rand() % board->height;
		if (board->players.empty())
			id = 'A';
		else
			id = board->players.back()->id + 1; // players are sorted by id, last player has max id
	}
	board->players.push_back(new Player(x, y, id, board, address, port));
}

void Game::remove_player(char id) {
	for (Player* p : board->players) {
		if (p->id == id) {
			board->players.remove(p);
			break;
		}
	}
}

void Game::process_message(struct message msg) {
	std::cout << "Dealing with message from " << msg.address << ':' << msg.port << '\n';
	// if someone asks to join
	if (strcmp(msg.buffer, "jo") == 0 && connection->type == Connection::Types::server) {
		add_player(msg.address, msg.port);		// add player and remember his ip address
		// use msg buffer to send confirmation - player position and id.
		msg.buffer[0] = 'U';		// specifies if it's the receiver player (U) or enemy player (E)
		msg.buffer[1] = board->players.back()->x;
		msg.buffer[2] = board->players.back()->y;
		msg.buffer[3] = board->players.back()->id;
		msg.buffer[4] = 0;
		std::cout << "jo (" << (int)msg.buffer[1] << ", " << (int)msg.buffer[2] << ") - " << msg.buffer[3] << '\n';
		connection->socket.send(msg.buffer, 5, msg.address, msg.port);
	}
	// if someone quits
	else if (strcmp(msg.buffer, "qu") == 0) {
		std::cout << "qu - " << msg.buffer[4] << '\n';
		connection->clients.remove(ConnectionData(msg.port, msg.address));
		remove_player(msg.buffer[4]); // buffer[4] = player.id
	}
	else if (msg.buffer[0] == 'U') {
		std::cout << "U (" << (int)msg.buffer[1] << ", " << (int)msg.buffer[2] << ") - " << msg.buffer[3] << '\n';
		board->players.front()->address = connection->address;
		board->players.front()->x = msg.buffer[1];
		board->players.front()->y = msg.buffer[2];
		board->players.front()->id = msg.buffer[3];
		board->players.front()->port = connection->port;
		gameStarted = true;
	}
	else if (msg.buffer[0] == 'E') {
		std::cout << "E (" << (int)msg.buffer[1] << ", " << (int)msg.buffer[2] << ") - " << msg.buffer[3] << '\n';
		bool isHere = false;
		for (Player* p : board->players) {
			if (p->id == msg.buffer[3]) {
				p->x = msg.buffer[1];
				p->y = msg.buffer[2];
				isHere = true;
			}
		}
		if (isHere == false) {
			std::cout << "Added new player.\n";
			add_player(msg.address, msg.port, msg.buffer[1], msg.buffer[2], msg.buffer[3]);
		}
	}
}

void Game::network_game() {
	while (gameStarted == false) {
		std::this_thread::sleep_for(100ms);
		while (connection->messages.empty() == false) {
			connection->process_msg(connection->messages.pop(), &Game::process_message);
		}
	}
	char buffer[64];
	bool game_over = false;
	int board_print_delay = 100;
	int board_last_print_time = 0;
	while (game_over == false) {
		int time_now = clock();
		while (connection->messages.empty() == false) {
			connection->process_msg(connection->messages.pop(), &Game::process_message);
		}
		board->players.front()->move();
		if (time_now - board_last_print_time > board_print_delay) {
			//board->print();
			board_last_print_time = time_now;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_over = true;
			break;
		}

		buffer[0] = 'E';
		buffer[1] = board->players.front()->x;
		buffer[2] = board->players.front()->y;
		buffer[3] = board->players.front()->id;
		buffer[4] = 0;
		if (connection->type == Connection::Types::server) {
			for (Player* p : board->players)
				if (p->id != board->players.front()->id) {
					//connection->socket.send(buffer, 5, p->address, connection->port);
					connection->socket.send(buffer, 5, p->address, p->port);
				}
		}
		else if (connection->type == Connection::Types::client) {
			connection->socket.send(buffer, 5, serverIP, 54000);
		}
		std::this_thread::sleep_for(500ms);
		std::cin.get();
	}
	connection->stopListening();
}

void Game::start()
{
	srand(time(NULL));
	std::cout << sf::IpAddress::getLocalAddress();
	board = new Board(10, 10);								// board is 10x10
	board->players.push_back(new Player(0, 0, 0, board, "", 0));
	connection = new Connection(choose_connection_type());	// after successfully joining a room, you can

	if (connection->type == Connection::Types::client) {
		join_a_room();
		network_game();
	}
	else if (connection->type == Connection::Types::server) {
		create_a_room();
		network_game();
	}
	else {
		// singleplayer later;
		return;
	}
	return;
}