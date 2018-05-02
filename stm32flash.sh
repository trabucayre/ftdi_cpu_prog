#!/bin/sh
DEV=""

for i in $@; do
    echo $i | grep "tty" > /dev/null
    if [ $? = 0 ]; then
        DEV=$i
    fi
done

if [ "x"$DEV"x" = "xx" ]; then
    echo "big pb"
    exit
fi
echo $DEV $@

ftdi_cpu_prog -r 2 -b 4 -d $DEV -m 0
./stm32flash/stm32flash $@
ftdi_cpu_prog -r 2 -b 4 -d $DEV -m 1

