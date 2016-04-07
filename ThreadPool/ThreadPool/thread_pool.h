#pragma once
#include <future>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "CWorker.h"


template <typename TResult>
class ThreadPool{
public:
	ThreadPool(size_t threads){
		if (threads == 0) {
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			size_t threads = sysinfo.dwNumberOfProcessors;
		}
		for (size_t i = 0; i<threads; i++)
		{
			std::shared_ptr<CWorker> pWorker(new CWorker);
			_workers.push_back(pWorker);
		}
	}
	ThreadPool()
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		size_t threads = sysinfo.dwNumberOfProcessors;
		if (threads == 0)
			threads = 1;
		for (size_t i = 0; i<threads; i++)
		{
			std::shared_ptr<CWorker> pWorker(new CWorker);
			_workers.push_back(pWorker);
		}
	}
	~ThreadPool() {}
	std::future<TResult> submit(std::function<TResult()> fn) {
		std::function<TResult()> func = std::bind(_fn, _args...);
		std::packaged_task<TResult(void)> tsk(func);   // set up packaged_task
		std::future<int> res;
		std::function<void()> fn = [=]()
		{
			res = tsk.get_future();
			std::move(tsk);
		};
		getFreeWorker()->appendFn(fn);
		return res;
	}

private:

	worker_ptr getFreeWorker() {
		shared_ptr<CWorker> pWorker;
		size_t minTasks = UINT32_MAX;
		for (auto &it : _workers){
			if (it->isEmpty()){
				return it;
			}
			else if (minTasks > it->getTaskCount()){
				minTasks = it->getTaskCount();
				pWorker = it;
			}
		}
		return pWorker;
	}

	std::vector<shared_ptr<CWorker>> _workers;

};