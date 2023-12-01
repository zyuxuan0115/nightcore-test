#!/bin/bash
BASE_DIR=`realpath $(dirname $0)`
ROOT_DIR=`realpath $BASE_DIR/../..`
REMOTE_SERVER_HOME_DIR=/users/zyuxuan
EXP_DIR=$BASE_DIR/results/$1
QPS=$2

SRC_DIR=$ROOT_DIR/DeathStarBench/socialNetwork
HELPER_SCRIPT=$ROOT_DIR/helper
WRK_BIN=/usr/local/bin/wrk
WRK_SCRIPT=compose-post.lua

MANAGER_HOST=`python3 $HELPER_SCRIPT get-docker-manager-host --base-dir=$BASE_DIR`
CLIENT_HOST=`python3 $HELPER_SCRIPT get-client-host --base-dir=$BASE_DIR`
#ENTRY_HOST=`$HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nginx-thrift`
ENTRY_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-gateway`
MONGO_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=socialnetwork-mongodb`
ENGINE_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-engine`
GATEWAY_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-gateway`
ALL_HOSTS=`python3 $HELPER_SCRIPT get-all-server-hosts --base-dir=$BASE_DIR`


for host in $ALL_HOSTS; do
    ssh -q $host -- sudo docker swarm leave -f
    ssh -q $host -- sudo systemctl restart docker.service
done

python3 $HELPER_SCRIPT start-machines

$HELPER_SCRIPT generate-docker-compose --base-dir=$BASE_DIR
scp -q $BASE_DIR/docker-compose-write.yml $MANAGER_HOST:~
scp -q $BASE_DIR/docker-compose-placement.yml $MANAGER_HOST:~

ssh -q $MANAGER_HOST -- docker stack rm socialnetwork
sleep 20
ssh -q $ENTRY_HOST -- sudo rm -rf /tmp/socialNetwork
ssh -q $ENTRY_HOST -- mkdir -p /tmp/socialNetwork
ssh -q $MONGO_HOST -- sudo rm -rf /mnt/inmem/db
ssh -q $MONGO_HOST -- sudo mkdir -p /mnt/inmem/db
ssh -q $ENGINE_HOST -- sudo rm -rf /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo mkdir -p /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo mkdir -p /mnt/inmem/nightcore/output /mnt/inmem/nightcore/ipc

for host in $ALL_HOSTS; do
    scp -q $BASE_DIR/nightcore_config.json  $host:/tmp
    scp -q $BASE_DIR/service-config.json    $host:/tmp
done

scp -q $BASE_DIR/run_launcher $ENGINE_HOST:/tmp/run_launcher
ssh -q $ENGINE_HOST -- sudo cp /tmp/run_launcher /mnt/inmem/nightcore/run_launcher
ssh -q $ENGINE_HOST -- sudo cp /tmp/nightcore_config.json /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo mv /mnt/inmem/nightcore/nightcore_config.json /mnt/inmem/nightcore/func_config.json

rsync -arq $SRC_DIR/nginx-web-server    $ENTRY_HOST:/tmp/socialNetwork
rsync -arq $SRC_DIR/media-frontend      $ENTRY_HOST:/tmp/socialNetwork
rsync -arq $SRC_DIR/gen-lua             $ENTRY_HOST:/tmp/socialNetwork
rsync -arq $SRC_DIR/docker              $ENTRY_HOST:/tmp/socialNetwork

ssh -q $MANAGER_HOST -- docker stack deploy \
    -c $REMOTE_SERVER_HOME_DIR/docker-compose-write.yml -c $REMOTE_SERVER_HOME_DIR/docker-compose-placement.yml socialnetwork
sleep 60

ENGINE_CONTAINER_ID=`python3 $HELPER_SCRIPT get-container-id --service nightcore-engine`
echo 4096 | ssh -q $ENGINE_HOST -- sudo tee /sys/fs/cgroup/cpu,cpuacct/docker/$ENGINE_CONTAINER_ID/cpu.shares

scp -q $SRC_DIR/scripts/init_social_graph.py                         $CLIENT_HOST:~
scp -q $SRC_DIR/wrk2_scripts/$WRK_SCRIPT                             $CLIENT_HOST:~
scp -q $SRC_DIR/datasets/social-graph/socfb-Reed98/socfb-Reed98.mtx  $CLIENT_HOST:~

ssh -q $CLIENT_HOST -- python3 $REMOTE_SERVER_HOME_DIR/init_social_graph.py $ENTRY_HOST $REMOTE_SERVER_HOME_DIR/socfb-Reed98.mtx

sleep 10

rm -rf $EXP_DIR
mkdir -p $EXP_DIR

ssh -q $CLIENT_HOST -- $WRK_BIN -t 4 -c 48 -d 30 -L -U \
    -s $REMOTE_SERVER_HOME_DIR/$WRK_SCRIPT \
    http://$ENTRY_HOST:8080 -R $QPS 2>/dev/null >$EXP_DIR/wrk_warmup.log

sleep 5

ssh -q $CLIENT_HOST -- $WRK_BIN -t 4 -c 48 -d 150 -L -U \
    -s ~/$WRK_SCRIPT \
    http://$ENTRY_HOST:8080 -R $QPS 2>/dev/null >$EXP_DIR/wrk.log

$HELPER_SCRIPT collect-container-logs --base-dir=$BASE_DIR --log-path=$EXP_DIR/logs

mkdir $EXP_DIR/logs/func_worker
rsync -arq $ENGINE_HOST:/mnt/inmem/nightcore/output/* $EXP_DIR/logs/func_worker
