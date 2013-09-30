#include <mutex>
#include <thread>
#include <condition_variable>
template<typename T>
class SyncQueue : boost::noncopyable
{
    bool IsFull() const
    {
        return m_queue.size()==m_maxSize;
    }

    bool IsEmpty() const
    {
        return m_queue.empty();
    }
public:
    SyncQueue(int maxSize):m_maxSize(maxSize)
    {
    }

    void Put(const T& x)
    {        
        {
            std::lock_guard< std::recursive_mutex> locker(m_mutex);
            while (IsFull())
            {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
                m_notFull.wait(m_mutex);
            }        

            m_queue.push_back(x);
        }
        m_notEmpty.notify_one();
    }

    void Take(T& x)
    {
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            while (IsEmpty())
            {
　　　　　　　　　 m_queue.shrink_to_fit();
                m_notEmpty.wait(m_mutex);
            }

            x = m_queue.front();
            m_queue.pop_front();
        }
        m_notFull.notify_one(); 
    }

    bool Empty() 
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        return m_queue.empty();
    }

    bool Full()
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        return m_queue.size()==m_maxSize;
    }

    size_t Size() 
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        return m_queue.size();
    }

private:
    std::deque<T> m_queue;
    std::recursive_mutex m_mutex;
    std::condition_variable_any m_notEmpty;
    std::condition_variable_any m_notFull;
    int m_maxSize;
};