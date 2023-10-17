#!/bin/bash

# before you run this script, you need to run the 
# following command
# docker login -u "myusername" -p "mypassword" docker.io

ROOT_DIR=$(pwd)

sudo docker build --no-cache -t zyuxuan0115/nightcore:test-diff-engine \
       -f Dockerfile.nightcore \
       .

sudo docker push zyuxuan0115/nightcore:test-diff-engine

