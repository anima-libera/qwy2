
#include "threadpool.hpp"
#include <cassert>

#include <iostream>

namespace qwy2
{

namespace
{

bool s_threads_should_finish = false;

void thread_endless_work(ThreadPool& thread_pool)
{
	while(true)
	{
		std::packaged_task<void()> task;
		{
			std::unique_lock<std::mutex> lock{thread_pool.task_queue_mutex};
			if (thread_pool.task_queue.empty())
			{
				thread_pool.condition_variable.wait(lock,
					[&]{return not thread_pool.task_queue.empty();});
			}
			task = std::move(thread_pool.task_queue.front());
			thread_pool.task_queue.pop_front();
		}
		if (s_threads_should_finish)
		{
			return;
		}
		task();
		{
			std::unique_lock<std::mutex> lock{thread_pool.free_thread_count_mutex};
			thread_pool.free_thread_count++;
		}
	}
}

} /* Anonymous namespace. */

ThreadPool::ThreadPool()
{
	;
}

ThreadPool::~ThreadPool()
{
	s_threads_should_finish = true;
	/* Distribue dummy tasks to all the threads for them to check for s_threads_should_finish. */
	for (unsigned int i = 0; i < this->thread_table.size(); i++)
	{
		this->give_task([](){});
	}
	for (std::thread& thread : this->thread_table)
	{
		thread.join();
	}
}

void ThreadPool::set_thread_number(unsigned int thread_number)
{
	assert(this->thread_table.empty());
	for (unsigned int i = 0; i < thread_number; i++)
	{
		this->thread_table.emplace_back(thread_endless_work, std::ref(*this));
	}
	this->free_thread_count = thread_number;
}

} /* qwy2 */
