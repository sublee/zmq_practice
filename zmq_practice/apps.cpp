#include "apps.hpp"
#include "utils.hpp"

#include "boost/format.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/thread.hpp"
#include "zmq.hpp"

#include <iostream>
#include <string>
#include <vector>

bool connect(const std::string &name, zmq::socket_t &socket,
             const std::string &endpoint) {
  const int max_tries = 8;
  log(name, boost::format("Connecting to the proxy: %1%...") % endpoint);
  for (int i = 0; i < max_tries; ++i) {
    try {
      socket.connect(endpoint.c_str());
    } catch (zmq::error_t e) {
      boost::format fmt("Failed to connect to the proxy: %1%");
      log(name, fmt % zmq_strerror(zmq_errno()));
      continue;
    }
    boost::format fmt("Connected to the proxy: %1%");
    log(name, fmt % endpoint);
    return true;
  }
  return false;
}

void run_server(zmq::context_t &ctx, const std::string &name,
                const std::vector<const std::string> &endpoints) {
  log(name, "Starting the server...");
  int64_t has_more = 0;
  zmq::socket_t socket(ctx, ZMQ_ROUTER);
  bool connected = false;
  for (const std::string endpoint : endpoints) {
    connected |= connect(name, socket, endpoint);
  }
  while (connected) {
    zmq::message_t msg(256);
    socket.recv(&msg);
    // check if the socket should receive more multi-part messages
    size_t opt_size = sizeof(has_more);
    socket.getsockopt(ZMQ_RCVMORE, &has_more, &opt_size);
    if (has_more) {
      continue;
    }
    size_t msg_size = msg.size();
    if (!msg_size) {
      break;
    }
    // print the received message
    std::string data((char *)msg.data(), msg_size);
    boost::format fmt("Received: %1% (%2% bytes)");
    log(name, fmt % data % msg_size);
  }
  log(name, "End of the server");
}

void run_client(zmq::context_t &ctx, const std::string &name,
                const std::vector<const std::string> &endpoints) {
  log(name, "Starting the client...");
  const std::string data = "Hello, I'm " + name;
  zmq::socket_t socket(ctx, ZMQ_DEALER);
  bool connected = false;
  for (const std::string endpoint : endpoints) {
    connected |= connect(name, socket, endpoint);
  }
  while (connected) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    // make message
    zmq::message_t msg(data.size());
    memcpy(msg.data(), data.data(), data.size());
    // send message
    boost::format fmt("Send message: %1% (%2% bytes)");
    log(name, fmt % data % msg.size());
    socket.send(msg);
  }
  log(name, "End of the client");
}

void run_proxy(zmq::context_t &ctx, const std::string &name,
               const std::string &frontend_endpoint,
               const std::string &backend_endpoint,
               zmq::socket_t *capture_socket_ptr) {
  boost::format fmt("Starting the proxy... %1% -> %2%");
  log(name, fmt % frontend_endpoint % backend_endpoint);
  zmq::socket_t frontend_socket(ctx, ZMQ_PULL);
  zmq::socket_t backend_socket(ctx, ZMQ_PUSH);
  frontend_socket.bind(frontend_endpoint.c_str());
  backend_socket.bind(backend_endpoint.c_str());
  zmq_proxy(frontend_socket, backend_socket, capture_socket_ptr); // it blocks
  log(name, "End of the proxy");
}