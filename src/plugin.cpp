// Copyright 2016 Coppelia Robotics AG. All rights reserved.
// marc@coppeliarobotics.com
// www.coppeliarobotics.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -------------------------------------------------------------------
// Authors:
// Federico Ferri <federico.ferri.it at gmail dot com>
// -------------------------------------------------------------------

#include "plugin.h"
// #include <chrono>
#include <map>
#include <set>
#include "config.h"
#include "simPlusPlus/Plugin.h"
#include "simPlusPlus/Handle.h"
#include "stubs.h"
#include "aseba_network.h"
#include "coppeliasim_thymio2.h"
#include "aseba_thymio2.h"


std::set<unsigned> uids = {};

unsigned free_uid(int i = -1) {
  if (i > 0 && !uids.count(i)) {
    return i;
  }
  i = 0;
  while (uids.count(i)) {
    i++;
  }
  return i;
}

uint64_t micros() {
    uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();
    return us;
}

// See interface at https://github.com/CoppeliaRobotics/libPlugin/blob/c5054ae5290abecec486652e546372ae2019d9ee/simPlusPlus/Plugin.h


class Plugin : public sim::Plugin {
 public:
    void onStart() {
        if (!registerScriptStuff())
            throw std::runtime_error("script stuff initialization failed");

        setExtVersion("ASEBA HERE");
        setBuildDate(BUILD_DATE);
    }

    void onScriptStateDestroyed(int scriptID) {
        // for(auto obj : handles.find(scriptID))
            // delete handles.remove(obj);
    }

    void onModuleHandle(char *customData) {
      simFloat time_step = simGetSimulationTimeStep();
      for (auto uid : standalone_thymios) {
        thymios.at(uid).do_step(time_step);
      }
      Aseba::spin(time_step);
    }

    void onGuiPass() {
    }

    void onRenderingPass() {
    }

    void onBeforeRendering() {
    }

    void create_thymio2(create_thymio2_in *in, create_thymio2_out *out) {
      simInt uid = free_uid();
      uids.insert(uid);
      simInt script_handle = simGetScriptHandleEx(sim_scripttype_childscript, in->handle, nullptr);
      thymios.emplace(uid, in->handle);
      if (in->with_aseba) {
        AsebaThymio2 * node = Aseba::create_node<AsebaThymio2>(
            uid, in->port, "thymio-II", script_handle);
        node->robot = &thymios.at(uid);
      } else {
        standalone_thymios.insert(uid);
      }
      out->handle = uid;
    }

    void create_node(create_node_in *in, create_node_out *out) {
      simInt uid = free_uid();
      uids.insert(uid);
      simInt script_handle = simGetScriptHandleEx(sim_scripttype_childscript, in->handle, nullptr);
      Aseba::create_node<DynamicAsebaNode>(uid, in->port, in->prefix, script_handle);
      out->handle = uid;
    }

    void destroy(destroy_in *in, destroy_out *out) {
      if (thymios.count(in->handle)) {
        thymios.erase(in->handle);
      }
      Aseba::destroy_node(in->handle);
      uids.erase(in->handle);
      if (standalone_thymios.count(in->handle)) {
        standalone_thymios.erase(in->handle);
      }
    }

    void set_led(set_led_in *in, set_led_out *out) {
      if (thymios.count(in->handle)) {
        thymios.at(in->handle).set_led_color(in->index, false, in->r, in->g, in->b);
      }
    }

    void set_target_speed(set_target_speed_in *in, set_target_speed_out *out) {
      if (thymios.count(in->handle)) {
        thymios.at(in->handle).set_target_speed(in->index, in->speed);
      }
    }

    void get_speed(get_speed_in *in, get_speed_out *out) {
      if (thymios.count(in->handle)) {
        out->speed = thymios.at(in->handle).get_speed(in->index);
      }
    }

    void get_proximity(get_proximity_in *in, get_proximity_out *out) {
      if (thymios.count(in->handle)) {
        out->reading = thymios.at(in->handle).get_proximity_value(in->index);
      }
    }

    void get_ground(get_ground_in *in, get_ground_out *out) {
      if (thymios.count(in->handle)) {
        out->reflected = thymios.at(in->handle).get_ground_reflected(in->index);
      }
    }

    void get_acceleration(get_acceleration_in *in, get_acceleration_out *out) {
      if (thymios.count(in->handle)) {
        const auto & thymio = thymios.at(in->handle);
        out->x = thymio.get_acceleration(0);
        out->y = thymio.get_acceleration(1);
        out->z = thymio.get_acceleration(2);
      }
    }

    void connect_node(connect_node_in *in, connect_node_out *out) {
      DynamicAsebaNode * node = Aseba::node_with_handle(in->handle);
      if (node)
        node->connect();
    }

    void addVariable(addVariable_in *in, addVariable_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->nodeHandle);
      if (node)
        node->add_variable(in->name, in->size);
    }

    void setVariable(setVariable_in *in, setVariable_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->nodeHandle);
      if (node)
        node->set_variable(in->name, in->value);
    }

    void getVariable(getVariable_in *in, getVariable_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->nodeHandle);
      if (node)
        out->value = node->get_variable(in->name);
    }

    void emitEvent(emitEvent_in *in, emitEvent_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->nodeHandle);
      if (node)
        node->emit(in->name);
    }

    void addEvent(addEvent_in *in, addEvent_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->nodeHandle);
      if (node)
        node->add_event(in->name, in->description);
    }

    void addFunction(addFunction_in *in, addFunction_out *out) {
      // printf("addFunction %s to node %d \n", in->name.data(), in->nodeHandle);
      // printf("with arguments:\n");
      // std::vector<argument> args = in->arguments;
      // for (auto &arg : args) {
      //   printf("\t %s %d\n", arg.name.data(), arg.size);
      // }
      DynamicAsebaNode *node = Aseba::node_with_handle(in->nodeHandle);
      if (node)
        node->add_function(in->name, in->description, in->arguments, in->callback);
    }

    void disconnectPort(disconnectPort_in *in, disconnectPort_out *out) {
      int port = in->port;
      if (port < 0) {
        Aseba::remove_all_networks();
      } else {
        Aseba::remove_network_with_port(port);
      }
    }

    void nodeList(nodeList_in *in, nodeList_out *out) {
      out->nodes = Aseba::node_list(in->port);
    }

 private:
  std::map<simInt, CoppeliaSimThymio2> thymios;
  std::set<int> standalone_thymios;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
