#ifndef PP_VIRTUAL_RAM_HH
# define PP_VIRTUAL_RAM_HH

namespace pp {
  template <unsigned long size>
  struct virtual_ram {
    virtual_ram() {
      ram = (char*) malloc(size);
    }

    template <typename T>
    void get(unsigned offset, T*& ptr) {
      ptr = ((T*) ram) + offset;
    }
  private:
    void* ram;
  };
} // ::pp

#endif // !PP_VIRTUAL_RAM_HH
