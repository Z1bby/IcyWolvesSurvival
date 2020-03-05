#pragma once
#include "SFML\Network.hpp"

#include "Player.h"

#include <thread>
#include <iostream>
#include <mutex>
#include <list>

#define ALL_DATA_SIZE 10 * sizeof(pData)
#define PLAYER_DATA_SIZE sizeof(pData)

using namespace std::chrono_literals;

class Game;

class cData {
public:
	unsigned short port;
	char address[16];
	cData() : port(0), address("") { }
	cData(unsigned short p, char* a, char i) :
		port(p) {
		strcpy_s(address, 16, a);
	}

};

class Connection {
public:
	sf::UdpSocket socket;		// udp socket
	unsigned short port;		// your port
	char address[16];			// your address
	char serverIP[16];			// server address
	unsigned short serverPORT;	// server port
	int connectionID;			// your id in multiplayer game

	std::map<int, cData> connections;		// addresses and identificators of all players in a room
	char* playerData;						// your player real time variables
	char allData[ALL_DATA_SIZE];			// all players real time variables

	std::mutex m_mutex;				// lock guard to protect real time variables from race situation
	std::thread receiverThread;		// thread to constantly receive data
	bool keepReceiving;			// yes

	// functions will be specified in child class, server or client
	virtual bool isServer() = 0;
	virtual void startListening() = 0;
	virtual void stopListening() = 0;
	virtual void save_message(char* buffer, unsigned short port, const char* address, int package_size) = 0;

	void bind(unsigned short p = 0) {
		port = p;
		if (socket.bind(p) != sf::Socket::Done) {
			//throw exception
			std::cout << "Couldn't bind socket with port " << p << ". that's bad.\n";
		}
	}
	// different constructors for practice
	Connection() :
		connectionID(-1), port(0), address(""), allData(""), playerData(nullptr),
		serverPORT(0), serverIP(""), keepReceiving(false)
	{
		strcpy_s(address, sf::IpAddress::getLocalAddress().toString().c_str());
	}
	Connection(int id, unsigned short p, const char* a) : 
		connectionID(id), port(p), address(""), allData(""), playerData(&allData[connectionID * PLAYER_DATA_SIZE]),
		serverPORT(0), serverIP(""), keepReceiving(false) 
	{
		playerData = &allData[connectionID * PLAYER_DATA_SIZE]; // repeated because i'm not sure about it
		strcpy_s(address, sf::IpAddress::getLocalAddress().toString().c_str());
	}
	Connection(Connection& c) :
		keepReceiving(c.keepReceiving), port(c.port), connectionID(c.connectionID) {
			memcpy(allData, c.allData, ALL_DATA_SIZE);
			playerData = &allData[connectionID * PLAYER_DATA_SIZE];
			strcpy_s(serverIP, sf::IpAddress::getLocalAddress().toString().c_str());
			strcpy_s(address, c.address);
			serverPORT = c.serverPORT;
	}

	~Connection() {
		socket.unbind();
	}
};

class Client : public Connection {
public:
	void start();
	void startListening();
	void stopListening();
	void save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size);
	bool isServer() { return false; }
	Client(int id, unsigned short p, char* a) : 
		Connection(id, p, a) {
	}
	Client(Client& c) : Connection(dynamic_cast<Connection&>(c)) { }
};

class Server : public Connection {
public:
	int occupiedSize;		// tells how many bytes are occupied in allData[]. related to board.players.size();
	void start();
	void startListening();
	void stopListening();
	void save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size);
	bool isServer() { return true; }
	Server(unsigned short p, const char* a) : Connection(0, p, a), occupiedSize(0) {  }
	Server(Server& s) : occupiedSize(s.occupiedSize), Connection(dynamic_cast<Connection&>(s)) {}
};