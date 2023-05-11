## Structures
| struct |
|----|
| [prox_comm_message_t](#prox_comm_message_t) |

## Enums
| enum |
|----|
| [simThymio.LED](#LED) |
| [simThymio.Button](#Button) |
| [simThymio.Motor](#Motor) |
| [simThymio.Behaviors](#Behaviors) |

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
| [simThymio.set_battery_voltage](#set_battery_voltage) |
| [simThymio.enable_behavior](#enable_behavior) |
| [simThymio.set_temperature](#set_temperature) |
| [simThymio.set_mic_intensity](#set_mic_intensity) |
| [simThymio.set_mic_threshold](#set_mic_threshold) |
| [simThymio.receive_rc_message](#receive_rc_message) |
| [simThymio.enable_sd_card](#enable_sd_card) |
| [simThymio.get_sd_card](#get_sd_card) |



#### prox_comm_message_t
A proximity communication
```C++
prox_comm_message_t = {int rx , int[] payloads , float[] intensities }
```
*fields*

  - **rx** The value, which is equal to the non-zero payloads

  - **payloads** The payload received by each sensor; set to 0 if the message was not recevied.

  - **intensities** The light intensity read by each sensor; set to 0 if the message was not recevied. 






#### LED
```C++
LED = {top=0, bottom_left=1, bottom_right=2, button_up=3, button_down=4, button_left=5, button_right=6, circle_0=7, circle_1=8, circle_2=9, circle_3=10, circle_4=11, circle_5=12, circle_6=13, circle_7=14, front_0=15, front_1=16, front_2=17, front_3=18, front_4=19, front_5=20, rear_0=21, rear_1=22, left_red=23, left_blue=24, right_red=25, right_blue=26, ground_0=27, ground_1=28, battery_0=29, battery_1=30, battery_2=31}
```



#### Button
```C++
Button = {backward=0, left=1, center=2, forward=3, right=4}
```



#### Motor
```C++
Motor = {left=0, right=1}
```



#### Behaviors
```C++
Behaviors = {battery=1, buttons=2, proximity=4, accelerometer=8, temperature=16, rc=32, mic=64, all=255}
```





#### create


Instantiate a Thymio2 controller
```C++
int id=simThymio.create(int handle,bool with_aseba=true,int behavior_mask=0,string friendly_name="Thymio II",int port=33333,int id=-1)
```
*parameters*

  - **handle** Handle of the Thymio2 model in CoppeliaSim

  - **with_aseba** Start emulating the Aseba firmware

  - **behavior_mask** The behaviors started at init and at reset

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
simThymio.enable_ground(int id,bool state,bool red=false,bool vision=false,float max_value=4741.9,float x0=0.00864)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **state** The state (enabled: true, disabled: false)

  - **red** Configure the sensors to respond only to the red-component of the reflecting material.

  - **vision** Configure the sensors to use vision to get the color of the reflecting material.

  - **max_value** Sensor model max value: at distance below x0, the response is 1/4 of the maximal value. Then it decreases smoothly and monotonically 

  - **x0** Sensor model x0: at distance below x0, the response is 1/4 of the maximal value. Then it decreases smoothly and monotonically. Larger values lead to a slower decrease with distance






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






#### set_battery_voltage


Set the current battery voltage, which is not simulated.
```C++
simThymio.set_battery_voltage(int id,float value)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **value** The voltage between 3.0 and 4.2 V






#### enable_behavior


Enable or disable one of the default behaviors
```C++
simThymio.enable_behavior(int id,bool value,int mask=255)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **value** Enabled: true, Disabled: false

  - **mask** An 8 bit-mask to specify the behavior. Set it to 255 to set all behaviors






#### set_temperature


Set the current temperature read by the sensor.
```C++
simThymio.set_temperature(int id,float temperature)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **temperature** A temperature in Celsius






#### set_mic_intensity


Set the current sound intensity read by the microphone.
```C++
simThymio.set_mic_intensity(int id,float intensity)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **intensity** A relative sound intensity in [0, 1].






#### set_mic_threshold


Set the microphone threshold the trigger the mic event.
```C++
simThymio.set_mic_threshold(int id,float threshold)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **threshold** A relative sound intensity in [0, 1].






#### receive_rc_message


Notify the rc receiver of a new message.
```C++
simThymio.receive_rc_message(int id,int address,int command)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **address** The 5-bit address between 0 and 31

  - **command** The 6-bit command between 0 and 63






#### enable_sd_card


Enable/disable the SD card, emulated as a rw directory
```C++
bool enabled=simThymio.enable_sd_card(int id,string path)
```
*parameters*

  - **id** The ID of the Thymio2 controller

  - **path** The path to the directory where to store files. Set to empty to disable.

*return*

  - **enabled** Whether the SD card is enabled or not.




#### get_sd_card


Enable/disable the SD card, emulated as a rw directory
```C++
bool enabled,string path=simThymio.get_sd_card(int id)
```
*parameters*

  - **id** The ID of the Thymio2 controller

*return*

  - **enabled** Whether the SD card is enabled.

  - **path** The path to the directory where to store files. Set to empty to disable.


