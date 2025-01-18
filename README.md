# PPM: Privacy-Preserving Monitoring

## How to run this project

Make sure you are using a version of Ubuntu,
and have `g++` and `make` installed on your system.

### Dependencies
First, install the following libraries;
for Ubuntu, all of them can be installed using `apt`:
* The GNU Multiple Precision Arithmetic Library (GMP):
```bash
sudo apt install libgmp-dev
```
* OpenSSL:
```bash
sudo apt install libssl-dev
```
* ZeroMQ Messaging Library:
```bash
sudo apt install libzmq3-dev
```

To install yosys, it's easiest to install the OSS CAD Suite.
Follow installation instructions for this suite at
[this link](https://github.com/YosysHQ/oss-cad-suite-build?tab=readme-ov-file#installation).
To check if this step was successful,
you can try the following command in your terminal:
```
yosys --version
```

### Building the binaries
Run `make` in the same directory as `Makefile`.
If this step succeeds, binaries `System` and `Monitor` are available
in directory `bin/`. Run these processes in separate terminal `windows'
(tabs work if you're using the default Ubuntu terminal).

### Running `Monitor` and `System`

Both `Monitor` and `System` need some arguments to be initialised properly.
Running either program with argument `-h` will print a list of these arguments.
```
$ ./Monitor -h
Usage: ./Monitor -proto p -security k -mslen m -sslen s -ngates n [-sys sys_name] [-spec spec_name]
```
Note that the specified system also needs some system-specific arguments.
For more information, have a look at the file `src/CommandLineInterface.cc`.

Before running the monitor:
1. Make sure the file `synth.blif` either doesn't exist
or is up-to-date with your spec.
2. Make sure the necessary parameters for your Verilog spec are
included in the synthesis script `synth.ys`. An example would be the following:
```
read_verilog -D P1=P1 -D P2=P2 ... SPEC_FILE
```

To run the system, call `./System` with identical arguments as `./Monitor`,
except for system-specific arguments.

**Note:** at the moment, I recommend using one of the experiment scripts
(such as `timekeeper-lwy.sh`).
You can use customised parameters by modifying these scripts.
