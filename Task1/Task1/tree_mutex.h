#pragma once
#include <iostream>
#include <atomic>
#include <vector>
#include <stdbool.h>
#include <thread> 
#include <array>
#include <exception>
#include <string>
#include <math.h>

class mutex_exeption : std::exception {
public:
	mutex_exeption(std::string str) {
		std::cout << str;
	}
};

class peterson_mutex {
public:
	peterson_mutex() {
		want[0].store(false);
		want[1].store(false);
		victim.store(0);
	}

	peterson_mutex(peterson_mutex&& from) {
		want[0].store(from.want[0]);
		want[1].store(from.want[1]);
		victim.store(from.victim);
	}

	void lock(unsigned int t) {
		if (t > 1) {
			throw new mutex_exeption("Wrong thread ID");
		}
		want[t].store(true);
		victim.store(t);
		while (want[1 - t].load() && (unsigned int)(victim.load()) == t) {
			std::this_thread::yield();
		}
	}

	void unlock(unsigned int t) {
		if (t > 1) {
			throw new mutex_exeption("Wrong thread ID");
		}
		want[t].store(false);
	}

private:
	std::array<std::atomic<bool>, 2> want;
	std::atomic<int> victim;
};


class tree_mutex {
public:
	tree_mutex(unsigned int num_threads){
		tree_size = (unsigned int)(pow(2, int(log(num_threads - 1)) + 1) - 1);
		tree = std::vector<node>(tree_size);
		for(auto& it : tree)
			it.owner = static_cast<unsigned int>(-1);
	}

	void lock(unsigned int t) {
		if (t >= size) {
			throw new mutex_exeption("Wrong thread ID");
		}
		unsigned int  id = t+tree_size;
		unsigned int prev = (id - 1)/2;
		while (id != 0) {
			prev = id;
			id = (id - 1)/2;
			tree[id].mut.lock(prev % 2);
			tree[id].owner = t;

		}
	}

	void unlock(unsigned int t) {
		if (t >= size) {
			throw new mutex_exeption("Wrong thread ID");
		}
		unsigned int  id = 0;
		unsigned int left = 0;
		unsigned int right = 0;
		while (id < tree_size) {
			left = id*2 + 1;
			right = left + 1;
			unsigned int where;

			if (left >= tree_size) {
				where = t + tree_size;
			} else {
				unsigned int owner_of_left = tree[left].owner;
				if (owner_of_left == t) {
					where = left;
				} else {
					where = right;
				}
			}
			tree[id].owner = static_cast<unsigned int>(-1);
			tree[id].mut.unlock(where % 2);
			id = where;
		}
	}




private:
	struct node {
		peterson_mutex mut;
		unsigned int owner;
	};
	unsigned int size = -1;
	std::vector<node> tree;
	unsigned int tree_size;
};	