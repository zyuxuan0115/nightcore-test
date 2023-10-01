#!/bin/bash

ROOT_DIR=`realpath $(dirname $0)/..`

docker build -t zyuxuan/nightcore:for-test \
       -f Dockerfile.nightcore \
       $ROOT_DIR/nightcore

docker push zyuxuan/nightcore:for-test


