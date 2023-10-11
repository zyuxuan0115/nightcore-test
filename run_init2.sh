#!/bin/bash
ROOT_DIR=/users/zyuxuan
BASE_DIR=$(pwd)
WRK_BIN=/usr/local/bin/wrk
WRK_SCRIPT=mixed_api.lua

HELPER_SCRIPT=$BASE_DIR/helper2

python3 $HELPER_SCRIPT start-machines

MANAGER_HOST=`python3 $HELPER_SCRIPT get-docker-manager-host --base-dir=$BASE_DIR`
CLIENT_HOST=`python3 $HELPER_SCRIPT get-client-host --base-dir=$BASE_DIR`
ENTRY_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-gateway`
ALL_HOSTS=`python3 $HELPER_SCRIPT get-all-server-hosts --base-dir=$BASE_DIR`

echo $MANAGER_HOST
echo $ENTRY_HOST
echo $ALL_HOSTS

ssh -q $MANAGER_HOST -- echo '127.0.0.1 nightcore-manager' | sudo tee -a /etc/hosts

ALL_ENGINE_NODES="nightcore-worker1 nightcore-worker2"

for name in $ALL_ENGINE_NODES; do
   HOST=`python3 $HELPER_SCRIPT get-host --base-dir=$BASE_DIR --machine-name=$name`
   ssh -q $HOST -- echo '127.0.0.1 $name' | sudo tee -a /etc/hosts
done


