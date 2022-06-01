/*
	Aseba - an event-based framework for distributed robot control
	Copyright (C) 2007--2010:
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

/*
	WARNING: you have to change the size of the UART 1 e-puck reception buffer to hold
	at the biggest possible packet (probably bytecode + header). Otherwise if you set
	a new bytecode while busy (for instance while sending description), you might end up
	in a dead-lock.
*/

#include <p30F6014A.h>

#include "utility/utility.h"
#include "acc_gyro/e_lsm330.h"
#include "motor_led/e_epuck_ports.h"
#include "motor_led/e_init_port.h"
#include "motor_led/e_led.h"
#include "motor_led/advance_one_timer/e_motors.h"
#include "motor_led/advance_one_timer/e_agenda.h"
#include "motor_led/advance_one_timer/e_remote_control.h"
#include "camera/fast_2_timer/e_poxxxx.h"
#include "uart/e_uart_char.h"
#include "a_d/advance_ad_scan/e_ad_conv.h"
#include "a_d/advance_ad_scan/e_prox.h"
#include "a_d/advance_ad_scan/e_acc.h"
#include "a_d/advance_ad_scan/e_micro.h"
#include "I2C/e_I2C_protocol.h"

#include "libpic30.h"


#include <string.h>

#define ASEBA_ASSERT
#include <common/consts.h>
#include <common/productids.h>
#include <vm/vm.h>
#include <vm/natives.h>
#include <transport/buffer/vm-buffer.h>

#define LIS_GROUND_SENSORS

#define vmVariablesSize (sizeof(struct EPuckVariables) / sizeof(sint16))
#define vmStackSize 32
#define argsSize 32


unsigned int __attribute((far)) cam_data[60];


// we receive the data as big endian and read them as little, so we have to acces the bit the right way
#define CAM_RED(pixel) ( 3 * (((pixel) >> 3) & 0x1f))

#define CAM_GREEN(pixel) ((3 * ((((pixel) & 0x7) << 3) | ((pixel) >> 13))) >> 1)

#define CAM_BLUE(pixel) ( 3 * (((pixel) >> 8) & 0x1f))

#define CLAMP(v, vmin, vmax) ((v) < (vmin) ? (vmin) : (v) > (vmax) ? (vmax) : (v))

// data

/*** In your code, put "SET_EVENT(EVENT_NUMBER)" when you want to trigger an
	 event. This macro is interrupt-safe, you can call it anywhere you want.
***/
#define SET_EVENT(event) do {events_flags |= 1 << event;} while(0)
#define CLEAR_EVENT(event) do {events_flags &= ~(1 << event);} while(0)
#define IS_EVENT(event) (events_flags & (1 << event))

/* VM */

/* The number of opcode an aseba script can have */
#define VM_BYTECODE_SIZE 1024
#define VM_STACK_SIZE 32

int _EEDATA(1) bytecode_version;
unsigned char _EEDATA(1) eeprom_bytecode[VM_BYTECODE_SIZE*2];
float lastTick;
signed char commError;

struct EPuckVariables
{
	// NodeID
	sint16 id;
	// source
	sint16 source;
	// args
	sint16 args[argsSize];
	// product id
	sint16 productId;
	// motor
	sint16 leftSpeed;
	sint16 rightSpeed;
	// leds
	sint16 leds[10];
	// prox
	sint16 prox[8];
	sint16 ambiant[8];
	// acc
	sint16 acc[3];
	// camera
	sint16 camLine;
	sint16 camR[60];
	sint16 camG[60];
	sint16 camB[60];
	// encoders
	sint16 leftSteps;
	sint16 rightSteps;
	// microphone
	sint16 mic[3];
	// selector
	sint16 selector;
	// tv remote
	sint16 tvRemote;
	// timer
	sint16 timer;
	// battery (percentage for e-puck 1.3, state for e-puck <= 1.2)
	sint16 battery;
	// gyro (only for e-puck 1.3)
	sint16 gyro[3];
	// free space
	sint16 freeSpace[128];
} __attribute__ ((far)) ePuckVariables;

char name[] = "e-puck0";

AsebaVMDescription vmDescription = {
	name, 	// Name of the microcontroller
	{
		{ 1, "id" },	// Do not touch it
		{ 1, "source" }, // nor this one
		{ argsSize, "args" },	// neither this one
		{ 1, ASEBA_PID_VAR_NAME },
		{1, "speed.left"},
		{1, "speed.right"},
	// leds
		{10, "leds"},
	// prox
	    {8, "prox"},
		{8, "ambiant"},
	// acc
		{3, "acc"},
	// camera
		{1, "cam.line"},
		{60, "cam.red"},
		{60, "cam.green"},
		{60, "cam.blue"},
	// encoders
		{1, "steps.left"},
		{1, "steps.right"},
	// mic
		{3, "mic"},
	// sel
		{1, "sel"},
	// tv remote
		{1, "rc5"},
	// timer
		{1, "timer.period"},
	// battery
		{1, "battery"},
	// gyro
		{3, "gyro"},
		{ 0, NULL }	// null terminated
	}
};

static uint16 vmBytecode[VM_BYTECODE_SIZE];

static sint16 vmStack[VM_STACK_SIZE];

static AsebaVMState vmState = {
	0x1,

	VM_BYTECODE_SIZE,
	vmBytecode,

	sizeof(ePuckVariables) / sizeof(sint16),
	(sint16*)&ePuckVariables,

	VM_STACK_SIZE,
	vmStack
};

const AsebaVMDescription* AsebaGetVMDescription(AsebaVMState *vm)
{
	return &vmDescription;
}


static unsigned int events_flags = 0;
enum Events
{
	EVENT_IR_SENSORS = 0,
	EVENT_CAMERA,
	EVENT_SELECTOR,
	EVENT_TIMER,
	EVENTS_COUNT
};

static const AsebaLocalEventDescription localEvents[] = {
	{"ir_sensors", "IR sensors updated"},
	{"camera", "camera updated"},
	{"sel", "Selector status changed"},
	{"timer", "Timer"},
	{ NULL, NULL }
};

const AsebaLocalEventDescription * AsebaGetLocalEventsDescriptions(AsebaVMState *vm)
{
	return localEvents;
}


#ifdef LIS_GROUND_SENSORS
void EpuckNative_get_ground_values(AsebaVMState *vm)
{
	// where to
	uint16 dest = AsebaNativePopArg(vm);
	uint16 i;
	e_i2cp_enable();

	for (i = 0; i < 3; i++)
	{
		unsigned int temp;
		temp = e_i2cp_read(0xC0,i*2) << 8;
		temp |= e_i2cp_read(0xC0,i*2+1);
		vm->variables[dest++] = temp;
	}
	e_i2cp_disable();
}

AsebaNativeFunctionDescription EpuckNativeDescription_get_ground_values =
{
	"ground.get_values",
	"read the values of the ground sensors",
	{
		{ 3, "dest" },
		{ 0, 0 }
	}
};
#endif

void EpuckNative_set_awb_ae(AsebaVMState *vm)
{
    uint16 awb = AsebaNativePopArg(vm);
    uint16 ae = AsebaNativePopArg(vm);
    e_poxxxx_set_awb_ae(vm->variables[awb], vm->variables[ae]);
    e_poxxxx_write_cam_registers();
}

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

void EpuckNative_set_exposure(AsebaVMState *vm)
{
	uint16 e = AsebaNativePopArg(vm);
	unsigned long _e = ((unsigned long)vm->variables[e]) << 8;
	e_poxxxx_set_exposure(_e);
	e_poxxxx_write_cam_registers();
}

AsebaNativeFunctionDescription EpuckNativeDescription_set_exposure =
{
	"cam.set_exposure",
	"set the camera exposure",
	{
		{ 1, "exposure" },
		{ 0, 0 }
	}
};

void EpuckNative_set_rgb_gain(AsebaVMState *vm)
{
	uint16 r = AsebaNativePopArg(vm);
	uint16 g = AsebaNativePopArg(vm);
	uint16 b = AsebaNativePopArg(vm);
	e_poxxxx_set_rgb_gain(vm->variables[r], vm->variables[g], vm->variables[b]);
	e_poxxxx_write_cam_registers();
}

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

static const AsebaNativeFunctionDescription* nativeFunctionsDescription[] = {
	ASEBA_NATIVES_STD_DESCRIPTIONS,
#ifdef LIS_GROUND_SENSORS
	&EpuckNativeDescription_get_ground_values,
#endif
	&EpuckNativeDescription_set_awb_ae,
	&EpuckNativeDescription_set_exposure,
	&EpuckNativeDescription_set_rgb_gain,
	0	// null terminated
};

static AsebaNativeFunctionPointer nativeFunctions[] = {
	ASEBA_NATIVES_STD_FUNCTIONS,
#ifdef LIS_GROUND_SENSORS
	EpuckNative_get_ground_values,
#endif
	EpuckNative_set_awb_ae,
	EpuckNative_set_exposure,
	EpuckNative_set_rgb_gain
};

void AsebaPutVmToSleep(AsebaVMState *vm)
{
}

void AsebaNativeFunction(AsebaVMState *vm, uint16 id)
{
	nativeFunctions[id](vm);
}

const AsebaNativeFunctionDescription * const * AsebaGetNativeFunctionsDescriptions(AsebaVMState *vm)
{
	return nativeFunctionsDescription;
}

void uartSendUInt8(uint8 value)
{
	e_send_uart1_char((char *)&value, 1);
	while (e_uart1_sending());
}

void uartSendUInt16(uint16 value)
{
	e_send_uart1_char((char *)&value, 2);
	while (e_uart1_sending());
}

void AsebaSendBuffer(AsebaVMState *vm, const uint8* data, uint16 length)
{
	uartSendUInt16(length - 2);
	uartSendUInt16(vmState.nodeId);
	uint16 i;
	for (i = 0; i < length; i++)
		uartSendUInt8(*data++);
}

uint8 uartGetUInt8()
{
	char c;
	commError = 0;
	getDiffTimeMsAndReset();
	while (!e_ischar_uart1()) {
		if(getDiffTimeMs() > 500) { // Timeout of 500 ms.
			commError = 1;
			return 0;
		}
	}
	e_getchar_uart1(&c);
	return c;
}

uint16 uartGetUInt16()
{
	uint16 value;
	// little endian
	value = uartGetUInt8();
	if(commError) {
		return 0;
	}
	value |= (uartGetUInt8() << 8);
	if(commError) {
		return 0;
	}
	return value;
}

uint16 AsebaGetBuffer(AsebaVMState *vm, uint8* data, uint16 maxLength, uint16* source)
{
	//BODY_LED = 1;
	uint16 ret = 0;
	if (e_ischar_uart1())
	{
		uint16 len = uartGetUInt16() + 2;
		if(commError) {
			return 0;
		}
		if(len > maxLength) { // Wrong data received.
			return 0;
		}
		*source = uartGetUInt16();
		if(commError) {
			return 0;
		}
		uint16 i;
		for (i = 0; i < len; i++) {
			*data++ = uartGetUInt8();
			if(commError) {
				return 0;
			}
		}
		ret = len;
	}
	//BODY_LED = 0;
	return ret;
}

void setCamLine(int line)
{
	int _camLine;
	if (line < 0)
		line = 0;
	if (line >= 100)
		line = 99;
	switch(e_poxxxx_get_orientation()) {
		case 0:
			_camLine = (line * 32) / 5;
			e_poxxxx_config_cam(_camLine,0,4,480,4,8,RGB_565_MODE);
			break;
		default:
			_camLine = (line * 24) / 5;
			e_poxxxx_config_cam(80,_camLine,480,4,8,4,RGB_565_MODE);
			break;
	}
}


extern int e_ambient_and_reflected_ir[8];
void updateRobotVariables()
{
	unsigned i;
	static int camline = 50;
	// motor
	static int leftSpeed = 0, rightSpeed = 0;
	static char selectorState = -1;

	if (ePuckVariables.leftSpeed != leftSpeed)
	{
		leftSpeed = CLAMP(ePuckVariables.leftSpeed, -1000, 1000);
		e_set_speed_left(leftSpeed);
	}
	if (ePuckVariables.rightSpeed != rightSpeed)
	{
		rightSpeed = CLAMP(ePuckVariables.rightSpeed, -1000, 1000);
		e_set_speed_right(rightSpeed);
	}

	if(e_ambient_and_reflected_ir[7] != 0xFFFF) {
		// leds and prox
		for (i = 0; i < 8; i++)
		{
			e_set_led(i, ePuckVariables.leds[i] ? 1 : 0);
			ePuckVariables.ambiant[i] = e_get_ambient_light(i);
			ePuckVariables.prox[i] =  e_get_calibrated_prox(i);
		}
		e_ambient_and_reflected_ir[7] = 0xFFFF;
		e_set_body_led(ePuckVariables.leds[8] ? 1 : 0);
		e_set_front_led(ePuckVariables.leds[9] ? 1 : 0);
		SET_EVENT(EVENT_IR_SENSORS);
	}

	for(i = 0; i < 3; i++) {
		ePuckVariables.acc[i] = e_get_acc(i);
		ePuckVariables.mic[i] = e_get_micro_volume(i);
	}

	// camera
	if(e_poxxxx_is_img_ready())
	{
		for(i = 0; i < 60; i++)
		{
			ePuckVariables.camR[i] = CAM_RED(cam_data[i]);
			ePuckVariables.camG[i] = CAM_GREEN(cam_data[i]);
			ePuckVariables.camB[i] = CAM_BLUE(cam_data[i]);
		}
		if(camline != ePuckVariables.camLine)
		{
			camline = ePuckVariables.camLine;
			setCamLine(camline);
		//	e_po3030k_set_ref_exposure(160);
		//	e_po3030k_set_ww(0);
			e_poxxxx_set_mirror(1,1);
			e_poxxxx_write_cam_registers();
		}
		e_poxxxx_launch_capture((char *)cam_data);
		SET_EVENT(EVENT_CAMERA);
	}

	// encoders
	ePuckVariables.leftSteps = e_get_steps_left();
	ePuckVariables.rightSteps = e_get_steps_right();

	// gyro
	getAllAxesGyro(&ePuckVariables.gyro[0], &ePuckVariables.gyro[1], &ePuckVariables.gyro[2]);

	// battery
	if (isEpuckVersion1_3()) {
		ePuckVariables.battery = getBatteryValuePercentage();
	} else {
		ePuckVariables.battery = BATT_LOW; // BATT_LOW=1 => battery ok, BATT_LOW=0 => battery<3.4V
	}

	ePuckVariables.selector = getselector();
	if(selectorState != ePuckVariables.selector) {
		SET_EVENT(EVENT_SELECTOR);
	}
	selectorState = ePuckVariables.selector;

	ePuckVariables.tvRemote = e_get_data();

	if(ePuckVariables.timer > 0) {
		if(getDiffTimeMs() >= ePuckVariables.timer) {	// About 1 ms resolution.
			getDiffTimeMsAndReset();
			SET_EVENT(EVENT_TIMER);
		}
	}

}



void AsebaAssert(AsebaVMState *vm, AsebaAssertReason reason)
{
	e_set_body_led(1);
	while (1);
}

void initRobot()
{
	// init stuff
	e_init_port();
	e_start_agendas_processing();
	e_init_motors();
	e_init_uart1();
	e_init_remote_control();
	e_init_ad_scan(0);
	e_poxxxx_init_cam();
	setCamLine(50);
//	e_po3030k_set_ref_exposure(160);
//	e_po3030k_set_ww(0);
	e_poxxxx_set_mirror(1,1);
	e_poxxxx_write_cam_registers();
	//e_poxxxx_launch_capture((char *)cam_data);    // do not call here otherwise it was noticed that sometimes the robot starts in a strange manner

	// reset if power on (some problem for few robots)
	if (RCONbits.POR)
	{
		RCONbits.POR = 0;
		RESET();
	}
}


void AsebaWriteBytecode(AsebaVMState *vm) {
	int i = 0;
	_prog_addressT EE_addr;

	_init_prog_address(EE_addr, bytecode_version);
	_erase_eedata(EE_addr, 2);
	_wait_eedata();

	i = ASEBA_PROTOCOL_VERSION;
	_write_eedata_word(EE_addr, i);

	_wait_eedata();

	_init_prog_address(EE_addr, eeprom_bytecode);

	for(i = 0; i < 2048; i+=2) {

		_erase_eedata(EE_addr, 2);
		_wait_eedata();

		_write_eedata_word(EE_addr, vm->bytecode[i/2]);
		_wait_eedata();

		EE_addr += 2;
	}
}
void AsebaResetIntoBootloader(AsebaVMState *vm) {
	asm __volatile__ ("reset");
}

void initAseba()
{
	// VM
	int selector = SELECTOR0 | (SELECTOR1 << 1) | (SELECTOR2 << 2);
	vmState.nodeId = selector + 1;
	AsebaVMInit(&vmState);
	ePuckVariables.id = selector + 1;
	ePuckVariables.productId = ASEBA_PID_EPUCK;
	ePuckVariables.camLine = 50;
	name[6] = '0' + selector;
	e_led_clear();
	//e_set_led(selector,1);
}

int main()
{
	int i;
	_prog_addressT EE_addr;

	initRobot();

//	do {
		initAseba();
//	} while(!e_ischar_uart1());
/*
	for (i = 0; i < 14; i++)
	{
		uartGetUInt8();
	}
*/
	e_calibrate_ir();
	e_acc_calibr();

	// Load the bytecode...
	_init_prog_address(EE_addr, bytecode_version);
	_memcpy_p2d16(&i, EE_addr, _EE_WORD);

	// ...only load bytecode if version is the same as current one
	if(i == ASEBA_PROTOCOL_VERSION)
	{
		_init_prog_address(EE_addr, eeprom_bytecode);

		for( i = 0; i< 2048; i += 2)
		{
			_memcpy_p2d16(vmState.bytecode + i/2 , EE_addr, 2);
			EE_addr += 2;
		}

		// Init the vm
		AsebaVMSetupEvent(&vmState, ASEBA_EVENT_INIT);
		AsebaVMRun(&vmState, 1000);
	}

	e_poxxxx_launch_capture((char *)cam_data);

	while (1)
	{
		updateRobotVariables();
		AsebaProcessIncomingEvents(&vmState);
		AsebaVMRun(&vmState, 1000);

		if (AsebaMaskIsClear(vmState.flags, ASEBA_VM_STEP_BY_STEP_MASK) || AsebaMaskIsClear(vmState.flags, ASEBA_VM_EVENT_ACTIVE_MASK))
		{
			unsigned i;
			// Find first bit from right (LSB)
			asm ("ff1r %[word], %[b]" : [b] "=r" (i) : [word] "r" (events_flags) : "cc");
			if(i)
			{
				i--;
				CLEAR_EVENT(i);
				ePuckVariables.source = vmState.nodeId;
				AsebaVMSetupEvent(&vmState, ASEBA_EVENT_LOCAL_EVENTS_START - i);
				AsebaVMRun(&vmState, 1000);
			}
		}
	}

	return 0;
}
