#!/bin/bash
# ./clients.sh port times

for ((i=0; i < $2; ++i))
do
    ./cliente 127.0.0.1 $1
done