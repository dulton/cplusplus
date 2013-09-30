#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "SyncQueue.h"

class ThreadPool : boost::noncopyable
{
public:

    ThreadPool():m_running(false),m_queue(MaxTaskCount)
    {

    }

    ~ThreadPool(void)
    {
    }

    typedef boost::function<void ()> Task;

    void Start(int numThreads)
    {
        m_running = true;
        m_threads.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i)
        {
            char id[32];
            itoa(i, id, sizeof(id));
            std::thread* thd = new std::thread(std::bind(&ThreadPool::RunInThread, this), id);
            m_threads.push_back(thd);
        }
    }

    void AddTask(const Task& task)
    {
        if (m_threads.empty())
        {
            task();
        }
        else
        {
            m_queue.Put(task);
        }
    }

    void Stop()
    {
　　 　　m_running = false;
    　　for_each(m_threads.begin(),m_threads.end(),std::bind(&std::thread::join, std::placeholders::_1));
    }

private:
    void RunInThread()
    {
        while (m_running)
        {
            Task task;
            m_queue.Take(task);
            if (task)
            {
                task();
            }                
        }
    }

    Task GetTask()
    {
        Task task;
        m_queue.Take(task);
        return task;
    }

    boost::ptr_vector<std::thread> m_threads;
    SyncQueue<Task> m_queue;
    bool m_running;
};