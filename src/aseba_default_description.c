#include "vm/natives.h"
#include "common/productids.h"

const AsebaNativeFunctionDescription* default_functions_description[] = {
    ASEBA_NATIVES_STD_DESCRIPTIONS, 0};
const AsebaLocalEventDescription default_events_description[] = {{NULL, NULL}};
const AsebaVMDescription default_variables_description = {
    "node", {{ 1, "id" }, { 1, "source" }, { 32, "args" },
             { 1, ASEBA_PID_VAR_NAME}, { 0, NULL }}};
const AsebaNativeFunctionPointer default_functions[] = {ASEBA_NATIVES_STD_FUNCTIONS};
