#pragma once
#include <future>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

template <typename TResult> 
class ThreadPool{
public:
	ThreadPool(size_t threads)
	{
		if (threads == 0) {
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			size_t threads = sysinfo.dwNumberOfProcessors;
		}
		for (size_t i = 0; i<threads; i++)
		{
			worker_ptr pWorker(new Worker);
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
			worker_ptr pWorker(new Worker);
			_workers.push_back(pWorker);
		}
	}
	~ThreadPool() {}
	std::future<TResult> submit(std::function<TResult()> fn) {
		std::function<TResult()> func = std::bind(_fn, _args...);
		std::future<TResult> res;
		std::function<void()> fn = [=]()
		{
			pData->data = rfn();
			pData->ready = true;
		};
		getFreeWorker()->appendFn(fn);
		return pData;
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