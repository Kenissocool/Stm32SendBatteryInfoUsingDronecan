# Libcanard

Minimal implementation of the DroneCAN protocol stack in C for resource constrained applications.



## Library Installation

The library that I use is Arduino DroneCAN. It is easier than libcanard itself because you don't have to convert the dsdl file to .c and .h yourself. 

```bash
git submodule add https://github.com/BeyondRobotix/Arduino-DroneCAN
```
go to lib folder inside there are ArduinoDroneCANlib,dronecan and libcanard. Copy dronecan and libcanard to your project

The libcanard folder contain three files:
  | File                | Description                                                                                    |
  |---------------------|------------------------------------------------------------------------------------------------|
  | `canard.c`          | the only translation unit; add it to your build or compile it into a separate static library   | 
  |`canard.h`           | the API header; include it in your application                                                 |
  |`canard_internals.h` | internal definitions of the library                                                            | 

The dronecan folder contain two folders:
  | Folder              | Description                                                                                    |
  |---------------------|------------------------------------------------------------------------------------------------|
  | `src`               | the only translation unit; add it to your build or compile it into a separate static library   | 
  |`inc`                | the API header; include it in your application                                                 |


Example for Make:

```make
# Adding the library.
INCLUDE += libcanard
CSRC += libcanard/canard.c

# Adding drivers, unless you want to use your own.
# In this example we're using Linux SocketCAN drivers.
INCLUDE += libcanard/drivers/socketcan
CSRC += libcanard/drivers/socketcan/socketcan.c
```

Example for CMake, first installing dependencies. 

```bash
sudo apt-get update && sudo apt-get install gcc-multilib g++-multilib
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest .
```

There is no dedicated documentation for the library API, because it is simple enough to be self-documenting.
Please check out the explanations provided in the comments in the header file to learn the basics.
Most importantly, check out the [examples](examples) directory for fully worked examples

For generation of de-serialisation and serialisation source code, please refer https://github.com/dronecan/dronecan_dsdlc .

## C++ Interface

The C++ interface is in the canard/ directory. See
[examples/ESCNode_C++](examples/ESCNode_C++) for a fully worked example of the C++ API.

## Library Development

This section is intended only for library developers and contributors.

The library design document can be found in [DESIGN.md](DESIGN.md)

### Building and Running Tests

```bash
mkdir build && cd build
cmake ../libcanard/tests    # Adjust path if necessary
make
./run_tests
```
