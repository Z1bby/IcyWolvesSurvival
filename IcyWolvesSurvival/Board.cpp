#include "Player.h"
#include "Board.h" // different order creates error in Rect.inl. "SFML includes should be before other includes"

void Board::print() {
	system("cls");
	for (int i = 0; i < width + 2; i++)
		std::cout << '=';
	std::cout << '\n';
	for (int y = 0; y < height; y++) {
		std::cout << '|';
		for (int x = 0; x < width; x++) {
			bool playerHere = false;
			for (std::pair<int, Player*> p : players) {
				if (p.second->data.x == x && p.second->data.y == y) {
					std::cout << p.second->data.body;
					playerHere = true;
					break;
				}
			}
			if(playerHere == false) 
				std::cout << ' ';
		}
		std::cout << "|\n";
	}
	for (int i = 0; i < width + 2; i++)
		std::cout << '=';
}