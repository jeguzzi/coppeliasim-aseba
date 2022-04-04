#include "aseba_node.h"

#include "vm/natives.h"
#include "common/productids.h"
#include <string>


// char *dydata(std::string value)
// {
//   char *c = (char *) malloc(value.length() + 1);
//   value.copy(c, value.length());
//   c[value.length()] = 0;
//   return c;
// }

char * dydata(std::string value) {
  // printf("dydata for %s %lu\n", value.c_str(), value.length());
  char *c = (char *) malloc(value.length() + 1);
  value.copy(c, value.length());
  c[value.length()] = 0;
  // printf("-> %p %s\n", (void *)c, c);
  return c;
}

// From Asabe v2
#define ASEBA_MESSAGE_DEVICE_INFO 0x900D
#define DEVICE_INFO_UUID 1
#define DEVICE_INFO_NAME 2


void DynamicAsebaNode::send_uuid(const std::array<uint8_t, 16> & uuid) {
  printf("Send device uuid\n");
  uint8_t size = uuid.size();
  std::vector<uint8_t> payload = {DEVICE_INFO_UUID, size};
  std::copy(uuid.begin(), uuid.end(), std::back_inserter(payload));
  AsebaSendMessage(&vm, ASEBA_MESSAGE_DEVICE_INFO, payload.data(), payload.size());
}

void DynamicAsebaNode::send_friendly_name(const std::string & name) {
  printf("Send device name %s\n", name.c_str());
  uint8_t size = name.length() + 1;
  std::vector<uint8_t> payload = {DEVICE_INFO_NAME, size};
  std::copy(name.c_str(), name.c_str() + name.length() + 1, std::back_inserter(payload));
  AsebaSendMessage(&vm, ASEBA_MESSAGE_DEVICE_INFO, payload.data(), payload.size());
}
