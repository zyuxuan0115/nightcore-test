#!/bin/bash
ROOT_DIR=/users/zyuxuan
BASE_DIR=$(pwd)
OUTPUT_DIR=$(pwd)/outputs

HELPER_SCRIPT=$BASE_DIR/helper

python3 $HELPER_SCRIPT start-machines

MANAGER_HOST=`python3 $HELPER_SCRIPT get-docker-manager-host --base-dir=$BASE_DIR`
CLIENT_HOST=`python3 $HELPER_SCRIPT get-client-host --base-dir=$BASE_DIR`
ENTRY_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-gateway`
ALL_HOSTS=`python3 $HELPER_SCRIPT get-all-server-hosts --base-dir=$BASE_DIR`

echo $MANAGER_HOST
echo $ENTRY_HOST
echo $ALL_HOSTS

ALL_ENGINE_NODES="nightcore-worker2 nightcore-manager"

python3 $HELPER_SCRIPT generate-docker-compose --base-dir=$BASE_DIR
scp -q $BASE_DIR/docker-compose.yml           $MANAGER_HOST:$ROOT_DIR
scp -q $BASE_DIR/docker-compose-placement.yml $MANAGER_HOST:$ROOT_DIR
scp -q $BASE_DIR/common.env                   $MANAGER_HOST:$ROOT_DIR

ssh -q $MANAGER_HOST -- sudo docker stack rm test2func
sleep 10

for host in $ALL_HOSTS; do
    scp -q $BASE_DIR/func_config.json  $host:/tmp/nightcore_config.json
done

for name in $ALL_ENGINE_NODES; do
    HOST=`python3 $HELPER_SCRIPT get-host --base-dir=$BASE_DIR --machine-name=$name`
    ssh -q $HOST -- sudo rm -rf /mnt/inmem/nightcore
    ssh -q $HOST -- sudo mkdir -p /mnt/inmem/nightcore
    ssh -q $HOST -- sudo mkdir -p /mnt/inmem/nightcore/output /mnt/inmem/nightcore/ipc
    ssh -q $HOST -- sudo cp /tmp/nightcore_config.json /mnt/inmem/nightcore/func_config.json
done

ssh -q $MANAGER_HOST -- sudo docker stack deploy \
    -c $ROOT_DIR/docker-compose.yml -c $ROOT_DIR/docker-compose-placement.yml test2func
sleep 30

for name in $ALL_ENGINE_NODES; do
    HOST=`python3 $HELPER_SCRIPT get-host --base-dir=$BASE_DIR --machine-name=$name`
    ENGINE_CONTAINER_ID=`python3 $HELPER_SCRIPT get-container-id --service test2func_nightcore-engine --machine-name=$name`
    ssh -q $HOST -- "echo 4096 | sudo tee /sys/fs/cgroup/cpuacct/docker/$ENGINE_CONTAINER_ID/cpu.shares"
    ssh -q $HOST -- "echo 4096 | sudo tee /sys/fs/cgroup/cpu/docker/$ENGINE_CONTAINER_ID/cpu.shares"
done

rm -rf $OUTPUT_DIR
mkdir -p $OUTPUT_DIR

#ssh -q $CLIENT_HOST -- curl -X POST -d "Hello" http://$ENTRY_HOST:8080/function/Foo > $OUTPUT_DIR/log


: <<'END'

ssh -q $CLIENT_HOST -- $WRK_BIN -t 4 -c 96 -d 30 -L -U \
    -s ~/$WRK_SCRIPT \
    http://$ENTRY_HOST:8080 -R $QPS 2>/dev/null >$EXP_DIR/wrk_warmup.log

sleep 5

$HELPER_SCRIPT collect-container-logs --base-dir=$BASE_DIR --log-path=$EXP_DIR/logs

for name in $ALL_ENGINE_NODES; do
    HOST=`$HELPER_SCRIPT get-host --base-dir=$BASE_DIR --machine-name=$name`
    mkdir $EXP_DIR/logs/func_worker_$name
    rsync -arq $HOST:/mnt/inmem/nightcore/output/* $EXP_DIR/logs/func_worker_$name
done
END
