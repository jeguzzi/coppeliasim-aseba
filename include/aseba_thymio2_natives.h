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

#include "vm/vm.h"
#include "vm/natives.h"

// sound
#ifdef __cplusplus
extern "C" {
#endif
extern void PlaygroundThymio2Native_sound_record(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_record;

extern void PlaygroundThymio2Native_sound_play(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_play;

extern void PlaygroundThymio2Native_sound_replay(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_replay;

extern void PlaygroundThymio2Native_sound_duration(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_duration;

extern void PlaygroundThymio2Native_sound_system(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_system;

extern void PlaygroundThymio2Native_sound_freq(AsebaVMState * vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_freq;

extern void PlaygroundThymio2Native_sound_wave(AsebaVMState * vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sound_wave;

// leds

extern void PlaygroundThymio2Native_leds_circle(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_circle;

extern void PlaygroundThymio2Native_leds_top(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_top;

extern void PlaygroundThymio2Native_leds_bottom_right(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_bottom_right;

extern void PlaygroundThymio2Native_leds_bottom_left(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_bottom_left;

extern void PlaygroundThymio2Native_leds_buttons(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_buttons;

extern void PlaygroundThymio2Native_leds_prox_h(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_prox_h;

extern void PlaygroundThymio2Native_leds_prox_v(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_prox_v;

extern void PlaygroundThymio2Native_leds_rc(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_rc;

extern void PlaygroundThymio2Native_leds_sound(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_leds_sound;

extern void PlaygroundThymio2Native_leds_temperature(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_led_temperature;

// comm

extern void PlaygroundThymio2Native_prox_comm_enable(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_prox_comm_enable;

// sd

extern void PlaygroundThymio2Native_sd_open(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sd_open;

extern void PlaygroundThymio2Native_sd_write(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sd_write;

extern void PlaygroundThymio2Native_sd_read(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sd_read;

extern void PlaygroundThymio2Native_sd_seek(AsebaVMState *vm);
extern AsebaNativeFunctionDescription PlaygroundThymio2NativeDescription_sd_seek;

// defines listing all native functions and their descriptions
#ifdef __cplusplus
}
#endif

#define PLAYGROUND_THYMIO2_NATIVES_DESCRIPTIONS \
	&PlaygroundThymio2NativeDescription_sound_record, \
	&PlaygroundThymio2NativeDescription_sound_play, \
	&PlaygroundThymio2NativeDescription_sound_replay, \
	&PlaygroundThymio2NativeDescription_sound_system, \
	&PlaygroundThymio2NativeDescription_leds_circle, \
	&PlaygroundThymio2NativeDescription_leds_top, \
	&PlaygroundThymio2NativeDescription_leds_bottom_right, \
	&PlaygroundThymio2NativeDescription_leds_bottom_left, \
	&PlaygroundThymio2NativeDescription_sound_freq, \
	&PlaygroundThymio2NativeDescription_leds_buttons, \
	&PlaygroundThymio2NativeDescription_leds_prox_h, \
	&PlaygroundThymio2NativeDescription_leds_prox_v, \
	&PlaygroundThymio2NativeDescription_leds_rc, \
	&PlaygroundThymio2NativeDescription_leds_sound, \
	&PlaygroundThymio2NativeDescription_led_temperature, \
	&PlaygroundThymio2NativeDescription_sound_wave, \
	&PlaygroundThymio2NativeDescription_prox_comm_enable, \
	&PlaygroundThymio2NativeDescription_sd_open, \
	&PlaygroundThymio2NativeDescription_sd_write, \
	&PlaygroundThymio2NativeDescription_sd_read, \
	&PlaygroundThymio2NativeDescription_sd_seek, \
	&PlaygroundThymio2NativeDescription_sound_duration

#define PLAYGROUND_THYMIO2_NATIVES_FUNCTIONS \
	PlaygroundThymio2Native_sound_record, \
	PlaygroundThymio2Native_sound_play, \
	PlaygroundThymio2Native_sound_replay, \
	PlaygroundThymio2Native_sound_system, \
	PlaygroundThymio2Native_leds_circle, \
	PlaygroundThymio2Native_leds_top, \
	PlaygroundThymio2Native_leds_bottom_right, \
	PlaygroundThymio2Native_leds_bottom_left, \
	PlaygroundThymio2Native_sound_freq, \
	PlaygroundThymio2Native_leds_buttons, \
	PlaygroundThymio2Native_leds_prox_h, \
	PlaygroundThymio2Native_leds_prox_v, \
	PlaygroundThymio2Native_leds_rc, \
	PlaygroundThymio2Native_leds_sound, \
	PlaygroundThymio2Native_leds_temperature, \
	PlaygroundThymio2Native_sound_wave, \
	PlaygroundThymio2Native_prox_comm_enable, \
	PlaygroundThymio2Native_sd_open, \
	PlaygroundThymio2Native_sd_write, \
	PlaygroundThymio2Native_sd_read, \
	PlaygroundThymio2Native_sd_seek, \
	PlaygroundThymio2Native_sound_duration

#endif // __ASEBA_THYMIO2_NATIVES_H
