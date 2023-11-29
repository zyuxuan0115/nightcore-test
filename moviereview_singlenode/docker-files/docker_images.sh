#!/bin/bash

ROOT_DIR=`realpath $(dirname $0)/..`
echo $ROOT_DIR

function build_nightcore {
    docker build -t zyuxuan0115/nightcore:main \
        -f $ROOT_DIR/Dockerfile.nightcore \
        $ROOT_DIR/nightcore
}

function push_nightcore {
    docker push zyuxuan0115/nightcore:main
}

function build_deathstarbench {
    docker build -t zyuxuan0115/cpp-microservice-deps:main \
        --build-arg NUM_CPUS=$(nproc) \
        $ROOT_DIR/workloads/DeathStarBench/socialNetwork/docker/cpp-microservice-deps

    docker build -t zjia/nightcore-moviereview:main \
        -f $ROOT_DIR/dockerfiles/Dockerfile.moviereview \
        --build-arg NUM_CPUS=$(nproc) \
        $ROOT_DIR/workloads/DeathStarBench
}

function push_deathstarbench {
    docker push zyuxuan0115/cpp-microservice-deps:main
    docker push zyuxuan0115/nightcore-moviereview:main
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
