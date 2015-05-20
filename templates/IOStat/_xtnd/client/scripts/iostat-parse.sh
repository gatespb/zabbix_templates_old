#!/bin/bash

unset NUMBER
UPDT=300
FILE="/tmp/iostat.cache"
DISK=$1
METRIC=$2

[ -z "$1" -o -z "$2" ] && { echo "Unknown parameters"; exit 1; }
[ ! -s "${FILE}" ] && { echo "Check file ${FILE}"; exit 1; }
[ $[$(date +%s) - $(stat --printf=%Y "${FILE}")] -gt ${UPDT} ] && { echo "Check file ${FILE}. Not updated > ${UPDT}"; exit 1; }

GETMAX=0

case "$2" in
    "MrkB/s")    GETMAX=1; NUMBER=6;;
    "MwkB/s")    GETMAX=1; NUMBER=7;;
    "rrqm/s")    NUMBER=2;;
    "wrqm/s")    NUMBER=3;;
    "r/s")       NUMBER=4;;
    "w/s")       NUMBER=5;;
    "rkB/s")     NUMBER=6;;
    "wkB/s")     NUMBER=7;;
    "avgrq-sz")  NUMBER=8;;
    "avgqu-sz")  NUMBER=9;;
    "await")     NUMBER=10;;
    "r_await")   NUMBER=11;;
    "w_await")   NUMBER=12;;
    "svctm")     NUMBER=13;;
    "util")      NUMBER=14;;
esac

if [ ${GETMAX} -eq 0 ]; then
    cat "${FILE}" | grep "${DISK} " | sed 's/,/./' | awk -v NUM=${NUMBER} 'BEGIN {sum=0.0;count=0;}; {sum=sum+$ NUM;count=count+1;} END {printf("%.2f\n", sum/count);}'
else
    cat "${FILE}" | grep "${DISK} " | cut -f${NUMBER} -d' ' | sed 's/,/./' | sort -n | tail -n1
fi