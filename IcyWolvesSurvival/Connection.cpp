#include "Connection.h"

#include <iostream>

void Client::startListening() {
	std::cout << "Client start listening\n";
	keepReceiving = true;
	receiverThread = std::thread(&Client::start, this);		// this creates new thread
}

void Server::startListening() {
	std::cout << "Server start listening\n";
	keepReceiving = true;
	receiverThread = std::thread(&Server::start, this);		// this creates new thread
}

void Client::stopListening() {
	socket.unbind();				// also repeated in destructor
	keepReceiving = false;
	std::cout << "Waiting for thread to join\n";
	receiverThread.join();
	std::cout << "Receiving thread joined\n";
}

void Server::stopListening() {
	socket.unbind();				// also repeated in destructor
	keepReceiving = false;
	std::cout << "Waiting for receiver to join\n";
	receiverThread.join();
	std::cout << "Receiver joined\n";
}

void Server::start() {
	std::size_t package_size;	// variables to receive message
	sf::IpAddress sender_ip;
	unsigned short sender_port;
	char buffer[PLAYER_DATA_SIZE];		// one player real time variables
	while (keepReceiving) {
		//std::cout << "Receiving new message...\n";
		if (socket.receive(buffer, PLAYER_DATA_SIZE, package_size, sender_ip, sender_port) != sf::Socket::Done) {
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
	while (keepReceiving) {
		//std::cout << "Receiving new message...\n";
		if (socket.receive(allData, ALL_DATA_SIZE, package_size, sender_ip, sender_port) != sf::Socket::Done) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " ERROR\n";
		}
		else if (package_size > 0) {
			std::cout << "Received " << package_size << " data From " << sender_ip << ':' << sender_port << " SUCCESS\n";
			save_message(allData, sender_port, sender_ip.toString().c_str(), package_size);
		}
		std::this_thread::sleep_for(100ms);
	}
}

void Client::save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size) {
	// message is already stored in allData[]
	if (package_size == 4) {
		connectionID = *(int*)buffer;
		return;
	}
	//std::lock_guard<std::mutex> locker(m_mutex);
	//memcpy(allData, buffer, ALL_DATA_SIZE);
}
void Server::save_message(char* buffer, unsigned short sender_port, const char* sender_ip, int package_size) {
	// if someone sends join request and there is a room for next player
	if (buffer[0] == 'j' && occupiedSize < ALL_DATA_SIZE) {
		int i = occupiedSize / PLAYER_DATA_SIZE;			// calculate index for new player
		connections[i].port = sender_port;					// set port
		strcpy_s(connections[i].address, 16, sender_ip);	// and address
		socket.send((char*)&i, sizeof(i), sender_ip, sender_port);	//send back the calculated index
		std::lock_guard<std::mutex> locker(m_mutex);
		reinterpret_cast<pData*>(&allData[occupiedSize])->id = i;	// save index to real time variables
		occupiedSize += PLAYER_DATA_SIZE;					// player takes space
		return;
	}
	else {	// assume it's just updated clients buffer
		std::lock_guard<std::mutex> locker(m_mutex);
		// allData[sender->ID] = buffer;
		memcpy(&allData[reinterpret_cast<pData*>(buffer)->id * PLAYER_DATA_SIZE], buffer, PLAYER_DATA_SIZE);
		return;
	}
}