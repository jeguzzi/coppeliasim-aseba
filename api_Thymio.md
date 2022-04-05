## Structures
| struct |
|----|
| [prox_comm_message_t](#prox_comm_message_t) |


## Functions
| function |
|----|
| [simThymio.create](#create) |
| [simThymio.enable_accelerometer](#enable_accelerometer) |
| [simThymio.enable_ground](#enable_ground) |
| [simThymio.enable_proximity](#enable_proximity) |
| [simThymio.enable_prox_comm](#enable_prox_comm) |
| [simThymio.set_led](#set_led) |
| [simThymio.set_led_intensity](#set_led_intensity) |
| [simThymio.get_speed](#get_speed) |
| [simThymio.set_target_speed](#set_target_speed) |
| [simThymio.get_acceleration](#get_acceleration) |
| [simThymio.get_ground](#get_ground) |
| [simThymio.get_proximity](#get_proximity) |
| [simThymio.get_prox_comm_rx](#get_prox_comm_rx) |
| [simThymio.set_prox_comm_tx](#set_prox_comm_tx) |
| [simThymio.get_button](#get_button) |
| [simThymio.set_button](#set_button) |




#### prox_comm_message_t
A proximity communication
```C++
prox_comm_message_t = {int rx , int[] payloads , float[] intensities }
```
*fields*

  - **rx** The value, which is equal to the non-zero payloads

  - **payloads** The payload received by each sensor; set to 0 if the message was not recevied.

  - **intensities** The light intensity read by each sensor; set to 0 if the message was not recevied. 






#### create


Instantiate a Thymio2 controller
```C++
int id=simThymio.create(int handle,bool with_aseba=true,string friendly_name="Thymio II",int port=33333,int id=-1)
```
*parameters*

  - **handle** Handle of the Thymio2 model in CoppeliaSim

  - **with_aseba** Start emulating the Aseba firmware

  - **friendly_name** A friendlier name used in Thymio Suite to label nodes. Must not be unique. If left empty, it is set to "Thymio II".

  - **port** The Aseba port number

  - **id** The Aseba node ID. Two nodes in the same Aseba network cannot have the same id. Set to -1 to choose the lowest available id.

*return*

  - **id** The ID assigned to the Thymio2 controller and its potential Aseba node.




#### enable_accelerometer


Enable or disable the accelerometer
```C++
simThymio.enable_accelerometer(int id,bool state)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **state** The state (enabled: true, disabled: false)






#### enable_ground


Enable, disable and configure the ground sensors
```C++
simThymio.enable_ground(int id,bool state,bool red=false,bool vision=false)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **state** The state (enabled: true, disabled: false)

  - **red** Configure the sensors to respond only to the red-compoment of the reflecting material.

  - **vision** Configure the sensors to use vision to get the color of the reflecting material.






#### enable_proximity


Enable, disable and configure the proximity sensors
```C++
simThymio.enable_proximity(int id,bool state,bool red=false)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **state** The state (enabled: true, disabled: false)

  - **red** Configure the sensors to respond only to the red-compoment of the reflecting material.






#### enable_prox_comm


Enable or disable the proximity communication
```C++
simThymio.enable_prox_comm(int id,bool state)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **state** The state (enabled: true, disabled: false)






#### set_led


Set the color of a LED
```C++
simThymio.set_led(int id,int index,float r,float g,float b)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the LED

  - **r** Red component in [0, 1]

  - **g** Green component in [0, 1]

  - **b** Blue component in [0, 1]






#### set_led_intensity


Set the color intentity of a LED
```C++
simThymio.set_led_intensity(int id,int index,float a)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the LED

  - **a** The color intensity in [0, 1]






#### get_speed


Get the currrent angular speed of a motor
```C++
float speed=simThymio.get_speed(int id,int index)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the motor (left: 0, right: 0)

*return*

  - **speed** The angular speed in rad/s




#### set_target_speed


Set the target angular speed of a motor
```C++
simThymio.set_target_speed(int id,int index,float speed)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the motor (left: 0, right: 0)

  - **speed** The angular speed in rad/s






#### get_acceleration


Get the current reading the accelerometer
```C++
float x,float y,float z=simThymio.get_acceleration(int id)
```
*parameters*

  - **id** The ID of the Thymio2 controller

*return*

  - **x** The x-component (left) in m/s^2

  - **y** The y-component (front) in m/s^2

  - **z** The z-component (down) in m/s^2




#### get_ground


Get the current reading of a ground sensor
```C++
float reflected=simThymio.get_ground(int id,int index)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the sensor (left: 0, right: 0)

*return*

  - **reflected** The current sensor reading




#### get_proximity


Get the current reading of a proximity sensor
```C++
float reading=simThymio.get_proximity(int id,int index)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the sensor

*return*

  - **reading** The current sensor reading




#### get_prox_comm_rx


Get the last messages received by the proximity communication
```C++
prox_comm_message_t[] messages=simThymio.get_prox_comm_rx(int id)
```
*parameters*

  - **id** The ID of the Thymio2 controller

*return*

  - **messages** A table of messages, one for each sender.




#### set_prox_comm_tx


Set the integer value to be transmitted at each step if the communication is enabled. The real robot is limited to 11 bits. The simulated robot can send 32-bit integers.
```C++
simThymio.set_prox_comm_tx(int id,int tx)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **tx** The integer value to send.






#### get_button


Get the current state of a button sensor
```C++
bool value=simThymio.get_button(int id,int index)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the button.

*return*

  - **value** Pressed: true, Released: false




#### set_button


Overwrite the current state of a button sensor
```C++
simThymio.set_button(int id,int index,bool value)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **index** The index of the button.

  - **value** Pressed: true, Released: false




