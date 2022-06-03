## Structures




## Functions
| function |
|----|
| [simEPuck.create](#create) |
| [simEPuck.get_speed](#get_speed) |
| [simEPuck.set_target_speed](#set_target_speed) |
| [simEPuck.get_acceleration](#get_acceleration) |
| [simEPuck.get_ground](#get_ground) |
| [simEPuck.get_proximity](#get_proximity) |
| [simEPuck.set_mic_intensity](#set_mic_intensity) |
| [simEPuck.set_battery_voltage](#set_battery_voltage) |
| [simEPuck.set_selector](#set_selector) |
| [simEPuck.receive_rc_message](#receive_rc_message) |
| [simEPuck.get_gyroscope](#get_gyroscope) |
| [simEPuck.set_ring_led](#set_ring_led) |
| [simEPuck.set_body_led](#set_body_led) |
| [simEPuck.set_front_led](#set_front_led) |
| [simEPuck.enable_accelerometer](#enable_accelerometer) |
| [simEPuck.enable_camera](#enable_camera) |
| [simEPuck.enable_proximity](#enable_proximity) |








#### create


Instantiate a e-puck controller
```C++
int id=simEPuck.create(int handle,bool with_aseba=true,string friendly_name="e-puck",int port=33333,int id=-1)
```
*parameters*

  - **handle** Handle of the e-puck model in CoppeliaSim

  - **with_aseba** Start emulating the Aseba firmware

  - **friendly_name** A friendlier name used in Thymio Suite to label nodes. Must not be unique. If left empty, it is set to "e-puck".

  - **port** The Aseba port number

  - **id** The Aseba node ID. Two nodes in the same Aseba network cannot have the same id. Set to -1 to choose the lowest available id.

*return*

  - **id** The ID assigned to the e-puck controller and its potential Aseba node.




#### get_speed


Get the currrent angular speed of a motor
```C++
float speed=simEPuck.get_speed(int id,int index)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **index** The index of the motor (left: 0, right: 0)

*return*

  - **speed** The angular speed in rad/s




#### set_target_speed


Set the target angular speed of a motor
```C++
simEPuck.set_target_speed(int id,int index,float speed)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **index** The index of the motor (left: 0, right: 0)

  - **speed** The angular speed in rad/s






#### get_acceleration


Get the current reading the accelerometer
```C++
float x,float y,float z=simEPuck.get_acceleration(int id)
```
*parameters*

  - **id** The ID of the e-puck controller

*return*

  - **x** The x-component (left) in m/s^2

  - **y** The y-component (front) in m/s^2

  - **z** The z-component (down) in m/s^2




#### get_ground


Get the current reading of a ground sensor
```C++
float reflected=simEPuck.get_ground(int id,int index)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **index** The index of the sensor (left: 0, right: 0)

*return*

  - **reflected** The current sensor reading




#### get_proximity


Get the current reading of a proximity sensor
```C++
float reading=simEPuck.get_proximity(int id,int index)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **index** The index of the sensor

*return*

  - **reading** The current sensor reading




#### set_mic_intensity


Set the current sound intensities read by the microphones.
```C++
simEPuck.set_mic_intensity(int id,float[] intensity)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **intensity** An array of size 3 with the relative sound intensity in [0, 1].






#### set_battery_voltage


Set the current battery voltage, which is not simulated.
```C++
simEPuck.set_battery_voltage(int id,float value)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **value** The voltage between 3.0 and 4.2 V






#### set_selector


Set the current selector position.
```C++
simEPuck.set_selector(int id,int position)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **position** The position between 0 and 15






#### receive_rc_message


Notify the rc receiver of a new message.
```C++
simEPuck.receive_rc_message(int id,int data)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **data** The payload between 0 and 255






#### get_gyroscope


Get the current reading the gyroscope
```C++
float x,float y,float z=simEPuck.get_gyroscope(int id)
```
*parameters*

  - **id** The ID of the e-puck controller

*return*

  - **x** The x-component in rad/s

  - **y** The y-component in rad/s

  - **z** The z-component in rad/s




#### set_ring_led


Switch on or off one of the 8 red LED
```C++
simEPuck.set_ring_led(int id,int index,bool value)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **index** The index of the LED (from 0 to 7)

  - **value** LED state






#### set_body_led


Switch on or off the green body LED
```C++
simEPuck.set_body_led(int id,bool value)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **value** LED state






#### set_front_led


Switch on or off the green front LED
```C++
simEPuck.set_front_led(int id,bool value)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **value** LED state






#### enable_accelerometer


Enable or disable the accelerometer
```C++
simEPuck.enable_accelerometer(int id,bool state)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **state** The state (enabled: true, disabled: false)






#### enable_camera


Enable or disable the camera
```C++
simEPuck.enable_camera(int id,bool state)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **state** The state (enabled: true, disabled: false)






#### enable_proximity


Enable, disable and configure the proximity sensors
```C++
simEPuck.enable_proximity(int id,bool state,bool red=false)
```
*parameters*

  - **id** The ID of the e-puck controller

  - **state** The state (enabled: true, disabled: false)

  - **red** Configure the sensors to respond only to the red-compoment of the reflecting material.




