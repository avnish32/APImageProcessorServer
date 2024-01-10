#include "ThreadPool.h"

#include<iostream>
#include<string>

//using namespace std;

using std::to_string;
using std::unique_lock;

typedef unsigned short u_short;

/*
This function initializes the thread pool with numOfThreads threads.
Each thread is configured to wait until a task is pushed into the task queue,
and then it picks the task and starts executing it. Once done, it goes into the
waiting state again until another task is pushed, or the thread pool is stopped,
 whichever occurs earlier. 
 The thread behaviour is defined using an internal lambda function.
*/
void ThreadPool::Init(int numThreads)
{
	auto executeThreadLambda = [this]() {
		while (true) {
			function<void()> task;
			{
				unique_lock<mutex> lock(mtx_);
				
				msg_logger_->LogDebug("Acquired lock. Releasing and waiting to be notified...");

				cv_.wait(lock, [this]() {
					return is_thread_pool_stopped_ || !tasks_.empty();
					});

				msg_logger_->LogDebug("Received notification.");

				if (!tasks_.empty()) {
					
					msg_logger_->LogDebug("Tasks are pending. Picking a task...");

					task = tasks_.front();
					tasks_.pop();

					msg_logger_->LogDebug("Picked a task. Task queue size: " + to_string(u_short(tasks_.size())));
				}
				else if (is_thread_pool_stopped_) {
					msg_logger_->LogDebug("Pool stopped and all tasks completed. Exiting.");
					return;
				}
			}
			task();
		}
		};

	for (int i = 0; i < numThreads; i++) {
		worker_threads_.push_back(thread(executeThreadLambda));
	}
}

ThreadPool::ThreadPool(int numThreads)
{
	Init(numThreads);
}

/*
Destructor for the thread pool.
It tells the thread pool to stop and notifies all running threads about the same.
Also joins all the worker threads to the main thread so each thread may finish its
execution before the application terminates.
*/
ThreadPool::~ThreadPool()
{
	msg_logger_->LogDebug("Thread pool destructor called.");

	{
		unique_lock<mutex> lock(mtx_);
		is_thread_pool_stopped_ = true;
		cv_.notify_all();
		msg_logger_->LogDebug("Thread pool stopped. All threads notified.");
	}

	for (thread &t : worker_threads_) {
		t.join();
	}
}

//void ThreadPool::operator()(int numThreads)
//{
//	init(numThreads);
//}


