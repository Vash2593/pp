#ifndef PP_SHARED_MEMORY_HH
# define PP_SHARED_MEMORY_HH

# include <unistd.h>
# include <stdlib.h>
# include <string.h>

# include <array>

# include <boost/asio.hpp>
# include <boost/array.hpp>

namespace pp {

  struct host {
    virtual void* read(void* addr, size_t size) = 0;
    virtual void write(void* src, void* dst, size_t size) = 0;
  };

  struct local_host : public host {
    void* read(void* addr, size_t size) {
      void* local = malloc(size);
      memcpy(local, addr, size);

      return local;
    }

    void write(void* src, void* dst, size_t size) {
      memcpy(src, dst, size);
    }
  };

  using boost::asio::ip::tcp;

  struct distributed_host : public host {
  private:
    using string = const std::string;
  public:
    distributed_host() {
      tcp::resolver resolver(io_service_);

      tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 4251);

      socket_ = new tcp::socket(io_service_);

      socket_->connect(endpoint);

    }
    ~distributed_host() {
      delete socket_;
    }

    virtual void* read(void* addr, size_t size) {
      void* data = malloc(size);

      header('r', addr, size);
      socket_->read_some(boost::asio::buffer(data, size));

      return data;
    }

    virtual void write(void* src, void* dst, size_t size) {
      header('w', dst, size);
      socket_->write_some(boost::asio::buffer(src, size));
    }

  private:
    void header(const char op, void* addr, size_t size) {
      boost::asio::write(*socket_, boost::asio::buffer(&op, 1));
      socket_->write_some(boost::asio::buffer(&addr, 8));
      socket_->write_some(boost::asio::buffer(&size, 4));
    }

  private:
    boost::asio::io_service io_service_;
    tcp::socket* socket_;

  };

  /**
   * This class should not be shared or at least with a policy
   * wrapper.
   */
  template <unsigned redundacy = 2>
  class accessor {
  public:
    accessor(std::array<host*, redundacy> hosts)
      : hosts_(hosts)
      , count_(0) {}

    template <typename T>
    T* read(void* addr, size_t number) {
      ++count_;
      if (count_ >= redundacy) {
        count_ = 0;
      }
      return (T*) hosts_[count_]->read(addr, number * sizeof(T));
    }

    template <typename T>
    void write(T* src, void* dst, size_t number) {
      for (unsigned i = 0; i < redundacy; ++i) {
        hosts_[i]->write(src, dst, number * sizeof (T));
      }
    }

    // void write(void* src, void* dst, size_t size) {
    //   for (int i = 0; i < redundacy; ++i) {
    //     hosts_[i]->write(src, dst, size);
    //   }
    // }

  private:
    std::array<host*, redundacy> hosts_;
    unsigned count_ = 0;
  };
} // ::pp

#endif // !PP_SHARED_MEMORY_HH
