#include "Game.h"

char Game::choose_connection_type() {
	char choice = 0;
	while (choice == 0) {
		std::cout << "Choose connection [C - client, S - server]\n";

		std::cin >> choice;
		if (choice == 'C' || choice == 'c') {
			return 'c';
		}
		else if (choice == 'S' || choice == 's') {
			return 's';
		}
		else
			choice = 0;
	}
	return 0;
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
	char buffer[8] = "join";
	connection->socket.send(buffer, 3, serverIP, 54000);
}

void Game::create_a_room() {
	serverIP = sf::IpAddress::getLocalAddress().toString();
	std::cout << "IP = " << connection->address << '\n'
		<< "Port = 54000\n";
	std::cin.get();
	std::cin.get();
	board->players[0] = new Player(rand() % board->width, rand() % board->height, 'A', board, true);
	board->players[0]->port = 54000;
	board->players[0]->address = connection->address.c_str();
	board->player_id = 'A';
	board->player_index = 0;
	board->player_count = 1;
	board->all_data[0] = 'A';
	board->players[0]->buffer[0] = 'A';
	board->players[0]->buffer[1] = board->players[0]->x;
	board->players[0]->buffer[2] = board->players[0]->y;
	board->players[0]->buffer[3] = board->players[0]->hp;
	board->players[0]->buffer[4] = '1';
	connection->id = 'A';
	((Server*)connection)->next_id = 'B';
	connection->bind(54000);
	((unsigned short*)&board->all_data[6])[0] = connection->port; /// check this
	board->players[0]->buffer[6] = ((char*)&connection->port)[0];
	board->players[0]->buffer[7] = ((char*)&connection->port)[1];
	strcpy_s(&board->players[0]->buffer[8], 17, connection->address.c_str());
	connection->startListening();
	gameStarted = true;
}

void Game::add_player(int x, int y, char id, int i) {
	board->players[i] = new Player(x, y, id, board, true);
}

void Game::remove_player(char id) {

}

void Game::process_network_data() {
	// max 10 players
	for (int i = 0; i < 10; i++) {
		if (board->all_data[i * 32] == 0)
			return;
		if (board->players[i] == nullptr) {
			if ((unsigned short)board->all_data[i * 32 + 6] == connection->port && 
				strcmp(&board->all_data[i * 32 + 8], connection->address.c_str()) == 0) {
				add_player(rand() % board->width, rand() % board->height, board->all_data[i * 32], i);
				board->player_id = board->all_data[i * 32];
				board->player_index = i;
				board->players[board->player_index]->port = *(unsigned short*)&board->all_data[i * 32 + 6];
				board->players[board->player_index]->address = &board->all_data[i * 32 + 8];
				gameStarted = true;
				board->player_count++;
			}
			else if (board->all_data[i * 32 + 4] == '1') {
				add_player(board->all_data[i * 32 + 1], board->all_data[i * 32 + 2], board->all_data[i * 32], i);
				board->player_count++;
			}
		}
		else if(i != board->player_index) {
			board->players[i]->x = board->all_data[i * 32 + 1];
			board->players[i]->y = board->all_data[i * 32 + 2];
			board->players[i]->hp = board->all_data[i * 32 + 3];
		}
	}
}

void Game::network_game() {
	while (!gameStarted) {
		process_network_data();
		std::this_thread::sleep_for(100ms);
	}
	std::cout << "Game Started:\n";
	board->players[board->player_index]->buffer[0] = board->players[board->player_index]->id;
	board->players[board->player_index]->buffer[4] = '1';
	bool game_over = false;
	while (game_over == false) {
		int time_now = clock();
		process_network_data();
		board->players[board->player_index]->move();
		if (time_now - board->last_print_time > board->print_delay) {
			//board->print();
			board->last_print_time = time_now;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_over = true;
			break;
		}
		board->players[board->player_index]->update_player();
		if (connection->isServer()) {
			connection->save_message(board->players[board->player_index]->buffer, 0, "");
			for (int i=1; i<board->player_count; i++)
				{
					//connection->socket.send(buffer, 5, p->address, connection->port);
					connection->socket.send(board->all_data, 5, board->players[i]->address, board->players[i]->port);
				}
		}
		else {
			connection->socket.send(board->players[board->player_index]->buffer, 32, serverIP, 54000);
		}
		std::this_thread::sleep_for(300ms);
		std::cin.get();
	}
	connection->stopListening();
}

void Game::start()
{
	srand(time(NULL));
	std::cout << sf::IpAddress::getLocalAddress() << '\n';
	board = new Board(10, 10);								// board is 10x10
	char type = choose_connection_type();
	if (type == 'c') {
		connection = new Client();
		board->all_data = connection->all_data;
		join_a_room();
	}
	else if (type == 's') {
		connection = new Server();
		board->all_data = connection->all_data;
		create_a_room();
	}
	else return;
	network_game();
	return;
}