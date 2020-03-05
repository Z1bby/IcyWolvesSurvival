#include "Connection.h"

#include <iostream>

void Client::startListening() {
	// if instead of "this", pass an outside pointer, then error
	std::cout << "Client start listening\n";
	keep_receiving = true;
	receiverThread = std::thread(&Client::start, this);
}

void Server::startListening() {
	// if instead of "this", pass an outside pointer, then error
	std::cout << "Server start listening\n";
	keep_receiving = true;
	receiverThread = std::thread(&Server::start, this);
}

void Client::stopListening() {
	socket.unbind();				// is it necessary?
	keep_receiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
}

void Server::stopListening() {
	socket.unbind();				// is it necessary?
	keep_receiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
}

void Server::start() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	char buffer[DATA_SEGMENT_SIZE];
	while (keep_receiving) {
		//std::cout << "Receiving new message...\n";
		if (socket.receive(buffer, DATA_SEGMENT_SIZE, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " ERROR\n";
		}
		else if (package_size > 0) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " SUCCESS\n";
			save_message(buffer, sender_port, sender_ip.toString().c_str(), package_size);
		}
		std::this_thread::sleep_for(100ms);
	}
}

void Client::start() {
	std::size_t package_size;
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	while (keep_receiving) {
		//std::cout << "Receiving new message...\n";
		if (socket.receive(allData, ALL_DATA_SIZE, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			//throw exception
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " ERROR\n";
		}
		else if (package_size > 0) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " SUCCESS\n";
			save_message(allData, sender_port, sender_ip.toString().c_str(), package_size);
		}
		std::this_thread::sleep_for(100ms);
	}
}

// store message in a container, send message as soon as container.empty() == false
void Client::save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size) {
	if (package_size == 4) {
		connectionID = *(int*)buffer;
		return;
	}
	std::lock_guard<std::mutex> locker(m_mutex);
	memcpy(allData, buffer, ALL_DATA_SIZE);
}
void Server::save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size) {
	if (buffer[0] == 'j' && occupiedSize < ALL_DATA_SIZE) {
		int i = occupiedSize / DATA_SEGMENT_SIZE;
		connections[i].port = sender_port;
		strcpy_s(connections[i].address, 16, sender_ip);
		socket.send((char*)&i, sizeof(i), sender_ip, sender_port);
		std::lock_guard<std::mutex> locker(m_mutex);
		reinterpret_cast<pData*>(&allData[occupiedSize])->id = i;
		occupiedSize += DATA_SEGMENT_SIZE;
		return;
	}
	else{
		std::lock_guard<std::mutex> locker(m_mutex);
		memcpy(&allData[reinterpret_cast<pData*>(buffer)->id * DATA_SEGMENT_SIZE], buffer, DATA_SEGMENT_SIZE);
		return;
	}
}