#!/bin/bash

ROOT_DIR=/proj/zyuxuanssf-PG0/nightcore-test

docker build -t zyuxuan/nightcore:for-test \
       -f Dockerfile.nightcore \
       $ROOT_DIR

