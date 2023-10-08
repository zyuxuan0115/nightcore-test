#!/bin/bash

# before you run this script, you need to run the 
# following command
# docker login -u "myusername" -p "mypassword" docker.io

ROOT_DIR=$(pwd)

sudo docker build -t zyuxuan0115/nightcore:test \
       -f Dockerfile.nightcore \
       .

sudo docker push zyuxuan0115/nightcore:test

sudo docker build -t zyuxuan0115/nightcore:test-foo \
       -f Dockerfile.foo \
       .

sudo docker push zyuxuan0115/nightcore:test-foo

sudo docker build -t zyuxuan0115/nightcore:test-bar \
       -f Dockerfile.bar \
       .

sudo docker push zyuxuan0115/nightcore:test-bar


