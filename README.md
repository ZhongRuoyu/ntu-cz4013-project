# Distributed Flight Information System

This is the repository for the Distributed Flight Information System (DFIS). It
provides a pair of client and servers.

## Directory structure

Below lists important files and directories found in this directory.

- `share`: This directory contains all the experiment results (`experiments`),
  as well as a list of flights (`flights.txt`) used to conduct the experiments.
  The list of flights is also required to run the server, unless a user-supplied
  list in the same format is given.
- `src`: This directory stores all the source code of DFIS.
- `srpc`: This directory stores source code of the SRPC submodule. SRPC is an
  independent module ready for general usage, so it is maintained in its own
  dedicated repository. DFIS uses SRPC by statically linking with it. Note that
  SRPC provides TCP components too, but they are not used by DFIS.
  - `examples`: This subdirectory provides several independent examples
    illustrating how to write simple programs using SRPC. DFIS is implemented in
    a similar way.
  - `include`: This subdirectory includes all the C++ header files that provide
    an interface to SRPC. Note that not all headers are listed there; some
    internal headers can be found in the `src` directory and they are not meant
    to be used by users.
  - `src`: This subdirectory contains all SRPC source code. Some internal
    headers are there, too.
  - `test`: This subdirectory contains all SRPC unit tests.
- `test`: This directory contains unit tests, mainly for marshalling and
  unmarshalling as well as some utility functions.
- `CMakeLists.txt`: This file contains build descriptions of DFIS.

## How to build

CMake is the build tool for DFIS. In addition to CMake, a recent C++ compiler is
also required, because DFIS and its dependency SRPC are written in C++20. If you
use Ubuntu 22.04, the default GCC is sufficient.

To build DFIS, run:

```plaintext
cmake -S . -B build
cmake --build build -j "$(nproc)"
```

To build unit tests, supply `-DWITH_TESTING=ON` to the first `cmake` invocation.
[GoogleTest](https://github.com/google/googletest) needs to be installed.

After a successful build, the client and server can be found under the `build` directory.

## How to use

The DFIS server can be started as follows:

```plaintext
build/dfis_server (at-least-once | at-most-once) <port> <flights-input>
```

The first argument is either `at-least-once` or `at-most-once`. That specifies
the invocation semantic to use. The second argument specified the port for the
server to listen. The third argument should be the path to the flight
information input. You may use `share/flight.txt`, or come up with your own one
following the same format.

The DFIS client can be started as follows:

```plaintext
build/dfis_client (at-least-once | at-most-once) <server-addr> <server-port>
```

The first argument is the same; either `at-least-once` or `at-most-once`. The
second and third arguments specify the address and port of the server to connect
to. A user-friendly command line interface will show you the usage:

```console
$ build/dfis_client at-most-once example.com 8000
Info: At-most-once semantic is used
Selections available:
0. Exit
1. Flight search
2. Flight info
3. Seat reservation
4. Seat availability monitoring
5. Price range search
6. Seat reservation cancellation
Enter selection:
```
