#include "zeroconf-thread.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else // _WIN32
#include <unistd.h>
#endif // _WIN32

static void advertise(const std::string &name, int port, float wait_time) {
  Aseba::ThreadZeroconf zeroconf;
  const Aseba::Zeroconf::TxtRecord txt{9, name, false};
  zeroconf.advertise(name, port, txt);
#ifdef _WIN32
  Sleep(static_cast<unsigned long>(wait_time * 1000));
#else  // _WIN32
  sleep(wait_time);
#endif // _WIN32
  exit(0);
}

static void show_usage(std::string name) {
  std::cout << "Usage: " << name << " <name> <port> [<wait seconds>]"
            << std::endl;
}

int main(int argc, char **argv) {
  unsigned port = 33333;
  char name[56] = "";
  float wait_time = 1.0;
  if (argc > 2) {
    if (argc > 3) {
      if (!sscanf(argv[3], "%f", &wait_time)) {
        show_usage(argv[0]);
        return 1;
      }
    }
    if (sscanf(argv[1], "%55c", name) && sscanf(argv[2], "%u", &port)) {
      std::cout << "Advertise " << name << " on port " << port << " for "
                << wait seconds << " seconds." << std::endl;
      advertise(name, port, wait_time);
      return 0;
    }
  }

  show_usage(argv[0]);
  return 1;
}
