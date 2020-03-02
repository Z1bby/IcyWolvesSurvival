#pragma once
#include "SFML\Network.hpp"

#include "msgHeap.h"
#include "Player.h"

#include <thread>
#include <iostream>
#include <mutex>
#include <list>

using namespace std::chrono_literals;

class Game;

class ConnectionData {
public:
	unsigned short port;
	std::string address;
	ConnectionData(unsigned short p, std::string a) :
		port(p), address(a) {}

};

class Connection {
public:
	sf::UdpSocket socket;
	unsigned short port;
	std::string address;
	std::list <ConnectionData> clients;

	std::mutex* m_mutex;
	std::thread receiverThread;
	std::thread senderThread;
	bool keep_receiving;

	std::vector<Player*> players;
	std::vector<std::string> addresses;
	static const int MaxPlayers;
	msgHeap messages;
	static const int MessageBufferSize;
	enum Flags { fullArray };
	enum class Types { client, server, undefined };
	Types type;


	void startListening();
	void stopListening();
	void receive_message();
	void send_message(const char* buffer, unsigned short port, std::string address);
	void save_message(const char* buffer, unsigned short sender_port, std::string sender_ip);
	void start_server();
	void start_client();
	void process_msg(struct message msg, void (Game::*player_function)(struct message));
	void bind(unsigned short p = 0) {
		port = p;
		if (socket.bind(p) != sf::Socket::Done) {
			//throw exception
			std::cout << "Couldn't bind socket with port " << p << ". that's bad.\n";
		}
	}
	Connection() :
		type(Types::undefined), keep_receiving(false) {
		port = 0;
		address = sf::IpAddress::getLocalAddress().toString();

		m_mutex = new std::mutex();
	}
	Connection(Types ctype) :
		type(ctype), keep_receiving(false) {
		port = 0;
		address = sf::IpAddress::getLocalAddress().toString();

		m_mutex = new std::mutex();
	}
};