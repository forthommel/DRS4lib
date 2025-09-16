#!/bin/sh

for ((i=1197; i<=1197; i++))
do
  run=$i
  echo $run
  pathIN="../data/raw/"
  fileOut="../data/merged/RUN${run}.root"
  ./listGoodEvents -r $run -p $pathIN
  ./maketree -drs 0 -r $run -p $pathIN
  ./maketree -drs 1 -r $run -p $pathIN
  ./merge
  cp output.root $fileOut
  root -l CheckSync.C+
done
