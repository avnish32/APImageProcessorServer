#include "ThreadPool.h"

#include<iostream>
#include<string>

using namespace std;

using std::to_string;

typedef unsigned short ushort;

void ThreadPool::init(int numThreads)
{
	auto executeThreadLambda = [this]() {
		while (true) {
			function<void()> task;
			{
				unique_lock<mutex> lock(_mtx);
				
				//cout << "\n\nThread " << this_thread::get_id() << " acquired lock. Releasing and waiting to be notified...";
				_msgLogger->LogDebug("Acquired lock. Releasing and waiting to be notified...");

				_cv.wait(lock, [this]() {
					return _isThreadPoolStopped || !_tasks.empty();
					});

				//cout << "\nThread " << this_thread::get_id() << " received notification.";
				_msgLogger->LogDebug("Received notification.");

				if (!_tasks.empty()) {
					
					//cout << "\nThread " << this_thread::get_id() << " | Tasks are pending. Picking a task...";
					_msgLogger->LogDebug("Tasks are pending. Picking a task...");

					task = _tasks.front();
					_tasks.pop();

					//cout << "\nThread " << this_thread::get_id() << " picked a task. Task queue size: " << _tasks.size();
					_msgLogger->LogDebug("Picked a task. Task queue size: " + to_string(ushort(_tasks.size())));
				}
				else if (_isThreadPoolStopped) {

					//cout << "\nThread " << this_thread::get_id() << " | Pool stopped and all tasks completed. Exiting.";
					_msgLogger->LogDebug("Pool stopped and all tasks completed. Exiting.");

					return;
				}
			}
			task();
		}
		};

	for (int i = 0; i < numThreads; i++) {
		_workerThreads.push_back(thread(executeThreadLambda));
	}
}

ThreadPool::ThreadPool(int numThreads)
{
	init(numThreads);
}

ThreadPool::~ThreadPool()
{
	//cout << "\nThread pool destructor called.";
	_msgLogger->LogDebug("Thread pool destructor called.");

	{
		unique_lock<mutex> lock(_mtx);
		_isThreadPoolStopped = true;
		_cv.notify_all();
		//cout << "\nThread pool stopped. All threads notified.";
		_msgLogger->LogDebug("Thread pool stopped. All threads notified.");
	}

	for (thread &t : _workerThreads) {
		t.join();
	}
}

//void ThreadPool::operator()(int numThreads)
//{
//	init(numThreads);
//}


