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

#ifndef __ASEBA_THYMIO2_NATIVES_H
#define __ASEBA_THYMIO2_NATIVES_H

#define FW 14

#include "vm/vm.h"
#include "vm/natives.h"

// sound
#ifdef __cplusplus
extern "C" {
#endif
extern void Thymio2Native_sound_record(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_record;

extern void Thymio2Native_sound_play(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_play;

extern void Thymio2Native_sound_replay(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_replay;

extern void Thymio2Native_sound_duration(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_duration;

extern void Thymio2Native_sound_system(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_system;

extern void Thymio2Native_sound_freq(AsebaVMState * vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_freq;

extern void Thymio2Native_sound_wave(AsebaVMState * vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sound_wave;

// leds

extern void Thymio2Native_leds_circle(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_circle;

extern void Thymio2Native_leds_top(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_top;

extern void Thymio2Native_leds_bottom_right(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_bottom_right;

extern void Thymio2Native_leds_bottom_left(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_bottom_left;

extern void Thymio2Native_leds_buttons(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_buttons;

extern void Thymio2Native_leds_prox_h(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_prox_h;

extern void Thymio2Native_leds_prox_v(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_prox_v;

extern void Thymio2Native_leds_rc(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_rc;

extern void Thymio2Native_leds_sound(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_leds_sound;

extern void Thymio2Native_leds_temperature(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_led_temperature;

extern void Thymio2Native_set_led(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_set_led;


// comm

extern void Thymio2Native_prox_comm_enable(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_prox_comm_enable;

// sd

extern void Thymio2Native_sd_open(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sd_open;

extern void Thymio2Native_sd_write(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sd_write;

extern void Thymio2Native_sd_read(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sd_read;

extern void Thymio2Native_sd_seek(AsebaVMState *vm);
extern AsebaNativeFunctionDescription Thymio2NativeDescription_sd_seek;

// wireless

extern AsebaNativeFunctionDescription Thymio2NativeDescription_rf_nodeid;
void Thymio2Native_set_rf_nodeid(AsebaVMState * vm);

extern AsebaNativeFunctionDescription Thymio2NativeDescription_rf_setup;
void Thymio2Native_set_rf_setup(AsebaVMState * vm);

// system (internal)

extern AsebaNativeFunctionDescription AsebaNativeDescription__system_reboot;
void AsebaNative__system_reboot(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription__system_settings_read;
void AsebaNative__system_settings_read(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription__system_settings_write;
void AsebaNative__system_settings_write(AsebaVMState *vm);

extern AsebaNativeFunctionDescription AsebaNativeDescription__system_settings_flash;
void AsebaNative__system_settings_flash(AsebaVMState* vm);

// defines listing all native functions and their descriptions
#ifdef __cplusplus
}
#endif

#define THYMIO2_NATIVES_DESCRIPTIONS \
  &Thymio2NativeDescription_sound_record, \
  &Thymio2NativeDescription_sound_play, \
  &Thymio2NativeDescription_sound_replay, \
  &Thymio2NativeDescription_sound_system, \
  &Thymio2NativeDescription_leds_circle, \
  &Thymio2NativeDescription_leds_top, \
  &Thymio2NativeDescription_leds_bottom_right, \
  &Thymio2NativeDescription_leds_bottom_left, \
  &Thymio2NativeDescription_sound_freq, \
  &Thymio2NativeDescription_leds_buttons, \
  &Thymio2NativeDescription_leds_prox_h, \
  &Thymio2NativeDescription_leds_prox_v, \
  &Thymio2NativeDescription_leds_rc, \
  &Thymio2NativeDescription_leds_sound, \
  &Thymio2NativeDescription_led_temperature, \
  &Thymio2NativeDescription_sound_wave, \
  &Thymio2NativeDescription_prox_comm_enable, \
  &Thymio2NativeDescription_sd_open, \
  &Thymio2NativeDescription_sd_write, \
  &Thymio2NativeDescription_sd_read, \
  &Thymio2NativeDescription_sd_seek, \
  &Thymio2NativeDescription_sound_duration

#define THYMIO2_NATIVES_DESCRIPTIONS_FW14 \
  &Thymio2NativeDescription_set_led, \
  &Thymio2NativeDescription_rf_nodeid, \
  &Thymio2NativeDescription_rf_setup, \
  &AsebaNativeDescription__system_reboot, \
  &AsebaNativeDescription__system_settings_read, \
  &AsebaNativeDescription__system_settings_write, \
  &AsebaNativeDescription__system_settings_flash

#define THYMIO2_NATIVES_FUNCTIONS \
  Thymio2Native_sound_record, \
  Thymio2Native_sound_play, \
  Thymio2Native_sound_replay, \
  Thymio2Native_sound_system, \
  Thymio2Native_leds_circle, \
  Thymio2Native_leds_top, \
  Thymio2Native_leds_bottom_right, \
  Thymio2Native_leds_bottom_left, \
  Thymio2Native_sound_freq, \
  Thymio2Native_leds_buttons, \
  Thymio2Native_leds_prox_h, \
  Thymio2Native_leds_prox_v, \
  Thymio2Native_leds_rc, \
  Thymio2Native_leds_sound, \
  Thymio2Native_leds_temperature, \
  Thymio2Native_sound_wave, \
  Thymio2Native_prox_comm_enable, \
  Thymio2Native_sd_open, \
  Thymio2Native_sd_write, \
  Thymio2Native_sd_read, \
  Thymio2Native_sd_seek, \
  Thymio2Native_sound_duration

#define THYMIO2_NATIVES_FUNCTIONS_FW14 \
  Thymio2Native_set_led, \
  Thymio2Native_set_rf_nodeid, \
  Thymio2Native_set_rf_setup, \
  AsebaNative__system_reboot, \
  AsebaNative__system_settings_read, \
  AsebaNative__system_settings_write, \
  AsebaNative__system_settings_flash


#endif // __ASEBA_THYMIO2_NATIVES_H
