#include "zeroconf-thread.h"
#include <iostream>
#include <string>
#include <unistd.h>

static void advertise(const std::string &name, int port, float wait_time) {
  Aseba::ThreadZeroconf zeroconf;
  const Aseba::Zeroconf::TxtRecord txt{9, name, false};
  zeroconf.advertise(name, port, txt);
  sleep(wait_time);
  exit(0);
}

static void show_usage(std::string name) {
  std::cout << "Usage: " << name << " <name> <port> [<wait seconds>]" << std::endl;
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
      advertise(name, port, wait_time);
      return 0;
    }
  }

  show_usage(argv[0]);
  return 1;
}
