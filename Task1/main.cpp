#include <queue>
#include <vector>
#define BOOST_TEST_MAIN
#include "boost\thread\thread.hpp"
#include "boost\test\included\unit_test.hpp"
#include "C:\Users\User\Source\Repos\MPI\Task1\Task1\tree_mutex.h"


BOOST_AUTO_TEST_CASE(Test1) {
	tree_mutex mut(5);

	mut.lock(2);
		printf("mew");
	mut.unlock(2);
}