#pragma once

#include <string>

struct message {
	char buffer[64];
	std::string address;
	unsigned short port;
	message(const char buf[], std::string a, unsigned short p) {
		for (int i = 0; i < 64; i++) {
			buffer[i] = buf[i];
			if (buf[i] == 0)
				break;
		}
		address = a;
		port = p;
	}
};

struct heapElement {
	struct message msg;
	struct heapElement* next;
	heapElement(struct message m) : msg(m) {
		next = nullptr;
	}
};

class msgHeap {
public:
	struct heapElement* first;
	msgHeap() {
		first = nullptr;
	}
	void push(struct message m) {
		struct heapElement* he = new struct heapElement(m);
		if (first == nullptr)
			first = he;
		else {
			struct heapElement* tmp = first;
			while (tmp->next != nullptr) tmp = tmp->next;
			tmp->next = first;
		}
	}
	struct message pop() {
		struct message m = first->msg;
		struct heapElement* tmp = first;
		first = first->next;
		delete tmp;
		return m;
	}
	bool empty() { return first == nullptr; }
};
