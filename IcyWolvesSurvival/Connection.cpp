#include "Connection.h"

#include <iostream>

void Client::startListening() {
	// if instead of "this", pass an outside pointer, then error
	std::cout << "Client start listening\n";
	keep_receiving = true;
	receiverThread = std::thread(&Client::start_client, this);
}

void Server::startListening() {
	// if instead of "this", pass an outside pointer, then error
	std::cout << "Server start listening\n";
	keep_receiving = true;
	receiverThread = std::thread(&Server::start_server, this);
}

void Client::stopListening() {
	socket.unbind();				// is it necessary?
	keep_receiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
	delete m_mutex;
}

void Server::stopListening() {
	socket.unbind();				// is it necessary?
	keep_receiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
	delete m_mutex;
}

void Server::start_server() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	char buffer[32];
	while (keep_receiving) {
		std::cout << "Receiving new message...\n";
		if (socket.receive(buffer, 32, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			//throw exception
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " ERROR\n";
		}
		else if (package_size > 0) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " SUCCESS\n";
			save_message(buffer, sender_port, sender_ip.toString());
		}
		std::this_thread::sleep_for(100ms);
	}
}

void Client::start_client() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	while (keep_receiving) {
		std::cout << "Receiving new message...\n";
		if (socket.receive(all_data, 320, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			//throw exception
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " ERROR\n";
		}
		else if (package_size > 0) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " SUCCESS\n";
			save_message(all_data, sender_port, sender_ip.toString());
		}
		std::this_thread::sleep_for(100ms);
	}
}

// store message in a container, send message as soon as container.empty() == false
void Client::save_message(const char* buffer, unsigned short sender_port, std::string sender_ip) {
	std::lock_guard<std::mutex> locker(*m_mutex);
	memcpy(all_data, buffer, 320);
}
void Server::save_message(const char* buffer, unsigned short sender_port, std::string sender_ip) {
	std::lock_guard<std::mutex> locker(*m_mutex);
	for (int i = 0; i < 10; i++) {
		if (all_data[i * 32] == 0 && buffer[0] == 'j') {
			all_data[i * 32] = next_id;
			((unsigned short*)&all_data[i * 32 + 6])[0] = sender_port; /// check this
			strcpy_s(&all_data[i * 32 + 8], 17, sender_ip.c_str());
			clients.push_back(ConnectionData(sender_port, sender_ip, next_id));
			next_id++;
			return;
		}
		else if (all_data[i * 32] == buffer[0]) {
			memcpy(&all_data[i * 32], buffer, 32);
			return;
		}
	}
}