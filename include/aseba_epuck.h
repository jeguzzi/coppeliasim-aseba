/*
Aseba - an event-based framework for distributed robot control
Copyright (C) 2007--2013:
  Stephane Magnenat <stephane at magnenat dot net>
  (http://stephane.magnenat.net)
  and other contributors, see authors.txt for details

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

Adapted by JG from https://github.com/aseba-community/aseba-target-epuck/blob/master/epuckaseba.c

*/

#ifndef __ASEBA_EPUCK_H
#define __ASEBA_EPUCK_H

#include "common/utils/utils.h"
#include "coppeliasim_aseba_node.h"
#include "aseba_epuck_natives.h"
#include <utility>

extern "C" const AsebaNativeFunctionDescription* epuck_functions_description[];
extern "C" const AsebaLocalEventDescription epuck_events_description[];
extern "C" const AsebaVMDescription epuck_variables_description;
extern "C" const AsebaNativeFunctionPointer epuck_functions[];

typedef struct {
 int16_t id;
 int16_t source;
 int16_t args[32];
 int16_t productId;
 int16_t motorLeftTarget;   // TODO(Jerome)
 int16_t motorRightTarget;  // TODO(Jerome)
 int16_t leds[10];          // TODO(Jerome)
 int16_t prox[8];           // TODO(Jerome)
 int16_t ambient[8];        // TODO(Jerome)
 int16_t acc[3];            // TODO(Jerome)
 int16_t camLine;           // TODO(Jerome)
 int16_t camR[60];          // TODO(Jerome)
 int16_t camG[60];          // TODO(Jerome)
 int16_t camB[60];          // TODO(Jerome)
 int16_t leftSteps;         // TODO(Jerome)
 int16_t rightSteps;        // TODO(Jerome)
 int16_t mic[3];            // TODO(Jerome)
 int16_t selector;          // TODO(Jerome)
 int16_t tvRemote;          // TODO(Jerome)
 int16_t timerPeriod;
 // battery (percentage for e-puck 1.3, state for e-puck <= 1.2)
 int16_t battery;           // TODO(Jerome)
 // gyro (only for e-puck 1.3)
 int16_t gyro[3];           // TODO(Jerome)
 int16_t freeSpace[128];
} epuck_variables_t;

class AsebaEPuck : public CoppeliaSimAsebaNode {

 public:
  enum EPuckEvents {
    EVENT_IR_SENSORS = 0,
    EVENT_CAMERA,
    EVENT_SELECTOR,
    EVENT_TIMER,
    EVENT_COUNT // number of events
  };


  const AsebaNativeFunctionDescription** native_functions_description () const override{
    return epuck_functions_description;
  }
  const AsebaLocalEventDescription* native_events_description () const override {
    return epuck_events_description;
  }
  const AsebaVMDescription* native_variables_description () const override {
    return &epuck_variables_description;
  }
  const AsebaNativeFunctionPointer* native_functions () const override {
    return epuck_functions;
  }

 protected:
  Aseba::SoftTimer timer;
  int16_t timer_period;
  int16_t motor_left_target;
  int16_t motor_right_target;
  int16_t camera_line;
  int16_t selector_state;
  bool is_version_1_3;
  Aseba::SoftTimer timer64Hz;
 public:
  AsebaEPuck(int node_id, const std::string & _name, const std::array<uint8_t, 16> & uuid_,
             const std::string & friendly_name_ = "");
  void notify_missing_feature() {};
  // TODO(Jerome)
  void * robot;
  virtual void step(float dt) override;
  virtual std::string advertized_name() const override {
    return "e-puck";
  }

  virtual void reset() override;


protected:
  epuck_variables_t * epuck_variables;
  void timerTimeout();
  void timer64HzTimeout();
  void update_leds();
  void update_camera();
  void update_proximity_sensors();
  bool first;

};


#endif // __ASEBA_EPUCK_H
