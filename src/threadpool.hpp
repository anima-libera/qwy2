
#ifndef QWY2_HEADER_THREADPOOL_
#define QWY2_HEADER_THREADPOOL_

#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <vector>
#include <deque>

/* Source:
 * https://riptutorial.com/cplusplus/example/15806/create-a-simple-thread-pool */

/* TODO:
 * Rename and comment everything in there to make sure
 * that I will still understand it in the future. */

namespace qwy2
{

class ThreadPool
{
public:
	std::vector<std::thread> thread_table;
	std::condition_variable condition_variable;
	std::deque<std::packaged_task<void()>> task_queue;
	std::mutex task_queue_mutex;
	int free_thread_count;
	std::mutex free_thread_count_mutex;

public:
	ThreadPool();
	~ThreadPool();
	void set_thread_number(unsigned int thread_number);

	template<typename FunctionType, typename ReturnType = std::invoke_result_t<FunctionType>>
	std::future<ReturnType> give_task(FunctionType&& function);
};

template<typename FunctionType, typename ReturnType>
std::future<ReturnType> ThreadPool::give_task(FunctionType&& function)
{
	std::packaged_task<ReturnType()> task{std::forward<FunctionType>(function)};
	std::future<ReturnType> future = task.get_future();
	{
		std::unique_lock<std::mutex> lock{this->task_queue_mutex};
		this->task_queue.emplace_back(std::move(task));
	}
	this->condition_variable.notify_one();
	{
		std::unique_lock<std::mutex> lock{this->free_thread_count_mutex};
		this->free_thread_count--;
	}
	return future;
}

} /* qwy2 */

#endif /* QWY2_HEADER_THREADPOOL_ */
