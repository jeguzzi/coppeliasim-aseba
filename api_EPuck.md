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


