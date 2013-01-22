#ifndef SERVER_ENDPOINT
//#define SERVER_ENDPOINT ("inproc://sublee-feed")
#define SERVER_ENDPOINT ("tcp://*:6942")
#endif
#ifndef CLIENT_ENDPOINT
//#define CLIENT_ENDPOINT ("inproc://sublee-feed")
#define CLIENT_ENDPOINT ("tcp://127.0.0.1:6942")
#endif

#include "dsl.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"
#include "boost/thread.hpp"
#include "boost/thread/locks.hpp"
#include "zmq.hpp"

#include <conio.h>

#include <iostream>
#include <string>
#include <vector>

void server(const std::string, const std::string);
void client(const std::string, const std::string);
void log(const std::string, const std::string);
void log(const std::string, const boost::format);

zmq::context_t ctx(1);
boost::mutex cout_lock;

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
	threads.add_thread(new boost::thread(server, "Server", SERVER_ENDPOINT));
	for (int i = 0; i < 10; ++i)
	{
		const std::string client_name = (boost::format("Client%1%") % (i + 1)).str();
		threads.add_thread(new boost::thread(client, client_name, CLIENT_ENDPOINT));
	}
	log(name, "Joining all threads...");
	threads.join_all();

	return 0;
}

void server(const std::string name, const std::string endpoint)
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

void client(const std::string name, const std::string endpoint)
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
			int err = zmq_errno();
			switch (err)
			{
			case ECONNREFUSED:
				log(name, "The routher refused the connection");
				break;
			default:
				log(name, boost::format("Failed to connect to the router by %1%") % err);
			}
			continue;
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