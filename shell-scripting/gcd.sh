#!/bin/bash

function gcd {
    local A=$1
    local B=$2

    while [ $B -ne 0 ]; do
        local T=$B
        B=$(($A % $B))
        A=$T
    done

    return $A
}

gcd 360 60
echo $?

