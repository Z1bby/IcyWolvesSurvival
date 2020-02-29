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
	std::cout << "Server address: ";
	std::cin >> serverIP;
	std::cout << "Server port: ";
	std::cin >> port;
	connection->bind(port);
	connection->startListening();
	char buffer[8] = "jo";
	connection->socket.send(buffer, 3, serverIP, port);

	bool waiting_for_confirmation = true;
	while (waiting_for_confirmation) {
		if (connection->messages.empty() == false) {
			deal_with_message(connection->messages.pop());
		}
	}
}

void Game::create_a_room() {
	board->players.front()->address = connection->address.toString();
	board->players.front()->x = rand() % board->width;
	board->players.front()->y = rand() % board->height;
	board->players.front()->id = 'A';
	std::cout << "IP = " << connection->address.toString() << '\n'
		<< "Port = 54000\n";
	std::cin.get();
	std::cin.get();
	connection->bind(54000);
	connection->startListening();
}

void Game::add_player(const char* address, int x, int y, char id) {
	// later board will specify the starting position of 
	if (id == 0) {
		x = std::rand() % board->width;
		y = std::rand() % board->height;
		if (board->players.empty())
			id = 'A';
		else
			id = board->players.back()->id + 1; // players are sorted by id, last players id is max id
	}
	board->players.push_back(new Player(x, y, id, board, address));
}

void Game::remove_player(char id) {
	for (Player* p : board->players) {
		if (p->id == id) {
			board->players.remove(p);
			break;
		}
	}
}

void Game::deal_with_message(struct message msg) {
	std::cout << msg.address << ": [" << msg.buffer << "]\n";
	// if someone asks to join
	if (strcmp(msg.buffer, "jo") == 0) {
		add_player(msg.address);		// add player and remember his ip address
		// use msg buffer to send confirmation - player position and id.
		msg.buffer[0] = 'U';		// specifies if it's the receiver player (U) or enemy player (E)
		msg.buffer[1] = board->players.back()->x;
		msg.buffer[2] = board->players.back()->y;
		msg.buffer[3] = board->players.back()->id;
		msg.buffer[4] = 0;
		connection->socket.send(msg.buffer, 5, msg.address, connection->port);

		msg.buffer[0] = 'E';
		for (Player* p : board->players) {
			if (p == board->players.back()) break;
			msg.buffer[1] = p->x;
			msg.buffer[2] = p->y;
			msg.buffer[3] = p->id;
			connection->socket.send(msg.buffer, 5, msg.address, connection->port);
		}
	}
	// if someone quits
	else if (strcmp(msg.buffer, "qu") == 0) {
		remove_player(msg.buffer[3]);
	}
	else if (msg.buffer[0] == 'U') {
		board->players.front()->address = connection->address.toString();
		board->players.front()->x = msg.buffer[1];
		board->players.front()->y = msg.buffer[2];
		board->players.front()->id = msg.buffer[3];
	}
	else if (msg.buffer[0] == 'E') {
		bool isHere = false;
		for (Player* p : board->players) {
			if (p->id == msg.buffer[3]) {
				p->x = msg.buffer[1];
				p->y = msg.buffer[2];
				isHere = true;
			}
		}
		if (isHere == false) {
			add_player(msg.address, msg.buffer[1], msg.buffer[2], msg.buffer[3]);
		}
		if (connection->type == Connection::Types::server) {
			for (Player* p : board->players) {
				if (p->id != msg.buffer[3] && p->id != board->players.front()->id) {
					connection->socket.send(msg.buffer, 5, p->address, connection->port);
				}
			}
		}
	}
}

void Game::server_game() {
	char buffer[64];
	bool game_over = false;
	int board_print_delay = 100;
	int board_last_print_time = 0;
	while (game_over == false) {
		int time_now = clock();
		while (connection->messages.empty() == false) {
			deal_with_message(connection->messages.pop());
		}
		board->players.front()->move();
		if (time_now - board_last_print_time > board_print_delay) {
			board->print();
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
		for (Player* p : board->players)
			if (p->id != board->players.front()->id)
				connection->socket.send(buffer, 5, p->address, connection->port);
	}
	connection->stopListening();
}

void Game::client_game() {
	char buffer[64];
	bool game_over = false;
	int board_print_delay = 100;
	int board_last_print_time = 0;
	while (game_over == false) {
		int time_now = clock();
		while (connection->messages.empty() == false) {
			deal_with_message(connection->messages.pop());
		}
		board->players.front()->move();
		if (time_now - board_last_print_time > board_print_delay) {
			board->print();
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
		connection->socket.send(buffer, 5, serverIP, connection->port);
	}
	connection->stopListening();
}

void Game::start()
{
	board = new Board(10, 10);								// board is 10x10
	board->players.push_back(new Player(0, 0, 0, board, ""));
	connection = new Connection(choose_connection_type());	// after successfully joining a room, you can

	if (connection->type == Connection::Types::client) {
		join_a_room();
		client_game();
	}
	else if (connection->type == Connection::Types::server) {
		create_a_room();
		server_game();
	}
	else {
		// singleplayer later;
		return;
	}
	return;
}