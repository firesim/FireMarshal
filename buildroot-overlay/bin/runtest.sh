#!/bin/bash

for i in {1100..141100..10000}
do
    echo "running $i"
    mutilatemaster.sh $i
done
