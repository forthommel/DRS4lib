# RADiCAL_TB_May2023

Files in this repository help to create ROOT TTree of uncorrupted and calibrated events from both DRS modules that belong to the same trigger. The following steps are needed

## Compile
```
make listGoodEvents
make maketree
make merge
```

## Make list of events

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

## TTree of calibrated events

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

## Example of analysis

A simple example showing 
* how to decode individual waveforms from ROOT TTree 
* validation that both DRS modules are in sync
* reconstruction of beam position using Wire Chambers

This step is done with
```
> root -l CheckSync.C
```
The output is an image, similar to ```230529_01.png```

## Script 
An example script that performs all steps above
```
./run.sh
```
