# Windows IOT - Galileo

A server application for the Intel Galileo w/ Windows
that monitors a light sensor.

![An Intel Galileo with a light sensor attached][galileo]

## Building

### Dependencies

This project requires CMake 2.8+ and Visual Studio 2013 or MinGW-w64.

### Compiling from source

From the project root, run the following commands:

```bash
mkdir build
cd build
cmake -G "Visual Studio 2013" ..
```

This will create a Visual Studio 2013 solution in the `build` directory.

To compile the sources into a working executable, run:

```bash
cmake --build . --config Release
```

or open the solution in visual studio and build it.

The resulting application will be available at `server/Release/lumiere.exe`

### Deploying to the Galileo

Upload the executable to the card with the method of your choice, then
run it from the remote command line.

## Running

Usage: `lumiere.exe [sampling time]`

Run the executable from the remote command line. The application may take
an optional parameter that indicates the time in ms between two brightness
samplings.

[galileo]: https://cdn.sparkfun.com/assets/learn_tutorials/2/7/5/Galileo-SIK-circuit_06.jpg
