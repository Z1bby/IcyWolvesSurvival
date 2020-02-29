#pragma once
#include "SFML\Network.hpp"

#include "msgHeap.h"
#include "Player.h"

#include <thread>
#include <map>
#include <mutex>
#include <list>

using namespace std::chrono_literals;

class Connection {
public:
	sf::UdpSocket socket;
	unsigned short port;
	sf::IpAddress address;

	std::mutex m_mutex;
	std::thread receiverThread;
	std::thread senderThread;
	bool keep_receiving;

	std::vector<Player*> players;
	std::vector<sf::IpAddress> addresses;
	static const int MaxPlayers;
	msgHeap messages;
	static const int MessageBufferSize;
	enum Flags { fullArray };
	enum class Types { client, server, undefined };
	Types type;


	void startListening();
	void stopListening();
	void startSending();
	void stopSending();
	void receive_message();
	void send_message(const char* buffer, unsigned short port, const char* address);
	void save_message(const char* buffer, unsigned short sender_port, const char* sender_ip);
	void bind(unsigned short p = 0) {
		port = p;
		if (socket.bind(port) != sf::Socket::Done) {
			//throw exception
		}
	}
	Connection() : 
		port(0), type(Types::undefined), keep_receiving(false) {
		address = sf::IpAddress::getLocalAddress();
	}
	Connection(Types ctype) :
		port(0), type(ctype), keep_receiving(false) {
		address = sf::IpAddress::getLocalAddress();
	}
};