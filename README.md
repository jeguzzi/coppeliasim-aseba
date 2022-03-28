# Simulate Aseba and Thymios in CoppeliaSIm


This repository contain code to simulate specific robots ([Thymio2](http://www.thymio.org)) and generic [Aseba](https://www.thymio.org/products/programming-with-thymio-suite/program-thymio-aseba/) nodes in [CoppeliaSim](https://coppeliarobotics.com).

You can control the robots/nodes
- in lua, using the interpreter embedded in CoppeliaSim
- in Aseba using [Aseba Studio](http://wiki.thymio.org/en:asebastudio)
- in ROS, using my own [Aseba-ROS bridge](http://jeguzzi.github.io/ros-aseba/).

## Installation

### Dependencies

- [aseba](https://github.com/aseba-community/aseba)  (available as submodule)
- [dashel](https://github.com/aseba-community/dashel) (available as submodule)
- [coppeliaSim >= v4.3](https://coppeliarobotics.com)
- cmake >= 3.5
- a C++17 compiler

### CoppeliaSim

To compile and then use the CoppeliaSim plugin you need ... [CoppeliaSim](https://www.coppeliarobotics.com).
Download the latest release. Export the location where you place it.
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

This will build the plugin and install it  together with the robot model[s].

## Running

Launch CoppeliaSim. In the model browser, you will find a model for the Thymio in `robots > mobile`, which you can drag into the scene. Press play.

### Aseba lua interface

If you want to define you own custom Aseba node, in the init callback of one of your script, use
```lua
node = simAseba.create_node()
```
You can then add Aseba variables, functions and events [using this lua API](api_Aseba.md).

### Thymio lua interface

The simulated Thymio, emulate the Aseba firmware available on real Thymios.
Inside CoppeliaSim, we expose a similar interface in lua as the Aseba interface: take a look [at the list of supported functions](api_Thymio.md). For example, in Aseba, you can change the color of the large top LED like this (Aseba is integer-only and in this case a value of 32 corresponds to maximal brightness)
```lua
call leds.top(32, 0, 0)
```
while inside CoppeliaSim, you can send the same command like this
```lua
  simThymio.set_led(0, 0, 1.0, 0.0, 0.0)
```
where the first to arguments identifies robot and LED and colors are encoded in float instead.

If you want, you can customize the Aseba node of the simulated Thymio too, using the [same API](api_Aseba.md).

### Multiple robots

You can have as many robots/nodes as you like and expose them to the same or different Aseba network. Nodes on the same network will be assigned different IDs.


## Thymio

The simulated Thymio implements most of the features of the real robot:
  - [x] motors
  - [x] LEDs
  - [x] proximity sensors
  - [x] ground sensors
  - [x] accelerometer
  - [x] buttons
  - [x] tap detection
  - [x] proximity communication

Missing:
  - [ ] speaker
  - [ ] SD card
  - [ ] temperature sensor
  - [ ] RC sensor
  - [ ] battery

The implementation is heavily inspired by the excellent 2D simulator [Enki](https://github.com/enki-community/enki). In particular, the LEDs and the object model are taken directly from Enki.

Enki provides the core functionality of [Aseba playground](http://wiki.thymio.org/en:asebaplayground), which is the simulator mostly commonly used with Thymios and Aseba.

With respect to Enki/Aseba playground, coppeliaSim together with this plugin, share this features:

  - simulate one or more Thymio, emulating the Aseba-based firmware (and are therefore compatible with Aseba Studio and/or VPL).
  - motors, LEDs, proximity sensors, buttons, ground sensors.

CoppeliaSim together with this plugin, adds:
  - Fully featured 3D robotics simulation (this from CoppeliaSim alone :-))
  - ground sensors detects steps/holes/ too
  - proximity sensors responds to material color too
  - accelerometer
  - proximity communication
  - tap detection from acceletometer
  - multiple robots on the same Aseba network (without the need of asebaswitch)
  - customizable Thymio aseba node
  - custom Aseba nodes implemented in lua
