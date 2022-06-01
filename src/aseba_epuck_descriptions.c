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

#include "vm/natives.h"
#include "common/productids.h"
#include "aseba_epuck_natives.h"

// variables memory layout

const AsebaVMDescription epuck_variables_description = {
  "",
  {
    { 1, "id" },
    { 1, "source" },
    { 32, "args" },
    { 1, ASEBA_PID_VAR_NAME },
    {1, "speed.left"},
    {1, "speed.right"},
    {10, "leds"},
    {8, "prox"},
    {8, "ambiant"},
    {3, "acc"},
    {1, "cam.line"},
    {60, "cam.red"},
    {60, "cam.green"},
    {60, "cam.blue"},
    {1, "steps.left"},
    {1, "steps.right"},
    {3, "mic"},
    {1, "sel"},
    {1, "rc5"},
    {1, "timer.period"},
    {1, "battery"},
    {3, "gyro"},
    {0, NULL }
  }
};

const AsebaLocalEventDescription epuck_events_description[] = {
  {"ir_sensors", "IR sensors updated"},
  {"camera", "camera updated"},
  {"sel", "Selector status changed"},
  {"timer", "Timer"},
  { NULL, NULL }
};

const AsebaNativeFunctionDescription* epuck_functions_description[] = {
   ASEBA_NATIVES_STD_DESCRIPTIONS, EPUCK_NATIVES_DESCRIPTIONS, 0};

const AsebaNativeFunctionPointer epuck_functions[] ={
     ASEBA_NATIVES_STD_FUNCTIONS, EPUCK_NATIVES_FUNCTIONS};

// native functions

AsebaNativeFunctionDescription EpuckNativeDescription_get_ground_values =
{
  "ground.get_values",
  "read the values of the ground sensors",
  {
    { 3, "dest" },
    { 0, 0 }
  }
};

AsebaNativeFunctionDescription EpuckNativeDescription_set_awb_ae =
{
  "cam.set_awb_ae",
  "enable (1)/disable (0) the automatic white balance and exposure",
  {
    { 1, "awb" },
    { 1, "ae" },
    { 0, 0 }
  }
};

AsebaNativeFunctionDescription EpuckNativeDescription_set_exposure =
{
  "cam.set_exposure",
  "set the camera exposure",
  {
    { 1, "exposure" },
    { 0, 0 }
  }
};

AsebaNativeFunctionDescription EpuckNativeDescription_set_rgb_gain =
{
  "cam.set_rgb_gain",
  "set the camera r/g/b gain",
  {
    { 1, "red_gain" },
    { 1, "green_gain" },
    { 1, "blue_gain" },
    { 0, 0 }
  }
};
