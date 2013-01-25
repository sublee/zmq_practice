#define localhost(port) (boost::format("tcp://127.0.0.1:%1%") % (port)).str()

#include "apps.hpp"
#include "utils.hpp"

#include "boost/format.hpp"
#include "boost/thread.hpp"
#include "zmq.hpp"

#include <conio.h>
#include <iostream>
#include <string>
#include <vector>

zmq::context_t ctx(1);

const std::string zmq_version_string() {
  int major, minor, patch;
  zmq::version(&major, &minor, &patch);
  return (boost::format("%1%.%2%.%3%") % major % minor % patch).str();
}

void close_context(const int delay_ms) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(delay_ms));
  ctx.close();
}

int main(int argc, char **argv) {
  std::string name = "Main";
  const int frontend_base_port = 50000;
  const int backend_base_port = 60000;
  // print the ZeroMQ version
  log(name, "ZeroMQ version is " + zmq_version_string());
  // spawn threads for the applications
  boost::thread_group threads;
  //threads.add_thread(new boost::thread(close_context, 5000));
  std::vector<const std::string> frontend_endpoints;
  std::vector<const std::string> backend_endpoints;
  for (int i = 0; i < 2; ++i) {
    const std::string name = (boost::format("Proxy%1%") % (i + 1)).str();
    const std::string frontend_endpoint = localhost(frontend_base_port + i);
    const std::string backend_endpoint = localhost(backend_base_port + i);
    frontend_endpoints.push_back(frontend_endpoint);
    backend_endpoints.push_back(backend_endpoint);
    threads.add_thread(new boost::thread(run_proxy, boost::ref(ctx), name,
                                         frontend_endpoint, backend_endpoint,
                                         (zmq::socket_t *)NULL));
  }
  for (int i = 0; i < 3; ++i) {
    const std::string name = (boost::format("Server%1%") % (i + 1)).str();
    threads.add_thread(new boost::thread(run_server, boost::ref(ctx), name,
                                         backend_endpoints));
  }
  for (int i = 0; i < 2; ++i) {
    const std::string name = (boost::format("Client%1%") % (i + 1)).str();
    threads.add_thread(new boost::thread(run_client, boost::ref(ctx), name,
                                         frontend_endpoints));
  }
  log(name, "Joining all threads...");
  threads.join_all();
  return 0;
}