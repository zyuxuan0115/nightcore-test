#!/bin/bash
BASE_DIR=`realpath $(dirname $0)`
ROOT_DIR=`realpath $BASE_DIR/../..`

EXP_DIR=$BASE_DIR/results/$1
QPS=$2

SRC_DIR=$BASE_DIR/DeathStarBench/mediaMicroservices
HELPER_SCRIPT=$BASE_DIR/helper
WRK_BIN=/usr/local/bin/wrk
WRK_SCRIPT=compose-review.lua

MANAGER_HOST=`python3 $HELPER_SCRIPT get-docker-manager-host --base-dir=$BASE_DIR`
CLIENT_HOST=`python3 $HELPER_SCRIPT get-client-host --base-dir=$BASE_DIR`
ENTRY_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nginx-thrift`
MONGO_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=media-mongodb`
ENGINE_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-engine`
GATEWAY_HOST=`python3 $HELPER_SCRIPT get-service-host --base-dir=$BASE_DIR --service=nightcore-gateway`
ALL_HOSTS=`python3 $HELPER_SCRIPT get-all-server-hosts --base-dir=$BASE_DIR`

echo "MANAGER_HOST=$MANAGER_HOST"
echo "CLIENT_HOST=$CLIENT_HOST"
echo "ENTRY_HOST=$ENTRY_HOST"
echo "MONGO_HOST=$MONGO_HOST"
echo "ENGINE_HOST=$ENGINE_HOST"
echo "GATEWAY_HOST=$GATEWAY_HOST"
echo "ALL_HOSTS=$ALL_HOSTS"

python3 $HELPER_SCRIPT generate-docker-compose --base-dir=$BASE_DIR
scp -q $BASE_DIR/docker-compose.yml $MANAGER_HOST:~
scp -q $BASE_DIR/docker-compose-placement.yml $MANAGER_HOST:~

ssh -q $MANAGER_HOST -- sudo docker stack rm media-microservices
sleep 20
ssh -q $ENTRY_HOST -- sudo rm -rf /tmp/mediaMicroservices
ssh -q $ENTRY_HOST -- mkdir -p /tmp/mediaMicroservices
ssh -q $MONGO_HOST -- sudo rm -rf /mnt/inmem/db
ssh -q $MONGO_HOST -- sudo mkdir -p /mnt/inmem/db
ssh -q $ENGINE_HOST -- sudo rm -rf /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo mkdir -p /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo mkdir -p /mnt/inmem/nightcore/output /mnt/inmem/nightcore/ipc

for host in $ALL_HOSTS; do
    echo $host
    scp -q $BASE_DIR/nightcore_config.json  $host:/tmp
    scp -q $BASE_DIR/service-config.json    $host:/tmp
done

scp -q $BASE_DIR/run_launcher $ENGINE_HOST:/tmp
ssh -q $ENGINE_HOST -- sudo cp /tmp/run_launcher /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo cp /tmp/nightcore_config.json /mnt/inmem/nightcore
ssh -q $ENGINE_HOST -- sudo mv /mnt/inmem/nightcore/nightcore_config.json /mnt/inmem/nightcore/func_config.json

rsync -arq $SRC_DIR/nginx-web-server    $ENTRY_HOST:/tmp/mediaMicroservices
rsync -arq $SRC_DIR/gen-lua             $ENTRY_HOST:/tmp/mediaMicroservices
rsync -arq $SRC_DIR/docker              $ENTRY_HOST:/tmp/mediaMicroservices


ssh -q $MANAGER_HOST -- docker stack deploy \
    -c ~/docker-compose.yml -c ~/docker-compose-placement.yml media-microservices
sleep 60

: <<'END'

ENGINE_CONTAINER_ID=`$HELPER_SCRIPT get-container-id --service nightcore-engine`
echo 4096 | ssh -q $ENGINE_HOST -- sudo tee /sys/fs/cgroup/cpu,cpuacct/docker/$ENGINE_CONTAINER_ID/cpu.shares

scp -q $SRC_DIR/scripts/register_users.sh    $CLIENT_HOST:~
scp -q $SRC_DIR/scripts/write_movie_info.py  $CLIENT_HOST:~
scp -q $SRC_DIR/wrk2_scripts/$WRK_SCRIPT     $CLIENT_HOST:~
scp -qr $SRC_DIR/datasets/tmdb               $CLIENT_HOST:~

ssh -q $CLIENT_HOST -- ~/register_users.sh $ENTRY_HOST
ssh -q $CLIENT_HOST -- python3 ~/write_movie_info.py -x $ENTRY_HOST \
    -c ~/tmdb/casts.json -m ~/tmdb/movies.json

sleep 10

rm -rf $EXP_DIR
mkdir -p $EXP_DIR

ssh -q $CLIENT_HOST -- $WRK_BIN -t 4 -c 48 -d 30 -L -U \
    -s ~/$WRK_SCRIPT \
    http://$ENTRY_HOST:8080 -R $QPS 2>/dev/null >$EXP_DIR/wrk_warmup.log

sleep 5

ssh -q $CLIENT_HOST -- $WRK_BIN -t 4 -c 48 -d 150 -L -U \
    -s ~/$WRK_SCRIPT \
    http://$ENTRY_HOST:8080 -R $QPS 2>/dev/null >$EXP_DIR/wrk.log

$HELPER_SCRIPT collect-container-logs --base-dir=$BASE_DIR --log-path=$EXP_DIR/logs

mkdir $EXP_DIR/logs/func_worker
rsync -arq $ENGINE_HOST:/mnt/inmem/nightcore/output/* $EXP_DIR/logs/func_worker
END
