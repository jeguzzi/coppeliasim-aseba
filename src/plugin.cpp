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
#include "coppeliasim_aseba_node.h"
#include "aseba_thymio2.h"
#include "coppeliasim_thymio2.h"
#include "logging.h"

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
        setExtVersion("ASEBA");
        setBuildDate(BUILD_DATE);
        sim::registerScriptVariable("simThymio", "require('simThymio-typecheck')", 0);
    }

    void onScriptStateDestroyed(int scriptID) {
        // for(auto obj : handles.find(scriptID))
            // delete handles.remove(obj);
    }

    void onSimulationAboutToStart() {
      simSetInt32Parameter(
          sim_intparam_prox_sensor_select_down, sim_objectspecialproperty_detectable);
      simSetInt32Parameter(
          sim_intparam_prox_sensor_select_up, sim_objectspecialproperty_detectable);
    }

    void onSimulationAboutToEnd() {
      while (thymios.size()) {
        auto it = thymios.begin();
        destroy_node_with_uid(it->first);
      }
      Aseba::destroy_all_nodes();
      Aseba::remove_all_networks();
    }

    void onModuleHandle(char *customData) {
      simFloat time_step = simGetSimulationTimeStep();
      for (auto uid : standalone_thymios) {
        thymios.at(uid).do_step(time_step);
      }
      for (auto & [uid, thymio] : thymios) {
        if (thymio.prox_comm_enabled()) {
            thymio.reset_prox_comm_rx();
          for (const auto & [tid, tx] : prox_comm_tx) {
            if (uid == tid) continue;
            // printf("push tx %d %d\n", tid, tx);
            thymio.update_prox_comm(thymios.at(tid).prox_comm_emitter_handles(), tx);
          }
        }
      }
      Aseba::spin(time_step);
      prox_comm_tx.clear();
      for (const auto & [uid, thymio] : thymios) {
        if (thymio.prox_comm_enabled()) {
          prox_comm_tx[uid] = thymio.prox_comm_tx();
          // printf("set tx %d %d\n", uid, prox_comm_tx[uid]);
        }
      }
    }

    void onGuiPass() {
    }

    void onRenderingPass() {
    }

    void onBeforeRendering() {
    }

    void onProxSensorSelectDown(int objectID, simFloat *clickedPoint, simFloat *normalVector) {
      // printf("DOWN %d\n", objectID);
      if (buttons.count(objectID)) {
        auto button = buttons.at(objectID);
        // printf("is button %d %d\n", button.first, button.second);
        thymios.at(button.first).set_button(button.second, true);
      }
    }

    void onProxSensorSelectUp(int objectID, simFloat *clickedPoint, simFloat *normalVector) {
      // printf("UP %d\n", objectID);
      if (buttons.count(objectID)) {
        auto button = buttons.at(objectID);
        // printf("is button %d %d\n", button.first, button.second);
        thymios.at(button.first).set_button(button.second, false);
      }
    }

    void _thymio2_create(_thymio2_create_in *in, _thymio2_create_out *out) {
      simInt uid = free_uid(in->id);
      uids.insert(uid);
      thymios.emplace(std::piecewise_construct, std::forward_as_tuple(uid),
                      std::forward_as_tuple(in->handle, in->behavior_mask));
      std::array<uint8_t, 16> uuid;
      char s[17];
      snprintf(s, sizeof(s), "coppeliasim %04d", uid);
      std::copy(s, s+16, uuid.data());
      CS::Thymio2 & thymio = thymios.at(uid);
      if (in->with_aseba) {
        AsebaThymio2 * node = Aseba::create_node<AsebaThymio2>(
            uid, in->port, "thymio-II", uuid, in->friendly_name);
        node->set_script_id(simGetScriptHandleEx(sim_scripttype_childscript, in->handle, nullptr));
        node->robot = &thymio;
      } else {
        standalone_thymios.insert(uid);
      }
      unsigned i = 0;
      for (simInt button_handle : thymio.button_handles()) {
        buttons[button_handle] = std::make_pair(uid, i);
        i++;
      }
      out->id = uid;
    }

    void create_node(create_node_in *in, create_node_out *out) {
      simInt uid = free_uid(in->id);
      uids.insert(uid);
      std::array<uint8_t, 16> uuid;
      std::string friendly_name;
      if (!in->friendly_name.empty()) {
        friendly_name = in->name;
      } else {
        friendly_name = friendly_name;
      }
      if (in->uuid.size() == 16) {
        std::move(in->uuid.begin(), in->uuid.end(), uuid.begin());
      } else {
        char s[17];
        snprintf(s, sizeof(s), "coppeliasim %04d", uid);
        std::copy(s, s+16, uuid.data());
      }
      CoppeliaSimAsebaNode * node = Aseba::create_node<CoppeliaSimAsebaNode>(
          uid, in->port, in->name, uuid, in->friendly_name);
      node->set_script_id(simGetScriptHandleEx(sim_scripttype_childscript, in->handle, nullptr));
      out->id = uid;
    }

    void destroy_node_with_uid(unsigned uid) {
      if (thymios.count(uid)) {
        CS::Thymio2 & thymio = thymios.at(uid);
        for (simInt button_handle : thymio.button_handles()) {
          buttons.erase(button_handle);
        }
        thymios.erase(uid);
      }
      Aseba::destroy_node(uid);
      uids.erase(uid);
      if (standalone_thymios.count(uid)) {
        standalone_thymios.erase(uid);
      }
    }

    void destroy_node(destroy_node_in *in, destroy_node_out *out) {
      destroy_node_with_uid(in->id);
    }

    void _thymio2_set_led(_thymio2_set_led_in *in, _thymio2_set_led_out *out) {
      if (in->id == -1) {
        for (auto & [_, thymio] : thymios) {
          thymio.set_led_color(in->index, false, in->r, in->g, in->b);
        }
      } else if (thymios.count(in->id)) {
        thymios.at(in->id).set_led_color(in->index, false, in->r, in->g, in->b);
      }
    }

    void _thymio2_set_led_intensity(_thymio2_set_led_intensity_in *in,
                                    _thymio2_set_led_intensity_out *out) {
      if (in->id == -1) {
        for (auto & [_, thymio] : thymios) {
          thymio.set_led_intensity(in->index, in->a);
        }
      } else if (thymios.count(in->id)) {
        thymios.at(in->id).set_led_intensity(in->index, in->a);
      }
    }

    void _thymio2_set_target_speed(_thymio2_set_target_speed_in *in,
                                   _thymio2_set_target_speed_out *out) {
      if (in->id == -1) {
         for (auto & [_, thymio] : thymios) {
           thymio.set_target_speed(in->index, in->speed);
         }
      } else if (thymios.count(in->id)) {
        thymios.at(in->id).set_target_speed(in->index, in->speed);
      }
    }

    void _thymio2_get_speed(_thymio2_get_speed_in *in, _thymio2_get_speed_out *out) {
      if (thymios.count(in->id)) {
        out->speed = thymios.at(in->id).get_speed(in->index);
      }
    }

    void _thymio2_get_proximity(_thymio2_get_proximity_in *in, _thymio2_get_proximity_out *out) {
      if (thymios.count(in->id)) {
        out->reading = thymios.at(in->id).get_proximity_value(in->index);
      }
    }

    void _thymio2_get_ground(_thymio2_get_ground_in *in, _thymio2_get_ground_out *out) {
      if (thymios.count(in->id)) {
        out->reflected = thymios.at(in->id).get_ground_reflected(in->index);
      }
    }

    void _thymio2_get_acceleration(_thymio2_get_acceleration_in *in,
                                   _thymio2_get_acceleration_out *out) {
      if (thymios.count(in->id)) {
        const auto & thymio = thymios.at(in->id);
        out->x = thymio.get_acceleration(0);
        out->y = thymio.get_acceleration(1);
        out->z = thymio.get_acceleration(2);
      }
    }

    // void connect_node(connect_node_in *in, connect_node_out *out) {
    //   DynamicAsebaNode * node = Aseba::node_with_handle(in->id);
    //   if (node)
    //     node->connect();
    // }

    void add_variable(add_variable_in *in, add_variable_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node)
        node->add_variable(in->name, in->size);
    }

    void set_variable(set_variable_in *in, set_variable_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node)
        node->set_variable(in->name, in->value);
    }

    void get_variable(get_variable_in *in, get_variable_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node)
        out->value = node->get_variable(in->name);
    }

    void emit_event(emit_event_in *in, emit_event_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node)
        node->emit(in->name);
    }

    void add_event(add_event_in *in, add_event_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node)
        node->add_event(in->name, in->description);
    }

    void add_function(add_function_in *in, add_function_out *out) {
      // printf("add_function %s to node %d \n", in->name.data(), in->id);
      // printf("with arguments:\n");
      // std::vector<argument> args = in->arguments;
      // for (auto &arg : args) {
      //   printf("\t %s %d\n", arg.name.data(), arg.size);
      // }
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node) {
        std::vector<std::tuple<int, std::string>> arguments;
        for (const auto & a : in->arguments) {
          arguments.push_back({a.size, a.name});
        }
        node->add_function(in->name, in->description, arguments, in->callback);
      }
    }

    void destroy_network(destroy_network_in *in, destroy_network_out *out) {
      int port = in->port;
      if (port < 0) {
        Aseba::remove_all_networks();
      } else {
        Aseba::remove_network_with_port(port);
      }
    }

    void list_nodes(list_nodes_in *in, list_nodes_out *out) {
      for (const auto & [port, aseba_nodes] : Aseba::node_list(in->port)) {
        for (const auto & aseba_node : aseba_nodes) {
          node_t z;
          z.name = aseba_node->name;
          z.id = aseba_node->vm.nodeId;
          z.port = port;
          out->nodes.push_back(z);
        }
      }
    }

    void load_script(load_script_in *in, load_script_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node) {
        out->result = node->load_script_from_file(in->path);
      }
    }
    void set_script(set_script_in *in, set_script_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node) {
        out->result = node->load_script_from_text(in->code);
      }
    }

    void _thymio2_enable_accelerometer(_thymio2_enable_accelerometer_in *in,
                                       _thymio2_enable_accelerometer_out *out) {
      if (in->id == -1) {
        for (auto & [_, thymio] : thymios) {
          thymio.enable_accelerometer(in->state);
        }
      } else if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.enable_accelerometer(in->state);
      }
    }

    void _thymio2_enable_ground(_thymio2_enable_ground_in *in, _thymio2_enable_ground_out *out) {
      if (in->id == -1) {
        for (auto & [_, thymio] : thymios) {
          thymio.enable_ground(in->state, in->red, in->vision);
        }
      } else if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.enable_ground(in->state, in->red, in->vision);
      }
    }

    void _thymio2_enable_proximity(_thymio2_enable_proximity_in *in,
                                   _thymio2_enable_proximity_out *out) {
    if (in->id == -1) {
       for (auto & [_, thymio] : thymios) {
         thymio.enable_proximity(in->state, in->red);
       }
    } else if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.enable_proximity(in->state, in->red);
      }
    }

    void _thymio2_get_button(_thymio2_get_button_in *in, _thymio2_get_button_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        out->value = thymio.get_button(in->index);
      }
    }

    void _thymio2_set_button(_thymio2_set_button_in *in, _thymio2_set_button_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.set_button(in->index, in->value);
      }
    }

    void _thymio2_enable_prox_comm(_thymio2_enable_prox_comm_in *in,
                                   _thymio2_enable_prox_comm_out *out) {
    if (in->id == -1) {
        for (auto & [_, thymio] : thymios) {
          thymio.enable_prox_comm(in->state);
        }
    } else if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.enable_prox_comm(in->state);
      }
    }

    void _thymio2_set_prox_comm_tx(
        _thymio2_set_prox_comm_tx_in *in,
        _thymio2_set_prox_comm_tx_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        // printf("set_prox_comm_tx %d\n", in->tx);
        thymio.set_prox_comm_tx(in->tx);
      }
    }

    void _thymio2_get_prox_comm_rx(
      _thymio2_get_prox_comm_rx_in *in,
      _thymio2_get_prox_comm_rx_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        for (const auto & msg : thymio.prox_comm_rx()) {
          prox_comm_message_t omsg;
          omsg.rx = msg.rx;
          omsg.payloads = std::vector<int>(msg.payloads.begin(), msg.payloads.end());
          omsg.intensities = std::vector<float>(msg.intensities.begin(), msg.intensities.end());
          out->messages.push_back(omsg);
        }
      }
    }

    void set_uuid(set_uuid_in *in, set_uuid_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node && in->uuid.size() == 16) {
          std::array<uint8_t, 16> uuid;
          std::move(in->uuid.begin(), in->uuid.end(), uuid.begin());
          node->set_uuid(uuid);
      }
    }

    void set_friendly_name(set_friendly_name_in *in, set_friendly_name_out *out) {
      DynamicAsebaNode *node = Aseba::node_with_handle(in->id);
      if (node)
          node->set_friendly_name(in->name);
    }

    void set_address(set_address_in *in, set_address_out *out) {
      Aseba::set_address(in->address);
    }

    void _thymio2_set_battery_voltage(_thymio2_set_battery_voltage_in *in,
                                      _thymio2_set_battery_voltage_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.set_battery_voltage(in->value);
      }
    }

    void _thymio2_enable_behavior(_thymio2_enable_behavior_in *in,
                                  _thymio2_enable_behavior_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.enable_behavior(in->value, in->mask);
      }
    }

    void _thymio2_set_temperature(_thymio2_set_temperature_in *in,
                                  _thymio2_set_temperature_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.set_temperature(in->temperature);
      }
    }

    void _thymio2_set_mic_intensity(_thymio2_set_mic_intensity_in *in,
                                    _thymio2_set_mic_intensity_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.set_mic_intensity(in->intensity);
      }
    }

    void _thymio2_set_mic_threshold(_thymio2_set_mic_threshold_in *in,
                                    _thymio2_set_mic_threshold_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.set_mic_threshold(in->threshold);
      }
    }

    void _thymio2_receive_rc_message(_thymio2_receive_rc_message_in *in,
                                     _thymio2_receive_rc_message_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.receive_rc_message(in->address, in->command);
      }
    }

    void _thymio2_enable_sd_card(_thymio2_enable_sd_card_in *in,
                                 _thymio2_enable_sd_card_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        thymio.sd_enable(in->path);
        out->enabled = thymio.sd_is_enabled();
      }
    }

    void _thymio2_get_sd_card(_thymio2_get_sd_card_in *in,
                              _thymio2_get_sd_card_out *out) {
      if (thymios.count(in->id)) {
        auto & thymio = thymios.at(in->id);
        out->enabled = thymio.sd_is_enabled();
        out->path = thymio.sd_path();
      }
    }

 private:
  std::map<simInt, CS::Thymio2> thymios;
  std::set<int> standalone_thymios;
  std::map<simInt, std::pair<simInt, unsigned>> buttons;
  std::map<int, int> prox_comm_tx;
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
