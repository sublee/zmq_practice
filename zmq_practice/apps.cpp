#include "utils.hpp"

#include "boost/format.hpp"
#include "boost/thread.hpp"
#include "zmq.hpp"

#include <iostream>
#include <string>

void server(zmq::context_t& ctx, const std::string name, const std::string endpoint)
{
    log(name, "Starting the server...");

    int64_t has_more = 0;
    zmq::socket_t socket(ctx, ZMQ_ROUTER);

    socket.bind(endpoint.c_str());
    log(name, boost::format("Bound to %1%") % endpoint);
    while (1)
    {
        zmq::message_t msg;
        socket.recv(&msg);

        // check if the socket should receive more multi-part messages
        size_t opt_size = sizeof(has_more);
        socket.getsockopt(ZMQ_RCVMORE, &has_more, &opt_size);
        if (has_more)
        {
            continue;
        }

        // print the received message
        std::string data(static_cast<char *>(msg.data()), msg.size());
        log(name, boost::format("Received: %1% (%2% bytes)") % data % msg.size());
    }
}

void client(zmq::context_t& ctx, const std::string name, const std::string endpoint)
{
    log(name, "Starting the client...");

    zmq::socket_t socket(ctx, ZMQ_DEALER);
    std::string data = "Hello, world";

    log(name, boost::format("Connecting to the router: %1%...") % endpoint);
    while (1)
    {
        try
        {
            socket.connect(endpoint.c_str());
            break;
        }
        catch (zmq::error_t e)
        {
            log(name, boost::format("Failed to connect to the router: %1%") % zmq_strerror(zmq_errno()));
        }
    }
    log(name, boost::format("Connected to the router: %1%") % endpoint);

    while (socket.connected())
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
        zmq::message_t msg(data.size());
        memcpy(msg.data(), data.data(), data.size());
        log(name, boost::format("Send message: %1% (%2% bytes)") % data % msg.size());
        socket.send(msg);
    }
}