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

#ifndef __ASEBA_EPUCK_NATIVES_H
#define __ASEBA_EPUCK_NATIVES_H

#include "vm/vm.h"
#include "vm/natives.h"

// sound
#ifdef __cplusplus
extern "C" {
#endif

void EpuckNative_get_ground_values(AsebaVMState *vm);
extern AsebaNativeFunctionDescription EpuckNativeDescription_get_ground_values;
void EpuckNative_set_awb_ae(AsebaVMState *vm);
extern AsebaNativeFunctionDescription EpuckNativeDescription_set_awb_ae;
void EpuckNative_set_exposure(AsebaVMState *vm);
extern AsebaNativeFunctionDescription EpuckNativeDescription_set_exposure;
void EpuckNative_set_rgb_gain(AsebaVMState *vm);
extern AsebaNativeFunctionDescription EpuckNativeDescription_set_rgb_gain;

// defines listing all native functions and their descriptions
#ifdef __cplusplus
}
#endif

#define EPUCK_NATIVES_DESCRIPTIONS \
  &EpuckNativeDescription_get_ground_values, \
  &EpuckNativeDescription_set_awb_ae, \
  &EpuckNativeDescription_set_exposure, \
  &EpuckNativeDescription_set_rgb_gain

#define EPUCK_NATIVES_FUNCTIONS \
  EpuckNative_get_ground_values, \
  EpuckNative_set_awb_ae, \
  EpuckNative_set_exposure, \
  EpuckNative_set_rgb_gain

#endif // __ASEBA_EPUCK_NATIVES_H
