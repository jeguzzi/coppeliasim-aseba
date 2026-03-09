# Installation

## CoppeliaSim

To use the CoppeliaSim plugin you need ... [CoppeliaSim](https://www.coppeliarobotics.com). At the moment, we support CoppeliaSim from v4.3 to v4.10 [latest]. Download one of the supported version.

> [!IMPORTANT]
> If you are going to install the pre-compiled CoppeliaSim plugin released on GitHub (see below), you need to install the version of CoppeliaSim it was built against and that is part of the file name.

## Install release

1. Download the [released](https://github.com/jeguzzi/robomaster_sim/releases) `tar.gz` archives. The archives are named like
```
simAseba-<PLUGIN VERSION>-CoppeliaSim-<COPPELIASIM VERSION>-<OS name>-<OS Architecture>.tar.gz
```
Pick the one that is compatible with the operating system, the architecture, and the version of CoppeliaSim.

> [!NOTE]
> Darwin is another name for macOs.

2. Unpack it inside the CoppeliaSim file tree

```bash
tar -vxf simAseba[...].tar.gz -C <CoppeliaSim path> --strip-components=1
```

where `<CoppeliaSim path>` is the location where `CoppeliaSim` is installed. For example, it could be similar to

**Linux**:
```bash
tar -vxf simAseba-1.0-CoppeliaSim-4.10-Linux-x86_64.tar.gz -C ~/CoppeliaSim_Edu_V4_10_0_Ubuntu24_04 --strip-components=1
```

**macOs**:
```bash
tar -vxf simAseba-1.0-CoppeliaSim-4.10-Darwin-arm64.tar.gz -C /Applications/coppeliaSim.app --strip-components=1
```

**Windows**:
```cmd
tar -vxf simAseba-1.0-CoppeliaSim-4.10-Windows-AMD64.tar.gz -C  "C:\Program Files\CoppeliaRobotics\CoppeliaSimEdu" --strip-components=1
```

3. Install zeroconf dependencies

**Linux**:
```bash
sudo apt install -y libavahi-compat-libdnssd
```

**Windows**:
```cmd
choco install -y bonjour
```

## Build from source

### Install Build tools

Install [cmake](https://cmake.org), git, and a C++-17 compiler.

**Linux (Ubuntu)**:

```bash
$ sudo apt update && sudo apt cmake gcc git
```

**macOs**:
If you don't have them already, install 
- the XCode command line tools: `$ xcode-select --install`
- and [homebrew](https://brew.sh).

Then `$ brew install cmake git`

**Windows**:
If you don't have them already, install [Visual Studio](https://visualstudio.microsoft.com), cmake and git.

> [!IMPORTANT]
> Subsequent Windows build commands should be executed in a “Native Tools Command Prompt for VS” console with admin privileges.

### Export the location of CoppeliaSim

**Linux and macOs**
```bash
$ export COPPELIASIM_ROOT_DIR=<path to the folder containing the programming subfolder>
```

**Windows**
```cmd
$ set COPPELIASIM_ROOT_DIR=<path to the folder containing the programming subfolder>
```

which on Linux is the root folder that you download, while on macOs is `/Applications/coppeliaSim.app/Contents/Resources`, if you install the app to the default location.

### Clone this repository

```bash
git clone --recursive https://github.com/jeguzzi/coppeliasim-aseba.git
cd coppeliasim-aseba
```

### Build without VCPKG

> [!WARNING]
> Windows not supported.

1. Install dependencies

**Linux**:
```bash
sudo apt install libopencv-dev xsltproc [libxml2-dev] [libavahi-compat-libdnssd-dev]
[python3 -m pip install Jinja2 xmlschema]
```

**macOs**:
```bash
brew install opencv [libxml2]
[python3 -m pip install Jinja2 xmlschema]
```

2. Compile the plugin
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
cmake --install build
```

This will build the plugin and install it together with the robot model[s].

### Build with VCPKG

1. Install VCPKG

Install [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell).
Then set `VCPKG_ROOT` to the path of the cloned vcpkg repository and add it to the `PATH`.

**Linux and macOs**
```bash
$ export VCPKG_ROOT=<vcpkg path>
$ export PATH=$VCPKG_ROOT:$PATH
```

**Windows**
```cmd
$ set VCPKG_ROOT=<vcpkg path>
$ set PATH=%VCPKG_ROOT%;%PATH%
```

2. Install dependencies

**Linux**:
```bash
sudo apt install xsltproc libavahi-compat-libdnssd-dev
```

**windows**:
```bash
choco install -y xsltproc
```

3. Compile the plugin

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release --preset release -DARCHIVE=OFF
cmake --build build --config Release --parallel 4
cmake --install build
```

This will build the plugin and install it together with the robot model[s].