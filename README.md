# Readout library for DRS4 outputs

DRS4lib is a C++ library for the unpacking and recasting into high level objects of raw waveforms acquired using the Domino Ring Sampling (DRS4) ASIC developed in PSI, Switzerland.
In particular, the packing performed by the CAEN V1742 (32+2 channels, VME) and N6742 (16+1 channels, NIM) commercial versions is handled, and methods to combine several modules into a single stream are provided.
It also performs the calibration of waveforms on the fly with the help of a calibration conditions dictionary holding values for each module.

## Compile

This library can be built using CMake, with its standard recipe:
```sh
mkdir build && cd build
cmake ..
make
```
