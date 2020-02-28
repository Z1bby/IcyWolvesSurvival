#include <iostream>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <map>
#include <mutex>
#include <list>
#include <stack>

#include "SFML\Graphics.hpp"
#include "SFML\Network.hpp"

#include <windows.h>

using namespace std::chrono_literals;

struct message {
	char buffer[4];
	sf::IpAddress address;
	unsigned short port;
	message(char buf[], sf::IpAddress a, unsigned short p) {
		//for(int i=0; i<4; i++)
		//	buffer[i] = buf[i];
		((int*)buffer)[0] = ((int*)buf)[0];
		address = a;
		port = p;
	}
};

struct heapElement {
	struct message msg;
	struct heapElement* next;
	heapElement(struct message m) : msg(m) {
		next = nullptr;
	}
};

class msgHeap {
public:
	struct heapElement* first;
	msgHeap() { first = nullptr; }
	void push(struct message m) {
		struct heapElement* he = new struct heapElement(m);
		if (first == nullptr)
			first = he;
		else {
			struct heapElement *tmp = first;
			while (tmp->next != nullptr) tmp = tmp->next;
			tmp->next = first;
		}
	}
	struct message pop() {
		struct message m = first->msg;
		struct heapElement* tmp = first;
		first = first->next;
		delete tmp;
		return m;
	}
	bool empty() { return first == nullptr; }
};

class Player;
class Connection {
public:
	sf::UdpSocket socket;
	bool keep_receiving;
	char buffer[4];
	unsigned short port;
	std::map<unsigned short, sf::IpAddress> computerID;

	std::mutex m_mutex;
	msgHeap messages;

	void receive_udp();
	void save_message(unsigned short sender_port, sf::IpAddress sender_ip);
	void bind() {
		//if (socket.bind(port) != sf::Socket::Done)
		//	std::cout << "Error. Cannot bind the socket" << std::endl;
		while (socket.bind(port) != sf::Socket::Done) {
			port++;
		}
	}

	Connection(unsigned short p = 54000) {
		keep_receiving = true;
		port = p;
		buffer[0] = 0;
	}
};

class Server : public Connection {
	Server() : Connection() { }
};

class Client : public Connection {
	Client() : Connection() { }
};

void Connection::receive_udp() {
	std::size_t rcv_bytes;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	while (keep_receiving) {
		if (socket.receive(buffer, 4, rcv_bytes, sender_ip, sender_port) != sf::Socket::Done)
			std::cout << "Error. Cannot get message from sender" << std::endl;
		else if (rcv_bytes > 0) {
			save_message(sender_port, sender_ip);
		}
		std::this_thread::sleep_for(100ms);
	}
}

void Connection::save_message(unsigned short sender_port, sf::IpAddress sender_ip) {
	std::lock_guard<std::mutex> locker(m_mutex);
	messages.push(message(buffer, sender_ip, sender_port));
}

class Board {
public:
	int width;
	int height;
	Board(int w, int h) {
		width = w;
		height = h;
	}
	void print();
};

void Board::print() {
	for (int i = 0; i < width + 2; i++) 
		std::cout << '=';
	std::cout << '\n';
	for (int y = 0; y < height; y++) {
		std::cout << '|';
		for (int x = 0; x < width; x++)
			std::cout << ' ';
		std::cout << "|\n";
	}
	for (int i = 0; i < width + 2; i++)
		std::cout << '=';
	std::cout << '\n';
}

class Player {
public:
	int x;
	int y;
	char id;
	void move();
	Board* board;

	Player(int px, int py, char pid, Board* b) {
		x = px;
		y = py;
		id = pid;
		board = b;
	}
};

void gotoxy(int x, int y)
{
	COORD c;
	c.X = x + 1;
	c.Y = y + 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void Player::move() {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && x > 0) {
		gotoxy(x, y);
		putchar(' ');
		x--;
		gotoxy(x, y);
		putchar(id);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && x < board->width - 1) {
		gotoxy(x, y);
		putchar(' ');
		x++;
		gotoxy(x, y);
		putchar(id);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && y > 0) {
		gotoxy(x, y);
		putchar(' ');
		y--;
		gotoxy(x, y);
		putchar(id);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && y < board->height - 1) {
		gotoxy(x, y);
		putchar(' ');
		y++;
		gotoxy(x, y);
		putchar(id);
	}
}

int main()
{
	std::list<std::pair<unsigned short, sf::IpAddress>> servers; // list of servers (port, ip)
	Board board(10, 10);
	Player player(2, 2, 'A', &board);
	std::vector<Player> opponents;
	Connection c;
	c.bind();
	std::thread receiver(&Connection::receive_udp, &c);

	while (c.keep_receiving) {
		player.move();
		c.buffer[0] = player.x;
		c.buffer[1] = player.y;
		c.buffer[2] = player.id;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
			c.keep_receiving = false;
		if (c.computerID.empty()) {
			c.socket.send((void*)c.buffer, 3, sf::IpAddress::Broadcast, c.port);
			//c.socket.send((void*)c.buffer, 3, sf::IpAddress::getLocalAddress(), c.port + 1); 
			// funkcja send przerywa funkcjê receive
		}
		for (std::pair<unsigned short, sf::IpAddress> id : c.computerID)
		{
			if (c.socket.send((void*)c.buffer, 3, id.second, id.first) != sf::Socket::Done)
				std::cout << "Error. Cannot send message" << std::endl;
		}
		while (c.messages.empty() == false) {
			struct message msg = c.messages.pop();
			if (msg.buffer[0] == 'i' && msg.buffer[1] == 'd') {
				player.id = msg.buffer[2];
				c.computerID[msg.port] = msg.address;
			}
			else {
				bool search_flag = true;
				for (Player p : opponents) {
					if (p.id == msg.buffer[2]) {
						gotoxy(p.x, p.y);
						putchar(' ');
						p.x = msg.buffer[0];
						p.y = msg.buffer[1];
						gotoxy(p.x, p.y);
						putchar(p.id);
						search_flag = false;
						break;
					}
				}
				if (search_flag == true) {
					if (msg.buffer[2] == 'A')
						msg.buffer[2] = 'B' + c.computerID.size();
					c.computerID[msg.port] = msg.address;
					opponents.push_back(Player(msg.buffer[0], msg.buffer[1], msg.buffer[2], &board));
					c.buffer[0] = 'i'; c.buffer[1] = 'd'; c.buffer[2] = msg.buffer[2];
					c.socket.send((void*)c.buffer, 3, msg.address, msg.port);
				}
			}
		}
		std::this_thread::sleep_for(100ms);
	}
	receiver.join();

	std::cin.get();
	return 0;
}