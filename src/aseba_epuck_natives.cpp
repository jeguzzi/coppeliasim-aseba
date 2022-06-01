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

#include "aseba_epuck_natives.h"
#include "aseba_epuck.h"
#include "common/consts.h"
#include "aseba_network.h"

// utility functions

// static int16_t clampValueTo32(int16_t v) {
//   if (v > 32)
//     return 32;
//   else if (v < 0)
//     return 32;
//   else
//     return v;
// }

static void missing(AsebaVMState *vm) {
  AsebaEPuck * node = dynamic_cast<AsebaEPuck *>(Aseba::node_for_vm(vm));
  if (node) {
    node->notify_missing_feature();
  }
}

// simulated native functions

void EpuckNative_get_ground_values(AsebaVMState *vm) {
  uint16_t dest(AsebaNativePopArg(vm));
  // TODO(Jerome): complete
  for (uint16_t i = 0; i < 3; i++) {
    vm->variables[dest++] = 101;
  }
}

void EpuckNative_set_awb_ae(AsebaVMState *vm) {
    const uint16_t awb(AsebaNativePopArg(vm));
    const uint16_t ae(AsebaNativePopArg(vm));
    missing(vm);
    // TODO(Jerome): complete
    // e_poxxxx_set_awb_ae(vm->variables[awb], vm->variables[ae]);
    // e_poxxxx_write_cam_registers();
}

void EpuckNative_set_exposure(AsebaVMState *vm) {
  const uint16_t e(AsebaNativePopArg(vm));
  const uint16_t _e(vm->variables[e]);
  missing(vm);
  // TODO(Jerome): complete
  // unsigned long _e = ((unsigned long)vm->variables[e]) << 8;
  // e_poxxxx_set_exposure(_e);
  // e_poxxxx_write_cam_registers();
}

void EpuckNative_set_rgb_gain(AsebaVMState *vm) {
  const uint16_t r(AsebaNativePopArg(vm));
  const uint16_t g(AsebaNativePopArg(vm));
  const uint16_t b(AsebaNativePopArg(vm));
  missing(vm);
  // TODO(Jerome): complete
  // e_poxxxx_set_rgb_gain(vm->variables[r], vm->variables[g], vm->variables[b]);
  // e_poxxxx_write_cam_registers();
}
