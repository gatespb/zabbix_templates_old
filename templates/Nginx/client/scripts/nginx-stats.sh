#!/bin/bash

##### OPTIONS VERIFICATION #####
if [ -z "$1" -o -z "$2" -o -z "$3" ]; then
    exit 1
fi

##### PARAMETERS #####
RESERVED="$1"
METRIC="$2"
STATSURL="$3"
CURL="/usr/bin/curl"
CACHE="0"
CACHE_TTL="55"
CACHE_FILE="/tmp/zabbix.nginx.$(echo $STATSURL | md5sum | cut -d" " -f1).cache"
EXEC_TIMEOUT="1"
NOW_TIME=$(date '+%s')

##### RUN #####
if [ "x${CACHE}" = "x1" ]; then
    if [ -s "${CACHE_FILE}" ]; then
        CACHE_TIME=$(stat -c"%Y" "${CACHE_FILE}")
    else
        CACHE_TIME=0
    fi
    DELTA_TIME=$((${NOW_TIME} - ${CACHE_TIME}))
    #
    if [ ${DELTA_TIME} -lt ${EXEC_TIMEOUT} ]; then
        sleep $((${EXEC_TIMEOUT} - ${DELTA_TIME}))
    elif [ ${DELTA_TIME} -gt ${CACHE_TTL} ]; then
        echo "" >> "${CACHE_FILE}" # !!!
        DATACACHE=`${CURL} -sS --insecure --max-time ${EXEC_TIMEOUT} "${STATSURL}" 2>&1`
        echo "${DATACACHE}" > "${CACHE_FILE}" # !!!
        chmod 640 "${CACHE_FILE}"
    fi
    #
	case ${METRIC} in
	    active) echo -e "${DATA}" | grep "Active connections" | cut -d':' -f2 | cut -d" " -f2
		      ;;
	   accepts) echo -e "${DATA}" | sed -n '3p' | cut -d" " -f2
	          ;;
	   handled) echo -e "${DATA}" | sed -n '3p' | cut -d" " -f3
	          ;;
	  requests) echo -e "${DATA}" | sed -n '3p' | cut -d" " -f4
	          ;;
	   reading) echo -e "${DATA}" | grep "Reading" | cut -d':' -f2 | cut -d' ' -f2
	          ;;
	   writing) echo -e "${DATA}" | grep "Writing" | cut -d':' -f3 | cut -d' ' -f2
	          ;;
	   waiting) echo -e "${DATA}" | grep "Waiting" | cut -d':' -f4 | cut -d' ' -f2
	          ;;
			 *) echo 0
			  ;;
	esac
else
    DATA=$(${CURL} -sS --insecure --max-time ${EXEC_TIMEOUT} "${STATSURL}" 2>&1)
	case ${METRIC} in
	    active) echo -e "${DATA}" | grep "Active connections" | cut -d':' -f2 | cut -d" " -f2
		      ;;
	   accepts) echo -e "${DATA}" | sed -n '3p' | cut -d" " -f2
	          ;;
	   handled) echo -e "${DATA}" | sed -n '3p' | cut -d" " -f3
	          ;;
	  requests) echo -e "${DATA}" | sed -n '3p' | cut -d" " -f4
	          ;;
	   reading) echo -e "${DATA}" | grep "Reading" | cut -d':' -f2 | cut -d' ' -f2
	          ;;
	   writing) echo -e "${DATA}" | grep "Writing" | cut -d':' -f3 | cut -d' ' -f2
	          ;;
	   waiting) echo -e "${DATA}" | grep "Waiting" | cut -d':' -f4 | cut -d' ' -f2
	          ;;
			 *) echo 0
			  ;;
	esac
fi
exit 0