#pragma once

#include<mutex>
#include<vector>
#include<queue>
#include<thread>
#include<functional>
#include<future>

#include "MsgLogger.h"

//using namespace std;
using std::thread;
using std::function;
using std::condition_variable;
using std::future;
using std::invoke_result_t;
using std::bind;
using std::packaged_task;
using std::make_shared;
using std::move;
using std::unique_lock;
using std::vector;
using std::queue;

class ThreadPool
{
private:
	vector<thread> _workerThreads;
	queue <function<void()>> _tasks;
	mutex _mtx;
	condition_variable _cv;
	bool _isThreadPoolStopped = false;
	MsgLogger* _msgLogger = MsgLogger::GetInstance();

	void init(int numThreads);

public:
	ThreadPool() = default;
	ThreadPool(int numThreads);
	~ThreadPool();

	//void operator() (int numThreads);

	template<typename F, typename ...Args>
	future <invoke_result_t<F, Args...>> enqueue(F&& fn, Args&& ...args);

};

/*
This function enqueues the task fn and its arguments args... into the task queue and notifies a thread.
*/
template<typename F, typename ...Args>
inline future<invoke_result_t<F, Args...>> ThreadPool::enqueue(F&& fn, Args&& ...args)
{
	//Wrapping the user submitted task F and its arguments Args into a function with no arguments
	//and void return type as the task queue is made up of generalized functions that do not take
	//any arguments and do not return anything.
	
	//Determining type of the result returned by the function type F when called with argument type Args...
	using return_type = invoke_result_t<F,Args...>;

	//Binding the function fn and its arguments args together into a function that does not take any arguments.
	auto bindedFunction = bind(fn, args...);

	//Packaging the binded function into a task to obtain its future.
	packaged_task<return_type()> packagedTask(bindedFunction);

	//Making a shared pointer of the task as packaged_task class does not implement a copy constructor
	auto taskSharedPtr = make_shared<packaged_task<return_type()>>(move(packagedTask));

	{
		unique_lock<mutex> lock(_mtx);
		//Using a lambda function to convert the packaged task into a function that does not return anything.
		_tasks.push([taskSharedPtr]() -> void {
			(*taskSharedPtr)();
			});
		//Notifying one thread that a new task has been pushed into the queue.
		_cv.notify_one();
	}

	//Returning the future of the task so that its return value can be accessed
	return taskSharedPtr->get_future();
}
