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

## Utilities

This library is based on @ledovsk's implementation in the context of past test beam campaigns (e.g. RADiCAL in May 2023 at CERN SPS), and therefore encompasses a few utilities developed for these specific usages.
For instance, a ROOT TTree can be built with high level information unpacked from streams, or a canvas can be shown with several channel waveforms.

### Make list of events

Uncorrupted DRS event is an event without bad flags:
* no board failure flag
* initialization is OK
* total event size is 6920
* two active groups are present
* all three control flags are zero
* TR0 is present

This step finds uncorrupted events in each DRS and makes a list of event numbers in each DRS that belong to the same trigger
```
<event number in DRS0>  <event number in DRS1>   <trigger number>
```
This step is done with 
```
./listGoodEvents
```

### TTree of calibrated events

Create TTree for all events in each DRS separately.
Calibration is applied at this step also:
* voltage calibration
* sample calibration
* time calibration
This step is done with 
```
./maketree
```

## Merge two DRS files

Merge uncorrupted events from both DRS files in one ROOT file
This step is done with 
```
./merge
```

### Example of analysis

A simple example showing 
* how to decode individual waveforms from ROOT TTree 
* validation that both DRS modules are in sync
* reconstruction of beam position using Wire Chambers

This step is done with
```
> root -l CheckSync.C
```
The output is an image, similar to ```230529_01.png```
![alt text](https://github.com/ledovsk/RADiCAL_TB_May2023/blob/main/230529_01.png)

## Script 
An example script that performs all steps above. One need to modify it to specify path to DRS files, desired run numbers and destination for results
```
./run.sh
```
