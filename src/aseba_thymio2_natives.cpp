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

#include "aseba_thymio2_natives.h"
#include "aseba_thymio2.h"
#include "common/consts.h"
#include "aseba_network.h"

// utility functions

static int16_t clampValueTo32(int16_t v) {
  if (v > 32)
    return 32;
  else if (v < 0)
    return 32;
  else
    return v;
}

static void missing(AsebaVMState *vm) {
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->notify_missing_feature();
  }
}

void logNativeFromThymio2(AsebaThymio2& thymio2, unsigned id, std::vector<int16_t>&& args) {
  if (thymio2.logThymioNativeCalls)
    thymio2.thymioNativeCallLog.emplace_back(AsebaThymio2::NativeCallLogEntry(id, std::move(args)));
}

void logNativeFromVM(AsebaVMState *vm, unsigned id, std::vector<int16_t>&& args) {
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node)
    logNativeFromThymio2(*node, id, std::move(args));
}

// simulated native functions

// sound

extern "C" void PlaygroundThymio2Native_sound_record(AsebaVMState *vm) {
  const int16_t number(vm->variables[AsebaNativePopArg(vm)]);

  logNativeFromVM(vm, 0, { number });
  missing(vm);
}

static const std::array<unsigned, 8> system_sound_duration = {41, 41, 14, 27, 24, 9, 14, 21};
static const unsigned missing_sound_duration = 10;

extern "C" void PlaygroundThymio2Native_sound_play(AsebaVMState *vm) {
  const int16_t number(vm->variables[AsebaNativePopArg(vm)]);
  logNativeFromVM(vm, 1, { number });
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->set_sound_duration(missing_sound_duration / 60.0);
  }
  // missing(vm);
}

extern "C" void PlaygroundThymio2Native_sound_replay(AsebaVMState *vm) {
  const int16_t number(vm->variables[AsebaNativePopArg(vm)]);
  logNativeFromVM(vm, 2, { number });
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->set_sound_duration(missing_sound_duration / 60.0);
  }
  // missing(vm);
}

extern "C" void PlaygroundThymio2Native_sound_duration(AsebaVMState *vm) {
  const int16_t number(vm->variables[AsebaNativePopArg(vm)]);
  const uint16_t durationAddr(AsebaNativePopArg(vm));
  vm->variables[durationAddr] = 0;
  logNativeFromVM(vm, 21, { number, static_cast<int16_t>(durationAddr) });
  missing(vm);
}

extern "C" void PlaygroundThymio2Native_sound_system(AsebaVMState *vm) {
  const int16_t number(vm->variables[AsebaNativePopArg(vm)]);
  logNativeFromVM(vm, 3, { number });
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    if (number == -1) {
      node->set_sound_duration(0);
    } else {
      node->set_sound_duration(
        (number < 8 ? system_sound_duration[number] : missing_sound_duration) / 60.0);
    }
  }
  // missing(vm);
}


extern "C" void PlaygroundThymio2Native_sound_freq(AsebaVMState * vm) {
  const int16_t freq(vm->variables[AsebaNativePopArg(vm)]);
  const int16_t time(vm->variables[AsebaNativePopArg(vm)]);
  logNativeFromVM(vm, 8, { freq, time });
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    if (time == -1) {
      node->set_sound_duration(0);
    } else {
      node->set_sound_duration(std::max((int16_t) 4, time) / 60.0);
    }
  }
  // missing(vm);
}

extern "C" void PlaygroundThymio2Native_sound_wave(AsebaVMState * vm) {
  const uint16_t waveAddr(AsebaNativePopArg(vm));
  // Note: if necessary, copy data of the wave form to the log
  logNativeFromVM(vm, 15, { static_cast<int16_t>(waveAddr) });
  missing(vm);
}

// leds

extern "C" void PlaygroundThymio2Native_leds_circle(AsebaVMState *vm) {
  const int16_t l0(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l1(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l2(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l3(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l4(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l5(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l6(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l7(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_intensity(CS::LED::RING_0, l0/32.);
    node->robot->set_led_intensity(CS::LED::RING_1, l1/32.);
    node->robot->set_led_intensity(CS::LED::RING_2, l2/32.);
    node->robot->set_led_intensity(CS::LED::RING_3, l3/32.);
    node->robot->set_led_intensity(CS::LED::RING_4, l4/32.);
    node->robot->set_led_intensity(CS::LED::RING_5, l5/32.);
    node->robot->set_led_intensity(CS::LED::RING_6, l6/32.);
    node->robot->set_led_intensity(CS::LED::RING_7, l7/32.);

    logNativeFromThymio2(*node, 4, { l0, l1, l2, l3, l4, l5, l6, l7 });

    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_ACC);
  }
}

extern "C" void PlaygroundThymio2Native_leds_top(AsebaVMState *vm) {
  const int16_t r(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t g(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t b(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  // const int16_t a(std::max(std::max(r, g), b));
  // const double param(1./std::max(std::max(r, g), std::max((int16_t)1, b)));
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_color(CS::LED::TOP,
                               false, r / 32.0, g / 32.0, b / 32.0);
    logNativeFromThymio2(*node, 5, { r, g, b });
  }
}

extern "C" void PlaygroundThymio2Native_leds_bottom_right(AsebaVMState *vm) {
  const int16_t r(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t g(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t b(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_color(CS::LED::BOTTOM_RIGHT,
                               false, r / 32.0, g / 32.0, b / 32.0);
    logNativeFromThymio2(*node, 6, { r, g, b });
  }
}

extern "C" void PlaygroundThymio2Native_leds_bottom_left(AsebaVMState *vm) {
  const int16_t r(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t g(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t b(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_color(CS::LED::BOTTOM_LEFT, false, r / 32.0, g / 32.0, b / 32.0);
    logNativeFromThymio2(*node, 7, { r, g, b });
  }
}

extern "C" void PlaygroundThymio2Native_leds_buttons(AsebaVMState *vm) {
  const int16_t l0(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l1(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l2(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l3(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_intensity(CS::LED::BUTTON_UP,    l0 / 32.0);
    node->robot->set_led_intensity(CS::LED::BUTTON_RIGHT, l1 / 32.0);
    node->robot->set_led_intensity(CS::LED::BUTTON_DOWN,  l2 / 32.0);
    node->robot->set_led_intensity(CS::LED::BUTTON_LEFT,  l3 / 32.0);

    logNativeFromThymio2(*node, 9, { l0, l1, l2, l3 });
    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_BUTTON);
  }
}

extern "C" void PlaygroundThymio2Native_leds_prox_h(AsebaVMState *vm) {
  const int16_t l0(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l1(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l2(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l3(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l4(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l5(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l6(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l7(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_intensity(CS::LED::IR_FRONT_0, l0/32.);
    node->robot->set_led_intensity(CS::LED::IR_FRONT_1, l1/32.);
    node->robot->set_led_intensity(CS::LED::IR_FRONT_2, l2/32.);
    node->robot->set_led_intensity(CS::LED::IR_FRONT_3, l3/32.);
    node->robot->set_led_intensity(CS::LED::IR_FRONT_4, l4/32.);
    node->robot->set_led_intensity(CS::LED::IR_FRONT_5, l5/32.);
    node->robot->set_led_intensity(CS::LED::IR_BACK_0,  l6/32.);
    node->robot->set_led_intensity(CS::LED::IR_BACK_1,  l7/32.);

    logNativeFromThymio2(*node, 10, { l0, l1, l2, l3, l4, l5, l6, l7 });

    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_PROX);
  }
}

extern "C" void PlaygroundThymio2Native_leds_prox_v(AsebaVMState *vm) {
  const int16_t l0(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t l1(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_intensity(CS::LED::IR_GROUND_0, l0/32.);
    node->robot->set_led_intensity(CS::LED::IR_GROUND_1, l1/32.);
    logNativeFromThymio2(*node, 10, { l0, l1 });
    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_PROX);
  }
  // logNativeFromVM(vm, 11, { l0, l1 });
  // missing(vm);
}

extern "C" void PlaygroundThymio2Native_leds_rc(AsebaVMState *vm) {
  const int16_t l0(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_intensity(CS::LED::RIGHT_RED, l0 / 32.0);
    logNativeFromThymio2(*node, 12, { l0});
    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_RC5);
  }
}

extern "C" void PlaygroundThymio2Native_leds_sound(AsebaVMState *vm) {
  const int16_t l0(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->set_led_intensity(CS::LED::RIGHT_BLUE, l0 / 32.0);
    logNativeFromThymio2(*node, 13, { l0});
    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_MIC);
  }
}

extern "C" void PlaygroundThymio2Native_leds_temperature(AsebaVMState *vm) {
  const int16_t r(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  const int16_t b(clampValueTo32(vm->variables[AsebaNativePopArg(vm)]));
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    // node->robot->set_led_intensity(CS::LED::LED_RED, r / 32.0);
    // node->robot->set_led_intensity(CS::LED::LED_BLUE, b / 32.0);
    node->robot->set_led_color(CS::LED::LEFT_BLUE, false, r / 32.0, 0, b/32.0);
    logNativeFromVM(vm, 14, { r, b });
    node->robot->enable_behavior(false, CS::BEHAVIOR::LEDS_NTC);
  }
}

extern "C" void PlaygroundThymio2Native_prox_comm_enable(AsebaVMState *vm) {
  const int16_t enable(vm->variables[AsebaNativePopArg(vm)]);

  logNativeFromVM(vm, 16, { enable });

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    node->robot->enable_prox_comm(bool(enable));
  }
  // missing(vm);
}

extern "C" void PlaygroundThymio2Native_sd_open(AsebaVMState *vm) {
  const int16_t number(vm->variables[AsebaNativePopArg(vm)]);
  const uint16_t statusAddr(AsebaNativePopArg(vm));

  // /missing(vm);
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    int16_t result(0);
    // number must be [0:32767], or -1
    if (number < -1) {
      result = -1;
    } else if (!node->robot->sd_open(number)) {  // try to open
      result = -1;
    }
    vm->variables[statusAddr] = result;
    logNativeFromThymio2(*node, 17, { number, result });
  }
}

extern "C" void PlaygroundThymio2Native_sd_write(AsebaVMState *vm) {
  const uint16_t dataAddr(AsebaNativePopArg(vm));
  const uint16_t statusAddr(AsebaNativePopArg(vm));
  const uint16_t dataLength(AsebaNativePopArg(vm));

  // missing(vm);

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    int16_t result = node->robot->sd_write(
        reinterpret_cast<const char*>(&vm->variables[dataAddr]), dataLength*2) / 2;
    // a failed write will report 0. It is unlikely that a partial write occurs,
    // but if so it is not handled properly.
    vm->variables[statusAddr] = result;
    // log the data written and the status
    std::vector<int16_t> data(&vm->variables[dataAddr], &vm->variables[dataAddr+dataLength]);
    data.push_back(result);
    logNativeFromThymio2(*node, 18, std::move(data));
  }
}

extern "C" void PlaygroundThymio2Native_sd_read(AsebaVMState *vm) {
  const uint16_t dataAddr(AsebaNativePopArg(vm));
  const uint16_t statusAddr(AsebaNativePopArg(vm));
  const uint16_t dataLength(AsebaNativePopArg(vm));

  // missing(vm);

  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    int16_t result = node->robot->sd_read(
        reinterpret_cast<char*>(&vm->variables[dataAddr]), dataLength*2) / 2;
    // a failed write will report 0. It is unlikely that a partial write occurs,
    // but if so it is not handled properly.
    vm->variables[statusAddr] = result;
    // log the data written and the status
    std::vector<int16_t> data(&vm->variables[dataAddr], &vm->variables[dataAddr+dataLength]);
    data.push_back(result);
    logNativeFromThymio2(*node, 18, std::move(data));
  }
}

extern "C" void PlaygroundThymio2Native_sd_seek(AsebaVMState *vm) {
  const int16_t seek(vm->variables[AsebaNativePopArg(vm)]);
  const int16_t statusAddr(AsebaNativePopArg(vm));

  // missing(vm);
  AsebaThymio2 * node = dynamic_cast<AsebaThymio2 *>(Aseba::node_for_vm(vm));
  if (node) {
    int16_t result(0);
    if (!node->robot->sd_seek(seek)) result = -1;
    vm->variables[statusAddr] = result;
    logNativeFromThymio2(*node, 20, { seek, result });
  }
}
