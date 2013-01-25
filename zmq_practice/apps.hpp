#include "zmq.hpp"

#include <string>
#include <vector>

void run_server(zmq::context_t &, const std::string &,
                const std::vector<const std::string> &);
void run_client(zmq::context_t &, const std::string &,
                const std::vector<const std::string> &);
void run_proxy(zmq::context_t &, const std::string &,
               const std::string &, const std::string &, zmq::socket_t *);