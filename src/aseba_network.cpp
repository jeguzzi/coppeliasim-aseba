/*
TODO: preamble
*/

#include "aseba_network.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stack>
#include <set>

#include "dashel/dashel.h"
#include "logging.h"
#include "common/zeroconf/zeroconf-dashelhub.h"


class AsebaDashel : public Dashel::Hub {
 private:
  inline static std::string address = "0.0.0.0";

 public:
  static void set_address(const std::string & a) {
    address = a;
  }

 private:
  // stream for listening to incoming connections
  Dashel::Stream *listenStream;
  int timeout;
  int port;
  int next_id;
  std::set<Dashel::Stream *> toDisconnect;
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
    // advertised_target = std::string("Not A Thymio 3: CoppeliaSim ") + std::to_string(port);
    advertised_target = std::string("CoppeliaSim ") + std::to_string(port);
    if (listen())
      log_info("Created Aseba network listening on tcp:port=%s",
               listenStream->getTargetParameter("port").c_str());
  }

  ~AsebaDashel() {
    for (auto kv : nodes) {
      delete kv.second;
    }
    log_info("Deleted network on tcp:port=%d", port);
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
    // if (nodes.size() == 0) {
    //   deadvertise();
    // }
    advertise();
#endif
  }

#ifdef ZEROCONF
  void advertise() {
    if(!listenStream) return;
    std::vector<unsigned int> ids;
    std::vector<unsigned int> pids;
    std::string name = "";
    unsigned protocolVersion{9};
    for (auto const & [id, node] : nodes) {
      std::string n_name = node->advertized_name();
      if (name.empty()) {
        name = n_name;
      } else if (name != n_name) {
        name = "Group";
      }
      ids.push_back(id);
      // names += node->name + " ";
      pids.push_back(8);  // product id
    }
    if (name.empty()) {
      name = "Empty Group";
    }
    // Aseba::Zeroconf::TxtRecord txt{protocolVersion, names, false, ids, pids};
    Aseba::Zeroconf::TxtRecord txt{protocolVersion, name, false, ids, pids};
    try {
      zeroconf.advertise(advertised_target, listenStream, txt);
      log_debug("Advertise Aseba network %s with %s",
                advertised_target.c_str(), txt.record().c_str());
    } catch (const std::runtime_error& e) {
      log_warn("Could not advertise: %s", e.what());
    }
  }

  void deadvertise() {
    if(!listenStream) return;
    log_debug("Deadvertise Aseba Network");
    zeroconf.forget(advertised_target, listenStream);
  }
#endif

  Dashel::Stream *listen() {
    // connect network
    try {
      std::ostringstream oss;
      oss << "tcpin:port=" << port << ";address=" << address;
      listenStream = Dashel::Hub::connect(oss.str());
    } catch (Dashel::DashelException e) {
      log_warn("Cannot create listening port %d: %s", port, e.what());
      listenStream = nullptr;
    }
    return listenStream;
  }

  virtual void connectionCreated(Dashel::Stream *stream) {
    std::string targetName = stream->getTargetName();
    log_info("Incoming Dashel connection from %s", targetName.c_str());
    if (targetName.substr(0, targetName.find_first_of(':')) == "tcp") {
      // schedule current stream for disconnection
      if (!this->stream) {
        this->stream = stream;
        log_info("Connection accepted");
      } else {
        log_info("Connection refused: we are already connected to a client stream");
        // ??? Dashel say not to call closeStream in connectionCreated ???
        // closeStream(stream);
        // but this (proper way) makes us crash (why?)
        toDisconnect.insert(stream);
      }
        // toDisconnect.push_back(this->stream);
      // set new stream as current stream
      // this->stream = stream;
      // printf("New client connected.\n");
    }
  }
#if 0
  virtual void connectionCreated(Dashel::Stream *stream) {
    std::string targetName = stream->getTargetName();
    printf("[DASHEL] connectionCreated %s\n", targetName.c_str());
    if (targetName.substr(0, targetName.find_first_of(':')) == "tcp") {
      // schedule current stream for disconnection
      if (this->stream)
        toDisconnect.push_back(this->stream);
      // set new stream as current stream
      this->stream = stream;
      printf("New client connected.\n");
    }
  }
#endif
  virtual void connectionClosed(Dashel::Stream *stream, bool abnormal) {
    log_info("Dashel connection closed");
#ifdef ZEROCONF_SUPPORT
    zeroconf.dashelConnectionClosed(stream);
#endif  // ZEROCONF_SUPPORT
    if (stream == this->stream) {
      this->stream = nullptr;
      // clear breakpoints
      for (auto kv : nodes) {
        (kv.second)->vm.breakpointsCount = 0;
      }
    }
    toDisconnect.erase(stream);
    if (abnormal)
      log_warn("Client has disconnected unexpectedly.");
    // else
      // printf("Client has disconnected properly.\n");
  }

  virtual void incomingData(Dashel::Stream *stream) {
#ifdef ZEROCONF_SUPPORT
  if (zeroconf.isStreamHandled(stream)) {
      log_debug("Incoming data for zeroconf");
      zeroconf.dashelIncomingData(stream);
      return;
  }
#endif  // ZEROCONF_SUPPORT

    // only process data for the current stream
    if (stream != this->stream) {
      // printf("[DASHEL] incomingData from %p (%p) -> ignore\n", stream, this->stream);
      return;
    }
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
    // uint16_t type = bswap16(lastMessageData[0]);
    uint16_t type;
    memcpy(&type, &lastMessageData[0], 2);
    type = bswap16(type);
    // memcpy(data, &node->lastMessageData[0], node->lastMessageData.size());

    // printf("[DASHEL] incomingData %d %d => %d\n", lastMessageData[0], lastMessageData[1], type);
    log_debug("Incoming data (%d bytes) of type %d from %d", len, type, lastMessageSource);
    std::vector<DynamicAsebaNode *> dest_nodes;
    /* from IDE to a specific node */
    if (type >= ASEBA_MESSAGE_SET_BYTECODE && type <= ASEBA_MESSAGE_GET_NODE_DESCRIPTION) {
      uint16_t dest;
      memcpy(&dest, &lastMessageData[2], 2);
      dest = bswap16(dest);
      // printf("Got message of type %d from IDE (%d) for node %d\n",
      //         type, lastMessageSource, dest);
      if (nodes.count(dest)) {
        DynamicAsebaNode *node = nodes.at(dest);
        if (node->finalized) {
          if (type == ASEBA_MESSAGE_GET_EXECUTION_STATE) {
            node->send_device_info((void *) stream);
          }

          node->lastMessageSource = lastMessageSource;
          node->lastMessageData = lastMessageData;
          AsebaProcessIncomingEvents(&(node->vm));
          AsebaVMRun(&(node->vm), 1000);
        }
      }
      return;
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
#ifdef ZEROCONF
    if (!zeroconf.dashelStep(timeout))
#else
    if (!step(timeout))
#endif  // ZEROCONF
      return false;

    for (const auto kv : nodes) {
      auto node = kv.second;
      // if (node->finalized)
      //   node->step(dt);
      node->finalized = true;
      node->step(dt);
    }
    // disconnect old streams
    lock();
    for (auto stream : toDisconnect) {
      closeStream(stream);
      log_info("Stream closed in spin");
    }
    toDisconnect.clear();
    unlock();
    return true;
  }
  //
  // void run() {
  //   while (spin()) {}
  // }
};

// --------------  node collections

namespace Aseba {


void set_address(const std::string & a) {
  AsebaDashel::set_address(a);
  log_info("Dashel IPv4 address set to %s", a.c_str());
}

// port -> network
static std::map<int, AsebaDashel *> networks;

AsebaDashel *network_with_port(int port, bool create = false) {
  if (networks.count(port))
    return networks[port];
  if (create) {
    networks[port] = new AsebaDashel(port);
    log_info("Added network with port %d", port);
    return networks[port];
  }
  return NULL;
}

void remove_network_with_port(int port) {
  if (!networks.count(port))
    return;
  AsebaDashel *network = networks[port];
  networks.erase(port);
  log_info("Removed network with port %d", port);
  delete network;
}

void remove_all_networks() {
  log_info("Will remove all networks");
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


void add_node(DynamicAsebaNode * node, unsigned port, unsigned uid) {
  AsebaDashel *network = network_with_port(port, true);
  add_node(node, network, uid);
}

// template<typename T>
// T * create_node(unsigned uid, unsigned port, const std::string & prefix,
//                 const std::array<uint8_t, 16> & uuid_, const std::string & friendly_name_) {
//   AsebaDashel *network = network_with_port(port, true);
//   // std::string name = prefix + "-" + std::to_string(uid);
//   std::string name = prefix;
//   log_info("Will create a node with id %d (%x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x) and name %s (%s) "
//            "and add it to the network with port %d",
//            uid, uuid_[0], uuid_[1], uuid_[2], uuid_[3], uuid_[4], uuid_[5], uuid_[6], uuid_[7],
//            uuid_[8], uuid_[9], uuid_[10], uuid_[11], uuid_[12], uuid_[13], uuid_[14], uuid_[15],
//            name.c_str(), friendly_name_.c_str(), port);
//   T *node = new T(uid, name, uuid_, friendly_name_);
//   add_node(node, network, uid);
//   log_info("Created aseba node with id %d", node->vm.nodeId);
//   return node;
// }

// template DynamicAsebaNode * create_node<DynamicAsebaNode>(
//     unsigned, unsigned, const std::string &, const std::array<uint8_t, 16> &,
//     const std::string &);

void destroy_node(unsigned uid) {
  DynamicAsebaNode *node = node_with_handle(uid);
  if (node) {
    AsebaDashel *network = network_for_vm(&(node->vm));
    remove_node(node, network, uid);
    log_info("destroyed node %d", uid);
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

std::map<unsigned, std::vector<DynamicAsebaNode *>> node_list(unsigned port) {
  std::map<unsigned, std::vector<DynamicAsebaNode *>> nodes;
  if (port < 0) {
    for (auto i : networks) {
      for (auto j : i.second->nodes) {
        nodes[i.first].push_back(j.second);
      }
    }
  } else {
    auto network = network_with_port(port);
    if (network) {
      for (auto j : network->nodes) {
        nodes[port].push_back(j.second);
      }
    }
  }
  return nodes;
}



// -------------- Implementation of aseba glue code

extern "C" void AsebaPutVmToSleep(AsebaVMState *vm) {
  log_info("Received request to go into sleep");
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
      log_warn("Cannot write to socket: %s", stream->getFailReason().c_str());
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

extern "C" const AsebaLocalEventDescription *
AsebaGetLocalEventsDescriptions(AsebaVMState *vm) {
  return node_for_vm(vm)->events_description;
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
  node->call_function(vm, id);
}

extern "C" void AsebaWriteBytecode(AsebaVMState *vm) {
  log_info("Received request to write bytecode into flash");
}

extern "C" void AsebaResetIntoBootloader(AsebaVMState *vm) {
  log_info("Received request to reset into bootloader");
}

extern "C" void AsebaAssert(AsebaVMState *vm, AsebaAssertReason reason) {
  const char * e;
  switch (reason) {
  case ASEBA_ASSERT_UNKNOWN:
    e = "undefined";
    break;
  case ASEBA_ASSERT_UNKNOWN_BINARY_OPERATOR:
    e = "unknown binary operator";
    break;
  case ASEBA_ASSERT_UNKNOWN_BYTECODE:
    e = "unknown bytecode";
    break;
  case ASEBA_ASSERT_STACK_OVERFLOW:
    e = "stack overflow";
    break;
  case ASEBA_ASSERT_STACK_UNDERFLOW:
    e = "stack underflow";
    break;
  case ASEBA_ASSERT_OUT_OF_VARIABLES_BOUNDS:
    e = "out of variables bounds";
    break;
  case ASEBA_ASSERT_OUT_OF_BYTECODE_BOUNDS:
    e = "out of bytecode bounds";
    break;
  case ASEBA_ASSERT_STEP_OUT_OF_RUN:
    e = "step out of run";
    break;
  case ASEBA_ASSERT_BREAKPOINT_OUT_OF_BYTECODE_BOUNDS:
    e = "breakpoint out of bytecode bounds";
    break;
  default:
    e = "unknown exception";
    break;
  }
  log_error("Fatal error; exception: %s; pc = %d, sp = %d", e, vm->pc, vm->sp);
  abort();
  log_info("Resetting VM");
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
