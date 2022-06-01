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
*/

#include <algorithm>
#include "aseba_epuck.h"
#include "aseba_epuck_natives.h"
#include "common/productids.h"
#include "common/utils/utils.h"

AsebaEPuck::AsebaEPuck(int node_id, const std::string & _name,
                       const std::array<uint8_t, 16> & uuid_,
                       const std::string & friendly_name_):
  CoppeliaSimAsebaNode(node_id, _name, uuid_, friendly_name_),
  timer(std::bind(&AsebaEPuck::timerTimeout, this), 0),
  timer64Hz(std::bind(&AsebaEPuck::timer64HzTimeout, this), 1.0 / 64.0),
  timer_period(0), camera_line(0), first(true),
  robot(nullptr), is_version_1_3(true) {
  epuck_variables = reinterpret_cast<epuck_variables_t *>(&variables);
  epuck_variables->id = node_id;
  epuck_variables->productId = ASEBA_PID_EPUCK;
  epuck_variables->camLine = camera_line;
  epuck_variables->timerPeriod = timer_period;
  // TODO(Jerome):
  // name[6] = '0' + selector;
  // e_led_clear();
}


// #define G 9.81f
// input in m^2/s

// inline uint16_t aseba_acc(float a) {
//   return round(std::max(std::min(a / G * 23.0f, 32.0f), -32.0f));
// }

void AsebaEPuck::step(float dt) {
  // get physical variables
  // TODO(Jerome)
  // robot->update_sensing(dt);

  for (size_t i = 0; i < 3; i++) {
    epuck_variables->acc[i] = 0;   // e_get_acc(i);
    epuck_variables->mic[i] = 0;   // e_get_micro_volume(i);
    epuck_variables->gyro[i] = 0;  // getAllAxesGyro
  }

  update_camera();

  // encoders
  epuck_variables->leftSteps = 0;  // e_get_steps_left();
  epuck_variables->rightSteps = 0;  // e_get_steps_right();

  // battery
  if (is_version_1_3) {
    epuck_variables->battery = 88;  // getBatteryValuePercentage();
  } else {
    epuck_variables->battery = 1;  // BATT_LOW=1 => battery ok, BATT_LOW=0 => battery<3.4V
  }

  epuck_variables->selector = 0;  // getselector();
  if (!first && selector_state != epuck_variables->selector) {
    emit(EVENT_SELECTOR);
  }
  selector_state = epuck_variables->selector;
  epuck_variables->tvRemote = 0;  // e_get_data();

  // run timers
  timer.step(dt);
  timer64Hz.step(dt);

  DynamicAsebaNode::step(dt);

  // reset a timer if its period changed
  if (epuck_variables->timerPeriod != timer_period) {
    timer_period = epuck_variables->timerPeriod;
    timer.setPeriod(epuck_variables->timerPeriod / 1000.);
  }

  int16_t desired_left_speed = std::clamp<int16_t>(epuck_variables->motorLeftTarget, -1000, 1000);
  int16_t desired_right_speed = std::clamp<int16_t>(epuck_variables->motorRightTarget, -1000, 1000);
  if (first || motor_left_target == epuck_variables->motorLeftTarget) {
    epuck_variables->motorLeftTarget = 0;  //  robot->get_target_speed(0) * 500. / 0.166;
  } else {
    // robot->set_target_speed(0, double(desired_left_speed) * 0.166 / 500.);
  }
  if (first || motor_right_target == epuck_variables->motorRightTarget) {
    epuck_variables->motorRightTarget = 0;  // robot->get_target_speed(1) * 500. / 0.166;
  } else {
    // robot->set_target_speed(1, double(motor_right_target) * 0.166 / 500.);
  }
  motor_left_target = epuck_variables->motorLeftTarget;
  motor_right_target = epuck_variables->motorRightTarget;

  first = false;
  // robot->update_actuation(dt);
}

void AsebaEPuck::update_camera() {
  for (size_t i = 0; i < 60; i++) {
    epuck_variables->camR[i] = 0;  // CAM_RED(cam_data[i]);
    epuck_variables->camG[i] = 0;  // CAM_GREEN(cam_data[i]);
    epuck_variables->camB[i] = 0;  // CAM_BLUE(cam_data[i]);
  }
  if (camera_line != epuck_variables->camLine) {
    camera_line = epuck_variables->camLine;
    // TODO(Jerome)
    // setCamLine(camline);
  }
  emit(EVENT_CAMERA);
}


void AsebaEPuck::update_leds() {
  for (size_t i = 0; i < 8; i++) {
    // e_set_led(i, epuck_variables->leds[i] ? 1 : 0);
  }
  // e_set_body_led(epuck_variables->leds[8] ? 1 : 0);
  // e_set_front_led(epuck_variables->leds[9] ? 1 : 0);
}

void AsebaEPuck::update_proximity_sensors() {
  for (size_t i = 0; i < 8; i++) {
    epuck_variables->ambient[i] = 313;  // e_get_ambient_light(i);
    epuck_variables->prox[i] = 131;     // e_get_calibrated_prox(i);
  }
}

void AsebaEPuck::timerTimeout() {
  emit(EVENT_TIMER);
}

void AsebaEPuck::timer64HzTimeout() {
  update_proximity_sensors();
  update_leds();
  emit(EVENT_IR_SENSORS);
}

void AsebaEPuck::reset() {
  DynamicAsebaNode::reset();
  epuck_variables->id = vm.nodeId;
  epuck_variables->productId = ASEBA_PID_EPUCK;
  epuck_variables->camLine = 50;
  epuck_variables->timerPeriod = 0;
  camera_line = 50;
  timer_period = 0;
  first = true;

  // robot->reset();

  log_info("Reset e-puck Aseba node");
}
