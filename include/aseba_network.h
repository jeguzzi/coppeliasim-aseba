#ifndef ASEBA_NETWORK_H_INCLUDED
#define ASEBA_NETWORK_H_INCLUDED

#ifndef ASEBA_ASSERT
#define ASEBA_ASSERT
#endif

#include "aseba_node.h"

namespace Aseba {

void set_address(const std::string &);
void configure_advertisement(bool enabled, bool external);
void spin(float dt);
void add_node(DynamicAsebaNode * node, unsigned port, unsigned uid);
void destroy_node(unsigned uid);
void destroy_all_nodes();
DynamicAsebaNode *node_with_handle(int handle);
void remove_all_networks();
void remove_network_with_port(int port);
std::map<unsigned, std::vector<DynamicAsebaNode *>> node_list(unsigned port);
DynamicAsebaNode * node_for_vm(AsebaVMState * vm);

template<typename T>
T * create_node(unsigned uid, unsigned port = 33333,
                const std::string & prefix = "node",
                const std::array<uint8_t, 16> & uuid_ = {0},
                const std::string & friendly_name_ = "") {
  log_info("Will create a node with id %d (%x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x) and name %s (%s) "
           "and add it to the network with port %d",
           uid, uuid_[0], uuid_[1], uuid_[2], uuid_[3], uuid_[4], uuid_[5], uuid_[6], uuid_[7],
           uuid_[8], uuid_[9], uuid_[10], uuid_[11], uuid_[12], uuid_[13], uuid_[14], uuid_[15],
           prefix.c_str(), friendly_name_.c_str(), port);
  T *node = new T(uid, prefix, uuid_, friendly_name_);
  node->init_descriptions();
  add_node(node, port, uid);
  log_info("Created aseba node with id %d", node->vm.nodeId);
  return node;
}

}

#endif // ASEBA_NETWORK_H_INCLUDED
