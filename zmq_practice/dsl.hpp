#include "boost/thread.hpp"

// from http://ricanet.com/new/view.php?id=blog/050807 and http://ricanet.com/new/view.php?id=blog/050811a
class AutoLock
{
public:
    AutoLock(boost::detail::basic_timed_mutex& mutex) : _mutex(mutex)
    {
        _mutex.lock();
    }
    ~AutoLock()
    {
        _mutex.unlock();
    }
    operator bool()
    {
        return false;
    }
private:
    boost::detail::basic_timed_mutex& _mutex;
};

#define synchronized(x) if (AutoLock __ = x) assert(0); else