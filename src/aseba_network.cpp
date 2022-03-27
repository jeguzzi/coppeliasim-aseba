/*
TODO: preamble
*/

#include "aseba_network.h"
#include "aseba_thymio2.h"
#include <dashel/dashel.h>
#include "common/zeroconf/zeroconf-dashelhub.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stack>


class AsebaDashel : public Dashel::Hub {
 private:
  // stream for listening to incoming connections
  Dashel::Stream *listenStream;
  int timeout;
  int port;
  int next_id;
  std::vector<Dashel::Stream *> toDisconnect;
  std::string advertised_target;

 public:
  std::map<int, DynamicAsebaNode *> nodes;
  // this must be public because of bindings to C functions
  Dashel::Stream *stream;
#ifdef ZEROCONF
  Aseba::DashelhubZeroconf zeroconf;
#endif
  // all streams that must be disconnected at next step
  explicit AsebaDashel(const int port = ASEBA_DEFAULT_PORT, int timeout = 0)
      : port(port), timeout(timeout), stream(NULL), next_id(0)
#ifdef ZEROCONF
      , zeroconf(*this)
#endif
      {
    advertised_target = std::string("CoppeliaSim ") + std::to_string(port);
    listenStream = listen();
    printf("Listening on tcp:port=%s\n", listenStream->getTargetParameter("port").c_str());
  }

  ~AsebaDashel() {
    for (auto kv : nodes) {
      delete kv.second;
    }
    printf("Deleted network on :%d\n", port);
  }

  void add_node(DynamicAsebaNode *node) {
    nodes[node->vm.nodeId] = node;
#ifdef ZEROCONF
    advertise();
#endif
  }

  void remove_node(DynamicAsebaNode *node) {
    nodes.erase(node->vm.nodeId);
#ifdef ZEROCONF
    if (nodes.size() == 0) {
      deadvertise();
    }
#endif
  }

#ifdef ZEROCONF
  void advertise() {
    printf("advertise network\n");
    std::vector<unsigned int> ids;
    std::vector<unsigned int> pids;
    std::string names = "";
    unsigned protocolVersion{99};
    for (auto const & [id, node] : nodes) {
      ids.push_back(id);
      names += node->name + " ";
      pids.push_back(8);  // product id
    }
    Aseba::Zeroconf::TxtRecord txt{protocolVersion, names, false, ids, pids};
    try {
      printf("Will advertise %s with %s\n", advertised_target.c_str(), txt.record().c_str());
      zeroconf.advertise(advertised_target, listenStream, txt);
    } catch (const std::runtime_error& e) {
      printf("Could not advertise: %s\n", e.what());
    }
  }

  void deadvertise() {
    zeroconf.forget(advertised_target, listenStream);
  }
#endif

  Dashel::Stream *listen() {
    // connect network
    try {
      std::ostringstream oss;
      oss << "tcpin:port=" << port;
      listenStream = Dashel::Hub::connect(oss.str());
    } catch (Dashel::DashelException e) {
      printf("Cannot create listening port %d: %s\n", port, e.what());
      abort();
    }
    return listenStream;
  }

  virtual void connectionCreated(Dashel::Stream *stream) {
    std::string targetName = stream->getTargetName();
    if (targetName.substr(0, targetName.find_first_of(':')) == "tcp") {
      // schedule current stream for disconnection
      if (this->stream)
        toDisconnect.push_back(this->stream);
      // set new stream as current stream
      this->stream = stream;
      printf("New client connected.\n");
    }
  }

  virtual void connectionClosed(Dashel::Stream *stream, bool abnormal) {
    this->stream = nullptr;
    // clear breakpoints
    for (auto kv : nodes) {
      (kv.second)->vm.breakpointsCount = 0;
    }
    if (abnormal)
      printf("Client has disconnected unexpectedly.\n");
    else
      printf("Client has disconnected properly.\n");
  }

  virtual void incomingData(Dashel::Stream *stream) {
    // only process data for the current stream
    if (stream != this->stream)
      return;

    uint16_t temp;
    uint16_t len;

    stream->read(&temp, 2);
    len = bswap16(temp);
    stream->read(&temp, 2);

    uint16_t lastMessageSource;
    std::valarray<uint8_t> lastMessageData;

    lastMessageSource = bswap16(temp);
    lastMessageData.resize(len + 2);
    stream->read(&lastMessageData[0], lastMessageData.size());

    uint16_t type = bswap16(lastMessageData[0]);
    std::vector<DynamicAsebaNode *> dest_nodes;
    /* from IDE to a specific node */
    if (type >= ASEBA_MESSAGE_SET_BYTECODE &&
        type <= ASEBA_MESSAGE_GET_NODE_DESCRIPTION) {
      uint16_t dest = bswap16(lastMessageData[1]);
      // printf("Got message of type %d from IDE (%d) for node %d\n",
      //         type, lastMessageSource, dest);
      if (nodes.count(dest)) {
        DynamicAsebaNode *node = nodes[dest];
        if (node->finalized) {
          node->lastMessageSource = lastMessageSource;
          node->lastMessageData = lastMessageData;
          AsebaProcessIncomingEvents(&(node->vm));
          AsebaVMRun(&(node->vm), 1000);
        }
        return;
      }
    }
    // printf("Got message of type %d from node %d\n", type, lastMessageSource);
    for (auto kv : nodes) {
      DynamicAsebaNode *node = kv.second;
      if (node->finalized) {
        node->lastMessageSource = lastMessageSource;
        node->lastMessageData = lastMessageData;
        AsebaProcessIncomingEvents(&(node->vm));
        AsebaVMRun(&(node->vm), 1000);
      }
    }
  }

  bool spin(float dt) {
    if (!step(timeout))
      return false;
    for (const auto kv : nodes) {
      auto node = kv.second;
      // if (node->finalized)
      //   node->step(dt);
      node->finalized = true;
      node->step(dt);
    }
    // disconnect old streams
    for (size_t i = 0; i < toDisconnect.size(); ++i) {
      closeStream(toDisconnect[i]);
      printf("Old client disconnected by new client.\n");
    }
    toDisconnect.clear();
    return true;
  }
  //
  // void run() {
  //   while (spin()) {}
  // }
};

// --------------  node collections

namespace Aseba {

// port -> network
static std::map<int, AsebaDashel *> networks;

AsebaDashel *network_with_port(int port, bool create = false) {
  if (networks.count(port))
    return networks[port];
  if (create) {
    networks[port] = new AsebaDashel(port);
    return networks[port];
  }
  return NULL;
}

void remove_network_with_port(int port) {
  if (!networks.count(port))
    return;
  AsebaDashel *network = networks[port];
  networks.erase(port);
  delete network;
}

void remove_all_networks() {
  for (auto kv : networks) {
    delete kv.second;
  }
  networks.clear();
}

// vm -> stream
static std::map<AsebaVMState *, std::pair<AsebaDashel *, DynamicAsebaNode *>> endpoints;

AsebaDashel *network_for_vm(AsebaVMState *vm) {
  if (endpoints.count(vm))
    return endpoints[vm].first;
  return NULL;
}

DynamicAsebaNode *node_for_vm(AsebaVMState *vm) {
  if (endpoints.count(vm))
    return endpoints[vm].second;
  return NULL;
}

// handle -> nodes
static std::map<int, DynamicAsebaNode *> nodes;

DynamicAsebaNode *node_with_handle(int handle) {
  if (nodes.count(handle))
    return nodes.at(handle);
  return NULL;
}

void add_node(DynamicAsebaNode *node, AsebaDashel *network, int handle) {
  endpoints[&(node->vm)] = std::make_pair(network, node);
  nodes[handle] = node;
  network->add_node(node);
}

void remove_node(DynamicAsebaNode *node, AsebaDashel *network, int handle) {
  endpoints.erase(&(node->vm));
  nodes.erase(handle);
  network->remove_node(node);
}

template<typename T>
T * create_node(unsigned uid, unsigned port, std::string prefix, int scriptID) {
  AsebaDashel *network = network_with_port(port, true);
  printf("Will create aseba node\n");
  // std::string name = prefix + "-" + std::to_string(uid);
  std::string name = prefix;
  printf("with name %s\n", name.c_str());
  T *node = new T(uid, name, scriptID);
  node->init_descriptions();
  printf("Add it to the network\n");
  add_node(node, network, uid);
  printf("Created aseba node with id %d (%d)\n", uid, node->vm.nodeId);
  return node;
}

template DynamicAsebaNode * create_node<DynamicAsebaNode>(unsigned, unsigned, std::string, int);
template AsebaThymio2 * create_node<AsebaThymio2>(unsigned, unsigned, std::string, int);


void destroy_node(unsigned uid) {
  DynamicAsebaNode *node = node_with_handle(uid);
  printf("destroy_node %p\n", node);
  if (node) {
    AsebaDashel *network = network_for_vm(&(node->vm));
    remove_node(node, network, uid);
    delete node;
  }
}

void destroy_all_nodes() {
  auto it = nodes.cbegin();
  while (it != nodes.cend()) {
    AsebaDashel *network = network_for_vm(&(it->second->vm));
    remove_node(it->second, network, it->first);
    nodes.erase(it++);
  }
}

std::vector<node_t> node_list(unsigned port) {
  std::vector<node_t> nodes;
  if (port < 0) {
    for (auto i : networks) {
      for (auto j : i.second->nodes) {
        auto node = j.second;
        node_t z;
        z.name = node->name;
        z.id = node->vm.nodeId;
        // z.handle = node->handle;
        z.port = i.first;
        nodes.push_back(z);
      }
    }
  } else {
    auto network = network_with_port(port);
    if (network) {
      for (auto j : network->nodes) {
        auto node = j.second;
        node_t z;
        z.name = node->name;
        z.id = node->vm.nodeId;
        // z.handle = node->handle;
        z.port = port;
        nodes.push_back(z);
      }
    }
  }
  return nodes;
}



// -------------- Implementation of aseba glue code

extern "C" void AsebaPutVmToSleep(AsebaVMState *vm) {
  printf("Received request to go into sleep\n");
}

extern "C" void AsebaSendBuffer(AsebaVMState *vm, const uint8_t *data,
                                uint16_t length) {
  Dashel::Stream *stream = network_for_vm(vm)->stream;
  if (stream) {
    try {
      uint16_t temp;
      temp = bswap16(length - 2);
      stream->write(&temp, 2);
      temp = bswap16(vm->nodeId);
      stream->write(&temp, 2);
      stream->write(data, length);
      stream->flush();
    } catch (Dashel::DashelException e) {
      printf("Cannot write to socket: %s", stream->getFailReason().c_str());
    }
  }
}

extern "C" uint16_t AsebaGetBuffer(AsebaVMState *vm, uint8_t *data,
                                   uint16_t maxLength, uint16_t *source) {
  DynamicAsebaNode *node = node_for_vm(vm);
  if (node->lastMessageData.size()) {
    *source = node->lastMessageSource;
    memcpy(data, &node->lastMessageData[0], node->lastMessageData.size());
  }
  return node->lastMessageData.size();
}

extern "C" const AsebaVMDescription * AsebaGetVMDescription(AsebaVMState *vm) {
  // printf("Got Node description name: %s\n",
  // node_with_vm[vm]->node_description->name);
  return node_for_vm(vm)->variables_description;
}

extern "C" const AsebaNativeFunctionDescription * const *
AsebaGetNativeFunctionsDescriptions(AsebaVMState *vm) {
  return node_for_vm(vm)->functions_description;
}

extern "C" void AsebaNativeFunction(AsebaVMState *vm, uint16_t id) {
  // printf("AsebaNativeFunction %d\n", id);
  DynamicAsebaNode *node = node_for_vm(vm);
  if (!node) return;
  if (id < node->number_of_native_function) {
    node->native_functions()[id](vm);
    return;
  }
  id -= node->number_of_native_function;
  if (id < node->lua_functions.size()) {
    int stack_id = simCreateStack();
    // write__`info.typespec.normalized()`Request(req, stack, &(proxy->wr_opt));
    // simCallScriptFunctionExE(proxy->serviceCallback.scriptId,
    // proxy->serviceCallback.name.c_str(), stack);
    // read__`info.typespec.normalized()`Response(stack, &res,
    // &(proxy->rd_opt));
    //
    // stack = -1;
    // return true;

    // How many arguments?
    auto pair = node->lua_functions[id];
    std::vector<int> sizes = pair.second;
    int number_of_arguments = sizes.size();
    std::stack<uint16_t> addresses;
    // TODO(Jerome): take into account dynamic lengths (i.e. size = -1, -2 , ...)
    for (auto size : sizes) {
      uint16_t address = AsebaNativePopArg(vm);
      addresses.push(address);
      // TODO(Jerome): variable length arrays are not part of any C++ standart!
      // Replace witha a vector -> vector.data ~(or array) no, it only accept
      // const expr as sizes~ int32_t values[size];
      std::vector<int32_t> values(size);

      for (size_t i = 0; i < size; i++) {
        values[i] = (int32_t)vm->variables[address + i];
      }
      simPushInt32TableOntoStack(stack_id, values.data(), size);
      printf("IN [%d] = [%d, ...]\n", address, values[0]);
    }

    printf("IN stack size %d\n", simGetStackSize(stack_id));

    printf("Will call LUA function %s for script %d with %d arguments\n",
           pair.first.c_str(), node->script_id, number_of_arguments);
    int res =
        simCallScriptFunctionEx(node->script_id, pair.first.c_str(), stack_id);
    printf("Has called LUA function -> %d\n", res);
    int number_of_results = simGetStackSize(stack_id);
    printf("OUT stack size %d\n", number_of_results);
    int num = std::min(number_of_arguments, number_of_results);
    for (int i = 0; i < num; i++) {
      int size = sizes[number_of_arguments - i - 1];
      uint16_t address = addresses.top();
      // int32_t values[size];
      // TODO(Jerome): check that it works!!!
      std::vector<int32_t> values(size);
      simGetStackInt32Table(stack_id, values.data(), size);
      printf("OUT [%d] = [%d, ...]\n", address, values[0]);
      for (size_t i = 0; i < size; i++) {
        // TODO(Jerome): check that the casting is correct
        vm->variables[address + i] = (uint16_t)values[i];
      }
      simPopStackItem(stack_id, 1);
      addresses.pop();
    }
    simReleaseStack(stack_id);
    return;
  }
}

extern "C" const AsebaLocalEventDescription *
AsebaGetLocalEventsDescriptions(AsebaVMState *vm) {
  return node_for_vm(vm)->events_description;
}

extern "C" void AsebaWriteBytecode(AsebaVMState *vm) {
  printf("Received request to write bytecode into flash\n");
}

extern "C" void AsebaResetIntoBootloader(AsebaVMState *vm) {
  printf("Received request to reset into bootloader\n");
}

extern "C" void AsebaAssert(AsebaVMState *vm, AsebaAssertReason reason) {
  printf("\nFatal error; exception: ");
  switch (reason) {
  case ASEBA_ASSERT_UNKNOWN:
    printf("undefined");
    break;
  case ASEBA_ASSERT_UNKNOWN_BINARY_OPERATOR:
    printf("unknown binary operator");
    break;
  case ASEBA_ASSERT_UNKNOWN_BYTECODE:
    printf("unknown bytecode");
    break;
  case ASEBA_ASSERT_STACK_OVERFLOW:
    printf("stack overflow");
    break;
  case ASEBA_ASSERT_STACK_UNDERFLOW:
    printf("stack underflow");
    break;
  case ASEBA_ASSERT_OUT_OF_VARIABLES_BOUNDS:
    printf("out of variables bounds");
    break;
  case ASEBA_ASSERT_OUT_OF_BYTECODE_BOUNDS:
    printf("out of bytecode bounds");
    break;
  case ASEBA_ASSERT_STEP_OUT_OF_RUN:
    printf("step out of run");
    break;
  case ASEBA_ASSERT_BREAKPOINT_OUT_OF_BYTECODE_BOUNDS:
    printf("breakpoint out of bytecode bounds");
    break;
  default:
    printf("unknown exception");
    break;
  }
  printf(".\npc = %d, sp = %d\n", vm->pc, vm->sp);
  abort();
  printf("\nResetting VM\n");
  AsebaVMInit(vm);
}

// Plugin class helpers

void spin(float dt) {
  for (const auto &kv : networks) {
    kv.second->spin(dt);
  }
}
}  // namespace Aseba

// simGetScriptAssociatedWithObject
