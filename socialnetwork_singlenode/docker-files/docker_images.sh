#!/bin/bash

ROOT_DIR=`realpath $(dirname $0)/..`
echo $ROOT_DIR

function build_nightcore {
    rm -rf nightcore
    git clone --recurse-submodules https://github.com/ut-osa/nightcore.git nightcore 
    sudo docker build -t zyuxuan0115/nightcore:main \
        -f $ROOT_DIR/docker-files/Dockerfile.nightcore \
        $ROOT_DIR/docker-files/nightcore
}

function push_nightcore {
    sudo docker push zyuxuan0115/nightcore:main
}

function build_deathstarbench {
    sudo docker build -t zyuxuan0115/cpp-microservice-deps:main \
        --build-arg NUM_CPUS=$(nproc) \
        $ROOT_DIR/DeathStarBench/socialNetwork/docker/cpp-microservice-deps

    sudo docker build --no-cache -t zyuxuan0115/nightcore-socialnetwork:main \
        -f $ROOT_DIR/docker-files/Dockerfile.socialnetwork \
        --build-arg NUM_CPUS=$(nproc) \
        $ROOT_DIR/DeathStarBench
}

function push_deathstarbench {
    sudo docker push zyuxuan0115/cpp-microservice-deps:main
    sudo docker push zyuxuan0115/nightcore-socialnetwork:main
}

HIPSTERSHOP_SERVICES="frontend-api \
productcatalogservice \
shippingservice \
checkoutservice \
currencyservice \
paymentservice \
adservice \
recommendationservice \
cartservice"

function build {
    build_nightcore
    build_deathstarbench
}

function push {
    push_nightcore
    push_deathstarbench
}

case "$1" in
build)
    build
    ;;
push)
    push
    ;;
esac
