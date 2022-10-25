#!/bin/bash
# ./clients.sh times port

for ((i=0; i < $1; ++i))
do
    ./cliente 127.0.0.1 $2 &
done