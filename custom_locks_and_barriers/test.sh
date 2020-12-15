#!/bin/sh

a=0

while [ $a -lt 3 ]
do
    OUTPUT=$(./counter -t 10 -i 50 --lock=mcs -o output.txt)
    #  OUTPUT=$(./a.out)
    # echo $OUTPUT
    if [ $OUTPUT = "500" ]
    then
        echo ""
    else
        echo "Did not work! $OUTPUT"
    fi
    a=`expr $a + 1`
done