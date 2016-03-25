#include <iostream>

#include <queue>
#include <vector>
#define BOOST_TEST_MAIN
#include "boost\thread\thread.hpp"
#include "boost\test\included\unit_test.hpp"
#include "C:\Users\User\Source\Repos\it_prac_3sem\BlockingQueue\BlockingQueue\CBlockingQueue.h"



CBlockingQueue<int> que;

void thread_0()
{
	que.push(0);
}

void thread_1()
{
	que.push(0);
}

BOOST_AUTO_TEST_CASE(Test1) {
	boost::thread trd0(&thread_0);
	boost::thread trd1(&thread_1);
	trd0.join();
	trd1.join();
	BOOST_REQUIRE_EQUAL(que.wait_pop(), 0);
}

BOOST_AUTO_TEST_CASE(Test2) {
	std::vector<boost::thread*> vect;
	for (int i = 0; i < 1000; ++i) {
		boost::thread trd0(&thread_0);
		vect.push_back(&trd0);
	}
	for (size_t i = 0; i < vect.size(); ++ i) {
		boost::thread*  tr1 = (vect[i]);
		tr1->join();
	}
	BOOST_REQUIRE_EQUAL(que.wait_pop(), 0);
}
