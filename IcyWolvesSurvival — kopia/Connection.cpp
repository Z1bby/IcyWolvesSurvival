#include "Game.h"
//#include "Connection.h"

#include <iostream>

const int Connection::MaxPlayers = 32;
const int Connection::MessageBufferSize = 256;		// 256 bytes

void Connection::startListening() {
	// if instead of "this", pass an outside pointer, then error
	std::cout << "Try to start listening\n";
	keep_receiving = true;
	if (type == Types::server)
		receiverThread = std::thread(&Connection::start_server, this);
	else
		receiverThread = std::thread(&Connection::start_client, this);
}

void Connection::stopListening() {
	socket.unbind();				// is it necessary?
	keep_receiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
	delete m_mutex;
}

void Connection::start_server() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	char* buffer = new char[Connection::MessageBufferSize];
	while (keep_receiving) {
		std::cout << "Receiving new message...\n";
		if (socket.receive(buffer, Connection::MessageBufferSize, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			//throw exception
			std::cout << "Received [" << buffer << "] From " << sender_ip << ':' << sender_port << '\n';
		}
		else if (package_size > 0) {
			std::cout << "Received [" << buffer[0] << "] From " << sender_ip << ':' << sender_port << '\n';
			save_message(buffer, sender_port, sender_ip.toString());
		}
		std::this_thread::sleep_for(100ms);
	}
	delete[] buffer;
}

void Connection::start_client() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	char* buffer = new char[Connection::MessageBufferSize];
	while (keep_receiving) {
		std::cout << "Receiving new message...\n";
		if (socket.receive(buffer, Connection::MessageBufferSize, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			//throw exception
			std::cout << "Received [" << buffer[0] << "] From " << sender_ip << ':' << sender_port << " ERROR\n";
		}
		else if (package_size > 0) {
			std::cout << "Received [" << buffer[0] << "] From " << sender_ip << ':' << sender_port << " SUCCESS\n";
			save_message(buffer, sender_port, sender_ip.toString());
			if(type == Types::server)
				for (ConnectionData cd : clients)
					if (cd.port != sender_port && cd.address != sender_ip)
						socket.send(buffer, package_size, cd.address, cd.port);
		}
		std::this_thread::sleep_for(100ms);
	}
	delete[] buffer;
}

// store message in a container, send message as soon as container.empty() == false
void Connection::save_message(const char* buffer, unsigned short sender_port, std::string sender_ip) {
	std::lock_guard<std::mutex> locker(*m_mutex);
	messages.push(message(buffer, sender_ip, sender_port));
}

void Connection::process_msg(struct message msg, void (Game::*player_function)(struct message)) {
	std::lock_guard<std::mutex> locker(*m_mutex);
	bool clientExist = false;
	for (ConnectionData cd : clients) {
		if (cd.address == msg.address && cd.port == msg.port) {
			clientExist = true;
			break;
		}
	}
	if (!clientExist) {
		clients.push_back(ConnectionData(msg.port, msg.address));
	}
	player_function(msg);
}