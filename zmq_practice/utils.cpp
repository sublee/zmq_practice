#include "utils.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"
#include "boost/thread.hpp"

#include <string>

boost::mutex cout_lock;

void log(const std::string name, const std::string line)
{
    using namespace boost::posix_time;

    ptime now(microsec_clock::local_time());
    
    std::ostringstream ss;
    time_facet* facet = new time_facet("%H:%M:%S.%f");
    ss.imbue(std::locale(ss.getloc(), facet));
    ss << now << " [" << name << "] " << line;

    synchronized (cout_lock)
    {
        std::cout << ss.str() << std::endl;
    }
}

void log(const std::string name, const boost::format fmt)
{
    log(name, fmt.str());
}