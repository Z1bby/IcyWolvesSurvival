#pragma once
#include "SFML\Network.hpp"

#include "Player.h"

#include <thread>
#include <iostream>
#include <mutex>
#include <list>

#define ALL_DATA_SIZE 10 * sizeof(pData)
#define DATA_SEGMENT_SIZE sizeof(pData)

using namespace std::chrono_literals;

class Game;

class cData {
public:
	unsigned short port;
	char address[16]; // change to char* const
	cData() : port(0) {}
	cData(unsigned short p, char* a, char i) :
		port(p) {
		strcpy_s(address, 16, a);
	}

};

class Connection {
public:
	sf::UdpSocket socket;
	unsigned short port;
	char address[16];
	char serverIP[16];
	unsigned short serverPORT;
	int connectionID;

	std::map<int, cData> connections;
	char* segmentData;
	char allData[ALL_DATA_SIZE];

	std::mutex m_mutex;
	std::thread receiverThread;
	bool keep_receiving;

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
	Connection() : connectionID(-1), port(0) {
		for (int i = 0; i < ALL_DATA_SIZE; i++)
			allData[i] = 0;
		segmentData = nullptr;
		strcpy_s(serverIP, sf::IpAddress::getLocalAddress().toString().c_str());
		serverPORT = 54000;
	}
	Connection(int id, unsigned short p, const char* a) : 
		connectionID(id), port(p), keep_receiving(false) {
		// real time data of all players
		for (int i = 0; i < ALL_DATA_SIZE; i++)
			allData[i] = 0;
		segmentData = &allData[connectionID * DATA_SEGMENT_SIZE];
		strcpy_s(serverIP, sf::IpAddress::getLocalAddress().toString().c_str());
		serverPORT = 54000;
	}
	Connection(Connection& c) :
		keep_receiving(c.keep_receiving), port(c.port), connectionID(c.connectionID) {
			memcpy(allData, c.allData, ALL_DATA_SIZE);
			segmentData = &allData[connectionID * DATA_SEGMENT_SIZE];
			strcpy_s(serverIP, sf::IpAddress::getLocalAddress().toString().c_str());
			strcpy_s(address, c.address);
			serverPORT = 54000;
	}

	~Connection() {
		delete[] allData;
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
	Client(Client& c) : Connection(static_cast<Connection&>(c)) { }
};

class Server : public Connection {
public:
	int occupiedSize;
	void start();
	void startListening();
	void stopListening();
	void save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size);
	bool isServer() { return true; }
	Server(unsigned short p, const char* a) : Connection(0, p, a), occupiedSize(0) {  }
	Server(Server& s) : occupiedSize(s.occupiedSize), Connection(static_cast<Connection&>(s)) {}
};