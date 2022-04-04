#ifndef ASEBA_NETWORK_H_INCLUDED
#define ASEBA_NETWORK_H_INCLUDED

#ifndef ASEBA_ASSERT
#define ASEBA_ASSERT
#endif

#include "aseba_node.h"

namespace Aseba {

void spin(float dt);

template<typename T>
T * create_node(unsigned uid, unsigned port = 33333,
                const std::string & prefix = "node", int script_handle = -1,
                const std::array<uint8_t, 16> & uuid_ = {0},
                const std::string & friendly_name_ = "");

void destroy_node(unsigned uid);
void destroy_all_nodes();
DynamicAsebaNode *node_with_handle(int handle);

void remove_all_networks();
void remove_network_with_port(int port);
std::vector<node_t> node_list(unsigned port);
DynamicAsebaNode * node_for_vm(AsebaVMState * vm);

}

#endif // ASEBA_NETWORK_H_INCLUDED
