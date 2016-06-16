# Windows IOT - Galileo

A server application for the Intel Galileo w/ Windows
that monitors a light sensor.

![An Intel Galileo with a light sensor attached][galileo]

## Building

### Dependencies

This project requires Visual Studio 2013.

### Compiling from source

Open the solution in visual studio and build it.

The resulting application will be available at `<Configuration>/server.exe`

### Deploying to the Galileo

Upload the executable to the card with the method of your choice (ftp or deploy from VS), then
run it from the remote command line.

## Running

Usage: `lumiere.exe [sampling time] [brightness threshold]`

Run the executable from the remote command line. The application may take
an optional parameter that indicates the time in ms between two brightness
samplings and a brightness threshold (0-1000) that indicates the ON/OFF
status of the ambient light.

[galileo]: https://cdn.sparkfun.com/assets/learn_tutorials/2/7/5/Galileo-SIK-circuit_06.jpg
