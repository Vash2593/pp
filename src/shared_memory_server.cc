// C
#include <unistd.h>

// C++
#include <unordered_map>
#include <iostream>
#include <thread>

// Boost
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

// Me
#include "virtual_ram.hh"

template <typename Ram>
void access(Ram ram, boost::asio::ip::tcp::socket& socket) {
}

int main(int argc, char** argv) {
  assert(argc == 2);
  using boost::asio::ip::tcp;
  using Ram = pp::virtual_ram<4096>;
  Ram ram;
  boost::asio::io_service io_service;

  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), boost::lexical_cast<int>(argv[1])));
  std::cout << "Waiting for connections..." << std::endl;

  for (;;) {
    tcp::socket socket(io_service);
    acceptor.accept(socket);

    while (true) {
      char type;
      unsigned long location;
      unsigned size;
      size_t len;

      len = socket.read_some(boost::asio::buffer(&type, 1));
      assert(len == 1);
      assert (type == 'r' || type == 'w');
      // Read the location
      len = socket.read_some(boost::asio::buffer(&location, sizeof (location)));
      assert(len == sizeof (location));
      // Read the size
      len = socket.read_some(boost::asio::buffer(&size, sizeof (size)));
      assert(len == sizeof (size));
      // Any way, we need this pointer
      unsigned* ptr;
      ram.get(location, ptr);
      if (type == 'r') {
        socket.write_some(boost::asio::buffer(ptr, size));
      }
      else {
        len = socket.read_some(boost::asio::buffer(ptr, size));
        assert(len == size);
        std::cout << "write at " << location << std::endl;
        for (unsigned i = 0; i < size; ++i) {
          std::cout << "write: " << (int) ((char*)ptr)[i] << std::endl;
        }
        std::cout << "end write" << std::endl;
      }
    }
  }
}
