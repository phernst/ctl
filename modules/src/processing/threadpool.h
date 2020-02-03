#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <algorithm>
#include <thread>
#include <vector>

namespace CTL {

/*!
 * \class ThreadPool
 *
 * \brief Provides a container of a fixed number of threads for parallel execution of similar jobs.
 *
 * This thread pool simply holds a set of `std::thread`s. Function (or callable object) executions
 * can be enqueued into the pool. It is implemented in terms of a ring buffer of threads, meaning
 * that if all threads running a job, it waits for the "oldest" job (first enqueued) to finish. This
 * class is handy if the workload for each thread is preferably equal.
 * The `ThreadPool` has the following properties:
 * \li The method `enqueueThread` has the same interface as the constructor of `std::thread`, i.e.
 * `enqueueThread(Function&& f, Args&&... args)`, where `f` is a callable and `args` are arguments
 * of `f`. The arguments are perfectly forwarded to a constructor of `std::thread`, thus copies of
 * `f` and `args` are stored in the constructed thread.
 * \li The destructor blocks until all threads have finished their jobs.
 * \li An instance of `ThreadPool` is movable but not copyable.
 *
 * Example snippet:
 * \code
 * const auto size = 42;
 * std::vector<int> sharedRessource(size);
 *
 * auto foo = [&sharedRessource](int i) {
 *     sharedRessource[i] = i * i;
 * };
 *
 * {
 *     ThreadPool tp;
 *     for(auto i = 0; i < size; ++i)
 *         tp.enqueueThread(foo, i); // parallel execution of `foo`
 *
 * } // blocking dtor of `tp`: wait for all threads in `tp` to finish
 * \endcode
 *
 * Note that the client code still needs to take care about data races, deadlocks etc.
 */

class ThreadPool
{
public:
    ThreadPool(size_t nbThreads = 0);

    ~ThreadPool();

    // deleted copy operations
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    // defaulted move operations
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;

    size_t nbThreads() const;

    template <class Function, class... Args>
    void enqueueThread(Function&& f, Args&&... args);

private:
    std::vector<std::thread> _pool;
    std::vector<std::thread>::iterator _curThread;
};

/*!
 * Constructs an instance of `ThreadPool` consisting of \a nbThreads threads.
 * If no number is provided by the client (calling the default constructor) or \a nbThreads is zero,
 * the number of threads defaults to `std::thread::hardware_concurrency()`. If this function is not
 * able to compute the supported number of threads (in this case it returns zero), the number of
 * threads is set to one.
 */
inline ThreadPool::ThreadPool(size_t nbThreads)
    : _pool(nbThreads == 0 ? std::max({ 1u, std::thread::hardware_concurrency() }) : nbThreads)
    , _curThread(_pool.begin())
{
}

/*!
 * Blocking destructor that waits for all running threads to be finished.
 */
inline ThreadPool::~ThreadPool()
{
    std::for_each(_pool.begin(), _pool.end(), [](std::thread& t)
    {
        if(t.joinable())
            t.join();
    });
}

/*!
 * Returns the size of the thread pool, i.e. the number of available threads.
 */
inline size_t ThreadPool::nbThreads() const
{
    return _pool.size();
}

/*!
 * Enqueues a job \a f to a thread by forwarding \a f and \a args to the constructor of a thread.
 * This function may blocks until a certain thread is ready. If `enqueueThread` has been called less
 * frequent than the available number of threads, it will not block.
 */
template <class Function, class... Args>
inline void ThreadPool::enqueueThread(Function&& f, Args&&... args)
{
    if(_curThread->joinable())
        _curThread->join();

    *_curThread = std::thread(std::forward<Function>(f), std::forward<Args>(args)...);

    ++_curThread;
    if(_curThread == _pool.end())
        _curThread = _pool.begin();
}

} // namespace CTL

#endif // THREADPOOL_H
