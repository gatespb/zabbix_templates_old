#!/bin/bash

PIDFILE="/var/run/iostat-collect.pid"

[ -s "${PIDFILE}" ] && exit 0

echo $$ >> "${PIDFILE}"

SECONDS=50
FILE=/tmp/iostat.cache
IOSTAT=/usr/bin/iostat

DISKIO=$(${IOSTAT} -k -x 1 ${SECONDS} | awk 'BEGIN{check=0}; {if($1==""){check=0}; if(check==1){print $0} if($1=="Device:"){check=1}}')
SKIP=$(echo -e "${DISKIO}" | awk '!x[$1]++' | wc -l)
echo -e "${DISKIO}" | tail -n +${SKIP} | tr -s ' ' > ${FILE}

rm -f "${PIDFILE}"

exit 0