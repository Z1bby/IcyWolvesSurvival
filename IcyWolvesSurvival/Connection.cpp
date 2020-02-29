#include "Connection.h"

#include <iostream>

const int Connection::MaxPlayers = 32;
const int Connection::MessageBufferSize = 256;		// 256 bytes

void Connection::startListening() {
	// if instead of "this", pass an outside pointer, then error
	receiverThread = std::thread(&Connection::receive_message, this);
}
void Connection::stopListening() {
	socket.unbind();				// is it necessary?
	keep_receiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
}

void Connection::receive_message() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	char* buffer = new char[Connection::MessageBufferSize];
	while (keep_receiving) {
		if (socket.receive(buffer, Connection::MessageBufferSize, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			//throw exception
		}
		else if (package_size > 0) {
			save_message(buffer, sender_port, sender_ip.toString().c_str());
		}
		std::this_thread::sleep_for(100ms);
	}
}

// store message in a container, send message as soon as container.empty() == false
void Connection::save_message(const char* buffer, unsigned short sender_port, const char* sender_ip) {
	std::lock_guard<std::mutex> locker(m_mutex);
	messages.push(message(buffer, sender_ip, sender_port));
}

void Connection::send_message(const char* buffer, unsigned short port, const char* address) {
	std::lock_guard<std::mutex> locker(m_mutex);
	if (socket.send(buffer, Connection::MessageBufferSize, sf::IpAddress(address), port) != sf::Socket::Done) {
		//throw exception
	}
}