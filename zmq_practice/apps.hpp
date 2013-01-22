#include "zmq.hpp"

#include <string>

void server(zmq::context_t&, const std::string, const std::string);
void client(zmq::context_t&, const std::string, const std::string);
void publisher(zmq::context_t&, const std::string, const std::string);
void subscriber(zmq::context_t&, const std::string, const std::string);