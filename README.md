# PPM: Privacy-Preserving Monitoring

## How to run this project

Make sure you are using a version of Ubuntu,
and have `g++` and `make` installed on your system.

### Libraries
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

### Building the binaries
Run `make` in the same directory as `Makefile`.
If this step succeeds, binaries `System` and `Monitor` are available
in directory `bin/`. Run these processes in separate terminal `windows'
(tabs work if you're using the default Ubuntu terminal).

---
**NOTE:** so far, `Monitor` and `System` have been hardcoded
to run a specific protocol with specific parameters.
This means you need to change the source (`src/{Monitor/System}.cc`)
to specify another protocol and its parameters.
We will add command-line options in a later pull request.
---
