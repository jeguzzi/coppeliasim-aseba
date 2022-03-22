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
