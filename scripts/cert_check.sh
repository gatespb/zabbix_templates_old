#!/bin/bash

SERVER=$1
unset EXPIRE
unset EXPIRE_AT
unset EXPIRE_SEC
unset NOW_SEC

EXPIRE_AT="$(echo | openssl s_client -connect ${SERVER}:443 2>/dev/null | openssl x509 -noout -dates | grep ^notAfter | cut -f2 -d'=')"
EXPIRE_SEC="$[ $(date -d "${EXPIRE_AT}" +%s) ]"
NOW_SEC="$(date +%s)"

[ $[ ${EXPIRE_SEC}-${NOW_SEC} ] -lt 0 ] && EXPIRE=0 || EXPIRE=$[(${EXPIRE_SEC}-${NOW_SEC})/86400]

echo ${EXPIRE}
