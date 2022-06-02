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
  timer_period(0), camera_line(50), first(true),
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
// TODO(Jerome): I don't know the conversion factor to get m/s^2 from aseba values
// For now I use a placeholder (x1000)

inline int16_t aseba_acc(float a) {
  return round(a * 1000);
}

// n = 1000 steps are about d = 12.8 cm
// assume radius is r = 0.021 cm
// value = angle * r / d * n = angle * 164.0625
// => 15 bit is about 4.2 m, or 200 rad (32 rounds)

inline int16_t to_aseba_steps(float angle) {
  return round(angle * 164.0625);
}

inline int16_t to_aseba_steps_speed(float speed) {
  return round(speed / 1.28e-4);
}

// returns a distance!
inline float from_aseba_steps(int16_t steps) {
  return steps * 1.28e-4;
}

// TODO(Jerome): I don't know the bounds
inline uint16_t aseba_mic(float value) {
  return round(1000 * value);
}

// TODO(Jerome): Verify that this is the same as done by the firmare at L34
// 100*(float)(e_acc_scan[2][e_last_acc_scan_id]-MIN_BATT_VALUE)/(float)BATT_VALUES_RANGE;
#define MIN_BATT_VALUE 3.4
#define MAX_BATT_VALUE 4.2
#define BATT_VALUES_RANGE (MAX_BATT_VALUE-MIN_BATT_VALUE)

inline uint16_t aseba_battery(float voltage) {
  uint16_t value = std::clamp<int16_t>(
      round(100 * (voltage - MIN_BATT_VALUE) / BATT_VALUES_RANGE), 0, 100);
  // printf("B %.4f -> %d\n", voltage, value);
  return value;
}

// TODO(Jerome): I don't know the conversion factor
inline uint16_t aseba_gyro(float speed) {
  return round(1000 * speed);
}

void AsebaEPuck::step(float dt) {
  // get physical variables
  robot->update_sensing(dt);

  for (size_t i = 0; i < 3; i++) {
    // TODO(Jerome): check orientation
    epuck_variables->acc[i] = aseba_acc(robot->get_acceleration(i));
    epuck_variables->mic[i] = aseba_mic(robot->get_mic_intensity(i));
    epuck_variables->gyro[i] = aseba_gyro(robot->get_angular_velocity(i));
  }

  update_camera();

  // encoders
  epuck_variables->leftSteps = to_aseba_steps(robot->get_odometry(CS::Wheel::LEFT));
  epuck_variables->rightSteps = to_aseba_steps(robot->get_odometry(CS::Wheel::RIGHT));

  // battery
  if (is_version_1_3) {
    epuck_variables->battery = aseba_battery(robot->get_battery_voltage());
  } else {
    epuck_variables->battery = (robot->get_battery_voltage() > 3.4) ? 1 : 0;
  }

  epuck_variables->selector = robot->get_selector();
  if (!first && selector_state != epuck_variables->selector) {
    emit(EVENT_SELECTOR);
  }
  selector_state = epuck_variables->selector;
  epuck_variables->tvRemote = robot->get_rc();

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
    epuck_variables->motorLeftTarget = to_aseba_steps_speed(
        robot->get_target_speed(CS::Wheel::LEFT));
  } else {
    robot->set_target_speed(CS::Wheel::LEFT, from_aseba_steps(desired_left_speed));
  }
  if (first || motor_right_target == epuck_variables->motorRightTarget) {
    epuck_variables->motorRightTarget = to_aseba_steps_speed(
        robot->get_target_speed(CS::Wheel::RIGHT));
  } else {
    robot->set_target_speed(CS::Wheel::RIGHT, from_aseba_steps(desired_right_speed));
  }
  motor_left_target = epuck_variables->motorLeftTarget;
  motor_right_target = epuck_variables->motorRightTarget;

  first = false;
  robot->update_actuation(dt);
}

// 5 bit per channel multiplied by 3 (i.e. between 0 and 93)
static uint8_t aseba_pixel(uint8_t pixel) {
  return 3 * (pixel >> 3);
}

void AsebaEPuck::update_camera() {
  const uint8_t * image = robot->get_camera_line(camera_line / 100.0);
  for (size_t i = 0; i < 60; i++) {
    epuck_variables->camR[i] = aseba_pixel(image[3 * i]);
    epuck_variables->camG[i] = aseba_pixel(image[3 * i + 1]);
    epuck_variables->camB[i] = aseba_pixel(image[3 * i + 2]);
  }
  if (camera_line != epuck_variables->camLine) {
    // line clamped between 0 and 99
    camera_line = std::clamp<uint16_t>(epuck_variables->camLine, 0, 99);
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
    epuck_variables->ambient[i] = 3800;
    epuck_variables->prox[i] = robot->get_proximity_value(i);
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
