# Simulate Aseba, Thymios and E-Pucks in CoppeliaSim

This repository contain code to simulate specific robots ([Thymio2](http://www.thymio.org) and [e-puck](http://http://www.e-puck.org)) and generic [Aseba](https://www.thymio.org/products/programming-with-thymio-suite/program-thymio-aseba/) nodes in [CoppeliaSim](https://coppeliarobotics.com).


You can control the robots/nodes
- in lua, using the interpreter embedded in CoppeliaSim
- in Aseba using [Aseba Studio](http://wiki.thymio.org/en:asebastudio), [ThymioSuite](https://www.thymio.org/products/programming-with-thymio-suite/), or even from CoppeliaSim (with a more limited support).
- in ROS, using my own [Aseba-ROS bridge](http://jeguzzi.github.io/ros-aseba/).

## Installation

### Dependencies

- [Aseba](https://github.com/aseba-community/aseba)  (available as submodule)
- [Dashel](https://github.com/aseba-community/dashel) (available as submodule)
- [CoppeliaSim v4.3, or v4.4](https://coppeliarobotics.com) (we do not yet support the latest v4.5)
- Cmake >= 3.5
- a C++17 compiler
- [OpenCV](https://opencv.org/)
- [optional, to parse Aseba scripts] [libxml2](https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home)
- [optional, for autodiscovery] Avahi or Bonjour
- [optional, to autogenerate the docs] [Jinjia2](https://jinja.palletsprojects.com)

#### Linux

```console
sudo apt install cmake libopencv-dev [libxml2-dev] [libavahi-compat-libdnssd-dev]
[python3 -m pip install Jinja2]
```

#### MacOs
```console
brew install cmake opencv [libxml2]
[python3 -m pip install Jinja2]
```

### CoppeliaSim

To compile and then use the CoppeliaSim plugin you need ... [CoppeliaSim](https://www.coppeliarobotics.com).
Download the latest release and export the location where you place it with
```bash
export COPPELIASIM_ROOT_DIR=<path to the folder containing the programming subfolder>
```
which on Linux is the root folder that you download, while on MacOs is `/Applications/coppeliaSim.app/Contents/Resources`, if you install the app to the default location.

### Build

#### Linux and MacOs
```bash
$ git clone --recursive https://github.com/jeguzzi/coppeliasim-aseba.git
$ cd coppeliasim-aseba
$ mkdir -p build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make install -j4
```

This will build the plugin and install it together with the robot model[s].

## Running

Launch CoppeliaSim. In the model browser, you will find models for Thymio, [MightyThymio](https://github.com/jeguzzi/mighty-thymio), and e-puck (called `e-puck-aseba` to distinguish it from the model already installed) in `robots > mobile`, which you can drag into the scene. Press play.

### Aseba lua interface

If you want to program your own custom Aseba node, in the init callback of a lua script, use
```lua
node = simAseba.create_node()
```
You can then add Aseba variables, functions and events [using this lua API](api_Aseba.md).

### Thymio lua interface

The simulated Thymio emulates the Aseba firmware available on real Thymios.
Inside CoppeliaSim, we also expose a lua interface similar to the Aseba interface: take a look [at the list of supported functions](api_Thymio.md). For example, in Aseba, you can change the color of the large top LED like this (Aseba is integer-only and in this case a value of 32 corresponds to maximal brightness)
```lua
call leds.top(32, 0, 0)
```
while inside CoppeliaSim, you control the LED color like this
```lua
simThymio.set_led(0, 0, 1.0, 0.0, 0.0)
```
where the first two arguments identifies the robot and the LED, and where rgb colors are encoded in float instead.

If you want, you can customize the Aseba node of the simulated Thymio too, using the [same API](api_Aseba.md).

### e-puck lua interface

Similarly, the simulated e-puck, emulates the [Aseba firmware](https://github.com/aseba-community/aseba-target-epuck) available for real e-pucks, while we also expose a [lua interface](api_EPuck.md).


### Multiple robots

You can have as many robots/nodes as you like and attach them to the same or to different Aseba networks. Nodes on the same network will be assigned different IDs and can exchange Aseba events among themselves.


## Thymio

The simulated Thymio implements all features of the real robot (firmware v14).

#### Full support
  - [x] motors
  - [x] LEDs
  - [x] proximity sensors
  - [x] ground sensors
  - [x] accelerometer
  - [x] buttons (you can click on them)
  - [x] tap detection
  - [x] proximity communication
  - [x] SD card

#### Partial support
##### Aseba interface and programmatic read/write access from coppeliaSim 
  - [x] battery
  - [x] temperature sensor
  - [x] RC sensor
  - [x] Microphone

##### Only Aseba interface
  - [x] speaker (accepts `sound.{play|replay|freq}` and triggers the event `sound.finished` after waiting during the sound duration without actually playing it.)


The implementation is heavily inspired by the excellent 2D simulator [Enki](https://github.com/enki-community/enki). In particular, the Thymio LEDs and  3D model are taken directly from Enki. Enki provides the core functionality of [Aseba playground](http://wiki.thymio.org/en:asebaplayground), which is the simulator mostly commonly used with Thymios and Aseba.

With respect to Enki/Aseba playground, coppeliaSim together with this plugin, share these features:
  - simulate one or more Thymios, emulating the Aseba-based firmware (and are therefore compatible with Aseba Studio and/or VPL)
  - motors, LEDs, proximity sensors, buttons, ground sensors
  - compatible with the new autodiscovery of Thymio Suite


CoppeliaSim together with this plugin, adds:
  - fully featured 3D robotics simulation (this from CoppeliaSim alone :-))
  - ground sensors detect steps/holes/ too
  - proximity sensors responds to material color too
  - some more LEDs (2 for ground sensors and 3 for the battery-level)
  - accelerometer
  - proximity communication
  - tap detection from accelerometer
  - basic support for rc, microphone, speaker, battery, and temperature sensor.
  - **option** to enable the same behavior of the real robot that provides feedback about proximity and ground sensors, battery level, accelerometer, microphone, temperature and rc, when the LEDs are not controlled by an Aseba script.
  - support for firmware v14
  - multiple robots on the same Aseba network (without the need of running `asebaswitch`)
  - customizable Thymio Aseba node
  - custom Aseba nodes implemented in lua
  - loading Aseba scripts from lua


## e-puck

The simulated e-puck implements all features of the real robot exposed in the Aseba firmware.

#### Full support
  - [x] motors
  - [x] LEDs
  - [x] proximity sensors
  - [x] accelerometer
  - [x] gyroscope
  - [x] camera

#### Partial support
##### Aseba interface and programmatic read/write access from coppeliaSim
  - [x] battery
  - [x] selector
  - [x] RC sensor
  - [x] microphones

Like for the Thymio, the object model is taken directly from Enki. With respect to Enki/Aseba playground, coppeliaSim together with this plugin, shares these features:
  - simulate one or more e-puck, emulating the Aseba-based firmware (therefore compatible with Aseba Studio).
  - motors, proximity sensors, camera

In addition to the list presented before for the Thymio, this simulation adds specific e-puck features:
  - 8 red LED on the ring
  - green body LED
  - green front LED
  - accelerometer
  - gyroscope
  - basic support for rc, microphones, battery, and selector
  - customizable e-puck Aseba node
