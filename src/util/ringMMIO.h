#ifndef UNTITLED_RINGMMIO_H
#define UNTITLED_RINGMMIO_H

#include <fcntl.h>
#include <string>
#include <sys/mman.h>

#define FILEMODE S_IRWXU | S_IRGRP | S_IROTH

template <class T> class RingMMIO {
public:
  RingMMIO(std::string address, int numDataPoints)
      : fileAddress(address), numDataPoints(numDataPoints) {
    fid = open(fileAddress.c_str(), O_RDWR | O_CREAT | O_TRUNC, FILEMODE);
    assert(fid);
    assert(ftruncate(fid, numDataPoints * sizeof(T)) == 0);
    data = reinterpret_cast<T *>(mmap(0, numDataPoints * sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fid, 0));
    close(fid);
  }

  RingMMIO &operator+=(const T &d) {
    data[dataCounter++ % numDataPoints] = d;
    return *this;
  }

private:
  int fid;
  std::string fileAddress;
  T *data;
  int dataCounter;
  int numDataPoints;
};

#endif // UNTITLED_RINGMMIO_H
