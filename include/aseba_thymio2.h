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

Adapted by JG from https://github.com/aseba-community/aseba/blob/master/aseba/targets/playground/robots/thymio2/Thymio2.h

  - commented out sd functions (for now)
  - refactored out aseba node

*/

#ifndef __ASEBA_THYMIO2_H
#define __ASEBA_THYMIO2_H

#include "common/utils/utils.h"
#include "coppeliasim_thymio2.h"
#include "coppeliasim_aseba_node.h"
#include "aseba_thymio2_natives.h"
#include <utility>

extern "C" const AsebaNativeFunctionDescription* thymio_functions_description[];
extern "C" const AsebaLocalEventDescription thymio_events_description[];
extern "C" const AsebaVMDescription thymio_variables_description;
extern "C" const AsebaNativeFunctionPointer thymio_functions[];

typedef struct {
 int16_t id;
 int16_t source;
 int16_t args[32];
 int16_t productId;
 int16_t fwversion[2];

 int16_t buttons[5];

 int16_t proxHorizontal[7];

 int16_t proxCommPayloads[7];
 int16_t proxCommIntensities[7];
 int16_t proxCommRx;
 int16_t proxCommTx;

 int16_t proxGroundAmbiant[2];
 int16_t proxGroundReflected[2];
 int16_t proxGroundDelta[2];

 int16_t motorLeftTarget;
 int16_t motorRightTarget;
 int16_t motorLeftSpeed;
 int16_t motorRightSpeed;
 int16_t motorLeftPwm;
 int16_t motorRightPwm;

 int16_t acc[3];

 int16_t temperature;

 int16_t rc5address;
 int16_t rc5command;

 int16_t micIntensity;
 int16_t micThreshold;

 int16_t timerPeriod[2];

 int16_t sdPresent;

#if FW == 14
 int16_t buttonsRaw[5];
 int16_t buttonsMean[5];
 int16_t buttonsNoise[5];
 int16_t vbat[2];
 int16_t imot[2];
 int16_t integrator[2];
 int16_t ledTop[3];
 int16_t ledBottomLeft[3];
 int16_t ledBottomRight[3];
 int16_t ledCircle[8];
 int16_t micMean;
 int16_t accTap;
#endif

 int16_t freeSpace[512];
} thymio_variables_t;

class AsebaThymio2 : public CoppeliaSimAsebaNode {

 public:
  enum Thymio2Events {
    EVENT_B_BACKWARD = 0,
    EVENT_B_LEFT,
    EVENT_B_CENTER,
    EVENT_B_FORWARD,
    EVENT_B_RIGHT,
    EVENT_BUTTONS,
    EVENT_PROX,
    EVENT_PROX_COMM,
    EVENT_TAP,
    EVENT_ACC,
    EVENT_MIC,
    EVENT_SOUND_FINISHED,
    EVENT_TEMPERATURE,
    EVENT_RC5,
    EVENT_MOTOR,
    EVENT_TIMER0,
    EVENT_TIMER1,
    EVENT_COUNT // number of events
  };

  // std::fstream sdCardFile;
  // int sdCardFileNumber;

  // Logging of Thymio native function calls
  //! A log entry consists of the number of the Thymio native function and the values of the arguments
  using NativeCallLogEntry = std::pair<unsigned, std::vector<int16_t>>;
  //! The log is a vector of entries
  using NativeCallLog = std::vector<NativeCallLogEntry>;
  //! The log of native calls, filled if logThymioNativeCalls is true.
  //! The code which set it is responsible to clear the log from time to time
  NativeCallLog thymioNativeCallLog;
  //! Whether thymioNativeCallLog should be filled each time a Thymio native function is called
  bool logThymioNativeCalls { false } ;

  const AsebaNativeFunctionDescription** native_functions_description () const override{
    return thymio_functions_description;
  }
  const AsebaLocalEventDescription* native_events_description () const override {
    return thymio_events_description;
  }
  const AsebaVMDescription* native_variables_description () const override {
    return &thymio_variables_description;
  }
  const AsebaNativeFunctionPointer* native_functions () const override {
    return thymio_functions;
  }

 protected:
  Aseba::SoftTimer timer0;
  Aseba::SoftTimer timer1;
  int16_t oldTimerPeriod[2];
  int16_t oldProxCommTx;
  int16_t oldMotorLeftTarget;
  int16_t oldMotorRightTarget;
  int16_t oldMicThreshold;
  int16_t oldLedRGB[9];
  int16_t oldLedCircle[8];
  Aseba::SoftTimer timer100Hz;
  unsigned counter100Hz;
  float sound_duration;
  bool playing_sound;
 public:
  AsebaThymio2(int node_id, const std::string & _name, const std::array<uint8_t, 16> & uuid_,
               const std::string & friendly_name_ = "");
  void notify_missing_feature() {};
  CS::Thymio2 * robot;
  virtual void step(float dt) override;
  // bool openSDCardFile(int number);
  //
  virtual std::string advertized_name() const override {
    return "Thymio II";
  }

  virtual void reset() override;

  void set_sound_duration(float duration) {
    if (duration > 0) {
      sound_duration = duration;
      playing_sound = true;
    } else {
      playing_sound = false;
    }
  }

protected:
  thymio_variables_t * thymio_variables;
  void timer0Timeout();
  void timer1Timeout();
  void timer100HzTimeout();
  bool first;

};


#endif // __ASEBA_THYMIO2_H
