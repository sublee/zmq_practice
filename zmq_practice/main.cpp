#ifndef SERVER_ENDPOINT
//#define SERVER_ENDPOINT ("inproc://sublee-feed")
#define SERVER_ENDPOINT ("tcp://*:6942")
#endif
#ifndef CLIENT_ENDPOINT
//#define CLIENT_ENDPOINT ("inproc://sublee-feed")
#define CLIENT_ENDPOINT ("tcp://127.0.0.1:6942")
#endif

#include "apps.hpp"
#include "utils.hpp"

#include "boost/format.hpp"
#include "boost/thread.hpp"
#include "zmq.hpp"

#include <conio.h>
#include <iostream>
#include <string>

zmq::context_t ctx(1);

int main(int argc, char** argv)
{
    std::string name = "Main";
    
    // print the ZeroMQ version
    int zmq_major;
    int zmq_minor;
    int zmq_patch;
    zmq::version(&zmq_major, &zmq_minor, &zmq_patch);
    log(name, boost::format("ZeroMQ version is %1%.%2%.%3%") % zmq_major % zmq_minor % zmq_patch);

    // spawn threads for a server and clients
    boost::thread_group threads;
    threads.add_thread(new boost::thread(server, boost::ref(ctx), "Server", SERVER_ENDPOINT));
    for (int i = 0; i < 10; ++i)
    {
        const std::string client_name = (boost::format("Client%1%") % (i + 1)).str();
        threads.add_thread(new boost::thread(client, boost::ref(ctx), client_name, CLIENT_ENDPOINT));
    }
    log(name, "Joining all threads...");
    threads.join_all();

    return 0;
}