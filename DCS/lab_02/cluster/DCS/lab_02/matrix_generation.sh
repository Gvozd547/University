#!/bin/bash

COUNT="1"

let SIZE=$1
let FINAL=$2
let STEP=$3

while [ $SIZE -le $FINAL ] ; do

    echo $SIZE

    ./Mul_Matrix.out -s -n $SIZE -f set_$COUNT.txt

    let COUNT=$COUNT+1
    let SIZE=$SIZE+$STEP

done
