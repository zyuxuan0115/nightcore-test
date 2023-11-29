#!/bin/bash

ROOT_DIR=`realpath $(dirname $0)/..`

function build_nightcore {
    docker build -t zjia/nightcore:asplos-ae \
        -f $ROOT_DIR/dockerfiles/Dockerfile.nightcore \
        $ROOT_DIR/nightcore
}

function push_nightcore {
    docker push zjia/nightcore:asplos-ae
}

function build_deathstarbench {
    docker build -t zjia/cpp-microservice-deps:asplos-ae \
        --build-arg NUM_CPUS=$(nproc) \
        $ROOT_DIR/workloads/DeathStarBench/socialNetwork/docker/cpp-microservice-deps

    docker build -t zjia/nightcore-moviereview:asplos-ae \
        -f $ROOT_DIR/dockerfiles/Dockerfile.moviereview \
        --build-arg NUM_CPUS=$(nproc) \
        $ROOT_DIR/workloads/DeathStarBench
}

function push_deathstarbench {
    docker push zjia/cpp-microservice-deps:asplos-ae
    docker push zjia/nightcore-moviereview:asplos-ae
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
