#include <iostream>

#include "logging.h"
#include "aseba_network.h"
#include <chrono>

static void show_usage(std::string name) {
  std::cout << "Usage: " << name << " <option(s)>" << std::endl
            << "Options:" << std::endl
            << "  --help\t\t\tShow this help message" << std::endl
            << "  --ip=<IP>\t\tDashel ip (default: 0.0.0.0)" << std::endl
            << "  --port=<PORT>\t\tDashel port (default: 33333)" << std::endl
            << "  --name=<NAME>\t\tFriedly name (default: '')" << std::endl
            << "  --wait=<WAIT>\t\tThe duration of the run" << std::endl;
}

int main(int argc, char **argv) {
  std::cout << std::endl << "Welcome" << std::endl;
  unsigned port = 33333;
  char address[17] = "0.0.0.0";
  char name[56] = "";
  unsigned wait = 5;
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i] << std::endl;
    if (sscanf(argv[i], "--ip=%16s", address)) {
      continue;
    }
    if (sscanf(argv[i], "--port=%u", &port)) {
      continue;
    }
    if (sscanf(argv[i], "--name=%55c", name)) {
      continue;
    }
    if (sscanf(argv[i], "--wait=%u", &wait)) {
      continue;
    }
    if (strcmp(argv[i], "--help") == 0) {
      show_usage(argv[0]);
      return 0;
    }
  }
  Aseba::set_address(address);
  Aseba::create_node<DynamicAsebaNode>(0, port, "node", {0}, name);
  auto start = std::chrono::system_clock::now();
  while (true) {
    Aseba::spin(0.1);
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = now - start;
    if (diff.count() > wait) break;
  }
  Aseba::remove_all_networks();
  std::cout << "Goodbye"<< std::endl;
  return 0;
}
