#pragma once
#include "SFML\Network.hpp"

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
	char id;
	ConnectionData(unsigned short p, std::string a, char i) :
		port(p), address(a), id(i) {}

};

class Connection {
public:
	sf::UdpSocket socket;
	unsigned short port;
	std::string address;
	char id;
	char* all_data;

	std::mutex* m_mutex;
	std::thread receiverThread;
	bool keep_receiving;

	virtual bool isServer() = 0;
	virtual void startListening() = 0;
	virtual void stopListening() = 0;
	virtual void save_message(const char* buffer, unsigned short port, std::string address) = 0;

	void bind(unsigned short p = 0) {
		port = p;
		if (socket.bind(p) != sf::Socket::Done) {
			//throw exception
			std::cout << "Couldn't bind socket with port " << p << ". that's bad.\n";
		}
	}
	Connection() : keep_receiving(false) {
		port = 0;
		id = 0;
		address = sf::IpAddress::getLocalAddress().toString();

		all_data = new char[320];
		for (int i = 0; i < 320; i++)
			all_data[i] = 0;

		m_mutex = new std::mutex();
	}
	~Connection() {
		delete[] all_data;
	}
};

class Client : public Connection {
public:
	void start_client();
	void startListening();
	void stopListening();
	void save_message(const char* buffer, unsigned short sender_port, std::string sender_ip);
	bool isServer() { return false; }
	Client() : Connection() {}
};

class Server : public Connection {
public:
	std::list <ConnectionData> clients;
	char next_id;
	void start_server();
	void startListening();
	void stopListening();
	void save_message(const char* buffer, unsigned short sender_port, std::string sender_ip);
	bool isServer() { return true; }
	Server() : Connection() { next_id = 0; }
};